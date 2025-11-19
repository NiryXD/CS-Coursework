// LoginScreen.js
import React, { useRef, useState } from "react";
import {
  Platform,
  KeyboardAvoidingView,
  ScrollView,
  View,
  Text,
  TextInput,
  TouchableOpacity,
  StyleSheet,
  Alert,
} from "react-native";
import { SUPABASE_URL, SUPABASE_ANON_KEY } from "./supabaseConfig";
import RememberMeBox from "./RememberMe";
import AsyncStorage from '@react-native-async-storage/async-storage';

export default function LoginScreen({ navigation }) {
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [loading, setLoading] = useState(false);
  const passwordRef = useRef(null);

  const onLogin = async () => {
    if (!email || !password) {
      Alert.alert("Missing info", "Enter email and password");
      return;
    }


    try {
      setLoading(true);
      const res = await fetch(
        `${SUPABASE_URL}/rest/v1/profiles?email=eq.${encodeURIComponent(email)}`,
        {
          headers: {
            apiKey: SUPABASE_ANON_KEY,
            Authorization: `Bearer ${SUPABASE_ANON_KEY}`,
            "Content-Type": "application/json",
          },
        }
      );

      if (!res.ok) throw new Error("Failed to fetch profile");
      const rows = await res.json();
      if (!rows || rows.length === 0) {
        Alert.alert("Login failed", "No profile found with that email");
        setCaptchaVerified(false);
        return;
      }

      const profile = rows[0];
      if (profile.password !== password) {
        Alert.alert("Login failed", "Incorrect password");
        setCaptchaVerified(false);
        return;
      }

      // Cache user id and email locally for quick access in other screens
      try {
        await AsyncStorage.setItem('@user_id', String(profile.id));
        await AsyncStorage.setItem('@user_email', profile.email || email);
      } catch (e) {
        console.warn('Failed to cache user info', e);
      }

      navigation.replace("Home", { userId: profile.id });
    } catch (e) {
      Alert.alert("Error", e.message);
      setCaptchaVerified(false);
    } finally {
      setLoading(false);
    }
  };

  return (
    <View style={styles.safe}>
      <KeyboardAvoidingView
        style={{ flex: 1 }}
        behavior={Platform.OS === "ios" ? "padding" : "height"}
        keyboardVerticalOffset={Platform.select({ ios: 64, android: 0 })}
      >
        <ScrollView contentContainerStyle={{ flexGrow: 1 }} keyboardShouldPersistTaps="handled">
          <View style={styles.header}>
            <Text style={styles.headerTitle}>VolBites</Text>
          </View>

          <View style={styles.container}>
            <Text style={styles.title}>Welcome back!</Text>

            <Text style={styles.label}>Email</Text>
            <TextInput
              style={styles.input}
              value={email}
              onChangeText={setEmail}
              autoCapitalize="none"
              autoCorrect={false}
              keyboardType="email-address"
              returnKeyType="next"
              blurOnSubmit={false}
              onSubmitEditing={() => passwordRef.current?.focus()}
            />

            <Text style={styles.label}>Password</Text>
            <TextInput
              ref={passwordRef}
              style={styles.input}
              value={password}
              onChangeText={setPassword}
              secureTextEntry
              returnKeyType="done"
            />
<RememberMeBox />


            <TouchableOpacity
              style={[styles.btn, loading && styles.btnDisabled]}
              onPress={onLogin}
              disabled={loading}
            >
              <Text style={styles.btnText}>{loading ? "Logging in..." : "Log in"}</Text>
            </TouchableOpacity>

            <View style={styles.row}>
              <Text style={styles.subtle}>No account? </Text>
              <TouchableOpacity onPress={() => navigation.replace("SignUp")}>
                <Text style={styles.link}>Sign up</Text>
              </TouchableOpacity>
            </View>
          </View>
        </ScrollView>
      </KeyboardAvoidingView>
    </View>
  );
}

const ORANGE = "#f97316", WHITE = "#fff";
const styles = StyleSheet.create({
  safe: { flex: 1, backgroundColor: WHITE },
  header: {
    backgroundColor: ORANGE,
    paddingHorizontal: 20,
    paddingVertical: 14,
    borderBottomLeftRadius: 16,
    borderBottomRightRadius: 16,
    shadowColor: "#000",
    shadowOpacity: 0.12,
    shadowRadius: 6,
    shadowOffset: { width: 0, height: 3 },
    elevation: 4,
  },
  headerTitle: { color: WHITE, fontSize: 20, fontWeight: "700", letterSpacing: 0.3 },
  container: { flex: 1, padding: 20, justifyContent: "center" },
  title: { fontSize: 22, fontWeight: "700", marginBottom: 16, color: "#111827" },
  label: { fontSize: 14, color: "#374151", marginBottom: 6 },
  input: {
    height: 48,
    borderWidth: 1,
    borderColor: "#e5e7eb",
    borderRadius: 10,
    paddingHorizontal: 12,
    marginBottom: 14,
    backgroundColor: WHITE,
  },
  btn: {
    backgroundColor: ORANGE,
    paddingVertical: 12,
    borderRadius: 10,
    alignItems: "center",
    marginTop: 6,
    marginBottom: 8,
  },
  btnDisabled: { opacity: 0.6 },
  btnText: { color: WHITE, fontWeight: "700" },
  row: { flexDirection: "row", justifyContent: "center", marginTop: 8 },
  link: { color: ORANGE, fontWeight: "700" },
  subtle: { color: "#6b7280" },
});
