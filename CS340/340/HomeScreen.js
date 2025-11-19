// HomeScreen.js
import React, { useState, useCallback, useContext, useEffect, useRef } from "react";
import {
  SafeAreaView,
  View,
  Text,
  FlatList,
  TouchableOpacity,
  StyleSheet,
  TextInput,
  ActivityIndicator,
  TouchableWithoutFeedback,
  Keyboard,
  Image,
} from "react-native";
import AsyncStorage from "@react-native-async-storage/async-storage";
import { useNavigation } from "@react-navigation/native";
import { Ionicons } from "@expo/vector-icons";
import { ThemeContext } from "./App";
import { searchRecipes, getRecipeDetails } from "./spoonacular";
import { SkeletonRecipeCard } from "./SkeletonLoader";
import FadeInView from "./FadeInView";
import RecipeImage from "./RecipeImage";

export default function HomeScreen({ route }) {
  const navigation = useNavigation();
  const { userId } = route.params;

  // State
  const [query, setQuery] = useState("");
  const [searchResults, setSearchResults] = useState([]);
  const [searching, setSearching] = useState(false);
  const [recentRecipes, setRecentRecipes] = useState([]);
  const [showRecent, setShowRecent] = useState(false);
  
  // Filter section state
  const [showFilter, setShowFilter] = useState(false);
  const [minCalories, setMinCalories] = useState("");
  const [maxCalories, setMaxCalories] = useState("");
  const [minProtein, setMinProtein] = useState("");
  const [maxProtein, setMaxProtein] = useState("");
  const [minFat, setMinFat] = useState("");
  const [maxFat, setMaxFat] = useState("");
  const [minCarbs, setMinCarbs] = useState("");
  const [maxCarbs, setMaxCarbs] = useState("");

  const { theme } = useContext(ThemeContext);
  const searchInputRef = useRef(null);

  // Determine if UI elements should be hidden
  const isSearchActive = showRecent || showFilter || query.trim().length > 0;

  // Load recent recipes from AsyncStorage
  useEffect(() => {
    const loadRecent = async () => {
      const stored = await AsyncStorage.getItem("recentRecipes");
      if (stored) setRecentRecipes(JSON.parse(stored));
    };
    loadRecent();
  }, []);

  // Add recipe to recent
  const addToRecent = async (recipe) => {
    let updated = [recipe, ...recentRecipes.filter((r) => r.id !== recipe.id)];
    updated = updated.slice(0, 3); // max 3
    setRecentRecipes(updated);
    await AsyncStorage.setItem("recentRecipes", JSON.stringify(updated));
  };

  // Handle search
  const handleSearch = async () => {
    if (!query.trim()) {
      setSearchResults([]);
      setShowRecent(false);
      return;
    }
    setSearching(true);
    setShowRecent(false);
    Keyboard.dismiss();

    const filters = {
      minCalories: minCalories ? Number(minCalories) : undefined,
      maxCalories: maxCalories ? Number(maxCalories) : undefined,
      minProtein: minProtein ? Number(minProtein) : undefined,
      maxProtein: maxProtein ? Number(maxProtein) : undefined,
      minFat: minFat ? Number(minFat) : undefined,
      maxFat: maxFat ? Number(maxFat) : undefined,
      minCarbs: minCarbs ? Number(minCarbs) : undefined,
      maxCarbs: maxCarbs ? Number(maxCarbs) : undefined,
    };

    try {
      const results = await searchRecipes(query.trim(), filters);
      setSearchResults(results);
    } catch (err) {
      console.error("Error fetching recipes:", err);
    } finally {
      setSearching(false);
    }
  };

  const handleQueryChange = (text) => {
    setQuery(text);
    if (text.trim().length === 0) {
      setSearchResults([]);
    }
  };

  const clearFilters = () => {
    setMinCalories("");
    setMaxCalories("");
    setMinProtein("");
    setMaxProtein("");
    setMinFat("");
    setMaxFat("");
    setMinCarbs("");
    setMaxCarbs("");
  };

  const exitSearch = () => {
    setQuery("");
    setSearchResults([]);
    setShowRecent(false);
    setShowFilter(false);
    Keyboard.dismiss();
  };
  
  // Render items with animations and image fallbacks
  const renderSearchItem = ({ item, index }) => (
    <FadeInView delay={index * 50}>
      <TouchableOpacity
        style={[styles.card, theme === "dark" && styles.darkCard]}
        onPress={async () => {
          try {
            const details = await getRecipeDetails(item.id);
            navigation.navigate("RecipeDetail", { recipe: details, userId });
            addToRecent({ id: details.id, title: details.title });
          } catch (err) {
            console.error("Error fetching instructions:", err);
          }
        }}
      >
        <View style={styles.searchResultContainer}>
          <RecipeImage 
            uri={item.image} 
            style={styles.searchResultImage}
          />
          <Text
            style={[
              styles.cardTitle,
              theme === "dark" && styles.darkText,
              styles.searchResultTitle,
            ]}
            numberOfLines={2}
          >
            {item.title}
          </Text>
        </View>
      </TouchableOpacity>
    </FadeInView>
  );

  const renderRecentItem = ({ item, index }) => (
    <FadeInView delay={index * 30}>
      <TouchableOpacity
        style={[styles.recentItem, theme === "dark" && styles.darkCard]}
        onPress={async () => {
          setQuery(item.title);
          setShowRecent(false);
          try {
            const details = await getRecipeDetails(item.id);
            navigation.navigate("RecipeDetail", { recipe: details, userId });
          } catch (err) {
            console.error("Error fetching recipe details:", err);
          }
        }}
      >
        <Text style={[styles.cardTitle, theme === "dark" && styles.darkText]}>
          {item.title}
        </Text>
      </TouchableOpacity>
    </FadeInView>
  );

  return (
    <TouchableWithoutFeedback
      onPress={() => {
        setShowRecent(false);
        Keyboard.dismiss();
      }}
    >
      <SafeAreaView style={[styles.safe, theme === "dark" && styles.darkSafe]}>
        {/* Exit Search Button - Top Left */}
        {isSearchActive && (
          <FadeInView duration={300}>
            <TouchableOpacity style={styles.exitButton} onPress={exitSearch}>
              <Ionicons
                name="close-circle"
                size={32}
                color="#ef4444"
              />
            </TouchableOpacity>
          </FadeInView>
        )}

        {/* Settings Icon - Top Right */}
        <TouchableOpacity
          style={styles.settingsButton}
          onPress={() => navigation.navigate("Settings")}
        >
          <Ionicons
            name="settings-outline"
            size={28}
            color={theme === "dark" ? "#f9fafb" : "#111827"}
          />
        </TouchableOpacity>

        {/* Body */}
        <View style={styles.container}>
          {/* VolBites Title - Tap for Random Recipe */}
          {!isSearchActive && (
            <FadeInView duration={600}>
              <TouchableOpacity
                onPress={async () => {
                  setSearching(true);
                  try {
                    const randomRecipes = await searchRecipes("", {});
                    if (randomRecipes.length > 0) {
                      const randomIndex = Math.floor(Math.random() * randomRecipes.length);
                      const randomRecipe = randomRecipes[randomIndex];
                      const details = await getRecipeDetails(randomRecipe.id);
                      navigation.navigate("RecipeDetail", { recipe: details, userId });
                    }
                  } catch (err) {
                    console.error("Error fetching random recipe:", err);
                  } finally {
                    setSearching(false);
                  }
                }}
                activeOpacity={0.7}
              >
                <Text style={[styles.appTitle, theme === "dark" && styles.darkText]}>
                  VolBites
                </Text>
              </TouchableOpacity>
            </FadeInView>
          )}
          
          {isSearchActive && (
            <Text style={[styles.appTitle, theme === "dark" && styles.darkText]}>
              VolBites
            </Text>
          )}
          
          {/* Search Bar */}
          <View
            style={[
              styles.searchBar,
              theme === "dark" && {
                borderColor: "#4b5563",
                backgroundColor: "#374151",
              },
            ]}
          >
            <TextInput
              ref={searchInputRef}
              style={[
                styles.searchInput,
                theme === "dark" && { color: "#ffffff" },
              ]}
              placeholder="Search"
              placeholderTextColor={theme === "dark" ? "#d1d5db" : "#9ca3af"}
              value={query}
              onChangeText={handleQueryChange}
              onSubmitEditing={handleSearch}
              onFocus={() => setShowRecent(true)}
            />
            <TouchableOpacity
              style={styles.filterButton}
              onPress={() => setShowFilter(!showFilter)}
            >
              <Ionicons
                name="options-outline"
                size={24}
                color="#ffffff"
              />
            </TouchableOpacity>
            <TouchableOpacity
              style={styles.searchButton}
              onPress={handleSearch}
              disabled={searching}
            >
              <Ionicons name="search" size={24} color="#ffffff" />
            </TouchableOpacity>
          </View>

          {/* Collapsible Filter Section */}
          {showFilter && (
            <FadeInView duration={300}>
              <View
                style={[
                  styles.filterContainer,
                  theme === "dark" && {
                    backgroundColor: "#1f2937",
                    borderColor: "#4b5563",
                  },
                ]}
              >
                <View style={styles.filterRow}>
                  <Text
                    style={[
                      styles.filterLabel,
                      theme === "dark" && { color: "#f9fafb" },
                    ]}
                  >
                    Calories:
                  </Text>
                  <TextInput
                    style={[
                      styles.filterInput,
                      theme === "dark" && styles.filterInputDark,
                    ]}
                    placeholder="Min"
                    value={minCalories}
                    onChangeText={setMinCalories}
                    keyboardType="numeric"
                    placeholderTextColor={theme === "dark" ? "#e5e7eb" : "#9ca3af"}
                  />
                  <TextInput
                    style={[
                      styles.filterInput,
                      theme === "dark" && styles.filterInputDark,
                    ]}
                    placeholder="Max"
                    value={maxCalories}
                    onChangeText={setMaxCalories}
                    keyboardType="numeric"
                    placeholderTextColor={theme === "dark" ? "#e5e7eb" : "#9ca3af"}
                  />
                </View>
                <View style={styles.filterRow}>
                  <Text
                    style={[
                      styles.filterLabel,
                      theme === "dark" && { color: "#f9fafb" },
                    ]}
                  >
                    Protein:
                  </Text>
                  <TextInput
                    style={[
                      styles.filterInput,
                      theme === "dark" && styles.filterInputDark,
                    ]}
                    placeholder="Min"
                    value={minProtein}
                    onChangeText={setMinProtein}
                    keyboardType="numeric"
                    placeholderTextColor={theme === "dark" ? "#e5e7eb" : "#9ca3af"}
                  />
                  <TextInput
                    style={[
                      styles.filterInput,
                      theme === "dark" && styles.filterInputDark,
                    ]}
                    placeholder="Max"
                    value={maxProtein}
                    onChangeText={setMaxProtein}
                    keyboardType="numeric"
                    placeholderTextColor={theme === "dark" ? "#e5e7eb" : "#9ca3af"}
                  />
                </View>
                <View style={styles.filterRow}>
                  <Text
                    style={[
                      styles.filterLabel,
                      theme === "dark" && { color: "#f9fafb" },
                    ]}
                  >
                    Fat:
                  </Text>
                  <TextInput
                    style={[
                      styles.filterInput,
                      theme === "dark" && styles.filterInputDark,
                    ]}
                    placeholder="Min"
                    value={minFat}
                    onChangeText={setMinFat}
                    keyboardType="numeric"
                    placeholderTextColor={theme === "dark" ? "#e5e7eb" : "#9ca3af"}
                  />
                  <TextInput
                    style={[
                      styles.filterInput,
                      theme === "dark" && styles.filterInputDark,
                    ]}
                    placeholder="Max"
                    value={maxFat}
                    onChangeText={setMaxFat}
                    keyboardType="numeric"
                    placeholderTextColor={theme === "dark" ? "#e5e7eb" : "#9ca3af"}
                  />
                </View>
                <View style={styles.filterRow}>
                  <Text
                    style={[
                      styles.filterLabel,
                      theme === "dark" && { color: "#f9fafb" },
                    ]}
                  >
                    Carbs:
                  </Text>
                  <TextInput
                    style={[
                      styles.filterInput,
                      theme === "dark" && styles.filterInputDark,
                    ]}
                    placeholder="Min"
                    value={minCarbs}
                    onChangeText={setMinCarbs}
                    keyboardType="numeric"
                    placeholderTextColor={theme === "dark" ? "#e5e7eb" : "#9ca3af"}
                  />
                  <TextInput
                    style={[
                      styles.filterInput,
                      theme === "dark" && styles.filterInputDark,
                    ]}
                    placeholder="Max"
                    value={maxCarbs}
                    onChangeText={setMaxCarbs}
                    keyboardType="numeric"
                    placeholderTextColor={theme === "dark" ? "#e5e7eb" : "#9ca3af"}
                  />
                </View>
                <View style={styles.filterButtonsRow}>
                  <TouchableOpacity
                    onPress={() => setShowFilter(false)}
                    style={styles.closeFiltersButton}
                  >
                    <Ionicons name="close" size={20} color="#fff" />
                  </TouchableOpacity>
                  <TouchableOpacity
                    onPress={clearFilters}
                    style={styles.clearFiltersButton}
                  >
                    <Text style={styles.clearFiltersText}>Clear</Text>
                  </TouchableOpacity>
                </View>
              </View>
            </FadeInView>
          )}

          {/* Recent recipes dropdown */}
          {showRecent && recentRecipes.length > 0 && (
            <FadeInView duration={300}>
              <View
                style={[
                  styles.recentDropdown,
                  theme === "dark" && styles.darkCard,
                ]}
              >
                <FlatList
                  data={recentRecipes}
                  keyExtractor={(item) => String(item.id)}
                  renderItem={renderRecentItem}
                />
              </View>
            </FadeInView>
          )}

          {/* Search results or buttons */}
          {searching ? (
            <>
              <SkeletonRecipeCard theme={theme} />
              <SkeletonRecipeCard theme={theme} />
              <SkeletonRecipeCard theme={theme} />
              <SkeletonRecipeCard theme={theme} />
            </>
          ) : searchResults.length > 0 ? (
            <FlatList
              data={searchResults}
              keyExtractor={(item) => String(item.id)}
              renderItem={renderSearchItem}
              showsVerticalScrollIndicator={false}
              contentContainerStyle={styles.listContent}
            />
          ) : (
            !isSearchActive && (
              <FadeInView duration={800} delay={200}>
                <View style={styles.buttonsContainer}>
                  <TouchableOpacity
                    style={[
                      styles.bigButton,
                      theme === "dark" && styles.darkCard,
                    ]}
                    onPress={() => navigation.navigate("Favorites", { userId })}
                  >
                    <Ionicons
                      name="heart"
                      size={64}
                      color={ORANGE}
                      style={styles.buttonIcon}
                    />
                    <Text
                      style={[
                        styles.buttonText,
                        theme === "dark" && styles.darkText,
                      ]}
                    >
                      Favorites
                    </Text>
                  </TouchableOpacity>

                  <TouchableOpacity
                    style={[
                      styles.bigButton,
                      theme === "dark" && styles.darkCard,
                    ]}
                    onPress={() => navigation.navigate("YourRecipes", { userId })}
                  >
                    <Ionicons
                      name="book"
                      size={64}
                      color={ORANGE}
                      style={styles.buttonIcon}
                    />
                    <Text
                      style={[
                        styles.buttonText,
                        theme === "dark" && styles.darkText,
                      ]}
                    >
                      My Recipes
                    </Text>
                  </TouchableOpacity>
                </View>
              </FadeInView>
            )
          )}
        </View>

        {/* Floating Chat Button - Hidden when search is active */}
        {!isSearchActive && (
          <FadeInView duration={800} delay={400}>
            <TouchableOpacity
              style={styles.floatingChatBtn}
              onPress={() => navigation.navigate("ChatBot")}
              activeOpacity={0.8}
            >
              <Ionicons name="chatbubble-ellipses" size={36} color="#ffffff" />
            </TouchableOpacity>
          </FadeInView>
        )}

        {/* Floating Map Button - Hidden when search is active */}
        {!isSearchActive && (
          <FadeInView duration={800} delay={500}>
            <TouchableOpacity
              style={styles.floatingMapBtn}
              onPress={() => navigation.navigate("Restaurants")}
              activeOpacity={0.8}
            >
              <Ionicons name="map" size={36} color="#ffffff" />
            </TouchableOpacity>
          </FadeInView>
        )}
      </SafeAreaView>
    </TouchableWithoutFeedback>
  );
}

const ORANGE = "#f97316";
const WHITE = "#ffffff";
const DARK_BG = "#1f2937";
const DARK_TEXT = "#f9fafb";

const styles = StyleSheet.create({
  safe: { flex: 1, backgroundColor: WHITE },
  darkSafe: { backgroundColor: DARK_BG },
  exitButton: {
    position: "absolute",
    top: 50,
    left: 20,
    zIndex: 10,
    padding: 8,
  },
  settingsButton: {
    position: "absolute",
    top: 50,
    right: 20,
    zIndex: 10,
    padding: 8,
  },
  container: { flex: 1, padding: 20, justifyContent: "center" },
  appTitle: {
    fontSize: 48,
    fontWeight: "700",
    color: ORANGE,
    textAlign: "center",
    marginBottom: 30,
  },
  searchBar: {
    flexDirection: "row",
    marginBottom: 20,
    borderWidth: 1,
    borderColor: "#e5e7eb",
    borderRadius: 12,
    overflow: "hidden",
    backgroundColor: WHITE,
    alignItems: "center",
  },
  searchInput: {
    flex: 1,
    paddingHorizontal: 16,
    paddingVertical: 14,
    fontSize: 16,
  },
  filterButton: {
    paddingHorizontal: 14,
    paddingVertical: 12,
    backgroundColor: ORANGE,
    justifyContent: "center",
    alignItems: "center",
  },
  searchButton: {
    paddingHorizontal: 14,
    paddingVertical: 12,
    backgroundColor: ORANGE,
    justifyContent: "center",
    alignItems: "center",
  },
  filterContainer: {
    padding: 12,
    borderRadius: 10,
    borderWidth: 1,
    borderColor: "#e5e7eb",
    marginBottom: 16,
    backgroundColor: WHITE,
  },
  filterRow: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 8,
  },
  filterLabel: {
    width: 70,
    fontSize: 14,
    color: "#111827",
  },
  filterInput: {
    flex: 1,
    borderWidth: 1,
    borderColor: "#e5e7eb",
    borderRadius: 6,
    paddingHorizontal: 8,
    paddingVertical: 6,
    marginLeft: 4,
    fontSize: 14,
  },
  filterInputDark: {
    backgroundColor: "#111827",
    borderColor: "#9ca3af",
    color: "#f9fafb",
  },
  filterButtonsRow: {
    flexDirection: "row",
    justifyContent: "flex-end",
    gap: 8,
    marginTop: 8,
  },
  closeFiltersButton: {
    paddingHorizontal: 12,
    paddingVertical: 6,
    backgroundColor: "#ef4444",
    borderRadius: 6,
    justifyContent: "center",
    alignItems: "center",
  },
  clearFiltersButton: {
    paddingHorizontal: 12,
    paddingVertical: 6,
    backgroundColor: "#ef4444",
    borderRadius: 6,
  },
  clearFiltersText: { color: "#fff", fontWeight: "700" },
  recentDropdown: {
    backgroundColor: WHITE,
    borderColor: "#e5e7eb",
    borderWidth: 1,
    borderRadius: 10,
    maxHeight: 200,
    marginBottom: 16,
  },
  recentItem: {
    padding: 12,
    borderBottomWidth: 1,
    borderBottomColor: "#eee",
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
  searchResultContainer: {
    flexDirection: "row",
    alignItems: "center",
    gap: 12,
  },
  searchResultImage: {
    width: 80,
    height: 80,
    borderRadius: 8,
  },
  searchResultTitle: {
    flex: 1,
  },
  darkText: { color: DARK_TEXT },
  buttonsContainer: {
    flexDirection: "row",
    gap: 16,
    marginTop: 20,
  },
  bigButton: {
    flex: 1,
    backgroundColor: WHITE,
    borderRadius: 16,
    paddingVertical: 40,
    paddingHorizontal: 20,
    alignItems: "center",
    justifyContent: "center",
    borderWidth: 2,
    borderColor: "#e5e7eb",
    shadowColor: "#000",
    shadowOpacity: 0.1,
    shadowRadius: 10,
    shadowOffset: { width: 0, height: 4 },
    elevation: 4,
  },
  buttonIcon: {
    marginBottom: 12,
  },
  buttonText: {
    fontSize: 18,
    fontWeight: "700",
    color: "#111827",
    textAlign: "center",
  },
  floatingChatBtn: {
    position: "absolute",
    bottom: 30,
    right: 30,
    width: 80,
    height: 80,
    backgroundColor: ORANGE,
    borderRadius: 40,
    justifyContent: "center",
    alignItems: "center",
    shadowColor: "#000",
    shadowOffset: { width: 0, height: 4 },
    shadowOpacity: 0.3,
    shadowRadius: 4.65,
    elevation: 8,
  },
  floatingMapBtn: {
    position: "absolute",
    bottom: 30,
    left: 30,
    width: 80,
    height: 80,
    backgroundColor: ORANGE,
    borderRadius: 40,
    justifyContent: "center",
    alignItems: "center",
    shadowColor: "#000",
    shadowOffset: { width: 0, height: 4 },
    shadowOpacity: 0.3,
    shadowRadius: 4.65,
    elevation: 8,
  },
});