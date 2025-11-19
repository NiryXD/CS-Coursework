import { GEOAPIFY_API_KEY } from "@env";

// Approx center of Knoxville, TN
export const KNOXVILLE_COORDS = {
  lat: 35.9606,
  lon: -83.9207,
  name: "Knoxville, TN",
};

// Radius in meters (5000 = 5 km)
const RADIUS = 5000;

/**
 * Geocode a city name to coordinates
 * @param {string} cityName - Name of the city (e.g., "New York, NY" or "Knoxville, TN")
 * @returns {Promise<{lat: number, lon: number, name: string}>}
 */
export async function geocodeCity(cityName) {
  try {
    const url = `https://api.geoapify.com/v1/geocode/search?text=${encodeURIComponent(cityName)}&apiKey=${GEOAPIFY_API_KEY}`;
    
    const res = await fetch(url);
    if (!res.ok) {
      const text = await res.text();
      throw new Error(`HTTP ${res.status}: ${text}`);
    }

    const data = await res.json();
    if (data.features && data.features.length > 0) {
      const feature = data.features[0];
      return {
        lat: feature.geometry.coordinates[1],
        lon: feature.geometry.coordinates[0],
        name: feature.properties.formatted || cityName,
      };
    }
    throw new Error("No results found for the given city name");
  } catch (err) {
    console.error("Error geocoding city:", err);
    throw err;
  }
}

/**
 * Get restaurants near a given location
 * @param {Object} coords - Object with lat, lon, and optionally name
 * @param {number} coords.lat - Latitude
 * @param {number} coords.lon - Longitude
 * @param {string} [coords.name] - Location name (optional)
 * @returns {Promise<Array>}
 */
export async function getRestaurants(coords = KNOXVILLE_COORDS) {
  try {
    const url = `https://api.geoapify.com/v2/places?categories=catering.restaurant&filter=circle:${coords.lon},${coords.lat},${RADIUS}&limit=20&apiKey=${GEOAPIFY_API_KEY}`;

    const res = await fetch(url);
    if (!res.ok) {
      const text = await res.text();
      throw new Error(`HTTP ${res.status}: ${text}`);
    }

    const data = await res.json();
    const restaurants = data.features.map((f) => ({
      id: f.properties.place_id,
      name: f.properties.name,
      address: f.properties.formatted,
      hours: f.properties.opening_hours || "Hours not available",
      lat: f.properties.lat,
      lon: f.properties.lon,
    }));

    return restaurants;
  } catch (err) {
    console.error("Error fetching restaurants:", err);
    throw err;
  }
}
