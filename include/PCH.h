#pragma once

#pragma warning(push)
#if defined(FALLOUT4)
#	include "F4SE/F4SE.h"
#	include "RE/Fallout.h"
#	define SKSE F4SE
#	define SKSEAPI F4SEAPI
#	define SKSEPlugin_Load F4SEPlugin_Load
#	define SKSEPlugin_Query F4SEPlugin_Query
#else
#	define SKSE_SUPPORT_XBYAK
#	include "RE/Skyrim.h"
#	include "SKSE/SKSE.h"
#	include <xbyak/xbyak.h>
#endif

#ifdef NDEBUG
#	include <spdlog/sinks/basic_file_sink.h>
#else
#	include <spdlog/sinks/msvc_sink.h>
#endif

#pragma warning(pop)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace std::literals;

namespace stl
{
	using namespace SKSE::stl;

	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		SKSE::AllocTrampoline(14);
		auto& trampoline = SKSE::GetTrampoline();
		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}

	template <class F, std::size_t idx, class T>
	void write_vfunc()
	{
		REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[0] };
		T::func = vtbl.write_vfunc(idx, T::thunk);
	}

	template <std::size_t idx, class T>
	void write_vfunc(REL::VariantID id)
	{
		REL::Relocation<std::uintptr_t> vtbl{ id };
		T::func = vtbl.write_vfunc(idx, T::thunk);
	}
}

namespace logger = SKSE::log;
namespace WinAPI = SKSE::WinAPI;

namespace util
{
	using SKSE::stl::report_and_fail;
}

#include "Plugin.h"

#include <CLIBUtil/simpleINI.hpp>

#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include <imgui.h>
#include <reshade/reshade.hpp>
