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

	float fPoissonRadiusScaleBase = 1.0f;

	float fSoftnessCurve = 4.0f;
	float fSoftnessMax = 20.0f;
	float fSoftnessInterior = 8.0f;

	float softness = 1.0f;

	float ambient = 1.0f;
	float sunlight = 1.0f;
	float sunlightAngle = 1.0f;

	void Load();
	void Save();

	void Update();

	void Menu();

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
		Load();
	};

	SoftShadows(const SoftShadows&) = delete;
	SoftShadows(SoftShadows&&) = delete;

	~SoftShadows() = default;

	SoftShadows& operator=(const SoftShadows&) = delete;
	SoftShadows& operator=(SoftShadows&&) = delete;
};
