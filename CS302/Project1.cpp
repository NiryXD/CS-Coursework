// Name: Dennis Madapatu
// Date: 1/22/2025
// Description: Music Library Information Processor
#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

struct Song {
   string songName;
   int songTime;
};

struct Album {
   string albumName;
   int albumRuntime = 0;
   int albumSongs = 0;
   map<int, Song> albumMap;
};

struct Artist {
   map<string, Album> discography;
   string artistName;
   int artistRuntime = 0;
   int artistSongs = 0;
};

string timeToString(int totalTime) {
   int minutes = totalTime / 60;
   int seconds = totalTime % 60;
   string paddedSeconds = (seconds < 10) ? "0" + to_string(seconds) : to_string(seconds);
   return to_string(minutes) + ":" + paddedSeconds;
}

int calculateTime(string minutes, string seconds) {
   int numMinutes = stoi(minutes);
   int numSeconds = stoi(seconds);
   return (numMinutes * 60) + numSeconds;
}

string replaceUnderscores(string str) {
   replace(str.begin(), str.end(), '_', ' ');
   return str;
}

int main(int argc, char *argv[]) {
   map <string, Artist> artistMap; // Map of all of the artists
   ifstream fin(argv[1]); // Opens the file given by the user
   string title;
   while(getline(fin, title, ' ')) { // Taking in all of the data and putting them inside of their respective variables
       string minutes;
       getline(fin, minutes, ':');
       string seconds;
       getline(fin, seconds, ' ');
       string artist;
       getline(fin, artist, ' ');
       string album;
       getline(fin, album, ' ');
       string genre;
       getline(fin, genre, ' ');
       string trackNum;
       getline(fin, trackNum);
       int time = calculateTime(minutes, seconds);
       
       title = replaceUnderscores(title);
       artist = replaceUnderscores(artist);
       album = replaceUnderscores(album);

       Artist newArtist;
       newArtist.artistName = artist;
       Album newAlbum;
       newAlbum.albumName = album;
       newAlbum.albumRuntime += time;
       newAlbum.albumSongs++;
       Song newSong;
       newSong.songName = title;
       newSong.songTime = time;

       if(artistMap.find(artist) != artistMap.end()) { // If true, we have found the artist inside of artistMap
           artistMap[artist].artistRuntime += time;
           artistMap[artist].artistSongs++;
           if(artistMap[artist].discography.find(album) != artistMap[artist].discography.end()) { 
               // If true, we have found the album in his discography
               artistMap[artist].discography[album].albumMap[stoi(trackNum)] = newSong; 
           }
           else {
               newAlbum.albumMap[stoi(trackNum)] = newSong;
               artistMap[artist].discography[album] = newAlbum;
           }
       }
       else {  // Put in all the details of the album, insert the song, and insert the album into the discography
           newAlbum.albumMap[stoi(trackNum)] = newSong;
           newArtist.artistRuntime += time;
           newArtist.artistSongs++;
           newArtist.discography[album] = newAlbum;
           artistMap[artist] = newArtist; // setting the new artist's key value
       }
   }

   // Iterate through artists in lexicographic order
   for (map<string, Artist>::const_iterator artistIt = artistMap.begin(); 
         artistIt != artistMap.end(); ++artistIt) {
        const Artist& artist = artistIt->second;
        
        // Print artist name, total songs, and total runtime
        cout << artist.artistName << ": " 
             << artist.artistSongs << ", " 
             << timeToString(artist.artistRuntime) << endl;

        // Iterate through albums for this artist in lexicographic order
        for (map<string, Album>::const_iterator albumIt = artist.discography.begin(); 
             albumIt != artist.discography.end(); ++albumIt) {
            const Album& album = albumIt->second;
            
            // Print album name, number of songs, and album runtime
            cout << "        " << album.albumName << ": " 
                 << album.albumSongs << ", " 
                 << timeToString(album.albumRuntime) << endl;

            // Iterate through songs in the album, sorted by track number
            map<int, Song>::const_iterator songIt = album.albumMap.begin();
            for (; songIt != album.albumMap.end(); ++songIt) {
                const Song& song = songIt->second;
                
                // Print track number, song name, and song runtime
                cout << "                " 
                     << songIt->first << ". " 
                     << song.songName << ": " 
                     << timeToString(song.songTime) << endl;
            }

            // Only print a newline if this is not the last album
            if (std::next(albumIt) != artist.discography.end()) {
                cout << endl;
            }
        }
    }

    return 0;
}