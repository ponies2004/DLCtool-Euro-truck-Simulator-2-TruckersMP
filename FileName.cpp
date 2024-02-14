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
        L"Super活动团队助手 By:忘心",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, // 禁止用户最大化和调整窗口大小
        (GetSystemMetrics(SM_CXSCREEN) - 800) / 2, (GetSystemMetrics(SM_CYSCREEN) - 400) / 2, 800, 400, // 在桌面中间打开
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

        // 获取服务器上的公告内容
        HINTERNET hInternetAnnounce = InternetOpen(L"HTTPGET", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        HINTERNET hConnectAnnounce = InternetOpenUrl(hInternetAnnounce, L"https://super.henrylogistics.cn/gonggao.txt", NULL, 0, INTERNET_FLAG_RELOAD, 0);

        std::wstring announcement; // 保存公告内容

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
        HINTERNET hConnect = InternetOpenUrl(hInternet, L"http://super.henrylogistics.cn/version.txt", NULL, 0, INTERNET_FLAG_RELOAD, 0);

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
                MessageBox(hwnd, L"当前已经是最新版本！", L"提示", MB_OK | MB_ICONINFORMATION);

                std::remove("Super活动助手V.0.0.1.exe");
                if (!announcement.empty()) {
                    MessageBox(hwnd, announcement.c_str(), L"公告", MB_OK | MB_ICONINFORMATION);
                }
            }
            else {
                // 版本不同
                MessageBox(hwnd, L"发现新版本，请及时更新！", L"提示", MB_OK | MB_ICONWARNING);
                URLDownloadToFile(NULL, L"http://super.henrylogistics.cn/Project3.exe", L"Super活动助手_new.exe", 0, NULL);

                // 获取服务器返回的新版本号
                std::string versionNumber = "V.0.0.2"; // 替换为从服务器获取的新版本号

                std::wstring newFileName = L"Super活动助手" + std::wstring(versionNumber.begin(), versionNumber.end()) + L".exe";
                std::wstring oldFileName = L"Super活动助手_new.exe";

                TCHAR szFileName[MAX_PATH];
                GetModuleFileName(NULL, szFileName, MAX_PATH);
                // 重命名新版本文件
                if (MoveFileEx(oldFileName.c_str(), newFileName.c_str(), MOVEFILE_REPLACE_EXISTING)) {
                    MessageBox(hwnd, L"更新完成，即将重启应用程序以应用新版本！", L"提示", MB_OK | MB_ICONINFORMATION);
                    // 启动新版本应用程序
                    ShellExecute(NULL, L"open", newFileName.c_str(), NULL, NULL, SW_SHOWNORMAL);

                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                }
                else {
                    MessageBox(hwnd, L"无法重命名新版本文件！", L"错误", MB_OK | MB_ICONERROR);
                }
            }


            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
        }
        else {
            MessageBox(hwnd, L"无法连接到服务器获取版本信息！", L"版本检测", MB_OK | MB_ICONERROR);
        }

        // 获取服务器上的数据内容
        HINTERNET hInternetData = InternetOpen(L"HTTPGET", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        HINTERNET hConnectData = InternetOpenUrl(hInternetData, L"https://super.henrylogistics.cn/data.txt", NULL, 0, INTERNET_FLAG_RELOAD, 0);

        std::wstring dataContent; // 保存数据内容

        if (hConnectData) {
            char bufferData[4096];
            DWORD bytesReadData = 0;
            InternetReadFile(hConnectData, bufferData, sizeof(bufferData), &bytesReadData);
            bufferData[bytesReadData] = '\0'; // 添加字符串结束标志

            // 将多字节字符串转换为宽字符串
            int lenWideData = MultiByteToWideChar(CP_UTF8, 0, bufferData, -1, NULL, 0);
            std::vector<wchar_t> wideBufferData(lenWideData);
            MultiByteToWideChar(CP_UTF8, 0, bufferData, -1, &wideBufferData[0], lenWideData);
            dataContent = std::wstring(wideBufferData.begin(), wideBufferData.end());

            InternetCloseHandle(hConnectData);
            InternetCloseHandle(hInternetData);
        }

        // 获取当前CC存档内容
        std::wstring currentCCSave = L"当前CC存档内容："; // 假设这是你的当前CC存档内容

        // 合并当前CC存档内容和从服务器获取的数据内容
        std::wstring combinedContent = currentCCSave + L"" + dataContent;

        // 创建静态文本控件来显示合并后的内容
        HWND hStaticTextCombined = CreateWindow(L"STATIC", combinedContent.c_str(),
            WS_VISIBLE | WS_CHILD | SS_LEFT | WS_EX_TRANSPARENT,
            10, 150, 780, 60,
            hwnd, NULL, GetModuleHandle(NULL), NULL);

        // 设置静态文本控件字体
        HFONT hFontCombined = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
        SendMessage(hStaticTextCombined, WM_SETFONT, (WPARAM)hFontCombined, TRUE);


        CreateWindow(L"BUTTON", L"获取存档路径",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 10, 180, 30,
            hwnd, (HMENU)ID_BUTTON1, GetModuleHandle(NULL), NULL);

        g_hTextBox1 = CreateWindow(L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            10, 50, 360, 50,
            hwnd, NULL, GetModuleHandle(NULL), NULL);

         CreateWindow(L"BUTTON", L"获取DLC路径",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            410, 10, 180, 30,
            hwnd, (HMENU)ID_BUTTON2, GetModuleHandle(NULL), NULL);

        g_hTextBox2 = CreateWindow(L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            410, 50, 360, 50,
            hwnd, NULL, GetModuleHandle(NULL), NULL);

        // 创建新的按钮
        CreateWindow(L"BUTTON", L"一键替换存档",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            120, 200, 150, 40,
            hwnd, (HMENU)ID_BUTTON3, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"一键禁用DLC",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            320, 200, 150, 40,
            hwnd, (HMENU)ID_BUTTON4, GetModuleHandle(NULL), NULL);

        CreateWindow(L"BUTTON", L"一键启用DLC",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            520, 200, 150, 40,
            hwnd, (HMENU)ID_BUTTON5, GetModuleHandle(NULL), NULL);

        // 创建静态文本控件
        HWND hStaticText = CreateWindow(L"STATIC", L"© 2024 Super活动团队助手 By:忘心",
            WS_VISIBLE | WS_CHILD | SS_LEFT | WS_EX_TRANSPARENT,
            10, 340, 780, 30,
            hwnd, (HMENU)IDC_STATIC_TEXT, GetModuleHandle(NULL), NULL);

        // 设置静态文本控件字体
        HFONT hFont = CreateFont(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
        SendMessage(hStaticText, WM_SETFONT, (WPARAM)hFont, TRUE);

        // 创建静态文本控件
        HWND hStaticText2 = CreateWindow(L"STATIC", L"V.0.0.2",
            WS_VISIBLE | WS_CHILD | SS_LEFT | WS_EX_TRANSPARENT,
            270, 340, 780, 30,
            hwnd, (HMENU)version, GetModuleHandle(NULL), NULL);

        HFONT hFont2 = CreateFont(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
        SendMessage(hStaticText2, WM_SETFONT, (WPARAM)hFont, TRUE);

        SendMessage(GetDlgItem(hwnd, ID_BUTTON1), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON2), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON3), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON4), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessage(GetDlgItem(hwnd, ID_BUTTON5), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);


        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON1), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON1), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON2), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON2), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON3), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON3), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON4), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON4), GWL_EXSTYLE) | WS_EX_STATICEDGE);
        SetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, ID_BUTTON5), GWL_EXSTYLE) | WS_EX_STATICEDGE);

        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_BUTTON1: {
            if (HIWORD(wParam) == BN_CLICKED) {
                std::wstring command = L"reg query \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders\" /v Personal";
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
                    std::wstring personalDir = lines[2].substr(lines[2].find(L"REG_EXPAND_SZ") + 14);
                    personalDir.erase(0, personalDir.find_first_not_of(L' '));
                    personalDir.erase(personalDir.find_last_not_of(L" \t\n\r\f\v") + 1);
                    SetWindowText(g_hTextBox1, (personalDir + L"\\Euro Truck Simulator 2\\profiles").c_str());
                }
            }
            break;
        }
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

        case ID_BUTTON3: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox1, textBuffer, 256);
            std::wstring filePath = textBuffer;

            if (filePath.empty()) {
                MessageBox(hwnd, L"请先获取存档路径！", L"提示", MB_OK);
            }
            else {
                std::wstring url = L"https://super.henrylogistics.cn/upload/profiles.zip";
                std::wstring savePath = filePath + L"\\profiles.zip";

                // 创建一个句柄用于进行网络请求
                HINTERNET hInternet = InternetOpen(L"HTTP", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
                if (hInternet) {
                    // 打开一个连接
                    HINTERNET hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
                    if (hConnect) {
                        // 获取文件大小
                        DWORD fileSize = 0;
                        DWORD fileSizeLen = sizeof(fileSize);
                        HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &fileSize, &fileSizeLen, NULL);

                        // 创建一个文件来保存下载的内容
                        std::ofstream file(savePath, std::ios::binary);
                        if (file.is_open()) {
                            char buffer[1024];
                            DWORD bytesRead = 0;
                            DWORD totalBytesRead = 0;

                            // 创建进度条控件或标签控件
                            HWND hProgressBar = CreateWindow(PROGRESS_CLASS, L"", WS_CHILD | WS_VISIBLE, 10, 300, 300, 20, hwnd, NULL, NULL, NULL);

                            // 设置进度条的范围
                            SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

                            // 开始读取网络响应并将其写入文件
                            while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
                                file.write(buffer, bytesRead);
                                totalBytesRead += bytesRead;

                                // 更新进度条控件的状态
                                double progress = static_cast<double>(totalBytesRead) / static_cast<double>(fileSize);
                                int progressBarPos = static_cast<int>(progress * 100);
                                SendMessage(hProgressBar, PBM_SETPOS, progressBarPos, 0);

                                // 处理消息队列，防止界面无响应
                                MSG msg;
                                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                                    TranslateMessage(&msg);
                                    DispatchMessage(&msg);
                                }
                            }

                            file.close();
                            std::wstring extractCommand = L"tar -xvf \"" + filePath + L"\\profiles.zip\" -C \"" + filePath + L"\"";
                            _wsystem(extractCommand.c_str());

                            std::wstring deleteCommand = L"del \"" + filePath + L"\\profiles.zip\"";
                            _wsystem(deleteCommand.c_str());

                            MessageBox(hwnd, L"存档替换成功！", L"提示", MB_OK);
                        }
                        else {
                            MessageBox(hwnd, L"无法创建文件！", L"错误", MB_OK | MB_ICONERROR);
                        }

                        // 关闭连接
                        InternetCloseHandle(hConnect);
                    }
                    else {
                        MessageBox(hwnd, L"无法打开链接！", L"错误", MB_OK | MB_ICONERROR);
                    }

                    // 关闭网络请求句柄
                    InternetCloseHandle(hInternet);
                }
                else {
                    MessageBox(hwnd, L"无法打开网络！", L"错误", MB_OK | MB_ICONERROR);
                }
            }

            break;
        }


        case ID_BUTTON4: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"请先获取DLC路径！", L"提示", MB_OK);
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
                            MessageBox(hwnd, L"无法移动文件！", L"错误", MB_OK | MB_ICONERROR);
                            break;
                        }
                    }
                }

                MessageBox(hwnd, L"DLC禁用成功！", L"提示", MB_OK);
            }
            break;
        }
        case ID_BUTTON5: {
            wchar_t textBuffer[256];
            GetWindowText(g_hTextBox2, textBuffer, 256);
            std::wstring dlcPath = textBuffer;

            if (dlcPath.empty()) {
                MessageBox(hwnd, L"请先获取DLC路径！", L"提示", MB_OK);
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
                            MessageBox(hwnd, L"无法移动文件！", L"错误", MB_OK | MB_ICONERROR);
                            break;
                        }
                    }
                }

                MessageBox(hwnd, L"DLC启用成功！", L"提示", MB_OK);
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
