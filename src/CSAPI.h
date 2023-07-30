#pragma once

#include <variant>
#include <imgui.h>
#include <Windows.h>

// https://github.com/Pentalimbed/cathub/blob/cathub-ng/src/cathub.h

constexpr REL::Version API_VER = {0, 1, 0, 0};

class CSAPI
{
public:
	virtual REL::Version GetVersion() = 0;
	virtual ImGuiContext* GetContext() = 0;
	virtual void AddMenu(std::string name, std::function<void()> draw_func) = 0;
};

[[nodiscard]] inline std::variant<CSAPI*, std::string> RequestCSAPI()
{
	typedef CSAPI* (*_RequestCSAPIFunc)();

	auto pluginHandle = GetModuleHandle(L"CommunityShaders.dll");
	if (!pluginHandle)
		return "Cannot find Cathub.";

	_RequestCSAPIFunc requestAPIFunc = (_RequestCSAPIFunc)GetProcAddress(pluginHandle, "GetCSInterface");
	if (requestAPIFunc) {
		auto api = requestAPIFunc();
		if (api->GetVersion() == API_VER)
			return api;
		else
			return std::format("Version mismatch! Requested {}. Get {}.", API_VER, api->GetVersion());
	}

	return "Failed to get";
}
