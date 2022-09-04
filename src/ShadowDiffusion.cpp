#include "ShadowDiffusion.h"

#include <ColorSpace/ColorSpace.h>
#include <SimpleIni.h>

float& ShadowDiffusion::GetGameSettingFloat(std::string a_name, std::string a_section)
{
	auto ini = RE::INISettingCollection::GetSingleton();
	return ini->GetSetting(std::format("{}:{}", a_name, a_section))->data.f;
}

void ShadowDiffusion::SetGameSettingFloat(std::string a_name, std::string a_section, float a_value)
{
	auto ini = RE::INISettingCollection::GetSingleton();
	ini->GetSetting(std::format("{}:{}", a_name, a_section))->data.f = a_value;
}

#define GetSettingFloat(a_section, a_setting) a_setting = (float)ini.GetDoubleValue(a_section, #a_setting, 1.0f);
#define SetSettingFloat(a_section, a_setting) ini.SetDoubleValue(a_section, #a_setting, a_setting);

#define GetSettingBool(a_section, a_setting) a_setting = ini.GetBoolValue(a_section, #a_setting, true);
#define SetSettingBool(a_section, a_setting) ini.SetBoolValue(a_section, #a_setting, a_setting);

void ShadowDiffusion::LoadINI()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);
	CSimpleIniA                        ini;
	ini.SetUnicode();
	ini.LoadFile(L"Data\\SKSE\\Plugins\\ShadowDiffusion.ini");

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

void ShadowDiffusion::SaveINI()
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

	ini.SaveFile(L"Data\\SKSE\\Plugins\\ShadowDiffusion.ini");
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

float ShadowDiffusion::GetENBParameterFloat(const char* a_filename, const char* a_category, const char* a_keyname)
{
	float                 fvalue;
	ENB_SDK::ENBParameter param;
	if (g_ENB && g_ENB->GetParameter(a_filename, a_category, a_keyname, &param)) {
		if (param.Type == ENB_SDK::ENBParameterType::ENBParam_FLOAT) {
			memcpy(&fvalue, param.Data, ENBParameterTypeToSize(ENB_SDK::ENBParameterType::ENBParam_FLOAT));
			return fvalue;
		}
	}
	logger::debug("Could not find ENB parameter {}:{}:{}", a_filename, a_category, a_keyname);
	return 1.0f;
}

void ShadowDiffusion::UpdateENBTimeOfDay()
{
	TimeOfDay.dawn = GetENBParameterFloat("enbseries.ini", "Statistics", "TimeOfDayFactorDawn");
	TimeOfDay.sunrise = GetENBParameterFloat("enbseries.ini", "Statistics", "TimeOfDayFactorSunrise");
	TimeOfDay.day = GetENBParameterFloat("enbseries.ini", "Statistics", "TimeOfDayFactorDay");
	TimeOfDay.sunset = GetENBParameterFloat("enbseries.ini", "Statistics", "TimeOfDayFactorSunset");
	TimeOfDay.dusk = GetENBParameterFloat("enbseries.ini", "Statistics", "TimeOfDayFactorDusk");
	TimeOfDay.night = GetENBParameterFloat("enbseries.ini", "Statistics", "TimeOfDayFactorNight");
	TimeOfDay.interiorDay = GetENBParameterFloat("enbseries.ini", "Statistics", "TimeOfDayFactorInteriorDay");
	TimeOfDay.interiorNight = GetENBParameterFloat("enbseries.ini", "Statistics", "TimeOfDayFactorInteriorNight");
	TimeOfDay.interiorFactor = GetENBParameterFloat("enbseries.ini", "Statistics", "InteriorFactor");
}

void ShadowDiffusion::Update()
{
	if (auto sky = RE::Sky::GetSingleton()) {
		if (bEnabled) {
			auto cell = RE::PlayerCharacter::GetSingleton()->GetParentCell();
			if (cell && !cell->IsInteriorCell()) {
				if (sky->currentWeather && sky->sun && sky->directionalAmbientColors) {
					RE::NiColor dalc = GetDALCAverage(sky->directionalAmbientColors);

					ambient = GetNiColorBrightness(dalc);
					sunlight = GetNiColorBrightness(((RE::NiLight*)sky->sun->light.get())->GetLightRuntimeData().diffuse);

					diffusion = pow(fDiffusionBase + (ambient / sunlight), fDiffusionCurve);

					//if (g_ENB) {
					//	UpdateENBTimeOfDay();
					//	auto multiplier = 0.0f;
					//	multiplier += TimeOfDay.dawn * fDawnMultiplier;
					//	multiplier += TimeOfDay.sunrise * fSunriseMultiplier;
					//	multiplier += TimeOfDay.day * fDayMultiplier;
					//	multiplier += TimeOfDay.sunset * fSunsetMultiplier;
					//	multiplier += TimeOfDay.dusk * fDuskMultiplier;
					//	multiplier += TimeOfDay.night * fNightMultiplier;
					//	diffusion *= multiplier;
					//}

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

#define TWDEF "group = 'MOD:Shadow Diffusion' precision = 2 step = 0.01 "

void ShadowDiffusion::RefreshUI()
{
	auto bar = g_ENB->TwGetBarByEnum(!REL::Module::IsVR() ? ENB_API::ENBWindowType::EditorBarEffects : ENB_API::ENBWindowType::EditorBarObjects);  // ENB misnames its own bar, whoops!
	g_ENB->TwAddVarRW(bar, "Enabled", ETwType::TW_TYPE_BOOLCPP, &bEnabled, TWDEF);
	g_ENB->TwAddVarRW(bar, "PoissonRadiusScaleBase", ETwType::TW_TYPE_FLOAT, &fPoissonRadiusScaleBase, TWDEF);
	g_ENB->TwAddVarRW(bar, "FirstSliceDistanceBase", ETwType::TW_TYPE_FLOAT, &fFirstSliceDistanceBase, TWDEF);
	g_ENB->TwAddVarRW(bar, "FirstSliceDistanceScale", ETwType::TW_TYPE_FLOAT, &fFirstSliceDistanceScale, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionBase", ETwType::TW_TYPE_FLOAT, &fDiffusionBase, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionCurve", ETwType::TW_TYPE_FLOAT, &fDiffusionCurve, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionMin", ETwType::TW_TYPE_FLOAT, &fDiffusionMin, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionMax", ETwType::TW_TYPE_FLOAT, &fDiffusionMax, TWDEF);
	g_ENB->TwAddVarRW(bar, "DiffusionInterior", ETwType::TW_TYPE_FLOAT, &fDiffusionInterior, TWDEF);
	g_ENB->TwAddVarRW(bar, "Diffusion", ETwType::TW_TYPE_FLOAT, &diffusion, TWDEF "readonly=true");
	g_ENB->TwAddVarRW(bar, "AmbientBrightness", ETwType::TW_TYPE_FLOAT, &ambient, TWDEF "readonly=true");
	g_ENB->TwAddVarRW(bar, "SunlightBrightness", ETwType::TW_TYPE_FLOAT, &sunlight, TWDEF "readonly=true");
	g_ENB->TwDefine("EditorBarEffects/MOD:ShadowDiffusion opened=false");
}
