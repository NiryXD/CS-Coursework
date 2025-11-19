import React, { useState } from 'react';
import { View, Text, Pressable, StyleSheet, StatusBar } from 'react-native';

function randomHex() {
  return (
    '#' +
    Math.floor(Math.random() * 0xffffff)
      .toString(16)
      .padStart(6, '0')
      .toUpperCase()
  );
}

export default function App() {
  const [bg, setBg] = useState('#FFFFFF');

  return (
    <View style={[styles.container, { backgroundColor: bg }]}>
      <StatusBar barStyle="dark-content" />
      <Text style={styles.title}>Tap to change the screen color</Text>
      <Text style={styles.hex}>{bg}</Text>

      <Pressable style={styles.button} onPress={() => setBg(randomHex())}>
        <Text style={styles.buttonText}>Change color</Text>
      </Pressable>

      <Pressable style={[styles.button, styles.reset]} onPress={() => setBg('#FFFFFF')}>
        <Text style={styles.buttonText}>Reset</Text>
      </Pressable>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, alignItems: 'center', justifyContent: 'center' },
  title: { fontSize: 20, fontWeight: '700', marginBottom: 8, color: '#000' },
  hex: { fontSize: 16, marginBottom: 16, color: '#000' },
  button: { paddingVertical: 12, paddingHorizontal: 16, borderRadius: 8, backgroundColor: '#222', marginTop: 8 },
  reset: { backgroundColor: '#555' },
  buttonText: { color: '#fff', fontWeight: '600' }
});

