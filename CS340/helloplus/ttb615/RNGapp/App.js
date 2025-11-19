import { useState } from "react";
import { StatusBar } from "expo-status-bar";
import { StyleSheet, Text, View, Pressable } from "react-native";

export default function App() {
  const [number, setNumber] = useState(null);

  const generateRandomNumber = () => {
    const randomNum = Math.floor(Math.random() * 100) + 1;
    setNumber(randomNum);
  };

  return (
    <View style={styles.container}>
      <Text style={styles.numberText}>
        {number !== null ? number : "Press the button!"}
      </Text>

      <Pressable style={styles.button} onPress={generateRandomNumber}>
        <Text style={styles.buttonText}>Generate</Text>
      </Pressable>

      <StatusBar style="auto" />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: "#fff",
    alignItems: "center",
    justifyContent: "center",
  },
  numberText: {
    fontSize: 40,
    marginBottom: 40,
    fontWeight: "bold",
  },
  button: {
    backgroundColor: "#007AFF", // iOS blue
    paddingVertical: 20,
    paddingHorizontal: 60,
    borderRadius: 15,
    elevation: 3,
  },
  buttonText: {
    color: "#fff",
    fontSize: 24,
    fontWeight: "600",
  },
});

