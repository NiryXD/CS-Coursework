# Sprint 2

1. name: Sidar Santhos
2. github: ssanthos
3. group: VolBites

## What you planned to do

1. Local recipe authoring flow (upload and view)
2. Improve Home screen UX for personal content
3. Basic data lifecycle for locally created items

## What you did not do

1. Cloud sync of user-created recipes
2. Edit existing local recipes (create/delete only for now)

## What problems you encountered

- Ensuring uploaded recipes matched the same structure consumed by `RecipeDetailScreen` (nutrition, ingredients, etc.).
- Coordinating navigation so newly saved recipes open seamlessly in the detail view.
- Handling image selection and permissions with `expo-image-picker`.

## Issues you worked on

- Local uploads and personal feed (no GitHub issue number provided)
- Home UX: quick entry point for uploads and local list
- Deletion flows for local items (confirmation, state refresh)

## Files you worked on

- `VolBites/userRecipesStorage.js`
- `VolBites/UploadRecipeScreen.js`
- `VolBites/HomeScreen.js`
- `VolBites/RecipeDetailScreen.js`
- `VolBites/App.js`

## Use of AI and/or 3rd party software

- Used ChatGPT/Cursor to scaffold storage and screen boilerplate, then refined to match app structure.
- Used `expo-image-picker` for selecting images from the device gallery.

## What you accomplished

- Implemented a complete local recipe upload flow with fields for title, summary, ingredients, instructions, and nutrition, producing objects compatible with `RecipeDetailScreen`.
- Added `UploadRecipe` screen to the navigator and an “Add” button in `HomeScreen` to launch it.
- Created a “Your Recipes” section in `HomeScreen` that lists locally saved items; tapping opens detail.
- Added deletion of local recipes from both the detail screen (trash icon) and via long-press on list items with confirmation.
