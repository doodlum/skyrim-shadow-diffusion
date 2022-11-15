#include "SoftShadows.h"

#include <ColorSpace/ColorSpace.h>
#include <SimpleIni.h>

extern ENB_API::ENBSDKALT1002* g_ENB;

float& SoftShadows::GetGameSettingFloat(std::string a_name, std::string a_section)
{
	auto ini = RE::INISettingCollection::GetSingleton();
	return ini->GetSetting(std::format("{}:{}", a_name, a_section))->data.f;
}

void SoftShadows::SetGameSettingFloat(std::string a_name, std::string a_section, float a_value)
{
	auto ini = RE::INISettingCollection::GetSingleton();
	ini->GetSetting(std::format("{}:{}", a_name, a_section))->data.f = a_value;
}

#define GetSettingFloat(a_section, a_setting) a_setting = (float)ini.GetDoubleValue(a_section, #a_setting, 1.0f);
#define SetSettingFloat(a_section, a_setting) ini.SetDoubleValue(a_section, #a_setting, a_setting);

#define GetSettingBool(a_section, a_setting) a_setting = ini.GetBoolValue(a_section, #a_setting, true);
#define SetSettingBool(a_section, a_setting) ini.SetBoolValue(a_section, #a_setting, a_setting);

void SoftShadows::LoadINI()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);
	CSimpleIniA                        ini;
	ini.SetUnicode();
	ini.LoadFile(L"Data\\SKSE\\Plugins\\SoftShadows.ini");

	GetSettingFloat("GameSetting", fPoissonRadiusScaleBase);
	GetSettingFloat("GameSetting", fFirstSliceDistanceBase);
	GetSettingFloat("GameSetting", fFirstSliceDistanceScale);

	GetSettingBool("Diffusion", bEnabled);
	GetSettingFloat("Diffusion", fDiffusionBase);
	GetSettingFloat("Diffusion", fDiffusionCurve);
	GetSettingFloat("Diffusion", fDiffusionMin);
	GetSettingFloat("Diffusion", fDiffusionMax);
	GetSettingFloat("Diffusion", fDiffusionInterior);

	GetSettingFloat("ENBSeries", fDawnMultiplier);
	GetSettingFloat("ENBSeries", fSunriseMultiplier);
	GetSettingFloat("ENBSeries", fDayMultiplier);
	GetSettingFloat("ENBSeries", fSunsetMultiplier);
	GetSettingFloat("ENBSeries", fDuskMultiplier);
	GetSettingFloat("ENBSeries", fNightMultiplier);
}

void SoftShadows::SaveINI()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);
	CSimpleIniA                        ini;
	ini.SetUnicode();

	SetSettingFloat("GameSetting", fPoissonRadiusScaleBase);
	SetSettingFloat("GameSetting", fFirstSliceDistanceBase);
	SetSettingFloat("GameSetting", fFirstSliceDistanceScale);

	SetSettingBool("Diffusion", bEnabled);
	SetSettingFloat("Diffusion", fDiffusionBase);
	SetSettingFloat("Diffusion", fDiffusionCurve);
	SetSettingFloat("Diffusion", fDiffusionMin);
	SetSettingFloat("Diffusion", fDiffusionMax);
	SetSettingFloat("Diffusion", fDiffusionInterior);

	SetSettingFloat("ENBSeries", fDawnMultiplier);
	SetSettingFloat("ENBSeries", fSunriseMultiplier);
	SetSettingFloat("ENBSeries", fDayMultiplier);
	SetSettingFloat("ENBSeries", fSunsetMultiplier);
	SetSettingFloat("ENBSeries", fDuskMultiplier);
	SetSettingFloat("ENBSeries", fNightMultiplier);

	ini.SaveFile(L"Data\\SKSE\\Plugins\\SoftShadows.ini");
}

float GetNiColorBrightness(RE::NiColor a_color)
{
	ColorSpace::Rgb rgbColor = { a_color.red, a_color.green, a_color.blue };
	ColorSpace::Lab labColor;
	rgbColor.To<ColorSpace::Lab>(&labColor);
	return (float)rgbColor.r;
}

RE::NiColor GetDALCAverage(RE::NiColor a_directionalAmbientColors[3][2])
{
	RE::NiColor avg = { 0.0f, 0.0f, 0.0f };
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 2; y++) {
			avg += a_directionalAmbientColors[x][y];
		}
	}
	avg /= 6;
	return avg;
}

//float GetWeatherDiffusion(RE::TESWeather* a_weather)
//{
//	if (a_weather->data.flags.any(RE::TESWeather::WeatherDataFlag::kCloudy)) {
//		return 3.0f;
//	} else if (a_weather->data.flags.any(RE::TESWeather::WeatherDataFlag::kRainy)) {
//		return 4.0f;
//	} else if (a_weather->data.flags.any(RE::TESWeather::WeatherDataFlag::kSnow)) {
//		return 5.0f;
//	}
//	return 1.0f;
//}

void SoftShadows::Update()
{
	if (auto sky = RE::Sky::GetSingleton()) {
		if (bEnabled) {
			auto cell = RE::PlayerCharacter::GetSingleton()->GetParentCell();
			if (cell && !cell->IsInteriorCell()) {
				if (sky->currentWeather && sky->sun && sky->directionalAmbientColors) {
					RE::NiColor dalc = GetDALCAverage(sky->directionalAmbientColors);

					ambient = GetNiColorBrightness(dalc);
					sunlight = GetNiColorBrightness(((RE::NiLight*)sky->sun->light.get())->GetLightRuntimeData().diffuse);

					////diffusion = pow(fDiffusionBase + (ambient / sunlight), fDiffusionCurve);

					//diffusion = GetWeatherDiffusion(sky->currentWeather);
					//if (sky->lastWeather) {
					//	diffusion = std::lerp(GetWeatherDiffusion(sky->lastWeather), diffusion, sky->currentWeatherPct);
					//}

					if (g_ENB) {
						auto TimeOfDay = g_ENB->GetTimeOfDay();
						auto multiplier = 0.0f;
						multiplier += TimeOfDay.dawn * fDawnMultiplier;
						multiplier += TimeOfDay.sunrise * fSunriseMultiplier;
						multiplier += TimeOfDay.day * fDayMultiplier;
						multiplier += TimeOfDay.sunset * fSunsetMultiplier;
						multiplier += TimeOfDay.dusk * fDuskMultiplier;
						multiplier += TimeOfDay.night * fNightMultiplier;
						diffusion = pow((ambient / sunlight) * fDiffusionBase, fDiffusionCurve);

						diffusion *= multiplier;
					} else {
						diffusion = pow((ambient / sunlight) * fDiffusionBase, fDiffusionCurve);
					}

					diffusion = pow(diffusion, fDiffusionCurve);

					diffusion = std::clamp(diffusion, fDiffusionMin, fDiffusionMax);

					if (sky->lastWeather) {
						SetGameSettingFloat("fPoissonRadiusScale", "Display", fPoissonRadiusScaleBase * diffusion);
						SetGameSettingFloat("fFirstSliceDistance", "Display", fFirstSliceDistanceBase * std::lerp(1.0f, diffusion, fFirstSliceDistanceScale));
					} else {
						SetGameSettingFloat("fPoissonRadiusScale", "Display", fPoissonRadiusScaleBase * diffusion);
						SetGameSettingFloat("fFirstSliceDistance", "Display", fFirstSliceDistanceBase * std::lerp(1.0f, diffusion, fFirstSliceDistanceScale));
					}
				}
			} else {
				diffusion = 1.0f * fDiffusionInterior;
				ambient = 0.0f;
				sunlight = 0.0f;
				SetGameSettingFloat("fPoissonRadiusScale", "Display", fPoissonRadiusScaleBase * diffusion);
				SetGameSettingFloat("fFirstSliceDistance", "Display", fFirstSliceDistanceBase * std::lerp(1.0f, diffusion, fFirstSliceDistanceScale));
			}
		} else {
			SetGameSettingFloat("fPoissonRadiusScale", "Display", fPoissonRadiusScaleBase);
			SetGameSettingFloat("fFirstSliceDistance", "Display", fFirstSliceDistanceBase);
		}
	}
}

#define TWDEF "group='MOD:Soft Shadows' precision=2 step=0.01 "

void SoftShadows::RefreshUI()
{
	auto bar = g_ENB->TwGetBarByEnum(!REL::Module::IsVR() ? ENB_API::ENBWindowType::EditorBarEffects : ENB_API::ENBWindowType::EditorBarObjects);  // ENB misnames its own bar, whoops!

	g_ENB->TwAddVarRW(bar, "SoftShadowsEnabled", ETwType::TW_TYPE_BOOLCPP, &bEnabled, TWDEF);

	g_ENB->TwAddVarRW(bar, "PoissonRadiusScaleBase", ETwType::TW_TYPE_FLOAT, &fPoissonRadiusScaleBase, TWDEF);
	g_ENB->TwAddVarRW(bar, "FirstSliceDistanceBase", ETwType::TW_TYPE_FLOAT, &fFirstSliceDistanceBase, TWDEF);
	g_ENB->TwAddVarRW(bar, "FirstSliceDistanceScale", ETwType::TW_TYPE_FLOAT, &fFirstSliceDistanceScale, TWDEF);

	g_ENB->TwAddVarRW(bar, "DiffusionBase", ETwType::TW_TYPE_FLOAT, &fDiffusionBase, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionCurve", ETwType::TW_TYPE_FLOAT, &fDiffusionCurve, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionMin", ETwType::TW_TYPE_FLOAT, &fDiffusionMin, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionMax", ETwType::TW_TYPE_FLOAT, &fDiffusionMax, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionInterior", ETwType::TW_TYPE_FLOAT, &fDiffusionInterior, TWDEF);

	g_ENB->TwAddVarRW(bar, "DiffusionDawnMultiplier", ETwType::TW_TYPE_FLOAT, &fDawnMultiplier, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionSunriseMultiplier", ETwType::TW_TYPE_FLOAT, &fSunriseMultiplier, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionDayMultiplier", ETwType::TW_TYPE_FLOAT, &fDayMultiplier, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionSunsetMultiplier", ETwType::TW_TYPE_FLOAT, &fSunsetMultiplier, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionDuskMultiplier", ETwType::TW_TYPE_FLOAT, &fDuskMultiplier, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionNightMultiplier", ETwType::TW_TYPE_FLOAT, &fNightMultiplier, TWDEF);

	g_ENB->TwAddVarRW(bar, "Diffusion", ETwType::TW_TYPE_FLOAT, &diffusion, TWDEF "readonly=true");
	g_ENB->TwAddVarRW(bar, "DiffusionAmbientBrightness", ETwType::TW_TYPE_FLOAT, &ambient, TWDEF "readonly=true");
	g_ENB->TwAddVarRW(bar, "DiffusionSunlightBrightness", ETwType::TW_TYPE_FLOAT, &sunlight, TWDEF "readonly=true");

	g_ENB->TwDefine("EditorBarEffects/'MOD:Soft Shadows' opened=false");
}
