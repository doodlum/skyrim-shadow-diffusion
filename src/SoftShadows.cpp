#include "SoftShadows.h"

#include <ColorSpace/ColorSpace.h>

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

void SoftShadows::Load()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);
	CSimpleIniA                        ini;
	ini.SetUnicode();
	ini.LoadFile(L"Data\\SKSE\\Plugins\\SoftShadows.ini");

	GetSettingFloat("GameSetting", fPoissonRadiusScaleBase);

	GetSettingBool("SoftShadows", bEnabled);
	GetSettingFloat("SoftShadows", fSoftnessCurve);
	GetSettingFloat("SoftShadows", fSoftnessMax);
	GetSettingFloat("SoftShadows", fSoftnessInterior);
}

void SoftShadows::Save()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);
	CSimpleIniA                        ini;
	ini.SetUnicode();

	SetSettingFloat("GameSetting", fPoissonRadiusScaleBase);

	SetSettingBool("SoftShadows", bEnabled);
	SetSettingFloat("SoftShadows", fSoftnessCurve);
	SetSettingFloat("SoftShadows", fSoftnessMax);
	SetSettingFloat("SoftShadows", fSoftnessInterior);

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
	avg /= 6.0f;
	return avg;
}

void SoftShadows::Update()
{
	if (bEnabled) {
		if (auto sky = RE::Sky::GetSingleton()) {
			if (auto cell = RE::PlayerCharacter::GetSingleton()->GetParentCell()) {
				if (!cell->IsInteriorCell() && sky->currentWeather && sky->mode.any(RE::Sky::Mode::kFull)) {
					RE::NiColor dalc = GetDALCAverage(sky->directionalAmbientColors);
					ambient = GetNiColorBrightness(dalc);

					auto& shaderState = RE::BSShaderManager::State::GetSingleton();

					auto sunLight = skyrim_cast<RE::NiDirectionalLight*>(shaderState.shadowSceneNode[0]->GetRuntimeData().sunLight->light.get());
					auto imageSpaceManager = RE::ImageSpaceManager::GetSingleton();

					sunlight = GetNiColorBrightness(sunLight->GetLightRuntimeData().diffuse) * sunLight->GetLightRuntimeData().fade * (REL::Module::IsVR() ? imageSpaceManager->data.baseData.cinematic.brightness : imageSpaceManager->data.baseData.hdr.sunlightScale);
					
					sunlightAngle = sky->sun->GetRoot()->local.translate.z / 200.0f;
					sunlightAngle = std::clamp(sunlightAngle, 0.0f, 1.0f);
					sunlightAngle = 1.0f - sunlightAngle;

					softness = 1.0f + (ambient / sunlight) + sunlightAngle;
					softness = pow(softness, fSoftnessCurve);
					softness = std::clamp(softness, 1.0f, fSoftnessMax);

					SetGameSettingFloat("fPoissonRadiusScale", "Display", fPoissonRadiusScaleBase * softness);
				} else {
					SetGameSettingFloat("fPoissonRadiusScale", "Display", fSoftnessInterior);
				}
			} else {
				SetGameSettingFloat("fPoissonRadiusScale", "Display", fSoftnessInterior);
			}
		} else {
			SetGameSettingFloat("fPoissonRadiusScale", "Display", 4);
		}
	} else {
		SetGameSettingFloat("fPoissonRadiusScale", "Display", 4);
	}
}

void SoftShadows::Menu()
{
	if (ImGui::Button("Load")) {
		Load();
	}

	ImGui::SameLine();

	if (ImGui::Button("Save")) {
		Save();
	}

	ImGui::Checkbox("Enable", &bEnabled);

	ImGui::InputFloat("fPoissonRadiusScaleBase", &fPoissonRadiusScaleBase);

	ImGui::InputFloat("fSoftnessCurve", &fSoftnessCurve);
	ImGui::InputFloat("fSoftnessMax", &fSoftnessMax);

	ImGui::InputFloat("fSoftnessInterior", &fSoftnessInterior);

	ImGui::InputFloat("Current Softness", &softness);
	ImGui::InputFloat("Current Ambient", &ambient);
	ImGui::InputFloat("Current Sunlight", &sunlight);
	ImGui::InputFloat("Current Sunlight Angle", &sunlightAngle);
}
