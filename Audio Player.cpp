#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <conio.h>    
#include <thread>
#include <chrono>
#include <windows.h>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

string filename;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void resetCursor() {
    COORD pos = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

string formatTime(sf::Time t) {
    int totalSec = static_cast<int>(t.asSeconds());
    int min = totalSec / 60;
    int sec = totalSec % 60;

    char buffer[10];
    sprintf_s(buffer, "%02d:%02d", min, sec);
    return string(buffer);
}

void showUI(sf::Music& music, const string& status, float volume, bool loop) {
    resetCursor();
    clearScreen();

    string color, icon;
    if (status == "Playing") { color = "\033[92m"; icon = ">> Playing"; }
    else if (status == "Paused") { color = "\033[93m"; icon = " || Paused"; }
    else { color = "\033[91m"; icon = "[] Stopped"; }

    int volBlocks = round(volume / 10);
    string bar = "[";
    for (int i = 0; i < 10; i++)
        bar += (i < volBlocks) ? "#" : "-";
    bar += "]";

    sf::Time current = music.getPlayingOffset();
    sf::Time total = music.getDuration();

    cout << "\033[96m";
    cout << "===================== CONSOLE AUDIO PLAYER =====================\n";
    cout << "\033[0m";

    cout << " Status : " << color << icon << "\033[0m" << "\n";
    cout << " Time   : " << formatTime(current) << " / " << formatTime(total) << "\n";
    cout << " Volume : " << volume << "% " << bar << "\n";
    cout << " Loop   : " << (loop ? "ON" : "OFF") << "\n";
    cout << " File   : " << filename << "\n";

    cout << "\033[96m";
    cout << "================================================================\n";
    cout << "\033[0m";

    cout << " Space : Play\n";
    cout << " S     : Stop\n";
    cout << " +     : Volume +\n";
    cout << " -     : Volume -\n";
    cout << " L     : Loop\n";
    cout << " M     : Mute / Unmute\n";
    cout << " Q     : Exit\n";

    cout << "\033[96m";
    cout << "================================================================\n";
    cout << "\033[0m";
    cout << " Choose: ";
}

int main() {
    sf::Music music;
    vector<string> audioFiles;
    float lastVolume = 50;
    bool muted = false;
    string status = "Stopped";

    string folder;
    cout << "Enter folder path of audio files: ";
    getline(cin, folder);

    for (const auto& entry : directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            string name = entry.path().string();
            if (name.ends_with(".wav") || name.ends_with(".mp3") || name.ends_with(".ogg") || name.ends_with(".flac"))
                audioFiles.push_back(name);
        }
    }

    if (audioFiles.empty()) {
        cout << "No audio files found!" << endl;
        return 0;
    }

    cout << "\nAvailable audio files:\n";
    for (int i = 0; i < audioFiles.size(); i++)
        cout << i + 1 << ". " << audioFiles[i] << "\n";

    int choice;
    cout << "\nEnter number to play: ";
    cin >> choice;

    if (choice < 1 || choice > audioFiles.size()) {
        cout << "Invalid choice!" << endl;
        return 0;
    }

    filename = audioFiles[choice - 1];

    if (!music.openFromFile(filename)) {
        cout << "Error loading file!" << endl;
        return 0;
    }

    music.setVolume(50);
    music.play();
    status = "Playing";

    bool running = true;

    while (running) {
        showUI(music, status, music.getVolume(), music.getLoop());
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (_kbhit()) {
            char key = _getch();

            switch (key) {
            case ' ':
                if (music.getStatus() == sf::Music::Playing) {
                    music.pause();
                    status = "Paused";
                }
                else {
                    music.play();
                    status = "Playing";
                }
                break;
            case 'S':
            case 's':
                music.stop(); status = "Stopped"; break;
            case '+': music.setVolume(min(100.f, music.getVolume() + 10)); break;
            case '-': music.setVolume(max(0.f, music.getVolume() - 10)); break;
            case 'L':
            case 'l':
                music.setLoop(!music.getLoop()); break;
            case 'M':
            case 'm':
                if (!muted) { lastVolume = music.getVolume(); music.setVolume(0); muted = true; }
                else { music.setVolume(lastVolume); muted = false; }
                break;
            case 'Q':
            case 'q':
                running = false; break;
            }
        }
    }

    return 0;
}