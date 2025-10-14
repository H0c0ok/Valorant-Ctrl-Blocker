#include <iostream>
#include <windows.h>
#include <chrono>
#include <thread>

// Глобальная переменная: время, до которого блокируем Ctrl
std::chrono::steady_clock::time_point blockUntil;
uint32_t KeyBlockDuration = 500;

// Функция для проверки, нужно ли блокировать сейчас
static bool shouldBlockCtrl() {
    auto now = std::chrono::steady_clock::now();
    return now < blockUntil;
}

// Low-level mouse hook
static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_LBUTTONDOWN) {
        // ЛКМ нажата — блокируем Ctrl на 400 мс
        blockUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(KeyBlockDuration);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Low-level keyboard hook
static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    
    if (nCode >= 0) {
        //KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        KBDLLHOOKSTRUCT* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        bool isCtrl = (kb->vkCode == VK_LCONTROL || kb->vkCode == VK_RCONTROL);


        if (isCtrl && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
            std::cout << "[?] Ctrl pressed [?]" << '\n';
            if (shouldBlockCtrl()) {
                // Блокируем: не вызываем CallNextHookEx → событие не доходит до системы
                std::cout << " [?] Skipping this ctrl DOWN [?]" << '\n';
                return 1;
            }
        }
        if (isCtrl && (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)) {
            std::cout << "[?] Ctrl pressed [?]" << '\n';
            if (shouldBlockCtrl()) {
                // Блокируем: не вызываем CallNextHookEx → событие не доходит до системы
                std::cout << " [?] Skipping this ctrl UP [?]" << '\n';
                return 1;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


int main() {
    
    HWND SelfConsole = GetConsoleWindow();
    RECT SizeSave{0};
    GetWindowRect(SelfConsole, &SizeSave);
    MoveWindow(SelfConsole, SizeSave.left, SizeSave.top, 200, 200, TRUE);


    HHOOK hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, GetModuleHandle(NULL), 0);
    HHOOK hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, GetModuleHandle(NULL), 0);

    if (!hMouseHook || !hKeyboardHook) {
        MessageBox(NULL, L"Failed to install", L"Error", MB_ICONERROR);
        std::cerr << "[-] Error code: " << GetLastError() << " [-]" << std::endl;
        return 1;
    }
    else {
        std::cout << "[+] Ready to go [+]" << '\n';
    }

    // Запускаем цикл сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Освобождаем хуки
    UnhookWindowsHookEx(hMouseHook);
    UnhookWindowsHookEx(hKeyboardHook);

    return 0;
}