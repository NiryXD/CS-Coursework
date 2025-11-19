// Includes necessary libraries for the program to work
#include <bits/stdc++.h>
using namespace std;

// Structure to represent a song, with its name and duration
struct Song
{
    string songName;   // The name of the song
    int songTime;      // The length of the song in seconds
};

// Structure to represent an album, with its name, runtime, number of songs, and a map of songs
struct Album
{
    string albumName;           // The name of the album
    int albumRuntime;           // Total runtime of all songs in the album (in seconds)
    int albumSongs;             // Total number of songs in the album
    map<int, Song> albumMap;    // Maps track number to the corresponding Song object
};

// Structure to represent an artist, with their name, total runtime, total number of songs, and their discography
struct Artist
{
    map<string, Album> discography; // Maps album name to the corresponding Album object
    string artistName;              // The name of the artist
    int artistRuntime;              // Total runtime of all the artist's songs (in seconds)
    int artistSongs;                // Total number of songs by the artist
};

// Helper function to convert time from seconds into a "minutes:seconds" format (e.g., 3:45)
string timeToString(int totalTime)
{
    int minutes = totalTime / 60;         // Calculate the number of minutes
    int seconds = totalTime % 60;         // Calculate the remaining seconds
    string time;                          // To store the formatted time
    if (seconds < 10)                     // Add a leading zero if seconds are less than 10
    {
        time = "0" + to_string(seconds);
    }
    else
    {
        time = to_string(seconds);        // Otherwise, just convert seconds to string
    }
    return to_string(minutes) + ":" + time; // Combine minutes and seconds into a single string
}

// Function to print all data stored in the artist map (artists, their albums, and songs)
void printEverything(map<string, Artist> &artistMap)
{
    // Loop through each artist in the artist map
    for (map<string, Artist>::const_iterator artistIterator = artistMap.begin(); artistIterator != artistMap.end(); ++artistIterator)
    {
        const Artist &artist = artistIterator->second; // Get the current artist

        // Print artist name, total number of songs, and total runtime
        cout << artist.artistName << ": " << artist.artistSongs << ", " << timeToString(artist.artistRuntime) << endl;

        // Loop through each album in the artist's discography
        for (map<string, Album>::const_iterator albumIterator = artist.discography.begin(); albumIterator != artist.discography.end(); ++albumIterator)
        {
            const Album &album = albumIterator->second; // Get the current album

            // Print album name, total number of songs, and total runtime
            cout << "        " << album.albumName << ": " << album.albumSongs << ", " << timeToString(album.albumRuntime) << endl;

            // Loop through each song in the album
            for (map<int, Song>::const_iterator songIterator = album.albumMap.begin(); songIterator != album.albumMap.end(); ++songIterator)
            {
                const Song &song = songIterator->second; // Get the current song

                // Print track number, song name, and song duration
                cout << "                " << songIterator->first << ". " << song.songName << ": " << timeToString(song.songTime) << endl;
            }
        }
    }
}

// Helper function to replace underscores in a string with spaces
string replaceUnderscores(string str)
{
    replace(str.begin(), str.end(), '_', ' '); // Replace all underscores with spaces
    return str;
}

// Helper function to calculate total time in seconds from "minutes:seconds" strings
int calculateTime(string minutes, string seconds)
{
    int numMinutes = stoi(minutes);       // Convert minutes from string to integer
    int numSeconds = stoi(seconds);       // Convert seconds from string to integer
    return (numMinutes * 60) + numSeconds; // Calculate total time in seconds
}

// Main function
int main(int argc, char *argv[])
{
    map<string, Artist> artistMap; // Map to store all artist data

    ifstream fin(argv[1]); // Open the file provided as the first command-line argument

    string line; // Variable to hold each line of the file

    // Read the file line by line
    while (getline(fin, line))
    {
        istringstream iss(line); // Use a string stream to parse the line
        string title, minutes, seconds, artist, album, genre, trackNumOne;

        // Extract song data from the line, separated by spaces or colons
        getline(iss, title, ' ');        // Song title
        getline(iss, minutes, ':');     // Minutes part of the duration
        getline(iss, seconds, ' ');     // Seconds part of the duration
        getline(iss, artist, ' ');      // Artist name
        getline(iss, album, ' ');       // Album name
        getline(iss, genre, ' ');       // Genre (not used)
        getline(iss, trackNumOne, ' '); // Track number as a string

        // Replace underscores with spaces for song title, artist, and album
        title = replaceUnderscores(title);
        artist = replaceUnderscores(artist);
        album = replaceUnderscores(album);

        // Convert time to seconds and track number to integer
        int time = calculateTime(minutes, seconds);
        int trackNumTwo = stoi(trackNumOne);

        // Check if the artist is already in the map; if not, add them
        if (artistMap.find(artist) == artistMap.end())
        {
            artistMap[artist] = Artist();           // Create a new Artist object
            artistMap[artist].artistName = artist; // Set the artist's name
        }
        Artist &newArtist = artistMap[artist]; // Reference to the current artist

        // Check if the album is already in the artist's discography; if not, add it
        if (newArtist.discography.find(album) == newArtist.discography.end())
        {
            newArtist.discography[album] = Album();           // Create a new Album object
            newArtist.discography[album].albumName = album; // Set the album's name
        }
        Album &newAlbum = newArtist.discography[album]; // Reference to the current album

        // Create a new Song object with the extracted data
        Song newSong{title, time};
        newAlbum.albumMap[trackNumTwo] = newSong; // Add the song to the album map

        // Update runtime and song counts for the artist and album
        newArtist.artistRuntime += time;
        newArtist.artistSongs++;
        newAlbum.albumRuntime += time;
        newAlbum.albumSongs++;
    }

    // Print the entire collection of artists, albums, and songs
    printEverything(artistMap);
}
