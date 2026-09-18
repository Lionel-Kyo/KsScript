#include "pch.h"
#include "framework.h"
#include "Main.h"
#include "MainDlg.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "psapi.lib")


CMainApp theApp;

bool CMainApp::IsRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;

    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (::AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        ::CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        ::FreeSid(adminGroup);
    }

    return isAdmin == TRUE;
}

bool CMainApp::RestartAsAdmin() {
    wchar_t exePath[MAX_PATH] = {};

    DWORD length = ::GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    if (length == 0 || length >= MAX_PATH) return false;

    SHELLEXECUTEINFOW sei = {};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"runas";
    sei.lpFile = exePath;
    sei.nShow = SW_SHOWNORMAL;

    if (!::ShellExecuteExW(&sei)) return false;

    if (sei.hProcess) ::CloseHandle(sei.hProcess);

    return true;
}

HWND CMainApp::FindMainWindow(DWORD processId)
{
    struct EnumData
    {
        DWORD processId;
        HWND  hwnd;
    };

    EnumData data{};
    data.processId = processId;
    data.hwnd = nullptr;

    ::EnumWindows(
        [](HWND hwnd, LPARAM lParam) -> BOOL {
            auto* data = reinterpret_cast<EnumData*>(lParam);
            DWORD windowProcessId = 0;
            ::GetWindowThreadProcessId(hwnd, &windowProcessId);

            if (windowProcessId != data->processId) return TRUE;

            if (!::IsWindowVisible(hwnd)) return TRUE;

            if (::GetWindow(hwnd, GW_OWNER) != nullptr) return TRUE;

            data->hwnd = hwnd;

            return FALSE;
        },
        reinterpret_cast<LPARAM>(&data)
    );

    return data.hwnd;
}

static bool GetProcessExecutablePath(HANDLE process, std::wstring& path) {
    std::wstring buffer;
    buffer.resize(32768);

    DWORD size = static_cast<DWORD>(buffer.size());

    if (!::QueryFullProcessImageNameW(process, 0, buffer.data(), &size)) {
        return false;
    }

    path.assign(buffer.data(), size);

    return true;
}

bool CMainApp::ShowAndFocusWindow() {
    bool result = false;

    DWORD currentProcessId = ::GetCurrentProcessId();

    wchar_t currentExePath[MAX_PATH] = {};

    DWORD currentPathLength = ::GetModuleFileNameW(nullptr, currentExePath, MAX_PATH);

    if (currentPathLength == 0 || currentPathLength >= MAX_PATH) return false;

    std::wstring currentPath(currentExePath, currentPathLength);

    HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) return false;


    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (!::Process32FirstW(snapshot, &entry)) {
        ::CloseHandle(snapshot);
        return false;
    }


    do {
        if (_wcsicmp(entry.szExeFile, ::PathFindFileNameW(currentExePath)) != 0) continue;

        if (entry.th32ProcessID == currentProcessId) continue;

        HANDLE process = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID);
        if (!process) continue;

        std::wstring processPath;
        bool sameExecutable = GetProcessExecutablePath(process, processPath);
        ::CloseHandle(process);

        if (!sameExecutable) continue;

        if (_wcsicmp(processPath.c_str(), currentPath.c_str()) != 0) continue;

        HWND hwnd = FindMainWindow( entry.th32ProcessID);
        if (!hwnd) continue;

        if (::IsIconic(hwnd)) ::ShowWindowAsync(hwnd, SW_NORMAL);

        if (::SetForegroundWindow(hwnd)) result = true;

        break;

    } while (::Process32NextW(snapshot, &entry));


    ::CloseHandle(snapshot);

    return result;
}

BOOL CMainApp::InitInstance() {
    CWinApp::InitInstance();
    ::timeBeginPeriod(1);

#ifndef _DEBUG

    //if (!IsRunAsAdmin()) {
    //    RestartAsAdmin();

    //    ::timeEndPeriod(1);
    //    return FALSE;
    //}

    HANDLE hMutex = ::CreateMutexW(nullptr, FALSE, L"Global\\KS_SCRIPT");

    if (hMutex == nullptr) {
        ::timeEndPeriod(1);
        return FALSE;
    }


    if (::GetLastError() == ERROR_ALREADY_EXISTS) {
        ShowAndFocusWindow();
        ::CloseHandle(hMutex);

        ::timeEndPeriod(1);
        return FALSE;
    }

#endif // !_DEBUG

    CMainDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();

#ifndef _DEBUG
    ::CloseHandle(hMutex);
#endif // !_DEBUG

    ::timeEndPeriod(1);
    return FALSE;
}
