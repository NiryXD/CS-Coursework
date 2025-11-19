// Name: Dennis Madapatu and Ar-Raniry Ar-Rasyid
// Date: 1/22/2025
// Description: Takes song data and organizes it.

#include <bits/stdc++.h>

using namespace std;

struct Song
{
    string songName;
    int songTime;
};

struct Album
{
    string albumName;
    int albumRuntime;
    int albumSongs;
    map<int, Song> albumMap; // int should be the tracking number
};

struct Artist
{
    map<string, Album> discography; // string should be the album's name
    string artistName;
    int artistRuntime;
    int artistSongs;
};

string timeToString(int totalTime)
{
    int minutes = totalTime / 60;
    int seconds = totalTime % 60;
    string time;
    if (seconds < 10)
    {
        time = "0" + to_string(seconds);
    }
    else
    {
        time = to_string(seconds);
    }
    return to_string(minutes) + ":" + time;
}

void printEverything(map<string, Artist> &artistMap)
{
    for (map<string, Artist>::const_iterator artistIt = artistMap.begin(); artistIt != artistMap.end(); ++artistIt)
    {
        const Artist &artist = artistIt->second;

        cout << artist.artistName << ": " << artist.artistSongs << ", " << timeToString(artist.artistRuntime) << endl;

        for (map<string, Album>::const_iterator albumIt = artist.discography.begin(); albumIt != artist.discography.end(); ++albumIt)
        {
            const Album &album = albumIt->second;

            cout << "        " << album.albumName << ": " << album.albumSongs << ", " << timeToString(album.albumRuntime) << endl;

            for (map<int, Song>::const_iterator songIt = album.albumMap.begin(); songIt != album.albumMap.end(); ++songIt)
            {
                const Song &song = songIt->second;

                cout << "                " << songIt->first << ". " << song.songName << ": " << timeToString(song.songTime) << endl;
            }
        }
    }
}

string replaceUnderscores(string str)
{
    replace(str.begin(), str.end(), '_', ' ');
    return str;
}

int calculateTime(string minutes, string seconds)
{
    int numMinutes = stoi(minutes);
    int numSeconds = stoi(seconds);
    int totalSeconds = (numMinutes * 60) + numSeconds;
    return totalSeconds;
}

int main(int argc, char *argv[])
{
    ifstream fin(argv[1]);
    map<string, Artist> artistMap;

    string line;
    while (getline(fin, line))
    {
        istringstream iss(line);
        string title, minutesStr, secondsStr, artist, album, genre, trackNumStr;
        getline(iss, title, ' ');
        getline(iss, minutesStr, ':');
        getline(iss, secondsStr, ' ');
        getline(iss, artist, ' ');
        getline(iss, album, ' ');
        getline(iss, genre, ' ');
        getline(iss, trackNumStr);

        title = replaceUnderscores(title);
        artist = replaceUnderscores(artist);
        album = replaceUnderscores(album);

        int time = calculateTime(minutesStr, secondsStr);
        int trackNum = stoi(trackNumStr);

        if (artistMap.find(artist) == artistMap.end())
        {
            artistMap[artist] = Artist();
            artistMap[artist].artistName = artist;
        }
        Artist &currentArtist = artistMap[artist];

        if (currentArtist.discography.find(album) == currentArtist.discography.end())
        {
            currentArtist.discography[album] = Album();
            currentArtist.discography[album].albumName = album;
        }
        Album &currentAlbum = currentArtist.discography[album];

        Song newSong{title, time};
        currentAlbum.albumMap[trackNum] = newSong;

        currentArtist.artistRuntime += time;
        currentArtist.artistSongs++;
        currentAlbum.albumRuntime += time;
        currentAlbum.albumSongs++;
    }

    printEverything(artistMap);

    return 0;
}