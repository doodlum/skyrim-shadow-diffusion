#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "ENB/ENBSeriesAPI.h"
#include <shared_mutex>

class ShadowDiffusion
{
public:
	static ShadowDiffusion* GetSingleton()
	{
		static ShadowDiffusion handler;
		return &handler;
	}

	static void InstallHooks()
	{
		Hooks::Install();
	}

	ENB_API::ENBSDKALT1001* g_ENB = nullptr;
	json                    JSONSettings;
	std::shared_mutex       fileLock;

	// GameSettings

	float& GetGameSettingFloat(std::string a_name, std::string a_section);
	void SetGameSettingFloat(std::string a_name, std::string a_section, float a_value);

	// Settings

	bool  bEnabled = true;
	float fPoissonRadiusScaleBase = 4.0f;
	float fFirstSliceDistanceBase = 2000.0f;
	float fFirstSliceDistanceScalar = 0.5f;

	float fDiffusionBase = 0.1f;
	float fDiffusionCurve = 20.0f;
	float fDiffusionMin = 1.0f;
	float fDiffusionMax = 20.0f;
	float fDiffusionInterior = 1.0f;

	float fDawnMultiplier = 1.0f;
	float fSunriseMultiplier = 1.0f;
	float fDayMultiplier = 1.0f;
	float fSunsetMultiplier = 1.0f;
	float fDuskMultiplier = 1.0f;
	float fNightMultiplier = 1.0f;

	float diffusion = 1.0f;
	float ambient = 1.0f;
	float sunlight = 1.0f;

	struct ENBTimeOfDay
	{
		float dawn;
		float sunrise;
		float day;
		float sunset;
		float dusk;
		float night;
		float interiorDay;
		float interiorNight;
		float interiorFactor;
		float nightFactor;
	};
	ENBTimeOfDay TimeOfDay;

	void LoadINI();
	void SaveINI();

	float GetENBParameterFloat(const char* a_filename, const char* a_category, const char* a_keyname);

	void UpdateENBTimeOfDay();

	void Update();

	// ENB UI

	void RefreshUI();

protected:
	struct Hooks
	{

		struct PlayerCharacter_Update
		{
			static void thunk(RE::PlayerCharacter* a_player, float a_delta)
			{
				func(a_player, a_delta);
				GetSingleton()->Update();
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_vfunc<RE::PlayerCharacter, 0xAD, PlayerCharacter_Update>();
		}
	};

private:
	ShadowDiffusion()
	{
		LoadINI();
	};

	ShadowDiffusion(const ShadowDiffusion&) = delete;
	ShadowDiffusion(ShadowDiffusion&&) = delete;

	~ShadowDiffusion() = default;

	ShadowDiffusion& operator=(const ShadowDiffusion&) = delete;
	ShadowDiffusion& operator=(ShadowDiffusion&&) = delete;
};
