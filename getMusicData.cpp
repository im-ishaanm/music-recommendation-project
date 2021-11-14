#include<iostream>
#include<string>
#include<fstream>
#include<map>
#include <algorithm> 
#include <cctype>
#include <locale>
#include<iterator>

using namespace std;


class ReadFileData {
    private:
        multimap<string, string> data_map;
    public:

        string removeWhiteSpace(string s) {
            s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !isspace(ch);
            }).base(), s.end());
            return s;
        }

        void printMap(multimap<string, string> mymap, string seperator) {
            multimap <string, string> :: iterator itr;
            for(itr = mymap.begin(); itr != mymap.end(); itr++){
                cout << itr->second << seperator << itr->first << endl;
            }
            cout << endl;
        }

        multimap<string, string> getMapFromFile(string file_name) {
            fstream file;
            string word;
            
            bool divider = false;
            string key_data = "", value_data = "";

            file.open(file_name.c_str());
            while(file >> word) {
                if(word == ":") {
                    divider = !divider;
                    continue;
                }
                if(word == ";") {
                    value_data = removeWhiteSpace(value_data);
                    key_data = removeWhiteSpace(key_data);
                    data_map.insert(pair <string,string> (value_data, key_data));
                    divider = !divider;
                    key_data = "";
                    value_data = "";
                    continue;
                }
                if(!divider) {
                    value_data = value_data + word + " ";
                } else {
                    key_data = key_data + word + " ";
                }
            }
            file.close();
            return data_map;
        }
};


class MusicList {
    private:
        multimap<string, string> music_map;
        ReadFileData musicData;
    public:

        void getMusic(string file_name) {
            music_map = musicData.getMapFromFile(file_name);
        }

        void printMusicPairs() {
            if(music_map.size() > 0) {
                musicData.printMap(music_map, " by ");
            } else {
                cout << "Something went wrong... We can't find any songs X_X" << endl;
            }
        }

        void printSongsByArtist(string artist) {
            multimap<string, string> songs_by_artist_map;
            songs_by_artist_map = getAllSongsByArtist(artist);

            if(songs_by_artist_map.size() > 0) {
                musicData.printMap(songs_by_artist_map, " : ");
            } else {
                cout << "No songs found from " << artist << " :(" << endl;
            }
        }

        multimap<string, string> getAllSongsByArtist(string artist) {
            multimap<string, string> songs_by_artist_map;
            multimap<string, string> :: iterator itr;
            for(itr = music_map.begin(); itr != music_map.end(); itr++) {
                if(itr->first == artist) {
                    songs_by_artist_map.insert(pair <string, string>(itr->first, itr->second));
                }
            }
            return songs_by_artist_map;
        }
};


int main() {
    MusicList mylist;
    mylist.getMusic("songs.txt");

    cout << "ALL SONGS : " << endl;
    mylist.printMusicPairs();

    string artist = "Owl City";
    cout << "SONGS BY " << artist << endl;
    mylist.printSongsByArtist(artist);
    

    return 0;
}