# music-recommendation-project
A simple terminal-based music recommendation application created using C++ for the final project of our C++ course.

## Execution Instructions
All you have to do is navigate to the project folder and run the following
commands via your terminal.

## How to execute the main program
```
g++ .\music_app_main.cpp .\sqlite3.o
.\a.exe
```
## How to execute the DB program
#### This program is used to play around with the database and is used for testing purposes only.
```
g++ .\music_app_admin_db.cpp .\sqlite3.o
.\a.exe
```

## misc folder
The "misc" folder contains an old version of the project which did not incorporate a database. This version of the project stored data in a text file and all operations were done using the built-in C++ file handling techniques.
