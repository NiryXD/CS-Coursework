// FavoritesScreen.js
import React, { useState, useCallback, useContext } from "react";
import {
  SafeAreaView,
  View,
  Text,
  FlatList,
  TouchableOpacity,
  RefreshControl,
  StyleSheet,
  Image,
} from "react-native";
import { SUPABASE_URL, SUPABASE_ANON_KEY } from "./supabaseConfig";
import { useNavigation, useFocusEffect } from "@react-navigation/native";
import { ThemeContext } from "./App";
import { getRecipeDetails } from "./spoonacular";

export default function FavoritesScreen({ route }) {
  const navigation = useNavigation();
  const { userId } = route.params;

  const [favorites, setFavorites] = useState([]);
  const [loading, setLoading] = useState(true);
  const [refreshing, setRefreshing] = useState(false);

  const { theme } = useContext(ThemeContext);

  // Fetch favorites from Supabase
  const fetchFavorites = useCallback(async () => {
    try {
      setLoading(true);
      const res = await fetch(
        `${SUPABASE_URL}/rest/v1/saved_recipes?user_id=eq.${userId}&select=*`,
        {
          headers: {
            apiKey: SUPABASE_ANON_KEY,
            Authorization: `Bearer ${SUPABASE_ANON_KEY}`,
          },
        }
      );
      const data = await res.json();
      setFavorites(Array.isArray(data) ? data : []);
    } catch (e) {
      console.warn("Failed to load favorites:", e?.message);
    } finally {
      setLoading(false);
    }
  }, [userId]);

  useFocusEffect(
    useCallback(() => {
      fetchFavorites();
    }, [fetchFavorites])
  );

  const onRefresh = async () => {
    setRefreshing(true);
    await fetchFavorites();
    setRefreshing(false);
  };

  const renderFavoriteItem = ({ item }) => (
    <TouchableOpacity
      style={[styles.card, theme === "dark" && styles.darkCard]}
      onPress={async () => {
        try {
          const details = await getRecipeDetails(item.recipe_id);
          navigation.navigate("RecipeDetail", { recipe: details, userId });
        } catch (err) {
          console.error("Error fetching recipe details:", err);
        }
      }}
    >
      {item.image && (
        <Image
          source={{ uri: item.image }}
          style={styles.favoriteImage}
          resizeMode="cover"
        />
      )}
      <View style={styles.favoriteInfo}>
        <Text
          style={[styles.cardTitle, theme === "dark" && styles.darkText]}
          numberOfLines={2}
        >
          {item.recipe_name}
        </Text>
        {item.created_at && (
          <Text style={[styles.meta, theme === "dark" && styles.darkText]}>
            Saved {new Date(item.created_at).toLocaleDateString()}
          </Text>
        )}
      </View>
    </TouchableOpacity>
  );

  return (
    <SafeAreaView style={[styles.safe, theme === "dark" && styles.darkSafe]}>
      <View style={styles.container}>
        <Text style={[styles.title, theme === "dark" && styles.darkText]}>
          Your Favorites
        </Text>
        <FlatList
          data={favorites}
          keyExtractor={(item) => String(item.recipe_id)}
          renderItem={renderFavoriteItem}
          refreshControl={
            <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
          }
          contentContainerStyle={[
            styles.listContent,
            favorites.length === 0 && styles.emptyStateContainer,
          ]}
          ListEmptyComponent={
            !loading && (
              <View style={styles.emptyState}>
                <Text
                  style={[styles.emptyTitle, theme === "dark" && styles.darkText]}
                >
                  No favorites yet
                </Text>
                <Text
                  style={[styles.emptyText, theme === "dark" && styles.darkText]}
                >
                  Save a recipe from search and it'll show up here.
                </Text>
              </View>
            )
          }
          showsVerticalScrollIndicator={false}
        />
      </View>
    </SafeAreaView>
  );
}

const ORANGE = "#f97316";
const WHITE = "#ffffff";
const DARK_BG = "#1f2937";
const DARK_TEXT = "#f9fafb";

const styles = StyleSheet.create({
  safe: { flex: 1, backgroundColor: WHITE },
  darkSafe: { backgroundColor: DARK_BG },
  container: { flex: 1, padding: 20 },
  title: {
    fontSize: 24,
    fontWeight: "700",
    marginBottom: 16,
    color: "#111827",
  },
  listContent: { paddingBottom: 24 },
  card: {
    backgroundColor: WHITE,
    borderRadius: 12,
    padding: 14,
    marginBottom: 12,
    borderWidth: 1,
    borderColor: "#e5e7eb",
    shadowColor: "#000",
    shadowOpacity: 0.06,
    shadowRadius: 8,
    shadowOffset: { width: 0, height: 4 },
    elevation: 2,
  },
  darkCard: { backgroundColor: "#374151", borderColor: "#4b5563" },
  cardTitle: { fontSize: 16, fontWeight: "600", color: "#111827" },
  meta: { marginTop: 6, color: "#6b7280", fontSize: 12 },
  emptyStateContainer: { flexGrow: 1, justifyContent: "center" },
  emptyState: { alignItems: "center", paddingVertical: 40 },
  emptyTitle: {
    fontSize: 18,
    fontWeight: "700",
    marginBottom: 8,
    color: "#111827",
  },
  emptyText: {
    color: "#6b7280",
    textAlign: "center",
    marginBottom: 16,
  },
  darkText: { color: DARK_TEXT },
  favoriteImage: {
    width: "100%",
    height: 160,
    borderRadius: 10,
    marginBottom: 8,
  },
  favoriteInfo: { flexDirection: "column" },
});