# Sprint 3

1. name: Sidar Santhos
2. github: ssanthos
3. group: VolBites

## What you planned to do

1. Location change functionality for restaurant search
2. Dark mode contrast improvements across UI components
3. Enhanced user experience for restaurant discovery

## What you did not do

1. Automatic location detection using device GPS
2. Location history/previous locations feature
3. Map integration for restaurant locations

## What problems you encountered

- Ensuring location persistence across app sessions using AsyncStorage
- Implementing geocoding to convert city names to coordinates
- Maintaining consistent dark mode styling across all screens
- Handling edge cases in location input validation
- Ensuring proper contrast ratios for accessibility in dark mode

## Issues you worked on

1. Location change functionality for restaurants (custom location feature)
2. Dark mode contrast improvements for Settings screen
3. Dark mode contrast improvements for Upload Recipe screen

## Files you worked on

1. VolBites/geoapify.js
2. VolBites/RestaurantsScreen.js
3. VolBites/SettingsScreen.js
4. VolBites/UploadRecipeScreen.js

## Use of AI and/or 3rd party software

- Used Cursor AI to assist with implementing location change functionality and geocoding API integration
- Utilized Geoapify Geocoding API for converting city names to coordinates
- Used AI assistance for improving dark mode contrast and accessibility

## What you accomplished

For this sprint, I focused on enhancing the restaurant discovery feature by implementing a location change functionality. Users can now select any city when searching for restaurants, not just the default Knoxville location. The location preference is saved using AsyncStorage and persists across app sessions. I also improved the dark mode experience by enhancing text contrast in the Settings screen (specifically for diet preferences) and the Upload Recipe screen, ensuring better readability and accessibility. The location change feature includes a modal interface with city input, geocoding validation, and a reset option to return to the default location.

