#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include "curl/curl.h"

//Tray icon code from stackoverflow and various other internet sources, I was unsure on how it worked.

using std::string;

bool isConsoleHidden = false;
bool isRcoEnabled = false;

std::string rootDir("C:\\RClientOptimizer2");
char* buf = nullptr;
size_t sz = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LPCWSTR lpszClass = L"__hidden__";

int traySystem() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASS wc;
    HWND hWnd;
    MSG msg;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = nullptr;
    wc.hCursor = nullptr;
    wc.hIcon = nullptr;
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = lpszClass;
    wc.lpszMenuName = nullptr;
    wc.style = 0;
    RegisterClass(&wc);

    hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
    static NOTIFYICONDATA nid;
    std::ofstream isHiddenFile;
    HWND consoleWindow = GetConsoleWindow();

    switch (iMsg) {
        case WM_CREATE:
            std::memset(&nid, 0, sizeof(nid));
            nid.cbSize = sizeof(nid);
            nid.hWnd = hWnd;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
            nid.uCallbackMessage = WM_APP + 1;
            nid.hIcon = (HICON)LoadImageA(NULL, (rootDir + "\\animegirl.ico").c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_SHARED);
            memcpy_s(nid.szTip, sizeof(nid.szTip), L"RCO <3\nClick to toggle console", sizeof(wchar_t[31]));
            Shell_NotifyIcon(NIM_ADD, &nid);
            return 0;
        case WM_APP + 1:
            switch (lParam) {
            case WM_LBUTTONDBLCLK:
                if (isConsoleHidden) {
                    ShowWindow(consoleWindow, SW_SHOW);
                    isConsoleHidden = false;
                    isHiddenFile.open(rootDir + "\\isHidden.rco");
                    isHiddenFile << "f";
                    isHiddenFile.close();
                } else {
                    ShowWindow(consoleWindow, SW_HIDE);
                    isConsoleHidden = true;
                    isHiddenFile.open(rootDir + "\\isHidden.rco");
                    isHiddenFile << "t";
                    isHiddenFile.close();
                }
                break;
            case WM_LBUTTONDOWN:
                if (isConsoleHidden) {
                    ShowWindow(consoleWindow, SW_SHOW);
                    isConsoleHidden = false;
                    isHiddenFile.open(rootDir + "\\isHidden.rco");
                    isHiddenFile << "f";
                    isHiddenFile.close();
                } else {
                    ShowWindow(consoleWindow, SW_HIDE);
                    isConsoleHidden = true;
                    isHiddenFile.open(rootDir + "\\isHidden.rco");
                    isHiddenFile << "t";
                    isHiddenFile.close();
                }
                break;
            }
            break;
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int main() {
    //Preinit
    if (std::filesystem::exists(rootDir) == false) {
        std::cout << "Could not find proper RCO files, please reinstall RCO | 0x1\n";
        std::cin.get();
        return 1;
    }

    if (std::filesystem::exists(rootDir + "\\animegirl.ico") == false) {
        FILE* file;
        if (fopen_s(&file, (rootDir + "\\animegirl.ico").c_str(), "wb") != 0) {
            std::cout << "Could not open filepoint... | 0x4\n";
            std::cin.get();
            return 4;
        }

        CURL* req = curl_easy_init();
        CURLcode res;
        curl_easy_setopt(req, CURLOPT_URL, "https://raw.githubusercontent.com/Kaiddd/RobloxClientOptimizer2/main/animegirl.ico");
        curl_easy_setopt(req, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(req, CURLOPT_WRITEDATA, file);
        res = curl_easy_perform(req);
        if (res != CURLE_OK) {
            std::cout << "NETWORK ERROR | PLEASE CHECK YOUR INTERNET CONNECTION | 0x4\n";
            std::cin.get();
            return 4;
        }
        curl_easy_cleanup(req);

        fclose(file);
    }

    if (std::filesystem::exists(rootDir + "\\isHidden.rco") == false) {
        std::ofstream isHiddenFile;
        isHiddenFile.open(rootDir + "\\isHidden.rco");
        isHiddenFile << "f";
        isHiddenFile.close();
    }

    if (std::filesystem::exists(rootDir + "\\isEnabled.rco") == false) {
        std::ofstream isEnabledFile;
        isEnabledFile.open(rootDir + "\\isEnabled.rco");
        isEnabledFile << "f";
        isEnabledFile.close();
    }
    //Initialize the tray icon system
    std::thread t1(traySystem);

    //Set roblox versions folder location variable
    string robloxVersionFolder;
    if (!(_dupenv_s(&buf, &sz, "localappdata") == 0 && buf != nullptr)) {
        std::cout << "Error finding LocalAppData folder | 0x2\n";
        std::cin.get();
        return 2;
    }

    robloxVersionFolder = buf + string("\\Roblox\\Versions");
    free(buf);

    if (std::filesystem::exists(robloxVersionFolder) == false) {
        std::cout << "Roblox not found. Please reinstall Roblox | 0x3\n";
        std::cin.get();
        return 3;
    }

    //Set Hidden and Enabled based on saved file
    std::ifstream hiddenFile(rootDir + "\\isHidden.rco");
    hiddenFile.seekg(0, std::ios::end);
    size_t size = hiddenFile.tellg();
    string buffer(size, ' ');
    hiddenFile.seekg(0);
    hiddenFile.read(&buffer[0], size);
    if (buffer == "t") {
        isConsoleHidden = true;
    }
    hiddenFile.close();

    std::ifstream enabledFile(rootDir + "\\isEnabled.rco");
    enabledFile.seekg(0, std::ios::end);
    size = enabledFile.tellg();
    buffer = string(size, ' ');
    enabledFile.seekg(0);
    enabledFile.read(&buffer[0], size);
    if (buffer == "t") {
        isRcoEnabled = true;
    }
    enabledFile.close();

    //Handle Hidden Value
    if (isConsoleHidden) {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    } else {
        ShowWindow(GetConsoleWindow(), SW_SHOW);
    }

    //Input loop
    while (true) {
        system("cls");
        std::cout << "Roblox Client Optimizer 2 coded in C++ by Kaid | fflag list by nul\n\nRCO is currently: ";
        if (isRcoEnabled) {
            SetConsoleTextAttribute(hConsole, 10);
            std::cout << "Enabled";
            SetConsoleTextAttribute(hConsole, 7);
            std::cout << "\nPress enter to ";
            SetConsoleTextAttribute(hConsole, 12);
            std::cout << "Disable";
            SetConsoleTextAttribute(hConsole, 7);
            std::cout << " RCO.\n\n";
        } else {
            SetConsoleTextAttribute(hConsole, 12);
            std::cout << "Disabled";
            SetConsoleTextAttribute(hConsole, 7);
            std::cout << "\nPress enter to ";
            SetConsoleTextAttribute(hConsole, 10);
            std::cout << "Enable";
            SetConsoleTextAttribute(hConsole, 7);
            std::cout << " RCO.\n\n";
        }

        string t; //Throwaway
        std::getline(std::cin, t);

        isRcoEnabled = !isRcoEnabled;
        std::ofstream isEnabledFile;
        isEnabledFile.open(rootDir + "\\isEnabled.rco");
        if (isRcoEnabled) {
            isEnabledFile << "t";
        } else {
            isEnabledFile << "f";
        }
        isEnabledFile.close();
    }
}