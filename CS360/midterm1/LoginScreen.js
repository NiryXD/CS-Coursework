import React, { useRef, useState } from "react";
import { Platform, KeyboardAvoidingView, ScrollView, TextInput, StyleSheet } from "react-native";
import { supabase } from "./supabaseClient";
import { Box, Heading, VStack, Button, Text, Link, HStack, useToast } from "native-base";

export default function LoginScreen({ navigation }) {
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const passwordRef = useRef(null);
  const [loading, setLoading] = useState(false);
  const toast = useToast();

  const onLogin = async () => {
    if (!email || !password) {
      toast.show({ description: "Enter email and password" });
      return;
    }
    setLoading(true);
    const { data, error } = await supabase.auth.signInWithPassword({ email, password });
    setLoading(false);

    if (error) {
      toast.show({ description: "Incorrect email or password", bg: "red.500" });
      return;
    }
    navigation.replace("Home", { userId: data.user.id });
  };

  return (
    <Box safeArea flex={1} bg="white">
      <KeyboardAvoidingView
        style={{ flex: 1 }}
        behavior={Platform.OS === "ios" ? "padding" : "height"}
        keyboardVerticalOffset={Platform.select({ ios: 64, android: 0 })}
      >
        <ScrollView
          contentContainerStyle={{ flexGrow: 1 }}
          keyboardShouldPersistTaps="handled"
        >
          <Box flex={1} p={6} justifyContent="center">
            <Heading color="brand.600" mb={6}>Welcome back</Heading>

            <VStack space={4}>
              <Box>
                <Text mb={2}>Email</Text>
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
              </Box>

              <Box>
                <Text mb={2}>Password</Text>
                <TextInput
                  ref={passwordRef}
                  style={styles.input}
                  value={password}
                  onChangeText={setPassword}
                  secureTextEntry
                  returnKeyType="done"
                />
              </Box>

              <Button isLoading={loading} onPress={onLogin}>Log in</Button>

              <HStack justifyContent="center">
                <Text mr={1}>No account?</Text>
                <Link onPress={() => navigation.replace("SignUp")} _text={{ color: "brand.600" }}>
                  Sign up
                </Link>
              </HStack>
            </VStack>
          </Box>
        </ScrollView>
      </KeyboardAvoidingView>
    </Box>
  );
}

const styles = StyleSheet.create({
  input: {
    height: 48,
    borderWidth: 1,
    borderColor: "#e2e8f0", // gray-300
    borderRadius: 8,
    paddingHorizontal: 12,
    backgroundColor: "white",
  },
});