#include <windows.h>
#include <string>
#include <vector>
#include <wininet.h>
#include <fstream>
#include <CommCtrl.h>
#include <iostream>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#include <sstream> // 添加字符串流头文件
#include <iomanip> // 添加用于 std::setw 的头文件

bool operator==(const std::wstring& lhs, const std::string& rhs) {
    return lhs == std::wstring(rhs.begin(), rhs.end());
}

bool operator==(const std::string& lhs, const std::wstring& rhs) {
    return std::wstring(lhs.begin(), lhs.end()) == rhs;
}


#define ID_BUTTON1 1
#define ID_BUTTON2 2
#define ID_BUTTON3 3
#define ID_BUTTON4 4
#define ID_BUTTON5 5
#define ID_BUTTON15 15
#define ID_BUTTON8 8
#define ID_BUTTON9 9
#define ID_BUTTON10 10
#define ID_BUTTON11 11
#define ID_BUTTON12 12
#define ID_BUTTON13 13
#define ID_BUTTON14 14
#define IDC_STATIC_TEXT 7
#define version 8

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND g_hTextBox1;
HWND g_hTextBox2;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t* CLASS_NAME = L"SteamUninstallQueryWndClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 设置背景为白色

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Dlctools By:iamwannngg",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, // 禁止用户最大化和调整窗口大小
        (GetSystemMetrics(SM_CXSCREEN) - 400) / 2, (GetSystemMetrics(SM_CYSCREEN) - 300) / 2, 400, 300, // 在桌面中间打开
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

std::vector<std::wstring> SplitString(const std::wstring& str, wchar_t delimiter) {
    std::vector<std::wstring> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::wstring::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {

        // 获取服务器上的announcement内容
        HINTERNET hInternetAnnounce = InternetOpen(L"HTTPGET", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        HINTERNET hConnectAnnounce = InternetOpenUrl(hInternetAnnounce, L"http://henrylogistics.cn/dlc/announcement.txt", NULL, 0, INTERNET_FLAG_RELOAD, 0);

        std::wstring announcement; // 保存announcement内容

        if (hConnectAnnounce) {
            char bufferAnnounce[4096];
            DWORD bytesReadAnnounce = 0;
            InternetReadFile(hConnectAnnounce, bufferAnnounce, sizeof(bufferAnnounce), &bytesReadAnnounce);
            bufferAnnounce[bytesReadAnnounce] = '\0'; // 添加字符串结束标志

            // 将多字节字符串转换为宽字符串
            int lenWide = MultiByteToWideChar(CP_UTF8, 0, bufferAnnounce, -1, NULL, 0);
            std::vector<wchar_t> wideBuffer(lenWide);
            MultiByteToWideChar(CP_UTF8, 0, bufferAnnounce, -1, &wideBuffer[0], lenWide);
            announcement = std::wstring(wideBuffer.begin(), wideBuffer.end());

            InternetCloseHandle(hConnectAnnounce);
            InternetCloseHandle(hInternetAnnounce);
        }

        // 获取服务器上的最新版本号
        HINTERNET hInternet = InternetOpen(L"HTTPGET", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        HINTERNET hConnect = InternetOpenUrl(hInternet, L"http://henrylogistics.cn/dlc/version.txt", NULL, 0, INTERNET_FLAG_RELOAD, 0);

        if (hConnect) {
            char buffer[1024];
            DWORD bytesRead = 0;
            InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead);
            buffer[bytesRead] = '\0'; // 添加字符串结束标志
            std::string serverVersion(buffer);

            // 当前程序的版本号
            std::wstring currentVersion = L"V.0.0.2";

            // 比较版本号
            if (serverVersion == currentVersion) {
                // 版本相同
                MessageBox(hwnd, L"It is already the latest version!", L"hint", MB_OK | MB_ICONINFORMATION);

                std::remove("DlctoolsV.0.0.1.exe");
                if (!announcement.empty()) {
                    MessageBox(hwnd, announcement.c_str(), L"announcement", MB_OK | MB_ICONINFORMATION);
                }
            }
            else {
                // 版本不同
                MessageBox(hwnd, L"If you find a new version, please update it in time!", L"hint", MB_OK | MB_ICONWARNING);
                URLDownloadToFile(NULL, L"http://henrylogistics.cn/dlc/Project3.exe", L"Dlctools_new.exe", 0, NULL);

                std::wstring newFileName = L"Dlctools" + std::wstring(serverVersion.begin(), serverVersion.end()) + L".exe";
                std::wstring oldFileName = L"Dlctools_new.exe";

                TCHAR szFileName[MAX_PATH];
                GetModuleFileName(NULL, szFileName, MAX_PATH);
                // 重命名新版本文件
                if (MoveFileEx(oldFileName.c_str(), newFileName.c_str(), MOVEFILE_REPLACE_EXISTING)) {
                    MessageBox(hwnd, L"The update is complete and the application will be restarted to apply the new version!", L"hint", MB_OK | MB_ICONINFORMATION);
                    // 启动新版本应用程序
                    ShellExecute(NULL, L"open", newFileName.c_str(), NULL, NULL, SW_SHOWNORMAL);

                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                }
                else {
                    MessageBox(hwnd, L"Unable to rename new version file!", L"Error", MB_OK | MB_ICONERROR);
                }
            }


            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
        }
        else {
            MessageBox(hwnd, L"Unable to connect to the server to get version information!", L"Version detection", MB_OK | MB_ICONERROR);
        }

         CreateWindow(L"BUTTON", L"Get DLC path",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 10, 180, 30,
            hwnd, (HMENU)ID_BUTTON2, GetModuleHandle(NULL), NULL);

        g_hTextBox2 = CreateWindow(L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            10, 50, 360, 50,
            hwnd, NULL, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"Disable All DLC",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 110, 100, 40,
            hwnd, (HMENU)ID_BUTTON4, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"Enable All DLC",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            150, 110, 100, 40,
            hwnd, (HMENU)ID_BUTTON5, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"East!",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 160, 50, 20,
            hwnd, (HMENU)ID_BUTTON8, GetModuleHandle(NULL), NULL);

        HFONT hFont8 = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"宋体");

        SendMessage(GetDlgItem(hwnd, ID_BUTTON1), WM_SETFONT, (WPARAM)hFont8, TRUE);

        CreateWindow(L"BUTTON", L"Scan",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            70, 160, 50, 20,
            hwnd, (HMENU)ID_BUTTON9, GetModuleHandle(NULL), NULL);

        HFONT hFont9 = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"宋体");

        SendMessage(GetDlgItem(hwnd, ID_BUTTON1), WM_SETFONT, (WPARAM)hFont9, TRUE);

        CreateWindow(L"BUTTON", L"France",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            130, 160, 50, 20,
            hwnd, (HMENU)ID_BUTTON10, GetModuleHandle(NULL), NULL);

        HFONT hFont10 = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"宋体");

        SendMessage(GetDlgItem(hwnd, ID_BUTTON1), WM_SETFONT, (WPARAM)hFont10, TRUE);

        CreateWindow(L"BUTTON", L"Italia",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            190, 160, 50, 20,
            hwnd, (HMENU)ID_BUTTON11, GetModuleHandle(NULL), NULL);

        HFONT hFont11 = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"宋体");

        SendMessage(GetDlgItem(hwnd, ID_BUTTON1), WM_SETFONT, (WPARAM)hFont11, TRUE);

        CreateWindow(L"BUTTON", L"Beyond",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 190, 50, 20,
            hwnd, (HMENU)ID_BUTTON12, GetModuleHandle(NULL), NULL);

        HFONT hFont12 = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"宋体");

        SendMessage(GetDlgItem(hwnd, ID_BUTTON1), WM_SETFONT, (WPARAM)hFont12, TRUE);

        CreateWindow(L"BUTTON", L"Black",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            70, 190, 50, 20,
            hwnd, (HMENU)ID_BUTTON13, GetModuleHandle(NULL), NULL);

        HFONT hFont13 = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"宋体");

        SendMessage(GetDlgItem(hwnd, ID_BUTTON1), WM_SETFONT, (WPARAM)hFont13, TRUE);

        CreateWindow(L"BUTTON", L"Iberia",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            130, 190, 50, 20,
            hwnd, (HMENU)ID_BUTTON14, GetModuleHandle(NULL), NULL);

        HFONT hFont14 = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"宋体");

        SendMessage(GetDlgItem(hwnd, ID_BUTTON1), WM_SETFONT, (WPARAM)hFont14, TRUE);

        CreateWindow(L"BUTTON", L"Balkans",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            190, 190, 50, 20,
            hwnd, (HMENU)ID_BUTTON15, GetModuleHandle(NULL), NULL);

        HFONT hFont15 = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"宋体");

        SendMessage(GetDlgItem(hwnd, ID_BUTTON1), WM_SETFONT, (WPARAM)hFont15, TRUE);


        // 创建静态文本控件
        HWND hStaticText = CreateWindow(L"STATIC", L"© 2024 Dlctools By:iamwannngg",
            WS_VISIBLE | WS_CHILD | SS_LEFT | WS_EX_TRANSPARENT,
            10, 240, 380, 30,
            hwnd, (HMENU)IDC_STATIC_TEXT, GetModuleHandle(NULL), NULL);

        // 设置静态文本控件字体
        HFONT hFont = CreateFont(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
        SendMessage(hStaticText, WM_SETFONT, (WPARAM)hFont, TRUE);

        // 创建静态文本控件
        HWND hStaticText2 = CreateWindow(L"STATIC", L"V.0.0.2",
            WS_VISIBLE | WS_CHILD | SS_LEFT | WS_EX_TRANSPARENT,
            270, 240, 380, 30,
            hwnd, (HMENU)version, GetModuleHandle(NULL), NULL);

        HFONT hFont2 = CreateFont(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
        SendMessage(hStaticText2, WM_SETFONT, (WPARAM)hFont, TRUE);

        SendMessage(GetDlgItem(hwnd, ID_BUTTON1), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON2), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON3), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON4), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON5), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON11), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON15), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON8), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON9), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON10), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON12), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON13), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON14), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);



        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON1), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON1), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON2), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON2), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON3), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON3), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON4), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON4), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON11), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON15), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON8), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON9), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON10), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON12), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON13), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON14), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE) | WS_EX_STATICEDGE);

        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_BUTTON2: {
            if (HIWORD(wParam) == BN_CLICKED) {
                std::wstring command = L"reg query \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 227300\" /v InstallLocation";
                std::wstring result = L"";

                wchar_t buffer[128];
                FILE* pipe = _wpopen(command.c_str(), L"r");
                if (pipe) {
                    while (!feof(pipe)) {
                        if (fgetws(buffer, 128, pipe) != nullptr)
                            result += buffer;
                    }
                    _pclose(pipe);
                }

                std::vector<std::wstring> lines = SplitString(result, L'\n');
                if (lines.size() >= 3) {
                    std::wstring installLocation = lines[2].substr(lines[2].find(L"REG_SZ") + 6);
                    installLocation.erase(0, installLocation.find_first_not_of(L' '));
                    installLocation.erase(installLocation.find_last_not_of(L" \t\n\r\f\v") + 1);
                    SetWindowText(g_hTextBox2, installLocation.c_str());

                    // 创建Temp文件夹
                    std::wstring tempDir = installLocation + L"\\Temp";
                    CreateDirectory(tempDir.c_str(), NULL);
                }
            }

            break;
        }


        case ID_BUTTON4: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"Please get the DLC path first!", L"hint", MB_OK);
            }
            else {
                std::vector<std::wstring> filesToMove = {
                    L"dlc_it.scs",
                    L"dlc_north.scs",
                    L"dlc_east.scs",
                    L"dlc_balt.scs",
                    L"dlc_fr.scs",
                    L"dlc_balkan_e.scs",
                    L"dlc_balkan_w.scs",
                    L"dlc_iberia.scs",
                    L"dlc_heavy_cargo.scs",
                    L"dlc_schwarzmuller.scs",
                    L"dlc_krone.scs",
                    L"dlc_feldbinder.scs",
                    L"dlc_wielton.scs",
                    L"dlc_tirsan.scs",
                    L"dlc_volvo_construction.scs"
                };

                std::wstring tempDir = dlcPath + L"\\Temp";
                CreateDirectory(tempDir.c_str(), NULL);

                for (const auto& file : filesToMove) {
                    std::wstring sourceFilePath = dlcPath + L"\\" + file;
                    std::wstring destinationFilePath = tempDir + L"\\" + file;

                    if (!MoveFileEx(sourceFilePath.c_str(), destinationFilePath.c_str(),
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
                        DWORD errorCode = GetLastError();
                        if (errorCode != ERROR_FILE_NOT_FOUND) {
                            MessageBox(hwnd, L"Unable to move file!", L"Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                    }
                }

                MessageBox(hwnd, L"DLC disabled successfully!", L"hint", MB_OK);
            }
            break;
        }
        case ID_BUTTON5: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"Please get the DLC path first!", L"hint", MB_OK);
            }
            else {
                std::vector<std::wstring> filesToMove = {
                    L"dlc_it.scs",
                    L"dlc_north.scs",
                    L"dlc_east.scs",
                    L"dlc_balt.scs",
                    L"dlc_fr.scs",
                    L"dlc_balkan_e.scs",
                    L"dlc_balkan_w.scs",
                    L"dlc_iberia.scs",
                    L"dlc_heavy_cargo.scs",
                    L"dlc_schwarzmuller.scs",
                    L"dlc_krone.scs",
                    L"dlc_feldbinder.scs",
                    L"dlc_wielton.scs",
                    L"dlc_tirsan.scs",
                    L"dlc_volvo_construction.scs"
                };

                std::wstring tempDir = dlcPath + L"\\Temp";

                for (const auto& file : filesToMove) {
                    std::wstring sourceFilePath = tempDir + L"\\" + file;
                    std::wstring destinationFilePath = dlcPath + L"\\" + file;

                    if (!MoveFileEx(sourceFilePath.c_str(), destinationFilePath.c_str(),
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
                        DWORD errorCode = GetLastError();
                        if (errorCode != ERROR_FILE_NOT_FOUND) {
                            MessageBox(hwnd, L"Unable to move file!", L"Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                    }
                }

                MessageBox(hwnd, L"DLC activated successfully!", L"hint", MB_OK);
            }
            break;
        }
        case ID_BUTTON8: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"Please get the DLC path first!", L"hint", MB_OK);
            }
            else {
                std::wstring sourceFilePath = dlcPath + L"\\Temp\\dlc_east.scs";
                std::wstring destinationFilePath = dlcPath + L"\\dlc_east.scs";

                if (MoveFileEx(sourceFilePath.c_str(), destinationFilePath.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
                    MessageBox(hwnd, L"Successfully moved Going East! back!", L"hint", MB_OK);
                }
                else {
                    DWORD errorCode = GetLastError();
                    if (errorCode != ERROR_FILE_NOT_FOUND) {
                        MessageBox(hwnd, L"Unable to move file!", L"Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
            break;
        }
        case ID_BUTTON9: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"Please get the DLC path first!", L"hint", MB_OK);
            }
            else {
                std::wstring sourceFilePath = dlcPath + L"\\Temp\\dlc_north.scs";
                std::wstring destinationFilePath = dlcPath + L"\\dlc_north.scs";

                if (MoveFileEx(sourceFilePath.c_str(), destinationFilePath.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
                    MessageBox(hwnd, L"Successfully moved Scandinavia back!", L"hint", MB_OK);
                }
                else {
                    DWORD errorCode = GetLastError();
                    if (errorCode != ERROR_FILE_NOT_FOUND) {
                        MessageBox(hwnd, L"Unable to move file!", L"Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
            break;
        }
        case ID_BUTTON10: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"Please get the DLC path first!", L"hint", MB_OK);
            }
            else {
                std::wstring sourceFilePath = dlcPath + L"\\Temp\\dlc_fr.scs";
                std::wstring destinationFilePath = dlcPath + L"\\dlc_fr.scs";

                if (MoveFileEx(sourceFilePath.c_str(), destinationFilePath.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
                    MessageBox(hwnd, L"Successfully moved Vive la France ! back!", L"hint", MB_OK);
                }
                else {
                    DWORD errorCode = GetLastError();
                    if (errorCode != ERROR_FILE_NOT_FOUND) {
                        MessageBox(hwnd, L"Unable to move file!", L"Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
            break;
        }
        case ID_BUTTON11: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"Please get the DLC path first!", L"hint", MB_OK);
            }
            else {
                std::wstring sourceFilePath = dlcPath + L"\\Temp\\dlc_it.scs";
                std::wstring destinationFilePath = dlcPath + L"\\dlc_it.scs";

                if (MoveFileEx(sourceFilePath.c_str(), destinationFilePath.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
                    MessageBox(hwnd, L"Successfully moved Italia back!", L"hint", MB_OK);
                }
                else {
                    DWORD errorCode = GetLastError();
                    if (errorCode != ERROR_FILE_NOT_FOUND) {
                        MessageBox(hwnd, L"Unable to move file!", L"Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
            break;
        }
        case ID_BUTTON12: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"Please get the DLC path first!", L"hint", MB_OK);
            }
            else {
                std::wstring sourceFilePath = dlcPath + L"\\Temp\\dlc_balt.scs";
                std::wstring destinationFilePath = dlcPath + L"\\dlc_balt.scs";

                if (MoveFileEx(sourceFilePath.c_str(), destinationFilePath.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
                    MessageBox(hwnd, L"Successfully moved Beyond the Baltic Sea back!", L"hint", MB_OK);
                }
                else {
                    DWORD errorCode = GetLastError();
                    if (errorCode != ERROR_FILE_NOT_FOUND) {
                        MessageBox(hwnd, L"Unable to move file!", L"Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
            break;
        }
        case ID_BUTTON13: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"Please get the DLC path first!", L"hint", MB_OK);
            }
            else {
                std::wstring sourceFilePath = dlcPath + L"\\Temp\\dlc_balkan_e.scs";
                std::wstring destinationFilePath = dlcPath + L"\\dlc_balkan_e.scs";

                if (MoveFileEx(sourceFilePath.c_str(), destinationFilePath.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
                    MessageBox(hwnd, L"Successfully moved Road to the Black Sea back!", L"hint", MB_OK);
                }
                else {
                    DWORD errorCode = GetLastError();
                    if (errorCode != ERROR_FILE_NOT_FOUND) {
                        MessageBox(hwnd, L"Unable to move file!", L"Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
            break;
        }
        case ID_BUTTON14: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"Please get the DLC path first!", L"hint", MB_OK);
            }
            else {
                std::wstring sourceFilePath = dlcPath + L"\\Temp\\dlc_iberia.scs";
                std::wstring destinationFilePath = dlcPath + L"\\dlc_iberia.scs";

                if (MoveFileEx(sourceFilePath.c_str(), destinationFilePath.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
                    MessageBox(hwnd, L"Successfully moved Iberia back!", L"hint", MB_OK);
                }
                else {
                    DWORD errorCode = GetLastError();
                    if (errorCode != ERROR_FILE_NOT_FOUND) {
                        MessageBox(hwnd, L"Unable to move file!", L"Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
            break;
        }
        case ID_BUTTON15: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"Please get the DLC path first!", L"hint", MB_OK);
            }
            else {
                std::wstring sourceFilePath = dlcPath + L"\\Temp\\dlc_balkan_w.scs";
                std::wstring destinationFilePath = dlcPath + L"\\dlc_balkan_w.scs";

                if (MoveFileEx(sourceFilePath.c_str(), destinationFilePath.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
                    MessageBox(hwnd, L"Successfully moved West Balkanss back!", L"hint", MB_OK);
                }
                else {
                    DWORD errorCode = GetLastError();
                    if (errorCode != ERROR_FILE_NOT_FOUND) {
                        MessageBox(hwnd, L"Unable to move file!", L"Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
            break;
        }

        }
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
