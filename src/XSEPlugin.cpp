#include "SoftShadows.h"
#include "CSAPI.h"

#include <imgui.h>

void DrawMenu()
{
	ImGui::Checkbox("SoftShadowsEnabled", &SoftShadows::GetSingleton()->bEnabled);
	ImGui::InputFloat("PoissonRadiusScaleBase", &SoftShadows::GetSingleton()->fPoissonRadiusScaleBase);
	ImGui::InputFloat("FirstSliceDistanceBase", &SoftShadows::GetSingleton()->fPoissonRadiusScaleBase);
	ImGui::InputFloat("FirstSliceDistanceScale", &SoftShadows::GetSingleton()->fPoissonRadiusScaleBase);

	ImGui::InputFloat("DiffusionBase", &SoftShadows::GetSingleton()->fDiffusionBase);
	ImGui::InputFloat("DiffusionCurve", &SoftShadows::GetSingleton()->fDiffusionCurve);
	ImGui::InputFloat("DiffusionMin", &SoftShadows::GetSingleton()->fDiffusionMin);
	ImGui::InputFloat("DiffusionMax", &SoftShadows::GetSingleton()->fDiffusionMax);
	ImGui::InputFloat("DiffusionInterior", &SoftShadows::GetSingleton()->fDiffusionInterior);
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		logger::info("Looking for CS...");

		auto result = RequestCSAPI();
		if (result.index() == 0) {
			auto csAPI = std::get<0>(result);
			ImGui::SetCurrentContext(csAPI->GetContext());
			csAPI->AddMenu("Kaputt", DrawMenu);
			logger::info("CS integration succeed!");
		} else
			logger::warn("CS integration failed! In-game config disabled. Error: {}", std::get<1>(result));
		break;
	}
}

void Load()
{
	SoftShadows::InstallHooks();
	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
}
