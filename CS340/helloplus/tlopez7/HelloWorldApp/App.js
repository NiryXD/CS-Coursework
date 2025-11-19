import React, { useState } from 'react';
import { SafeAreaView, Text, Button, StyleSheet } from 'react-native';

export default function App() {
  const [count, setCount] = useState(0);

  return (
    <SafeAreaView style={styles.container}>
      <Text style={styles.text}>Hello World!</Text>
      <Text style={styles.text}>You pressed the button {count} times</Text>
      <Button title="Press me" onPress={() => setCount(count + 1)} />
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, justifyContent: 'center', alignItems: 'center' },
  text: { fontSize: 24, marginBottom: 20 },
});

