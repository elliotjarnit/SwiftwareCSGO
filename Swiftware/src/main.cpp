#include <Windows.h>
#include "memory.hpp"
#include "offsets.hpp"
#include "MathUtils.hpp"
#include "DrawUtils.hpp"
#include "common.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_stdlib.h>

#include "wtypes.h"
#include <iostream>
#include <format>
#include <dwmapi.h>
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <string>
#include <thread>

const std::string VERSION = "EARLY ACCESS 0.0.1";

#ifdef NDEBUG
const bool DEV = false;
#else
const bool DEV = true;
#endif

DWORD processPID;
HANDLE processHandle;
DWORD clientDLL;
DWORD engineDLL;
MathUtils::ViewMatrix viewMatrix;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param)) {
		return 0L;	
	}

	if (message == WM_DESTROY) {
		PostQuitMessage(0);
		return 0L;
	}

	return DefWindowProc(window, message, w_param, l_param);
}

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {
	// Show dev console if dev
	if (DEV) {
		if (!AllocConsole()) return false;

		// Redirect IO
		FILE* file = nullptr;
		freopen_s(&file, "CONIN$", "r", stdin);
		freopen_s(&file, "CONOUT$", "w", stdout);
		freopen_s(&file, "CONOUT$", "w", stderr);
	}

	std::cout << "Swiftware Dev Console\n\n";
	std::cout << "Creating window...\n";

	// Get the res ya know
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);

	// Window shit ion know
	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = window_procedure;
	wc.hInstance = instance;
	wc.lpszClassName = L"Swiftware";

	RegisterClassExW(&wc);

	// Create the window
	const HWND window = CreateWindowExW(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		wc.lpszClassName,
		L"Swiftware",
		WS_POPUP,
		0,
		0,
		desktop.right,
		desktop.bottom,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	// Make it so I can draw
	SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);
	{
		RECT client_area{};
		RECT window_area{};
		POINT diff{};

		GetClientRect(window, &client_area);
		GetWindowRect(window, &window_area);
		ClientToScreen(window, &diff);

		const MARGINS margins{
			window_area.left + (diff.x - window_area.left),
			window_area.top + (diff.y - window_area.top),
			client_area.right,
			client_area.bottom
		};

		DwmExtendFrameIntoClientArea(window, &margins);
	}

	// DX11 Stuff
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.RefreshRate.Numerator = 60U;
	sd.BufferDesc.RefreshRate.Denominator = 1U;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = 1U;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2U;
	sd.OutputWindow = window;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Create the DX11 Device
	constexpr D3D_FEATURE_LEVEL levels[2]{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* device_context = nullptr;
	IDXGISwapChain* swap_chain = nullptr;
	ID3D11RenderTargetView* render_target_view = nullptr;
	D3D_FEATURE_LEVEL level;


	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		levels,
		2U,
		D3D11_SDK_VERSION,
		&sd,
		&swap_chain,
		&device,
		&level,
		&device_context
	);

	ID3D11Texture2D* back_buffer = nullptr;
	swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

	if (back_buffer) {
		device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
		back_buffer->Release();
	}
	else {
		return 1;
	}

	// Finally show the window
	ShowWindow(window, cmd_show);
	UpdateWindow(window);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, device_context);
	std::cout << "Window created!\n";

	bool running = true;
	bool injected = false;
	
	// Get PID of CSGO
	std::cout << "Waiting for CSGO...\n";
	std::thread pidThread([&]() {
			do {
				processPID = MemoryManager::getProcessID("csgo.exe");
				if (processPID) {
					std::cout << "CSGO found! PID: "<< processPID << "\n";
					// Pid found, do rest
					clientDLL = MemoryManager::getProcessAddress(processPID, "client.dll");
					engineDLL = MemoryManager::getProcessAddress(processPID, "engine.dll");
					processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processPID);
					std::cout << "Modules found!\n";
					injected = true;
				}
				Sleep(200UL);
			} while (!processHandle);
			return 0;
		});
	

	while (running) {
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				running = false;
			}
		}

		if (GetKeyState(VK_DELETE) & 0x8000) running = false;
		if (!running) break;

		// RENDERING!!!
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Render here
		// \/\/\/\/\/

		ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
		drawlist->AddText(ImVec2(0, 0), ImColor(173, 216, 230), ("Swiftware " + VERSION).c_str());
		drawlist->AddText(ImVec2(0, ImGui::GetFontSize() + 3), ImColor(227, 206, 14), "Press DELETE to quit");
		if (injected) {
			drawlist->AddText(ImVec2(0, (ImGui::GetFontSize() + 3) * 2), ImColor(0, 255, 0), "Injected");
		} else {
			drawlist->AddText(ImVec2(0, (ImGui::GetFontSize() + 3) * 2), ImColor(255, 0, 0), "Not Injected");
			drawlist->AddText(ImVec2(0, (ImGui::GetFontSize() + 3) * 3), ImColor(255, 0, 0), "Waiting for CSGO.EXE...");
		}

		// Get the PID of the focused window to only render when CSGO is focused
		DWORD focusedWinPID = 0;
		GetWindowThreadProcessId(GetForegroundWindow(), &focusedWinPID);

		// Hack Logic here
		if (injected && focusedWinPID == processPID) {
			const auto local_player = MemoryManager::rpm<DWORD>(clientDLL + offsets::localPlayer);

			if (local_player) {
				const auto lp_team = MemoryManager::rpm<DWORD>(local_player + offsets::teamNum);
				viewMatrix = MemoryManager::rpm<MathUtils::ViewMatrix>(clientDLL + offsets::viewMatrix);
				//std::cout << "Local Player Team: " << lp_team << "\n";

				// Loop over all players
				for (int i = 1; i < 32; i++) {
					const auto player = MemoryManager::rpm<DWORD>(clientDLL + (offsets::entityList + (i - 1) * 0x10));
					if (!player) continue;
					const auto player_dormant = MemoryManager::rpm<bool>(player + offsets::dormant);
					const auto player_team = MemoryManager::rpm<int>(player + offsets::teamNum);
					const auto player_alive = MemoryManager::rpm<int>(player + offsets::lifeState);
					const auto player_health = MemoryManager::rpm<int>(player + offsets::health);
					const auto player_bones = MemoryManager::rpm<DWORD>(player + offsets::boneMatrix);
					auto player_feet = MemoryManager::rpm<MathUtils::Vector3>(player + offsets::origin);
					if ((player_dormant) || (player_team == lp_team) || (player_alive != 0) || (!player_bones)) continue;
					MathUtils::Vector3 head_pos = MathUtils::Vector3(
						MemoryManager::rpm<float>(player_bones + 0x30 * 8 + 0x0C),
						MemoryManager::rpm<float>(player_bones + 0x30 * 8 + 0x1C),
						MemoryManager::rpm<float>(player_bones + 0x30 * 8 + 0x2C)
					);

					MathUtils::Vector3 boxtop;
					MathUtils::Vector3 boxbottom;
					
					// Check if should draw
					if (MathUtils::world_to_screen(head_pos + MathUtils::Vector3(0, 0, 11.f), boxtop, viewMatrix) && MathUtils::world_to_screen(player_feet - MathUtils::Vector3(0, 0, 11.f), boxbottom, viewMatrix)) {
						const float box_height = boxbottom.y - boxtop.y;
						const float box_width = box_height * 0.35;

						const float left = boxtop.x - box_width;
						const float right = boxtop.x + box_width;
						const float top = boxtop.y;
						const float bottom = boxbottom.y;

						// Draw box
						drawlist->AddRect({ left, top }, { right, bottom }, ImColor(255, 0, 0));

						// Draw health
						const float healthPercent = player_health * 0.01;
						drawlist->AddRectFilled({ left - 7, bottom - (box_height * healthPercent) }, { left - 3, bottom }, ImColor(0, 255, 0), 5);
						drawlist->AddRectFilled({ left - 7, top }, { left - 3, top + (box_height - (box_height * healthPercent)) }, ImColor(0, 0, 0), 5);

						// Draw bones
						MathUtils::Vector2 bones[13]{
							// Chest
							MathUtils::Vector2(0, 6),
							MathUtils::Vector2(6, 7),
							MathUtils::Vector2(7, 8),
							// Left Arm
							MathUtils::Vector2(13, 12),
							MathUtils::Vector2(12, 38),
							MathUtils::Vector2(38, 6),
							// Right Arm
							MathUtils::Vector2(43, 42),
							MathUtils::Vector2(42, 68),
							MathUtils::Vector2(68, 6),
							// Left Leg
							MathUtils::Vector2(72, 71),
							MathUtils::Vector2(71, 0),
							// Right leg
							MathUtils::Vector2(79, 78),
							MathUtils::Vector2(78, 0),
						};

						for (MathUtils::Vector2 i : bones) {
							DrawUtils::DrawBone(player_bones, i);
						}

						// If dev, draw bones nums
						if (DEV) {
							for (int i = 0; i < 128; i++) {
								DrawUtils::DrawJointNum(player_bones, i);
							}
						}
					}
				}
			}
		}

		ImGui::Render();

		constexpr float color[4]{ 0.f, 0.f, 0.f, 0.f };
		device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
		if (render_target_view != 0) device_context->ClearRenderTargetView(render_target_view, color);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		swap_chain->Present(1U, 0U);
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	if (swap_chain) swap_chain->Release();
	if (device_context) device_context->Release();
	if (device) device->Release();
	if (render_target_view) render_target_view->Release();

	DestroyWindow(window);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}