#!/bin/bash
# myblame.sh — show only Ar-Raniry's changes in key project files
# and save the output to myblame-report.txt

OUTPUT_FILE="myblame-report.txt"
FILES="LoginScreen.js PuzzleCaptcha.js SettingsScreen.js SignUpScreen.js package-lock.json package.json RememberMe.js"

# Clear previous file
echo "Generating blame report for Ar-Raniry Nurdin Ar-Rasyid..." > "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Loop through files
for f in $FILES; do
  echo "## $f" >> "$OUTPUT_FILE"
  git blame --color-lines "$f" | grep "Ar-Raniry Nurdin Ar-Rasyid" >> "$OUTPUT_FILE"
  echo "" >> "$OUTPUT_FILE"
done

echo "✅ Report saved to $OUTPUT_FILE"

