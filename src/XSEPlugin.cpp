#include "ShadowDiffusion.h"

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		ShadowDiffusion::GetSingleton()->g_ENB = reinterpret_cast<ENB_API::ENBSDKALT1001*>(ENB_API::RequestENBAPI(ENB_API::SDKVersion::V1001));
		if (ShadowDiffusion::GetSingleton()->g_ENB) {
			logger::info("Obtained ENB API");
			ShadowDiffusion::GetSingleton()->g_ENB->SetCallbackFunction([](ENBCallbackType calltype) {
				switch (calltype) {
				case ENBCallbackType::ENBCallback_PostLoad:
					ShadowDiffusion::GetSingleton()->RefreshUI();
					ShadowDiffusion::GetSingleton()->LoadINI();
					break;
				case ENBCallbackType::ENBCallback_PreSave:
					ShadowDiffusion::GetSingleton()->SaveINI();
					break;
				case ENBCallbackType::ENBCallback_PostReset:
					ShadowDiffusion::GetSingleton()->RefreshUI();
					break;
				}
			});
		} else
			logger::info("Unable to acquire ENB API");
		break;
	}
}

void Init()
{
	ShadowDiffusion::InstallHooks();
	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
}
void InitializeLog()
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format("{}.log"sv, Plugin::NAME);
	auto       sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
	const auto level = spdlog::level::trace;
#else
	const auto level = spdlog::level::info;
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);
}

EXTERN_C [[maybe_unused]] __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {};
#endif

	InitializeLog();

	logger::info("Loaded plugin");

	SKSE::Init(a_skse);

	Init();

	return true;
}

EXTERN_C [[maybe_unused]] __declspec(dllexport) constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName("PluginName");
	v.PluginVersion({ 1, 0, 0, 0 });
	v.UsesAddressLibrary(true);
	return v;
}();

EXTERN_C [[maybe_unused]] __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}
