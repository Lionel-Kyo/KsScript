#include "pch.h"
#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <mmsystem.h>
#include <shlwapi.h>

#include <thread>
#include <chrono>

#include "libs/ImGui/imgui.h"
#include "libs/ImGui/imgui_impl_dx11.h"
#include "libs/ImGui/imgui_impl_win32.h"

#include "res/Resource.h"
#include "MainWnd.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shlwapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
std::optional<std::wstring> GetCurrentExePath();
bool IsRunAsAdmin();
bool RestartAsAdmin();
HWND FindMainWindow(DWORD processId);
bool GetProcessExecutablePath(HANDLE process, std::wstring& path);
bool ShowAndFocusWindow();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void EnableDarkTitleBar(HWND hwnd) {
    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
}

void ApplyUiTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Geometry & Spacing
    style.WindowRounding = 12.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;

    style.FramePadding = ImVec2(14, 8);
    style.ItemSpacing = ImVec2(10, 10);
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 2.0f;
    style.PopupBorderSize = 1.0f;

    /// Palette
    // Containers
    const ImVec4 color_container = ImVec4(28.0f / 255.0f, 28.0f / 255.0f, 28.0f / 255.0f, 1.0f);
    const ImVec4 color_component_subtle = ImVec4(37.0f / 255.0f, 37.0f / 255.0f, 38.0f / 255.0f, 1.0f);
    const ImVec4 color_component = ImVec4(45.0f / 255.0f, 45.0f / 255.0f, 48.0f / 255.0f, 1.0f);

    // Text
    const ImVec4 color_text = ImVec4(241.0f / 255.0f, 241.0f / 255.0f, 241.0f / 255.0f, 1.0f);
    const ImVec4 color_text_disabled = ImVec4(241.0f / 255.0f, 241.0f / 255.0f, 241.0f / 255.0f, 0.38f);

    // Borders / separators
    const ImVec4 color_border_subtle = ImVec4(63.0f / 255.0f, 63.0f / 255.0f, 70.0f / 255.0f, 1.0f);
    const ImVec4 color_border = ImVec4(85.0f / 255.0f, 85.0f / 255.0f, 85.0f / 255.0f, 1.0f);
    const ImVec4 color_separator = ImVec4(51.0f / 255.0f, 51.0f / 255.0f, 51.0f / 255.0f, 1.0f);

    // Interaction states
    const ImVec4 color_hover = ImVec4(65.0f / 255.0f, 55.0f / 255.0f, 85.0f / 255.0f, 1.0f);
    const ImVec4 color_focus = ImVec4(40.0f / 255.0f, 35.0f / 255.0f, 55.0f / 255.0f, 1.0f);
    const ImVec4 color_primary_focus = ImVec4(190.0f / 255.0f, 160.0f / 255.0f, 255.0f / 255.0f, 1.0f);
    const ImVec4 color_press = ImVec4(100.0f / 255.0f, 80.0f / 255.0f, 140.0f / 255.0f, 1.0f);

    // Checked / selected
    const ImVec4 color_checked = ImVec4(180.0f / 255.0f, 160.0f / 255.0f, 220.0f / 255.0f, 1.0f);
    const ImVec4 color_checked_text = ImVec4(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f);
    const ImVec4 color_checked_focus = ImVec4(200.0f / 255.0f, 185.0f / 255.0f, 235.0f / 255.0f, 1.0f);
    const ImVec4 color_checked_hover = ImVec4(220.0f / 255.0f, 210.0f / 255.0f, 250.0f / 255.0f, 1.0f);
    const ImVec4 color_checked_press = ImVec4(235.0f / 255.0f, 225.0f / 255.0f, 255.0f / 255.0f, 1.0f);
    const ImVec4 transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    /// General
    colors[ImGuiCol_Text] = color_text;
    colors[ImGuiCol_TextDisabled] = color_text_disabled;
    colors[ImGuiCol_TextSelectedBg] = color_focus;

    colors[ImGuiCol_WindowBg] = color_container;
    colors[ImGuiCol_ChildBg] = color_component_subtle;

    colors[ImGuiCol_PopupBg] = color_component;
    colors[ImGuiCol_Border] = color_border;
    colors[ImGuiCol_BorderShadow] = transparent;

    colors[ImGuiCol_MenuBarBg] = color_component;

    // Headers / Selection
    colors[ImGuiCol_Header] = color_focus;
    colors[ImGuiCol_HeaderHovered] = color_hover;
    colors[ImGuiCol_HeaderActive] = color_press;

    // Buttons
    colors[ImGuiCol_Button] = color_component;
    colors[ImGuiCol_ButtonHovered] = color_hover;
    colors[ImGuiCol_ButtonActive] = color_press;

    // Textboxes / Inputs
    colors[ImGuiCol_FrameBg] = color_component;
    colors[ImGuiCol_FrameBgHovered] = color_hover;
    colors[ImGuiCol_FrameBgActive] = color_focus;

    // Tables
    colors[ImGuiCol_TableHeaderBg] = color_component_subtle;
    colors[ImGuiCol_TableBorderStrong] = color_border;
    colors[ImGuiCol_TableBorderLight] = color_separator;

    colors[ImGuiCol_TableRowBg] = transparent;
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.02f);

    // Window title bar
    colors[ImGuiCol_TitleBg] = color_component;
    colors[ImGuiCol_TitleBgActive] = color_component;
    colors[ImGuiCol_TitleBgCollapsed] = color_container;

    // Tabs
    colors[ImGuiCol_Tab] = color_component;
    colors[ImGuiCol_TabHovered] = color_hover;
    colors[ImGuiCol_TabActive] = color_focus;
    colors[ImGuiCol_TabUnfocused] = color_component_subtle;
    colors[ImGuiCol_TabUnfocusedActive] = color_component;

    // Sliders / Checkboxes
    colors[ImGuiCol_SliderGrab] = color_checked;
    colors[ImGuiCol_SliderGrabActive] = color_checked_press;

    colors[ImGuiCol_CheckMark] = color_checked;

    // Plots
    colors[ImGuiCol_PlotLines] = color_checked;
    colors[ImGuiCol_PlotHistogram] = color_checked;

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = color_container;
    colors[ImGuiCol_ScrollbarGrab] = color_component;
    colors[ImGuiCol_ScrollbarGrabHovered] = color_hover;
    colors[ImGuiCol_ScrollbarGrabActive] = color_press;

    // Resize grips
    colors[ImGuiCol_ResizeGrip] = color_component;
    colors[ImGuiCol_ResizeGripHovered] = color_hover;
    colors[ImGuiCol_ResizeGripActive] = color_press;

    // Separators
    colors[ImGuiCol_Separator] = color_separator;
    colors[ImGuiCol_SeparatorHovered] = color_hover;
    colors[ImGuiCol_SeparatorActive] = color_press;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
#ifndef _DEBUG
    //if (!IsRunAsAdmin()) {
    //    RestartAsAdmin();
    //    return 0;
    // 
    //}

    HANDLE hMutex = ::CreateMutexW(nullptr, FALSE, L"Global\\KS_SCRIPT");

    if (hMutex == nullptr) {
        return 0;
    }

    if (::GetLastError() == ERROR_ALREADY_EXISTS) {
        ShowAndFocusWindow();
        ::CloseHandle(hMutex);
        return 0;
    }

#endif // !_DEBUG

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, hInstance, nullptr, nullptr, nullptr, nullptr, L"KsScriptAppClass", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"KsScript", WS_OVERLAPPEDWINDOW, 100, 100, 600, 500, nullptr, nullptr, wc.hInstance, nullptr);
    EnableDarkTitleBar(hwnd);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // disable imgui.ini
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ApplyUiTheme();

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 18.0f, &config, io.Fonts->GetGlyphRangesDefault());
    io.Fonts->Build();

    MainHelper::GetInstance().SetHWnd(hwnd);
    MainHelper::GetInstance().StartKeyboardHook();

    bool done = false;
    while (!done) {
        // Wait for a Windows message
        DWORD result = MsgWaitForMultipleObjectsEx(0, nullptr, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);

        if (result == WAIT_OBJECT_0) {
            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    done = true;
                    break;
                }
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }

        if (done) break;

        //if (::IsIconic(hwnd)) {
        //    std::this_thread::sleep_for(std::chrono::duration<int32_t, std::milli>(50));
        //    continue;
        //}

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        MainWnd::RenderWindow(hwnd);

        ImGui::Render();
        const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    MainHelper::GetInstance().StopKeyboardHook();
    {
        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
        auto& entries = MainHelper::GetInstance().LoadedScripts();
        for (auto& entry : entries) {
            StopScriptExecution(entry.data, true, false);
        }
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

#ifndef _DEBUG
    ::CloseHandle(hMutex);
#endif // !_DEBUG

    return 0;
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2,
        D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (SUCCEEDED(g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)))) {
        if (pBackBuffer != nullptr) {
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
            pBackBuffer->Release();
        }
    }
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

std::optional<std::wstring> GetCurrentExePath() {
    DWORD bufferSize = MAX_PATH;
    std::wstring exePath;

    while (true) {
        exePath.resize(bufferSize);
        DWORD length = ::GetModuleFileNameW(nullptr, &exePath[0], bufferSize);

        if (length == 0) return std::nullopt;

        if (length < bufferSize) {
            exePath.resize(length);
            break;
        }

        bufferSize *= 2;
        if (bufferSize > 32768) return std::nullopt;
    }

    return exePath;
}

bool IsRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;

    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (::AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        ::CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        ::FreeSid(adminGroup);
    }

    return isAdmin == TRUE;
}

bool RestartAsAdmin() {
    auto exePath = GetCurrentExePath();
    if (!exePath.has_value()) return false;

    std::wstring verb = L"runas";
    SHELLEXECUTEINFOW sei = {};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = verb.data();
    sei.lpFile = exePath->data();
    sei.nShow = SW_SHOWNORMAL;

    if (!::ShellExecuteExW(&sei)) return false;

    if (sei.hProcess) ::CloseHandle(sei.hProcess);

    return true;
}

HWND FindMainWindow(DWORD processId)
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

bool GetProcessExecutablePath(HANDLE process, std::wstring& path) {
    std::wstring buffer;
    buffer.resize(32768);

    DWORD size = static_cast<DWORD>(buffer.size());

    if (!::QueryFullProcessImageNameW(process, 0, buffer.data(), &size)) {
        return false;
    }

    path.assign(buffer.data(), size);

    return true;
}

bool ShowAndFocusWindow() {
    bool result = false;
    DWORD currentProcessId = ::GetCurrentProcessId();

    auto currentExePath = GetCurrentExePath();
    if (!currentExePath.has_value()) return false;

    HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) return false;


    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (!::Process32FirstW(snapshot, &entry)) {
        ::CloseHandle(snapshot);
        return false;
    }


    do {
        if (_wcsicmp(entry.szExeFile, ::PathFindFileNameW(currentExePath->c_str())) != 0) continue;

        if (entry.th32ProcessID == currentProcessId) continue;

        HANDLE process = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID);
        if (!process) continue;

        std::wstring processPath;
        bool sameExecutable = GetProcessExecutablePath(process, processPath);
        ::CloseHandle(process);

        if (!sameExecutable) continue;

        if (_wcsicmp(processPath.c_str(), currentExePath->c_str()) != 0) continue;

        HWND hwnd = FindMainWindow(entry.th32ProcessID);
        if (!hwnd) continue;

        if (::IsIconic(hwnd)) ::ShowWindowAsync(hwnd, SW_NORMAL);

        if (::SetForegroundWindow(hwnd)) result = true;

        break;

    } while (::Process32NextW(snapshot, &entry));


    ::CloseHandle(snapshot);

    return result;
}


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}