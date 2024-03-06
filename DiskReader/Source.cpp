#include <iostream>
#include <iomanip>
#include <fstream>
#include <windows.h>

using namespace std;

void readDirectory(const wchar_t* directoryPath, int startOffset) {
    WIN32_FIND_DATAW fileData;
    HANDLE hFind = FindFirstFileW((wstring(directoryPath) + L"\\*").c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        int offset = 0;
        bool startedPrinting = false; // Flag to indicate whether printing has started
        do {
            if (offset >= startOffset) {
                if (!startedPrinting) {
                    startedPrinting = true;
                    cout << "Printing from offset " << hex << uppercase << startOffset << "h" << endl;
                }

                cout << setw(8) << setfill('0') << hex << uppercase << offset << "h ";
                cout << dec << setfill(' ');

                unsigned char buffer[16];
                memcpy(buffer, &fileData, sizeof(buffer)); // Copy fileData to buffer

                for (int i = 0; i < 16; ++i) {
                    if (i % 4 == 0) cout << ' '; // Print space every 4 bytes
                    cout << setw(2) << setfill('0') << hex << uppercase << (int)buffer[i] << ' ';
                    cout << dec; // Reset to decimal after printing each byte
                }
                cout << endl;
            }

            offset += 16;
        } while (FindNextFileW(hFind, &fileData) != 0);

        FindClose(hFind);
    }
    else {
        cerr << "Error: Unable to open directory." << endl;
    }
}

void readRDET(const wchar_t* directoryPath) {
    WIN32_FIND_DATAW fileData;
    HANDLE hFind = FindFirstFileW((wstring(directoryPath) + L"\\*").c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        cout << "Root Directory Entry Table (RDET):" << endl;
        do {
            if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                cout << "Directory: ";
            }
            else {
                cout << "File: ";
            }
            wcout << fileData.cFileName << endl;
        } while (FindNextFileW(hFind, &fileData) != 0);

        FindClose(hFind);
    }
    else {
        cerr << "Error: Unable to open directory." << endl;
    }
}
//
//int main() {
//    const wchar_t* directoryPath = L"D:\\";
//    int startOffset = 0x0; // Specify the starting offset here
//    readDirectory(directoryPath, startOffset);
//    readRDET(directoryPath);
//    return 0;
//}
