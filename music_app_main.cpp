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
        }

        static void insertData(string artist, string song, string genres) {
            sqlite3 *DB;
            char *error;
            
            string query = "INSERT INTO SONGS(SONG_NAME, ARTIST_NAME, GENRES) VALUES('" + song + "', '" + artist +"', '" + genres +"');";
            
            int response = sqlite3_open("./db/projectDB.db", &DB);
            response = sqlite3_exec(DB, query.c_str(), NULL, 0, &error);

            if(response != SQLITE_OK) {
                cout << "Error inserting data" << endl;
                sqlite3_free(error);
            } else {
                cout << "Data inserted successfully!" << endl;
            }
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
                const char * artist_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                const char * song_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                string artist(artist_data);
                string song(song_data);
                artist_song_pair_map.insert(pair <string, string>(artist, song));
            }
            if(rc != SQLITE_DONE) {
                cout << "Something blew up." << endl;
            }
            sqlite3_finalize(stmt);
        }
        
        multimap<string, string> getArtistSongPair() {
            return artist_song_pair_map;
        }
};

class MusicList {
    private:
        multimap<string, string> artist_song_pair_map;
    public:
        MusicList(SQLiteDatabase database) {
            artist_song_pair_map = database.getArtistSongPair();
        }
        void printArtistSongPair() {
            multimap<string, string> :: iterator itr;
            for(itr = artist_song_pair_map.begin(); itr != artist_song_pair_map.end(); itr++) {
                cout << itr->first << " : " << itr->second << endl;
            }
            cout << endl;
        }
};

int main() {
    // SQLiteDatabase::createDB();
    // SQLiteDatabase::createTable();
    // SQLiteDatabase::insertData("Owl City", "Fireflies", "Electronic, Dance");
    // SQLiteDatabase::insertData("Timeflies", "Alkaline", "Pop");
    SQLiteDatabase myDB;
    myDB.fetchAllData();
    MusicList music_list = MusicList(myDB);
    music_list.printArtistSongPair();
    return 0;
}