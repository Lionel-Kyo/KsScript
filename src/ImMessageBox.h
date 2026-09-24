#pragma once
#ifndef _IM_MESSAGE_BOX_H_
#define _IM_MESSAGE_BOX_H_

#include <string>
#include <functional>
#include "libs/ImGui/imgui.h"
#include "Utils.h"
#include "XorString.h"

enum class ImMessageBoxResult {
    Ok,
    Cancel
};

class ImMessageBox {
private:
    std::u8string m_title;
    std::u8string m_message;
    bool m_ShouldOpen = false;
    std::function<void(ImMessageBoxResult)> m_callback;
public:
    void Show(const std::u8string& message, const std::u8string& title, std::function<void(ImMessageBoxResult)> callback = nullptr);
    void Show(const std::wstring& message, const std::wstring& title, std::function<void(ImMessageBoxResult)> callback = nullptr);
    void Render();
};

#endif // !_IM_MESSAGE_BOX_H_