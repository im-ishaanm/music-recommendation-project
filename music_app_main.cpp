#include<iostream>
#include<string>
#include<cstring>
#include<map>
#include<iterator>
#include<vector>

#include "sqlite3.h"

using namespace std;

class SQLiteDatabase {
    private:
        multimap<string, string> artist_song_pair_map;
        multimap<string, string> song_genre_pair_map;
    public:
        void fetchPlaylistData() {
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

        void fetchAllData() {
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

        void mainMenu() {
            SQLiteDatabase myDB;
            myDB.fetchAllData();
            MusicList music_list = MusicList(myDB);

            bool exit = false;
            while(!exit) {
                cout << "\n\n" << endl;
                int choice;
                cout << "Welcome to NotSpotify! Let's get you started: " << endl;
                cout << "1. Show All Songs\n2. Search for a Song\n3. Create Playlist\n4. Exit" << endl;
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