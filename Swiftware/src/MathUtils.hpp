#pragma once
#include <imgui/imgui.h>

namespace MathUtils {
	struct Vector2 {
		Vector2() noexcept : x(), y() {}
		Vector2(float x, float y) noexcept : x(x), y(y) {}

		Vector2& operator+(const Vector2& v) noexcept {
			x += v.x;
			y += v.y;
			return *this;
		}

		Vector2& operator-(const Vector2& v) noexcept {
			x -= v.x;
			y -= v.y;
			return *this;
		}

		float x, y;
	};

	struct Vector3 {
		Vector3() noexcept: x(), y(), z() {}
		Vector3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}

		Vector3& operator+(const Vector3& v) noexcept {
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		Vector3& operator-(const Vector3& v) noexcept {
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}

		float x, y, z;
	};

	struct ViewMatrix {
		ViewMatrix() noexcept: data() {}

		float* operator[](int index) noexcept {
			return data[index];
		}
		
		const float* operator[](int index) const noexcept {
			return data[index];
		}

		float data[4][4];
	};

	static bool world_to_screen(const Vector3& world, Vector3& screen, const ViewMatrix& vm) noexcept {
		float w = vm[3][0] * world.x + vm[3][1] * world.y + vm[3][2] * world.z + vm[3][3];
		if (w < 0.001f) return false;

		const float x = world.x * vm[0][0] + world.y * vm[0][1] + world.z * vm[0][2] + vm[0][3];
		const float y = world.x * vm[1][0] + world.y * vm[1][1] + world.z * vm[1][2] + vm[1][3];

		w = 1.f / w;
		float nx = x * w;
		float ny = y * w;

		const ImVec2 size = ImGui::GetIO().DisplaySize;
		screen.x = (size.x * 0.5f * nx) + (nx + size.x * 0.5f);
		screen.y = -(size.y * 0.5f * ny) + (ny + size.y * 0.5f);
		
		return true;
	}
}