// App.js
import React, { useState, createContext } from "react";
import { NavigationContainer } from "@react-navigation/native";
import { createNativeStackNavigator } from "@react-navigation/native-stack";
import LoginScreen from "./LoginScreen";
import SignUpScreen from "./SignUpScreen";
import HomeScreen from "./HomeScreen";
import SettingsScreen from "./SettingsScreen";
import RecipeDetailScreen from "./RecipeDetailScreen";
import ChatBotScreen from "./ChatBotScreen";
import UploadRecipeScreen from "./UploadRecipeScreen";
import FavoritesScreen from "./FavoritesScreen";
import YourRecipesScreen from "./YourRecipesScreen";
import RestaurantsScreen from "./RestaurantsScreen";

// Create theme context
export const ThemeContext = createContext();

const Stack = createNativeStackNavigator();

export default function App() {
  const [theme, setTheme] = useState("light"); // "light" or "dark"

  return (
    <ThemeContext.Provider value={{ theme, setTheme }}>
      <NavigationContainer>
        <Stack.Navigator initialRouteName="Login">
          <Stack.Screen
            name="Login"
            component={LoginScreen}
            options={{ headerShown: false }}
          />
          <Stack.Screen
            name="SignUp"
            component={SignUpScreen}
            options={{ headerShown: false }}
          />
          <Stack.Screen
            name="Home"
            component={HomeScreen}
            options={{ headerShown: false }}
          />
          <Stack.Screen
            name="Settings"
            component={SettingsScreen}
            options={{ title: "Settings" }}
          />
          <Stack.Screen
            name="RecipeDetail"
            component={RecipeDetailScreen}
            options={{ title: "Recipe Details" }}
          />
          <Stack.Screen
            name="ChatBot"
            component={ChatBotScreen}
            options={{ headerShown: false }}
          />
          <Stack.Screen
            name="UploadRecipe"
            component={UploadRecipeScreen}
            options={{ title: "Upload Recipe" }}
          />
          <Stack.Screen
            name="Favorites"
            component={FavoritesScreen}
            options={{ title: "Favorites" }}
          />
          <Stack.Screen
            name="YourRecipes"
            component={YourRecipesScreen}
            options={{ title: "My Recipes" }}
          />
          <Stack.Screen
            name="Restaurants"
            component={RestaurantsScreen}
            options={{ headerShown: false }}
          />
        </Stack.Navigator>
      </NavigationContainer>
    </ThemeContext.Provider>
  );
}
