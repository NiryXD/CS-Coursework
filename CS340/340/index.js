// index.js
import { BackHandler } from "react-native";
import { registerRootComponent } from "expo";
import App from "./App";

// Polyfill for libs that might expect the old API.
// (Harmless even if nothing uses it.)
if (typeof BackHandler.removeEventListener !== "function") {
  BackHandler.removeEventListener = () => {};
}

registerRootComponent(App);

