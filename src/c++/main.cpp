#include "Core/Core.hpp"

bool InitLogger()
{
	auto path = logger::log_directory();
	if (!path)
		return false;

	*path /= fmt::format(FMT_STRING("{}.log"), Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%^%l%$] %v"s);

	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

	return true;
}

#ifdef SKYRIM_SUPPORT_AE

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);
	v.AuthorName("Qudix"sv);
	v.UsesAddressLibrary(true);
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });
	return v;
}();

#else

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	if (!InitLogger())
		return false;

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = stl::version_pack(Plugin::VERSION);

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical("Unsupported runtime version {}"sv, ver.string());
		return false;
	}

	return true;
}

#endif

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
#ifdef SKYRIM_SUPPORT_AE
	if (!InitLogger())
		return false;
#endif

	SKSE::Init(a_skse);
	Core::Init();

	logger::info("{} loaded"sv, Plugin::NAME);

	return true;
}
