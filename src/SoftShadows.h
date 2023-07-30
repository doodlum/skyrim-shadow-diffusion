#pragma once

#include <shared_mutex>

class SoftShadows
{
public:
	static SoftShadows* GetSingleton()
	{
		static SoftShadows handler;
		return &handler;
	}

	static void InstallHooks()
	{
		Hooks::Install();
	}

	std::shared_mutex       fileLock;

	// GameSettings

	float& GetGameSettingFloat(std::string a_name, std::string a_section);
	void   SetGameSettingFloat(std::string a_name, std::string a_section, float a_value);

	// Settings

	bool  bEnabled = true;
	float fPoissonRadiusScaleBase = 4.0f;
	float fFirstSliceDistanceBase = 2000.0f;
	float fFirstSliceDistanceScale = 0.5f;

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

	void LoadINI();
	void SaveINI();

	void Update();

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
	SoftShadows()
	{
		LoadINI();
	};

	SoftShadows(const SoftShadows&) = delete;
	SoftShadows(SoftShadows&&) = delete;

	~SoftShadows() = default;

	SoftShadows& operator=(const SoftShadows&) = delete;
	SoftShadows& operator=(SoftShadows&&) = delete;
};
