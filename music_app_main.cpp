#include<iostream>
#include<string>
#include<cstring>
#include<map>
#include<iterator>
#include<vector>
#include<algorithm>

#include "sqlite3.h"

using namespace std;

class SQLiteDatabase {
    private:
        multimap<string, string> artist_song_pair_map;
        multimap<string, string> song_genre_pair_map;
        
        multimap<string, string> playlist_name_song_map;
        multimap<string, string> playlist_name_artist_map;
        multimap<string, string> playlist_name_genre_map;
    public:

        static int insertPlaylist(string playlist_name, string artist_list, string song_list, string genres_list) {
            sqlite3 *DB;
            char *error;
            
            string query = "INSERT INTO PLAYLISTS(PLAYLIST_NAME, SONGS_LIST, ARTISTS_LIST, GENRES_LIST) VALUES('" + playlist_name + "', '" + song_list + "', '" + artist_list +"', '" + genres_list +"');";
            
            int response = sqlite3_open("./db/projectDB.db", &DB);
            response = sqlite3_exec(DB, query.c_str(), NULL, 0, &error);

            if(response != SQLITE_OK) {
                cout << "Playlist " << playlist_name << " created successfully!" << endl;
                sqlite3_free(error);
            } else {
                sqlite3_close(DB);
                return 1;
            }
            sqlite3_close(DB);
            return 0;
        }
    
        void fetchPlaylistData() {
            sqlite3* DB;
            char* error;

            sqlite3_stmt *stmt;
            int response = sqlite3_open("./db/projectDB.db", &DB);

            int rc = sqlite3_prepare_v2(DB, "SELECT * FROM PLAYLISTS", -1, &stmt, NULL);

            if(rc != SQLITE_OK) {
                cout << "Error fetching data" << endl;
            }
            while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                const char * playlist_name_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                const char * songs_list_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                const char * artists_list_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                const char * genres_list_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                
                string playlist_name(playlist_name_data);
                string songs_list(songs_list_data);
                string artists_list(artists_list_data);
                string genres_list(genres_list_data);

                playlist_name_song_map.insert(pair <string, string>(playlist_name, songs_list));
                playlist_name_artist_map.insert(pair <string, string>(playlist_name, artists_list));
                playlist_name_genre_map.insert(pair <string, string>(playlist_name, genres_list));
            }
            if(rc != SQLITE_DONE) {
                cout << "Something went wrong." << endl;
            }
            sqlite3_finalize(stmt);
            sqlite3_close(DB);
        }

        void fetchMusicData() {
            sqlite3* DB;
            char* error;

            sqlite3_stmt *stmt;
            int response = sqlite3_open("./db/projectDB.db", &DB);

            int rc = sqlite3_prepare_v2(DB, "SELECT * FROM SONGS", -1, &stmt, NULL);

            if(rc != SQLITE_OK) {
                cout << "Error fetching data" << endl;
            }
            while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                const char * song_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                const char * artist_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                const char * genre_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                string artist(artist_data);
                string genres(genre_data);
                string song(song_data);
                string genre_pair_key = song + ":" + artist;
                artist_song_pair_map.insert(pair <string, string>(artist, song));
                song_genre_pair_map.insert(pair <string, string>(genre_pair_key, genres));
            }
            if(rc != SQLITE_DONE) {
                cout << "Something went wrong." << endl;
            }
            sqlite3_finalize(stmt);
            sqlite3_close(DB);
        }
        
        multimap<string, string> getArtistSongPair() {
            return artist_song_pair_map;
        }
        multimap<string, string> getSongGenrePair() {
            return song_genre_pair_map;
        }
        multimap<string, string> getPlaylistSongPair() {
            return playlist_name_song_map;
        }
        multimap<string, string> getPlaylistArtistPair() {
            return playlist_name_artist_map;
        }
        multimap<string, string> getPlaylistGenrePair() {
            return playlist_name_genre_map;
        }
};

class Playlist {
    private:
        multimap<string, string> artist_song_pair_map;
        multimap<string, string> song_genre_pair_map;

        multimap<string, string> playlist_name_song_map;
        multimap<string, string> playlist_name_artist_map;
        multimap<string, string> playlist_name_genre_map;
    public:
        Playlist(SQLiteDatabase database) {
            playlist_name_song_map = database.getPlaylistSongPair();
            playlist_name_artist_map = database.getPlaylistArtistPair();
            playlist_name_genre_map = database.getPlaylistGenrePair();

            artist_song_pair_map = database.getArtistSongPair();
            song_genre_pair_map = database.getSongGenrePair();
        }

        string getSongDetails(string song_name, string artist_name) {
            string details;

            auto iterator = artist_song_pair_map.equal_range(artist_name);

            if(iterator.first == iterator.second) {
                cout << "No artists found by the name " << artist_name << endl;
                return("error");
            }

            for(auto itr = iterator.first; itr != iterator.second; itr++) {
                if(itr->second == song_name) {
                    string genre_pair_key = song_name + ":" + artist_name;
                    details = song_genre_pair_map.find(genre_pair_key)->second;
                    return details;
                }
            }

            cout << "No songs found by the name " << song_name << endl;
            return("error");
        }


        void remove_duplicates(vector<string>& vec)
        {
            std::sort(vec.begin(), vec.end());
            vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
        }

        string removeDuplicateGenres(string s) {
            string genre_list;
            vector<string> vec = expandString(s);
            remove_duplicates(vec);
            for(int i = 0; i < vec.size(); i++) {
                if(genre_list.length() <= 0) {
                    genre_list = vec[i];
                } else {
                    genre_list = genre_list + ", " + vec[i];
                }
            }
            return genre_list;
        }

        vector<string> expandString(string s) {
            vector<string> returnVector;
            string delimiter = ", ";
            size_t pos = 0;
            string token;
            while ((pos = s.find(delimiter)) != string::npos) {
                token = s.substr(0, pos);
                returnVector.push_back(token);
                s.erase(0, pos + delimiter.length());
            }
            returnVector.push_back(s);
            return returnVector;
        }

        void createPlaylist() {
            string playlist_name, song_name, artist_name;

            string songs_list, genres_list, artists_list;
            cout << "Enter name of the playlist: ";
            getline(cin, playlist_name);
            while(true) {
                cout << "Enter name of song to add [Type '!stop' to stop]: ";
                getline(cin, song_name);

                if(song_name == "!stop") {
                    break;
                }

                cout << "Enter the artist of the song " << song_name << ": ";
                getline(cin, artist_name);

                if(song_name == "!stop" || artist_name == "!stop") {
                    break;
                }

                string song_details = getSongDetails(song_name, artist_name);
                if(song_details != "error") {
                    if(songs_list.length() <= 0) {
                        songs_list = song_name;
                    } else {
                        songs_list = songs_list + ", " + song_name;
                    }
                    if(artists_list.length() <= 0) {
                        artists_list = artist_name;
                    } else {
                        artists_list = artists_list + ", " + artist_name;
                    }
                    if(genres_list.length() <= 0) {
                        genres_list = song_details;
                    } else {
                        genres_list = genres_list + ", " + song_details;
                    }
                    cout << song_name << " by " << artist_name << " added successfully!" << endl;
                } else {
                    cout << "Error adding song to playlist. Please try again!" << endl;
                }
            }
            if(songs_list.length() > 0) {
                while(true) {
                    if(playlist_name_song_map.lower_bound(playlist_name)->first == playlist_name) {
                        cout << "Playlist by this name already exists!" << endl;
                        cout << "Enter new playlist name: ";
                        getline(cin, playlist_name);
                    } else {
                        break;
                    }
                }
                genres_list = removeDuplicateGenres(genres_list);
                SQLiteDatabase::insertPlaylist(playlist_name, artists_list, songs_list, genres_list);
            }
        }

        void printPlaylists() {
            multimap<string, string> :: iterator itr;
            vector<string> songs_list;
            int id = 1;
            for(itr = playlist_name_song_map.begin(); itr != playlist_name_song_map.end(); itr++) {
                songs_list = expandString(itr->second);
                cout << id << ". " << itr->first << " [" << songs_list.size() << " Songs]" << endl;
                id++;
            }
            cout << endl;       
        } 

        void openPlaylist(string playlist_name) {
            multimap<string, string> :: iterator itr;
            vector<string> songs_list, artists_list, genres_list;

            auto song_iterator = playlist_name_song_map.equal_range(playlist_name);
            auto artist_iterator = playlist_name_artist_map.equal_range(playlist_name);
            auto genre_iterator = playlist_name_genre_map.equal_range(playlist_name);

            if(song_iterator.first == song_iterator.second) {
                cout << "This playlist does not exist" << endl;
                return;
            }

            for (auto itr = song_iterator.first; itr != song_iterator.second; ++itr) {
                songs_list = expandString(itr->second);
            }
            for (auto itr = artist_iterator.first; itr != artist_iterator.second; ++itr) {
                artists_list = expandString(itr->second);
            }
            for (auto itr = genre_iterator.first; itr != genre_iterator.second; ++itr) {
                genres_list = expandString(itr->second);
            }

            cout << "----- " << playlist_name <<  " ----- " << endl << endl;
            cout << "This playlist contains " << songs_list.size() << " songs with a genre mix of: " << endl;
            int i;
            for(i = 0; i < genres_list.size()-1; i++) {
                cout << genres_list[i] << ", ";
            }
            cout << genres_list[i] << endl << endl;

            for(int i = 0; i < songs_list.size(); i++) {
                cout << i + 1 << ". " << songs_list[i] << " by " << artists_list[i] << endl;
            }
        }
};

class MusicList {
    private:
        multimap<string, string> artist_song_pair_map;
        multimap<string, string> song_genre_pair_map;
        
    public:
        MusicList(SQLiteDatabase database) {
            artist_song_pair_map = database.getArtistSongPair();
            song_genre_pair_map = database.getSongGenrePair();
        }

        vector<string> decryptGenreKey(string key) {
            string delimiter = ":";
            string token_song = key.substr(0, key.find(delimiter));
            key.erase(0, key.find(delimiter) + delimiter.length());
            string token_artist = key.substr(0, key.find(delimiter));
            return { token_song, token_artist };
        }

        bool doesSongHaveGenre(string s, string delimiter, string genre) {
            size_t pos = 0;
            string token;
            while ((pos = s.find(delimiter)) != string::npos) {
                token = s.substr(0, pos);
                if(token == genre) {
                    return true;
                }
                s.erase(0, pos + delimiter.length());
            }
            if(s == genre) {
                return true;
            }
            return false;
        }

        void printAllSongs() {
            multimap<string, string> :: iterator itr;
            int id = 1;
            for(itr = artist_song_pair_map.begin(); itr != artist_song_pair_map.end(); itr++) {
                string genre_pair_key = itr->second + ":" + itr->first;
                cout << id << ". " << itr->second << " by " << itr->first << " [" << song_genre_pair_map.find(genre_pair_key)->second << "]" << endl;
                id++;
            }
            cout << endl;
        }
        

        void findSongByGenre(string genre) {
            int id = 1;
            bool songHasGenre, found = false;
            vector<string> song_artist_pair;
            multimap<string, string> :: iterator itr;

            for(itr = song_genre_pair_map.begin(); itr != song_genre_pair_map.end(); itr++) {
                songHasGenre = doesSongHaveGenre(itr->second, ", ", genre);
                if(songHasGenre) {
                    found = true;
                    song_artist_pair = decryptGenreKey(itr->first);
                    
                    multimap<string, string> :: iterator itr2;
                    for(itr2 = artist_song_pair_map.begin(); itr2 != artist_song_pair_map.end(); itr2++) {
                        if(itr2->second == song_artist_pair[0] && itr2->first == song_artist_pair[1]) {
                            cout << id << ". " << itr2->second << " by " << itr2->first << " [" << itr->second << "]" << endl;
                            id++;
                        }
                    }
                }
            }
            if(!found) {
                cout << "No songs found with genre " << genre << endl;
            }
        }

        void findSongByName(string name) {
            int id = 1;
            bool found = false;
            multimap<string, string> :: iterator itr;
            cout << endl;
            for(itr = artist_song_pair_map.begin(); itr != artist_song_pair_map.end(); itr++) {
                string genre_pair_key = itr->second + ":" + itr->first;
                if(itr->second == name) {
                    cout << id << ". " << itr->second << " by " << itr->first << " [" << song_genre_pair_map.find(genre_pair_key)->second << "]" << endl;
                    found = true;
                    id++;
                }
            }
            cout << endl;
            if(!found) {
                cout << "Could not find the song: " << name << ". Try again X_X" << endl;
            }
        }

        void findSongByArtist(string artist) {
            int id = 1;
            auto iterator = artist_song_pair_map.equal_range(artist);

            if(iterator.first == iterator.second) {
                cout << "No artists found by the name " << artist << endl;
                return;
            }

            for(auto itr = iterator.first; itr != iterator.second; itr++) {
                string genre_pair_key = itr->second + ":" + itr->first;
                cout << id << ". " << itr->second << " by " << itr->first << " [" << song_genre_pair_map.find(genre_pair_key)->second << "]" << endl;
                id++;
            }
        }

        multimap<string, string> getSongGenrePair() {
            return song_genre_pair_map;
        }
        multimap<string, string> getArtistSongPair() {
            return artist_song_pair_map;
        }
};

class AppUI {
    private:
        void spacify() {
            cout << "\n\n\n" << endl;
            cout << "-----------------" << endl;
        }
    public:
        void searchMenu(MusicList music_list) {

            string song_name, artist_name, genre;

            int choice;
            cout << "Search song by: " << endl;
            cout << "1. Name\n2. Artist\n3. Genre" << endl;
            cout << "Choice: ";
            cin >> choice;
            cin.ignore();
            switch(choice) {
                case 1:
                    cout << "Enter the name of the song: ";
                    getline(cin, song_name);
                    spacify();
                    music_list.findSongByName(song_name);
                    break;
                case 2:
                    cout << "Enter the name of the artist: ";
                    getline(cin, artist_name);
                    spacify();
                    music_list.findSongByArtist(artist_name);
                    break;
                case 3:
                    cout << "Enter the genre: ";
                    getline(cin, genre);
                    spacify();
                    music_list.findSongByGenre(genre);
                    break;
                default:
                    cout << "Invalid choice! Please try again." << endl;
            }

        }

        void playlistMenu(Playlist playlist) {
            string playlist_name;
            int choice;
            cout << "Playlist Options: " << endl;
            cout << "1. Show all Playlists\n2. Open a Playlist\n3. Create a Playlist" << endl;
            cout << "Choice: ";
            cin >> choice;
            cin.ignore();

            switch(choice) {
                case 1:
                    spacify();
                    playlist.printPlaylists();
                    break;
                case 2:
                    cout << "Enter the name of the playlist: ";
                    getline(cin, playlist_name);
                    spacify();
                    playlist.openPlaylist(playlist_name);
                    break;
                case 3:
                    spacify();
                    playlist.createPlaylist();
                    break;
                default:
                    cout << "Invalid choice! Please try again." << endl;
            }
        }

        void mainMenu() {
            SQLiteDatabase myDB;
            myDB.fetchMusicData();
            myDB.fetchPlaylistData();
            MusicList music_list = MusicList(myDB);
            Playlist playlist = Playlist(myDB);

            bool exit = false;
            while(!exit) {
                cout << "\n\n" << endl;
                int choice;
                cout << "Welcome to NotSpotify! Let's get you started: " << endl;
                cout << "1. Show All Songs\n2. Search for a Song\n3. Playlists\n4. Exit" << endl;
                cout << "Choice: ";
                cin >> choice;

                switch(choice) {
                    case 1:
                        spacify();
                        music_list.printAllSongs();
                        break;
                    case 2:
                        spacify();
                        searchMenu(music_list);
                        break;
                    case 3:
                        spacify();
                        playlistMenu(playlist);
                        break;
                    case 4:
                        exit = true;
                        break;
                    default:
                        cout << "Invalid choice. Please try again." << endl;
                }
            }
            cout << "See you soon!" << endl;
        }
};

int main() {
    AppUI UI;
    UI.mainMenu();
    return 0;
}