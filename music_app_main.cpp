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
        static void createDB() {
            sqlite3* DB;
            int response = 0;
            response = sqlite3_open("./db/projectDB.db", &DB);
            sqlite3_close(DB);
        }
        
        static void createTable() {
            sqlite3 *DB;
            char *error;

            string query = "CREATE TABLE IF NOT EXISTS SONGS("
                "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                "SONG_NAME TEXT NOT NULL,"
                "ARTIST_NAME TEXT NOT NULL,"
                "GENRES TEXT NOT NULL);";
    
            try {
                int response = 0;
                response = sqlite3_open("./db/projectDB.db", &DB);

                response = sqlite3_exec(DB, query.c_str(), NULL, 0, &error);

                if(response != SQLITE_OK) {
                    cout << "Error creating table." << endl;
                    sqlite3_free(error);
                } else {
                    cout << "Table created successfully!" << endl;
                }
                sqlite3_close(DB);
            } catch (const exception& e) {
                cout << e.what();
            }
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
                artist_song_pair_map.insert(pair <string, string>(artist, song));
                song_genre_pair_map.insert(pair <string, string>(song, genres));
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
        void printAllSongs() {
            multimap<string, string> :: iterator itr;
            for(itr = artist_song_pair_map.begin(); itr != artist_song_pair_map.end(); itr++) {
                cout << itr->second << " by " << itr->first << " [" << song_genre_pair_map.find(itr->second)->second << "]" << endl;
            }
            cout << endl;
        }
};

int main() {
    SQLiteDatabase myDB;
    myDB.fetchAllData();
    MusicList music_list = MusicList(myDB);
    music_list.printAllSongs();
    return 0;
}