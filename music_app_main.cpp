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
        // This function deletes a single record from the playlist table in the database.
        // Deletion is based on the name of the playlist.
        static int deletePlaylist(string playlist_name) {
            sqlite3 *DB;
            char *error;
            
            // Query to be sent for SQL execution.
            string query = "DELETE FROM PLAYLISTS WHERE PLAYLIST_NAME = '" + playlist_name + "';";
            
            // Responds with an integer response code which indicates status of the SQL Request.
            int response = sqlite3_open("./db/projectDB.db", &DB);
            response = sqlite3_exec(DB, query.c_str(), NULL, 0, &error);

            if(response != SQLITE_OK) {
                cout << "Error deleting playlist." << endl;
                sqlite3_free(error);
            } else {
                // Success
                cout << "Playlist " << playlist_name << " deleted successfully!" << endl;
                // Free memory and close the database because we don't need it anymore.
                sqlite3_free(error);
                sqlite3_close(DB);
                return 1;
            }
            sqlite3_close(DB);
            return 0;
        }

        // This function adds a single record to the playlist table in the database. The table contains the following columns:
        // PLAYLIST_NAME | SONGS_LIST | ARTISTS_LIST | GENRES_LIST
        static int insertPlaylist(string playlist_name, string artist_list, string song_list, string genres_list) {
            sqlite3 *DB;
            char *error;
            
            string query = "INSERT INTO PLAYLISTS(PLAYLIST_NAME, SONGS_LIST, ARTISTS_LIST, GENRES_LIST) VALUES('" + playlist_name + "', '" + song_list + "', '" + artist_list +"', '" + genres_list +"');";
            
            int response = sqlite3_open("./db/projectDB.db", &DB);
            response = sqlite3_exec(DB, query.c_str(), NULL, 0, &error);

            if(response != SQLITE_OK) {
                cout << "Error deleting Playlist" << endl;
            } else {
                cout << "Playlist " << playlist_name << " created successfully!" << endl;
                sqlite3_free(error);
                sqlite3_close(DB);
                return 1;
            }
            sqlite3_close(DB);
            return 0;
        }

        // This function returns all records from the playlist table. The data returned is then casted onto the
        // multimaps which store the following data as pairs
        // PLAYLIST_NAME : SONGS | PLAYLIST_NAME : ARTISTS | PLAYLIST_NAME : GENRES
        void fetchPlaylistData() {
            sqlite3* DB;
            char* error;

            // Clear whatever current data is stored in the maps. This is useful to obtain fresh data from the DB
            // which allows the maps to update immediately with new data instead of having to re-execute the program.
            playlist_name_song_map.clear();
            playlist_name_artist_map.clear();
            playlist_name_genre_map.clear();

            sqlite3_stmt *stmt;
            int response = sqlite3_open("./db/projectDB.db", &DB);

            int rc = sqlite3_prepare_v2(DB, "SELECT * FROM PLAYLISTS", -1, &stmt, NULL);

            if(rc != SQLITE_OK) {
                cout << "Error fetching data" << endl;
            }
            // Loops through all the rows in the database. In order to ensure that STMT returns any
            // sort of non-empty value we check if the data returned is equal to SQLITE_ROW.
            while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                // sqlite3_column_text returns each single column data based on index. For example,
                // sqlite3_column_text(stmt, 0) returns data from the first column
                // sqlite3_column_text(stmt, 2) returns data from the third column
                int id = sqlite3_column_int(stmt, 0);
                const char * playlist_name_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                const char * songs_list_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                const char * artists_list_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                const char * genres_list_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                
                // Convert char* to string.
                string playlist_name(playlist_name_data);
                string songs_list(songs_list_data);
                string artists_list(artists_list_data);
                string genres_list(genres_list_data);

                // Checks if current playlist data already exists in the map or not.
                // Not sure if this check is required or not. !!!
                if(playlist_name_song_map.count(playlist_name) == 0) {
                    playlist_name_song_map.insert(pair <string, string>(playlist_name, songs_list));
                    playlist_name_artist_map.insert(pair <string, string>(playlist_name, artists_list));
                    playlist_name_genre_map.insert(pair <string, string>(playlist_name, genres_list));
                }
            }
            if(rc != SQLITE_DONE) {
                cout << "Something went wrong." << endl;
            }
            sqlite3_finalize(stmt);
            sqlite3_close(DB);
        }

        // This function fetches all the songs in the SONGS table of the database.
        // Returned data is the song, the artist and the genre of the song.
        void fetchMusicData() {
            sqlite3* DB;
            char* error;

            sqlite3_stmt *stmt;
            int response = sqlite3_open("./db/projectDB.db", &DB);

            int rc = sqlite3_prepare_v2(DB, "SELECT * FROM SONGS", -1, &stmt, NULL);

            if(rc != SQLITE_OK) {
                cout << "Error fetching data" << endl;
            }
            // Loops through all the rows in the database. In order to ensure that STMT returns any
            // sort of non-empty value we check if the data returned is equal to SQLITE_ROW.
            while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                // sqlite3_column_text returns each single column data based on index. For example,
                // sqlite3_column_text(stmt, 0) returns data from the first column
                // sqlite3_column_text(stmt, 2) returns data from the third column
                int id = sqlite3_column_int(stmt, 0);
                const char * song_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                const char * artist_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                const char * genre_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                string artist(artist_data);
                string genres(genre_data);
                string song(song_data);
                // We're storing the song_genre_pair_map as a map of a key and the genres. We first noticed a problem that
                // storing it as just simply a song : genre pair will lead to critical errors because there can be multiple songs
                // of the same name by different artists. So instead, we're storing the key as a combination of the song and artist
                // represented as SONG:ARTIST which we will decrypt later on to extract data.
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
        
        // Updates a single row from the PLAYLIST table based on the playlist name passed. This update function allows us to change:
        // The songs in the playlist & it's artists which in turn changes the list of genres in the playlist and also allows us to change
        // the name of the playlist.
        static void updatePlaylist(string playlist_name, string new_songs_list, string new_artists_list, string new_genres_list, string new_playlist_name) {
            sqlite3 *DB;
            char *error;

            string query = "UPDATE PLAYLISTS SET PLAYLIST_NAME = '" + new_playlist_name + "', SONGS_LIST = '" + new_songs_list + "', ARTISTS_LIST = '" + new_artists_list + "', GENRES_LIST = '" + new_genres_list + "' WHERE PLAYLIST_NAME = '" + playlist_name + "';";
    
            try {
                int response = 0;
                response = sqlite3_open("./db/projectDB.db", &DB);

                response = sqlite3_exec(DB, query.c_str(), NULL, 0, &error);

                if(response != SQLITE_OK) {
                    cout << "Error updating playlist." << endl;
                    sqlite3_free(error);
                } else {
                    cout << "Playlist updated successfully!" << endl;
                }
                sqlite3_close(DB);
            } catch (const exception& e) {
                cout << e.what();
            }
            sqlite3_close(DB);  
        }

        // These set of methods allow this class to pass on data to other classes and share the maps among each other.
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

class MusicList {
    private:
        multimap<string, string> artist_song_pair_map;
        multimap<string, string> song_genre_pair_map;
        
    public:
        // Get data from the database and store it the respective maps.
        MusicList(SQLiteDatabase database) {
            artist_song_pair_map = database.getArtistSongPair();
            song_genre_pair_map = database.getSongGenrePair();
        }

        // Decrypts the SONG:ARTIST key in song_genre_pair_map.
        static vector<string> decryptGenreKey(string key) {
            // Delimiter is the seperator character value. In this case ':' is the character.
            string delimiter = ":";
            // Get the first token from the string before the delimiter.
            // SONG:ARTIST ->  return SONG
            string token_song = key.substr(0, key.find(delimiter));
            // Erases the token from the string. Therefore, SONG:ARTIST => ARTIST
            key.erase(0, key.find(delimiter) + delimiter.length());
            // Gets the remaining string. The substr() function isn't required but will be useful
            // in the future in case another parameter gets added to the key. For example in the
            // future our key might look like TOKEN1:TOKEN2:TOKEN3 which is why the substr() function will
            // help in the future.
            string token_artist = key.substr(0, key.find(delimiter));
            return { token_song, token_artist };
        }
        
        // Function checks if a certain song belongs to a certain genre.
        bool doesSongHaveGenre(string s, string delimiter, string genre) {
            // Genres are stored in the following string format:
            // "Genre1, Genre2, Genre3"... etc. Therefore in order to
            // evaluate each individual genre, we have to split this string.
            // The below while loop splits the string and returns each genre in the string
            // one by one to evaluate.
            size_t pos = 0;
            string token;
            // string::npos signifies the end of the string i.e. \0
            // token holds each substring which in this case, is each genre
            // if the token matches the passed genre, the song DOES contain the genre
            // and returns true.
            while ((pos = s.find(delimiter)) != string::npos) {
                token = s.substr(0, pos);
                if(token == genre) {
                    return true;
                }
                // Removes the genre from the string after evaluation
                // Resultant string will look like:
                // "Genre1, Genre2, Genre3" => "Genre2, Genre3"
                s.erase(0, pos + delimiter.length());
            }
            // 's' holds the last genre of the string so we need
            // to evaluate that as well
            if(s == genre) {
                return true;
            }
            return false;
        }

        // Prints all the songs, along with its artist and genres
        void printAllSongs() {
            // An iterator of the multimap will help iterate through it.
            multimap<string, string> :: iterator itr;
            // ID used purely for decorative indexing and serves no actual functional purpose.
            int id = 1;
            // .begin() holds the first value of the map and .end() marks the last element of the multimap
            // While iterating through the multimap, itr->first holds the value of the KEY and
            // itr->second holds the VALUE of the key.
            for(itr = artist_song_pair_map.begin(); itr != artist_song_pair_map.end(); itr++) {
                // Genres are stored in a seperate multimap whose key is in the form of SONG_NAME:ARTIST_NAME
                // By passing this genre_pair_key into the find() function of the genre_pair map, the function returns
                // the value of that key which holds the genre of the current song which we are printing.
                string genre_pair_key = itr->second + ":" + itr->first;
                cout << id << ". " << itr->second << " by " << itr->first << " [" << song_genre_pair_map.find(genre_pair_key)->second << "]" << endl;
                id++;
            }
            cout << endl;
        }
        
        // Searches songs by a specific genre
        void findSongByGenre(string genre) {
            // ID for printing purposes.
            int id = 1;
            bool songHasGenre, found = false;
            vector<string> song_artist_pair;
            multimap<string, string> :: iterator itr;
            // Looping through the song_genre_pair multimap and checking each song if at least
            // one of it's genres matches the genre passed as parameter.
            // itr->second is the list of genres, itr->first is the key = SONG:ARTIST
            for(itr = song_genre_pair_map.begin(); itr != song_genre_pair_map.end(); itr++) {
                // Checks if the particular song contains the genre specified.
                songHasGenre = doesSongHaveGenre(itr->second, ", ", genre);
                if(songHasGenre) {
                    found = true;
                    // Decrypts the key and returns a vector where
                    // [0] => song, [1] => artist
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

        // Finds song based on a particular name
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

        // Finds song by a particular artist
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

        // The below methods allow data sharing between classes.
        multimap<string, string> getSongGenrePair() {
            return song_genre_pair_map;
        }
        multimap<string, string> getArtistSongPair() {
            return artist_song_pair_map;
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
        
        // Recommends songs to the user based on the genre mix of the playlist.
        void recommendSongs(string playlist_name, string playlist_songs, string playlist_artists, string playlist_genres) {
            vector<string> songs_list, artists_list, genre_list;
            // Converts the string of data into a vector for easier evaluation.
            songs_list = expandString(playlist_songs);
            artists_list = expandString(playlist_artists);
            genre_list = expandString(playlist_genres);

            multimap<string, string>::iterator itr;
            vector<string> song_artist_vector, genre_vector;

            int id = 1;
            for(itr = song_genre_pair_map.begin(); itr != song_genre_pair_map.end(); itr++) {
                bool recommend = false, exists = false;

                song_artist_vector = MusicList::decryptGenreKey(itr->first);
                genre_vector = expandString(itr->second);

                for(int i = 0; i < genre_vector.size(); i++) {
                    if(count(genre_list.begin(), genre_list.end(), genre_vector[i])) {
                        recommend = true;
                    }
                }

                if(recommend) {
                    for(int i = 0; i < songs_list.size(); i++) {
                        if(songs_list[i] == song_artist_vector[0] && artists_list[i] == song_artist_vector[1]) {
                            exists = true;
                            break;
                        }
                    }

                    if(!exists) {
                        cout << id << ". " << song_artist_vector[0] << " by " << song_artist_vector[1] << endl;
                        id++;
                    }

                }
            }
        }

        void removeSongFromPlaylist(string playlist_name, string playlist_songs, string playlist_artists, string playlist_genres) {
            string song_name_delete, artist_name_delete;
            cout << "Enter the name of the song to delete: ";
            getline(cin, song_name_delete);
            cout << "Enter the artist of the song " << song_name_delete << ": ";
            getline(cin, artist_name_delete);

            string genre_name_delete = getSongDetails(song_name_delete, artist_name_delete);
            if(genre_name_delete == "error") {
                return;
            }

            vector<string> playlist_songs_list = expandString(playlist_songs);
            vector<string> playlist_artists_list = expandString(playlist_artists);
            vector<string> playlist_genres_list = expandString(playlist_genres);

            for(int i = 0; i < playlist_songs_list.size(); i++) {
                if(playlist_songs_list[i] == song_name_delete && playlist_artists_list[i] == artist_name_delete) {
                    playlist_songs_list.erase(playlist_songs_list.begin() + i);
                    playlist_artists_list.erase(playlist_artists_list.begin() + i);
                }
            }
            string genres, songs, artists;
            for(int i = 0; i < playlist_songs_list.size(); i++) {
                if(songs.length() <= 0) {
                    songs = playlist_songs_list[i];
                } else {
                    songs = songs + ", " + playlist_songs_list[i];
                }
                if(artists.length() <= 0) {
                    artists = playlist_artists_list[i];
                } else {
                    artists = artists + ", " + playlist_artists_list[i];
                }
                if(genres.length() <= 0) {
                    genres = getSongDetails(playlist_songs_list[i], playlist_artists_list[i]);
                } else {
                    genres = genres + ", " + getSongDetails(playlist_songs_list[i], playlist_artists_list[i]);
                }
            }
            genres = removeDuplicateGenres(genres);
            SQLiteDatabase::updatePlaylist(playlist_name, songs, artists, genres, playlist_name);
        }

        void addSongToPlaylist(string playlist_name, string playlist_songs, string playlist_artists, string playlist_genres) {
            string song_name, artist_name;

            while(true) {
                cout << "Enter name of the song to add [Type !stop to end]: ";
                getline(cin, song_name);

                if(song_name == "!stop") {
                    break;
                }

                cout << "Enter the artist of the song " << song_name << " [Type !stop to end]: ";
                getline(cin, artist_name);

                if(artist_name == "!stop") {
                    break;
                }

                string response = getSongDetails(song_name, artist_name);
                if(response == "error") {
                    continue;
                }

                if(doesSongExist(playlist_songs, playlist_artists, song_name, artist_name)) {
                    cout << song_name << " by " << artist_name << " already exists in the Playlist!" << endl;
                    continue;
                }

                vector<string> playlist_songs_vector = expandString(playlist_songs);
                vector<string> playlist_artists_vector = expandString(playlist_artists);

                playlist_songs_vector.push_back(song_name);
                playlist_artists_vector.push_back(artist_name);

                playlist_genres = playlist_genres + ", " + response;
                playlist_genres = removeDuplicateGenres(playlist_genres);

                playlist_songs = VectorToString(playlist_songs_vector);
                playlist_artists = VectorToString(playlist_artists_vector);


                SQLiteDatabase::updatePlaylist(playlist_name, playlist_songs, playlist_artists, playlist_genres, playlist_name);
            }


        }

        string VectorToString(vector<string> vect) {
            string response;
            for(int i = 0; i < vect.size(); i++) {
                if(response.length() <= 0) {
                    response = vect[i];
                } else {
                    response = response + ", " + vect[i];
                }
            }
            return response;
        }

        bool doesSongExist(string songs, string artists, string req_song, string req_artist) {
            vector<string> list_songs = expandString(songs);
            vector<string> list_artists = expandString(artists);
            for(int i = 0; i < list_songs.size(); i++) {
                if(req_song == list_songs[i] && req_artist == list_artists[i]) {
                    return true;
                }
            }
            return false;
        }

        vector<string> getPlaylistDetails(string playlist_name) {
            vector<string> response;
            auto song_it = playlist_name_song_map.equal_range(playlist_name);
            auto artist_it = playlist_name_artist_map.equal_range(playlist_name);
            auto genre_it = playlist_name_genre_map.equal_range(playlist_name);

            for(auto itr = song_it.first; itr != song_it.second; itr++) {
                response.push_back(itr->second);
            }
            for(auto itr = artist_it.first; itr != artist_it.second; itr++) {
                response.push_back(itr->second);
            }
            for(auto itr = genre_it.first; itr != genre_it.second; itr++) {
                response.push_back(itr->second);
            }
            return response;
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

        void deletePlaylist(string playlist_name) {
            int response = SQLiteDatabase::deletePlaylist(playlist_name);
            if(response) {

            }
        }

        void createPlaylist() {
            string playlist_name, song_name, artist_name;

            string songs_list, genres_list, artists_list;
            cout << "Enter name of the playlist: ";
            getline(cin, playlist_name);
            while(true) {
                cout << "Enter name of song to add (YOU MUST ADD AT LEAST ONE) [Type '!stop' to stop]: ";
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
                int response = SQLiteDatabase::insertPlaylist(playlist_name, artists_list, songs_list, genres_list);
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

        int openPlaylist(string playlist_name) {
            multimap<string, string> :: iterator itr;
            vector<string> songs_list, artists_list, genres_list;

            auto song_iterator = playlist_name_song_map.equal_range(playlist_name);
            auto artist_iterator = playlist_name_artist_map.equal_range(playlist_name);
            auto genre_iterator = playlist_name_genre_map.equal_range(playlist_name);

            if(song_iterator.first == song_iterator.second) {
                cout << "This playlist does not exist" << endl;
                return 0;
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
            return 1;
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

        void editPlaylistMenu(string playlist_name, Playlist playlist) {
            int foundPlaylist = playlist.openPlaylist(playlist_name);
            if(!foundPlaylist) {
                return;
            } else {
                string new_playlist_name;
                vector<string> playlist_details = playlist.getPlaylistDetails(playlist_name);

                /*
                    playlist_details[] mapping details:
                    [0] = songs
                    [1] = artists
                    [2] = genres
                */

                int choice;
                cout << "\n\nWhat would you like to do?\n1. Add a Song\n2. Remove a Song\n3. Edit Playlist Name\n4. Recommend Songs\n5. Delete Playlist" << endl;
                cout << "Choice: ";
                cin >> choice;
                cin.ignore();

                switch(choice) {
                    case 1:
                        playlist.addSongToPlaylist(playlist_name, playlist_details[0], playlist_details[1], playlist_details[2]);
                        break;
                    case 2:
                        playlist.removeSongFromPlaylist(playlist_name, playlist_details[0], playlist_details[1], playlist_details[2]);
                        break;
                    case 3:
                        cout << "Enter a new name for the playlist " << playlist_name << ": ";
                        getline(cin, new_playlist_name);
                        SQLiteDatabase::updatePlaylist(playlist_name, playlist_details[0], playlist_details[1], playlist_details[2], new_playlist_name);
                        break;
                    case 4:
                        spacify();
                        cout << "Based on your song preferences in the playlist " << playlist_name << ". We recommend you the following songs: " << endl;
                        playlist.recommendSongs(playlist_name, playlist_details[0], playlist_details[1], playlist_details[2]);
                        break;
                    case 5:
                        SQLiteDatabase::deletePlaylist(playlist_name);
                        break;
                    default:
                        cout << "Invalid choice." << endl;
                }
            }
        }

        void playlistMenu(Playlist playlist) {
            string playlist_name;
            int choice;
            cout << "Playlist Options: " << endl;
            cout << "1. Show all Playlists\n2. Open a Playlist\n3. Create a Playlist\n4. Edit a Playlist" << endl;
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
                case 4:
                    cout << "Enter the name of the playlist: ";
                    getline(cin, playlist_name);
                    spacify();
                    editPlaylistMenu(playlist_name, playlist);
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
                        myDB.fetchPlaylistData();
                        playlist = Playlist(myDB);
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