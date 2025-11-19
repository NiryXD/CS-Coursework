// favoritesStorage.js
import AsyncStorage from '@react-native-async-storage/async-storage';

const FAVORITES_KEY = 'FAVORITE_RECIPES';

export async function getFavorites() {
  try {
    const jsonValue = await AsyncStorage.getItem(FAVORITES_KEY);
    return jsonValue != null ? JSON.parse(jsonValue) : [];
  } catch (e) {
    console.error('Error reading favorites', e);
    return [];
  }
}

export async function saveFavorites(favorites) {
  try {
    const jsonValue = JSON.stringify(favorites);
    await AsyncStorage.setItem(FAVORITES_KEY, jsonValue);
  } catch (e) {
    console.error('Error saving favorites', e);
  }
}

export async function toggleFavorite(recipe) {
  const current = await getFavorites();
  const exists = current.find(r => r.id === recipe.id);
  let updated;
  if (exists) {
    updated = current.filter(r => r.id !== recipe.id);
  } else {
    updated = [...current, recipe];
  }
  await saveFavorites(updated);
  return updated;
}

export async function isFavorite(recipeId) {
  const current = await getFavorites();
  return current.some(r => r.id === recipeId);
}

