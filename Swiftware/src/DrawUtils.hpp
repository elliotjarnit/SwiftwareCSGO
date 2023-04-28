#pragma once
#include <imgui/imgui.h>
#include "MathUtils.hpp"
#include "memory.hpp"
#include <string>;

namespace DrawUtils {
	void DrawBone(DWORD bones, MathUtils::Vector2 bone) {
		ImDrawList* drawlist = ImGui::GetBackgroundDrawList();

		MathUtils::Vector3 joint1pos = MathUtils::Vector3(
			MemoryManager::rpm<float>(bones + 0x30 * (INT)(bone.x) + 0x0C),
			MemoryManager::rpm<float>(bones + 0x30 * (INT)(bone.x) + 0x1C),
			MemoryManager::rpm<float>(bones + 0x30 * (INT)(bone.x) + 0x2C)
		);

		MathUtils::Vector3 joint2pos = MathUtils::Vector3(
			MemoryManager::rpm<float>(bones + 0x30 * (INT)(bone.y) + 0x0C),
			MemoryManager::rpm<float>(bones + 0x30 * (INT)(bone.y) + 0x1C),
			MemoryManager::rpm<float>(bones + 0x30 * (INT)(bone.y) + 0x2C)
		);

		MathUtils::Vector3 joint1screen;
		MathUtils::Vector3 joint2screen;
		if (MathUtils::world_to_screen(joint1pos, joint1screen, viewMatrix) && MathUtils::world_to_screen(joint2pos, joint2screen, viewMatrix)) {
			drawlist->AddLine({ joint1screen.x, joint1screen.y }, { joint2screen.x, joint2screen.y }, ImColor(255, 255, 255));
		}
	}

	void DrawJoint(DWORD bones, int joint) {
		ImDrawList* drawlist = ImGui::GetBackgroundDrawList();

		MathUtils::Vector3 jointpos = MathUtils::Vector3(
			MemoryManager::rpm<float>(bones + 0x30 * joint + 0x0C),
			MemoryManager::rpm<float>(bones + 0x30 * joint + 0x1C),
			MemoryManager::rpm<float>(bones + 0x30 * joint + 0x2C)
		);

		MathUtils::Vector3 jointscreen;
		if (MathUtils::world_to_screen(jointpos, jointscreen, viewMatrix)) {
			drawlist->AddCircleFilled({ jointscreen.x, jointscreen.y }, 3.0f, ImColor(255, 255, 255), 100);
		}
	}

	void DrawJointNum(DWORD bones, int joint) {
		ImDrawList* drawlist = ImGui::GetBackgroundDrawList();

		MathUtils::Vector3 jointpos = MathUtils::Vector3(
			MemoryManager::rpm<float>(bones + 0x30 * joint + 0x0C),
			MemoryManager::rpm<float>(bones + 0x30 * joint + 0x1C),
			MemoryManager::rpm<float>(bones + 0x30 * joint + 0x2C)
		);

		MathUtils::Vector3 jointscreen;
		if (MathUtils::world_to_screen(jointpos, jointscreen, viewMatrix)) {
			drawlist->AddText({ jointscreen.x, jointscreen.y }, ImColor(255, 255, 255), std::to_string(joint).c_str());
		}
	}
}