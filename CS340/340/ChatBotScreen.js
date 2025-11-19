// ChatBotScreen.js
import React, { useState, useRef, useEffect, useContext } from "react";
import {
  View,
  Text,
  TextInput,
  TouchableOpacity,
  StyleSheet,
  ScrollView,
  KeyboardAvoidingView,
  Platform,
  ActivityIndicator,
  Alert,
} from "react-native";

const OPENAI_API_KEY = process.env.OPENAI_API_KEY || "YOUR_API_KEY_HERE";

import { ThemeContext } from "./App";
import AsyncStorage from '@react-native-async-storage/async-storage';
import { searchRecipes, getRecipeDetails } from "./spoonacular";
import { getRestaurants } from "./geoapify";
import { SUPABASE_URL, SUPABASE_ANON_KEY } from "./supabaseConfig";

export default function ChatBotScreen({ navigation }) {
  const { theme } = useContext(ThemeContext);
  const [preferences, setPreferences] = useState(null);
  const [userId, setUserId] = useState(null);
  const [messages, setMessages] = useState([]);
  const [inputText, setInputText] = useState("");
  const [loading, setLoading] = useState(false);
  const scrollViewRef = useRef(null);

  useEffect(() => {
    scrollViewRef.current?.scrollToEnd({ animated: true });
  }, [messages]);

  useEffect(() => {
    // load cached preferences and user id
    (async () => {
      try {
        const prefs = await AsyncStorage.getItem('userPreferences');
        if (prefs) setPreferences(JSON.parse(prefs));
        const uid = await AsyncStorage.getItem('@user_id');
        if (uid) setUserId(uid);
      } catch (e) {
        console.warn('Failed to load preferences', e);
      }
    })();
  }, []);

  // compose initial greeting after preferences load
  useEffect(() => {
    const introBase = "Hello! I'm VolBot, your friendly VolBites assistant! 🥕✨\n\n" +
      "I recommend recipes based on what your preferences and ingredients you have available. You can specify calories or time as well. ";

    let prefsText = '';
    if (preferences) {
      const diet = preferences.diet && preferences.diet !== 'None' ? `Diet: ${preferences.diet}` : null;
      const allergens = preferences.allergens && preferences.allergens.length ? `Allergens: ${preferences.allergens.join(', ')}` : null;
      const parts = [diet, allergens].filter(Boolean);
  if (parts.length) prefsText = `I see your preferences — ${parts.join(' • ')}. I'll keep that in mind.`;
    }

    const intro = prefsText ? introBase + prefsText : introBase + "What can I help you cook today?";
    // set initial message only if empty
    if (messages.length === 0) {
      setMessages([{ id: Date.now(), text: intro, isBot: true }]);
    }
  }, [preferences]);

  const sendMessage = async () => {
    if (!inputText.trim()) return;
    const userText = inputText.trim();
    const userMessage = { id: Date.now(), text: userText, isBot: false };
    setMessages((prev) => [...prev, userMessage]);
    setInputText("");
    setLoading(true);

    // Detect recipe or restaurant intents
    const recipePattern = /recipe|make|cook|ingredients|dish|calories|servings|minutes|min|ready in/i;
    const restaurantPattern = /restaurant|near me|nearby|eat|restaurants/i;

    // helper: parse numeric filters (calories, time) from user text and return a cleaned query
    const parseRecipeFilters = (text) => {
      const filters = {};
      let cleaned = String(text);

      // calories
      const aboveCal = cleaned.match(/(?:above|over|greater than|more than|>=)\s*(\d{2,5})\s*calorie/i);
      const belowCal = cleaned.match(/(?:under|below|less than|<=|no more than)\s*(\d{2,5})\s*calorie/i);
      const explicitCal = cleaned.match(/(\d{2,5})\s*calorie/i);
      if (aboveCal) {
        filters.minCalories = Number(aboveCal[1]);
        cleaned = cleaned.replace(aboveCal[0], '');
      } else if (belowCal) {
        filters.maxCalories = Number(belowCal[1]);
        cleaned = cleaned.replace(belowCal[0], '');
      } else if (explicitCal) {
        filters.maxCalories = Number(explicitCal[1]);
        cleaned = cleaned.replace(explicitCal[0], '');
      }

      // time (in minutes)
      const underTime = cleaned.match(/(?:under|below|less than|<=|or under|or less than|no more than|within)\s*(\d{1,3})\s*(min|mins|minute|minutes|hr|hour|hours)/i);
      const exactTime = cleaned.match(/(?:takes|in|ready in)\s*(\d{1,3})\s*(min|mins|minute|minutes|hr|hour|hours)/i);
      const overTime = cleaned.match(/(?:over|more than|greater than|>)\s*(\d{1,3})\s*(min|mins|minute|minutes|hr|hour|hours)/i);
      if (underTime) {
        let n = Number(underTime[1]);
        const unit = (underTime[2] || '').toLowerCase();
        if (unit.includes('hour') || unit.includes('hr')) n = n * 60;
        filters.maxReadyTime = n;
        cleaned = cleaned.replace(underTime[0], '');
      } else if (exactTime) {
        let n = Number(exactTime[1]);
        const unit = (exactTime[2] || '').toLowerCase();
        if (unit.includes('hour') || unit.includes('hr')) n = n * 60;
        filters.maxReadyTime = n;
        cleaned = cleaned.replace(exactTime[0], '');
      } else if (overTime) {
        let n = Number(overTime[1]);
        const unit = (overTime[2] || '').toLowerCase();
        if (unit.includes('hour') || unit.includes('hr')) n = n * 60;
        filters.minReadyTime = n;
        cleaned = cleaned.replace(overTime[0], '');
      }

      // remove filler words and common verbs to produce a clean search query
      cleaned = cleaned.replace(/\b(give me|show me|find|please|recipe|recipes|that|for|me|a|an|the|give|find|list of)\b/ig, '');
      // remove leftover numeric tokens
      cleaned = cleaned.replace(/\b\d+\b/g, '');
      cleaned = cleaned.replace(/[^\w\s-]/g, '');
      cleaned = cleaned.replace(/\s+/g, ' ').trim();

      return { filters, cleanedQuery: cleaned };
    };

    try {
      if (restaurantPattern.test(userText)) {
        // Use geoapify to fetch restaurants (RestaurantsScreen also exists)
        const results = await getRestaurants();
        const summary = results && results.length > 0
          ? `I found ${results.length} restaurants nearby. Showing top results.`
          : "I couldn't find nearby restaurants.";
        setMessages((prev) => [...prev, { id: Date.now() + 1, text: summary, isBot: true, type: 'restaurants', payload: results }]);
      } else if (recipePattern.test(userText)) {
        // Use Spoonacular to search for recipes
  const { filters, cleanedQuery } = parseRecipeFilters(userText);
  const queryForSearch = cleanedQuery && cleanedQuery.length > 2 ? cleanedQuery : 'recipe';
  const results = await searchRecipes(queryForSearch, filters);
        if (results && results.length > 0) {
          const top = results[0];
          const details = await getRecipeDetails(top.id);
          const summary = `${top.title}\n\n🕒 ${details.readyInMinutes} min | 🍽️ ${details.servings} servings\n\nMain ingredients: ${details.extendedIngredients?.slice(0,5).map(i=>i.name).join(", ")}`;
          setMessages((prev) => [...prev, { id: Date.now() + 1, text: summary, isBot: true, type: 'recipe', payload: { summary, recipe: { ...top, ...details } } }]);
        } else {
          setMessages((prev) => [...prev, { id: Date.now() + 1, text: "I couldn't find a recipe for that. Try a different query.", isBot: true }]);
        }
      } else {
        // Default to OpenAI conversation with persona and preferences
  const systemPrompt = `You are Angel — a hip, friendly VolBites nutritionist 🥕✨. Keep replies upbeat, helpful and concise (under 150 words). When users ask for recipes, prefer returning structured recipe info (title, cook time, servings, main ingredients)` + (preferences ? ` User preferences: ${JSON.stringify(preferences)}.` : '');

        const response = await fetch("https://api.openai.com/v1/chat/completions", {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${OPENAI_API_KEY}`,
          },
          body: JSON.stringify({
            model: "gpt-3.5-turbo",
            messages: [
              { role: "system", content: systemPrompt },
              { role: "user", content: userText },
            ],
            max_tokens: 200,
            temperature: 0.7,
          }),
        });

        const data = await response.json();
        if (!response.ok) throw new Error(data.error?.message || 'API request failed');
        const botText = data.choices?.[0]?.message?.content || "Hmm, I didn’t get that.";
        setMessages((prev) => [...prev, { id: Date.now() + 1, text: botText, isBot: true }]);
      }
    } catch (err) {
      console.error(err);
      setMessages((prev) => [...prev, { id: Date.now() + 1, text: "Something went wrong 😅", isBot: true }]);
    } finally {
      setLoading(false);
    }
  };

  const viewFullRecipe = async (recipe) => {
    try {
      const details = await getRecipeDetails(recipe.id);
      navigation.navigate('RecipeDetail', { recipe: details, userId });
    } catch (e) {
      Alert.alert('Error', 'Failed to load recipe details');
    }
  };

  const saveToFavorites = async (recipe) => {
    if (!userId) return Alert.alert('Not signed in', 'You need to be signed in to save favorites');
    try {
      const res = await fetch(`${SUPABASE_URL}/rest/v1/saved_recipes`, {
        method: 'POST',
        headers: {
          apiKey: SUPABASE_ANON_KEY,
          Authorization: `Bearer ${SUPABASE_ANON_KEY}`,
          'Content-Type': 'application/json',
          Prefer: 'return=representation',
        },
        body: JSON.stringify({ user_id: userId, recipe_id: recipe.id, recipe_name: recipe.title || recipe.name, image: recipe.image }),
      });
      if (!res.ok) throw new Error('Failed to save');
      Alert.alert('Saved', 'Recipe saved to your favorites');
    } catch (e) {
      console.error('Save favorite error', e);
      Alert.alert('Error', 'Could not save recipe');
    }
  };
  

  return (
    <View style={[styles.container, theme === 'dark' && styles.containerDark]}>
      {/* Header */}
      <View style={styles.header}>
        <TouchableOpacity onPress={() => navigation.goBack()} style={styles.backBtn}>
          <Text style={styles.backText}>←</Text>
        </TouchableOpacity>
        <Text style={styles.headerTitle}>Chat Assistant</Text>
        <View style={styles.placeholder} />
      </View>

      {/* Chat Messages */}
      <KeyboardAvoidingView
        style={{ flex: 1 }}
        behavior={Platform.OS === "ios" ? "padding" : "height"}
      >
        <ScrollView
          ref={scrollViewRef}
          style={[styles.messagesContainer, theme === 'dark' && styles.messagesContainerDark]}
          contentContainerStyle={styles.messagesContent}
          onContentSizeChange={() => scrollViewRef.current?.scrollToEnd({ animated: true })}
        >
          {messages.map((message) => (
            <View
              key={message.id}
              style={[
                styles.messageBubble,
                message.isBot ? styles.botBubble : styles.userBubble,
                message.isBot && theme === 'dark' ? styles.botBubbleDark : null,
              ]}
            >
              <Text
                style={[
                  styles.messageText,
                  message.isBot ? (theme === 'dark' ? styles.botTextDark : styles.botText) : styles.userText,
                ]}
              >
                {message.text}
              </Text>

              {/* Inline actions for structured messages */}
              {message.type === 'recipe' && message.payload && (
                <View style={styles.actionRow}>
                  <TouchableOpacity style={styles.actionBtn} onPress={() => viewFullRecipe(message.payload.recipe)}>
                    <Text style={styles.actionBtnText}>View Full Recipe</Text>
                  </TouchableOpacity>
                  <TouchableOpacity style={styles.actionBtn} onPress={() => saveToFavorites(message.payload.recipe)}>
                    <Text style={styles.actionBtnText}>Save to Favorites</Text>
                  </TouchableOpacity>
                </View>
              )}

              {message.type === 'restaurants' && message.payload && (
                <View style={styles.actionRow}>
                  <TouchableOpacity style={styles.actionBtn} onPress={() => navigation.navigate('Restaurants')}>
                    <Text style={styles.actionBtnText}>Open Restaurants</Text>
                  </TouchableOpacity>
                </View>
              )}
            </View>
          ))}
          {loading && (
            <View style={[styles.messageBubble, styles.botBubble, theme === 'dark' && styles.botBubbleDark]}>
              <ActivityIndicator color="#f97316" />
              <Text style={[styles.loadingText, theme === 'dark' && { color: '#d1d5db' }]}>Thinking...</Text>
            </View>
          )}
        </ScrollView>

        {/* Input Area */}
        <View style={[styles.inputContainer, theme === 'dark' && styles.inputContainerDark]}>
          <TextInput
            style={[styles.input, theme === 'dark' && styles.inputDark]}
            value={inputText}
            onChangeText={setInputText}
            placeholder="Ask me anything..."
            placeholderTextColor={theme === 'dark' ? '#d1d5db' : '#9ca3af'}
            multiline
            maxLength={500}
            returnKeyType="send"
            onSubmitEditing={sendMessage}
          />
          <TouchableOpacity
            style={[styles.sendBtn, !inputText.trim() && styles.sendBtnDisabled]}
            onPress={sendMessage}
            disabled={!inputText.trim() || loading}
          >
            <Text style={styles.sendText}>→</Text>
          </TouchableOpacity>
        </View>
      </KeyboardAvoidingView>
    </View>
  );
}

const ORANGE = "#f97316";
const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: "#fff" },
  containerDark: { backgroundColor: '#1f2937' },
  header: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
    backgroundColor: ORANGE,
    paddingHorizontal: 16,
    paddingVertical: 14,
    paddingTop: Platform.OS === "ios" ? 50 : 14,
    borderBottomLeftRadius: 16,
    borderBottomRightRadius: 16,
    shadowColor: "#000",
    shadowOpacity: 0.12,
    shadowRadius: 6,
    shadowOffset: { width: 0, height: 3 },
    elevation: 4,
  },
  backBtn: { width: 40, height: 40, justifyContent: "center", alignItems: "center" },
  backText: { color: "#fff", fontSize: 28, fontWeight: "700" },
  headerTitle: { color: "#fff", fontSize: 18, fontWeight: "700" },
  placeholder: { width: 40 },
  messagesContainer: { flex: 1, backgroundColor: "#f9fafb" },
  messagesContainerDark: { backgroundColor: '#1f2937' },
  messagesContent: { padding: 16, paddingBottom: 8 },
  messageBubble: { maxWidth: "80%", padding: 12, borderRadius: 16, marginBottom: 12 },
  botBubble: {
    alignSelf: "flex-start",
    backgroundColor: "#fff",
    borderWidth: 1,
    borderColor: "#e5e7eb",
  },
  botBubbleDark: {
    backgroundColor: '#1f2937',
    borderColor: '#4b5563',
  },
  userBubble: { alignSelf: "flex-end", backgroundColor: ORANGE },
  messageText: { fontSize: 15, lineHeight: 20 },
  botText: { color: "#374151" },
  botTextDark: { color: '#d1d5db' },
  userText: { color: "#fff" },
  loadingText: { color: "#6b7280", fontSize: 14, marginLeft: 8, fontStyle: "italic" },
  inputContainer: {
    flexDirection: "row",
    padding: 12,
    backgroundColor: "#fff",
    borderTopWidth: 1,
    borderTopColor: "#e5e7eb",
    alignItems: "flex-end",
  },
  inputContainerDark: {
    backgroundColor: '#1f2937',
    borderTopColor: '#4b5563',
  },
  input: {
    flex: 1,
    backgroundColor: "#f3f4f6",
    borderRadius: 20,
    paddingHorizontal: 16,
    paddingVertical: 10,
    maxHeight: 100,
    fontSize: 15,
    color: "#111827",
  },
  inputDark: {
    backgroundColor: '#4b5563',
    color: '#ffffff',
  },
  sendBtn: {
    width: 44,
    height: 44,
    backgroundColor: ORANGE,
    borderRadius: 22,
    justifyContent: "center",
    alignItems: "center",
    marginLeft: 8,
  },
  sendBtnDisabled: { opacity: 0.4 },
  sendText: { color: "#fff", fontSize: 24, fontWeight: "700" },
  actionRow: { flexDirection: 'row', marginTop: 10, justifyContent: 'flex-start', flexWrap: 'wrap' },
  actionBtn: { backgroundColor: '#111827', paddingVertical: 8, paddingHorizontal: 12, borderRadius: 8, marginRight: 8, marginTop: 6 },
  actionBtnText: { color: '#fff', fontWeight: '700', fontSize: 12 },
});
