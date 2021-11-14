#include<iostream>
#include<string>
#include<fstream>
#include<map>
#include <algorithm> 
#include <cctype>
#include <locale>

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

        multimap<string, string> getMapFromFile(string file_name) {
            fstream file;
            string word;
            
            bool divider = false;
            string song_name = "", artist_name = "";

            file.open(file_name.c_str());
            while(file >> word) {
                if(word == ":") {
                    divider = !divider;
                    continue;
                }
                if(word == ";") {
                    artist_name = removeWhiteSpace(artist_name);
                    song_name = removeWhiteSpace(song_name);
                    data_map.insert(pair <string,string> (artist_name, song_name));
                    divider = !divider;
                    song_name = "";
                    artist_name = "";
                    continue;
                }
                if(!divider) {
                    artist_name = artist_name + word + " ";
                } else {
                    song_name = song_name + word + " ";
                }
            }
            file.close();
            return data_map;
        }
};


class MusicList {
    private:
        multimap<string, string> music_map;
    public:
        void getMusic(string file_name) {
            ReadFileData musicData;
            music_map = musicData.getMapFromFile(file_name);
        }

        void printMusicPairs() {
            for(auto x : music_map) {
                cout << x.second << " by " << x.first << endl;
            }
        }
};


int main() {
    MusicList mylist;
    mylist.getMusic("songs.txt");
    mylist.printMusicPairs();

    return 0;
}