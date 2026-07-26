#ifndef SRL_TUI_HPP
#define SRL_TUI_HPP

#include <string>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

namespace srl {

class TUI {
public:
    static void init() {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            GetConsoleMode(hOut, &dwMode);
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
            SetConsoleMode(hOut, dwMode);
        }
#endif
    }

    static void clear() {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
                DWORD count;
                DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
                COORD homeCoords = { 0, 0 };
                FillConsoleOutputCharacter(hOut, (TCHAR)' ', cellCount, homeCoords, &count);
                FillConsoleOutputAttribute(hOut, csbi.wAttributes, cellCount, homeCoords, &count);
                SetConsoleCursorPosition(hOut, homeCoords);
            }
        }
#endif
        std::cout << "\x1b[2J\x1b[H" << std::flush;
    }

    static void moveCursor(int row, int col) {
        std::cout << "\x1b[" << row << ";" << col << "H" << std::flush;
    }

    static void hideCursor() {
        std::cout << "\x1b[?25l" << std::flush;
    }

    static void showCursor() {
        std::cout << "\x1b[?25h" << std::flush;
    }

    static void setColor(int fgCode) {
        std::cout << "\x1b[" << fgCode << "m" << std::flush;
    }

    static void resetColor() {
        std::cout << "\x1b[0m" << std::flush;
    }

    static bool keyPressed() {
#ifdef _WIN32
        return _kbhit() != 0;
#else
        int bytesWaiting = 0;
        ioctl(0, FIONREAD, &bytesWaiting);
        return bytesWaiting > 0;
#endif
    }

    static int getKey() {
#ifdef _WIN32
        if (_kbhit()) return _getch();
        return -1;
#else
        if (keyPressed()) return getchar();
        return -1;
#endif
    }
};

} // namespace srl

#endif // SRL_TUI_HPP
