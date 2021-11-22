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

        void deleteData(int row_id) {
            sqlite3 *DB;
            char *error;
            
            string id = to_string(row_id);
            string query = "DELETE FROM SONGS WHERE ID = " + id + ";";
        
            int response = sqlite3_open("./db/projectDB.db", &DB);
            response = sqlite3_exec(DB, query.c_str(), NULL, 0, &error);
            if(response != SQLITE_OK) {
                cout << "Error deleting record." << endl;
                sqlite3_free(error);
            } else {
                cout << "Record deleted successfully!" << endl;
            }
            sqlite3_close(DB);
        }

        int insertData(string artist, string song, string genres) {
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
                sqlite3_close(DB);
                return 1;
            }
            sqlite3_close(DB);
            return 0;
        }

        void deleteDataUI() {
            int id;
            cout << "Enter ROW ID to delete : ";
            cin >> id;
            deleteData(id);
        }

        void insertDataUI() {
            string artist_name, song_name, genres;
            cout << "Enter Artist Name : ";
            getline(cin, artist_name);
            cout << "Enter Song Name : ";
            getline(cin, song_name);
            cout << "Enter Genres (comma seperated) [Eg: Pop, Rock, Jazz] : ";
            getline(cin, genres);

            if(artist_name.empty() || song_name.empty() || genres.empty()) {
                cout << endl << endl << "ERROR : Data cannot be NULL / Empty" << endl << endl;
                return;
            }

            auto iterator = artist_song_pair_map.equal_range(artist_name);
            for(auto itr = iterator.first; itr != iterator.second; itr++) {
                if(itr->second == song_name) {
                    cout << endl << endl << "ERROR: Song by the same Artist already exists" << endl << endl;
                }
            }

            int res = 0;
            //int res = insertData(artist_name, song_name, genres);
            if(res) {
                artist_song_pair_map.insert(pair <string, string>(artist_name, song_name));
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
    // SQLiteDatabase::createDB();
    // SQLiteDatabase::createTable();
    bool loop = true;
    SQLiteDatabase myDB;
    myDB.fetchAllData();
    MusicList music_list = MusicList(myDB);
    while(loop) {
        int choice;
        cout << "Database Access : " << endl;
        cout << "1. Add a record\n2. Delete a record\n3. Print Table\n4. Exit" << endl;
        cout << ">> ";
        cin >> choice;
        cin.ignore();
        switch(choice) {
            case 1: myDB.insertDataUI();
                    break;
            case 2: myDB.deleteDataUI();
                    break;
            case 3: music_list.printAllSongs();
                    break;
            case 4: loop = false;
                    break;
            default: 
                    cout << "Incorrect entry. Try again." << endl;
                    break;
        }
        music_list = MusicList(myDB);
    }
}