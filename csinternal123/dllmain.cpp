#include "common.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

EndScene oEndScene = NULL;
WNDPROC oWndProc;
static HWND window = NULL;

void InitGui(LPDIRECT3DDEVICE9 lpDevice)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(lpDevice);
}

bool init = false;

bool bShowMain = false;

bool bBhop = false;

bool bGlow = false;

ImColor teamColor;
ImColor enemyColor;

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	if (!init)
	{
		InitGui(pDevice);
		init = true;
	}

	if (GetAsyncKeyState(VK_DELETE) & 1)
	{
		bShowMain = !bShowMain;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (bShowMain)
	{
		ImGui::Begin(WINDOW_NAME, 0, ImGuiWindowFlags_NoResize);
		ImGui::SetWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);

		ImGui::Checkbox("Bhop", &bBhop);
		ImGui::Checkbox("Glow", &bGlow);

		ImGui::ColorPicker4("Team Glow Color", (float*)&teamColor);
		ImGui::ColorPicker4("Enemy Glow Color", (float*)&enemyColor);

		ImGui::End();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK EnumWindowsCallback(HWND wHandle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(wHandle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE;

	window = wHandle;
	return FALSE;
}

HWND GetWindow()
{
	window = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return window;
}

DWORD WINAPI Init(LPVOID lpReserved)
{
	while (!GetModuleHandleA("serverbrowser.dll"))
	{
		Sleep(250);
	}

	bool attached = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
		{
			kiero::bind(42, (void**)&oEndScene, hkEndScene);
			do
				window = GetWindow();
			while (window == NULL);
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
			attached = true;
		}
	} while (!attached);

	DWORD gameClient = (DWORD)GetModuleHandle("client.dll");
	DWORD localPlayer = *(DWORD*)(gameClient + dwLocalPlayer);
	while (localPlayer == NULL)
	{
		localPlayer = *(DWORD*)(gameClient + dwLocalPlayer);
	}

	teamColor.Value.w = 1;
	enemyColor.Value.w = 1;

	while (true)
	{
		if (bBhop)
		{
			DWORD flag = *(BYTE*)(localPlayer + m_fFlags);
			if (GetAsyncKeyState(VK_SPACE) && flag & (1 << 0))
			{
				*(DWORD*)(gameClient + dwForceJump) = 6;
			}
		}
		if (GetAsyncKeyState(VK_END)) // panic
		{
			Sleep(250);
			break;
		}
	}
	kiero::shutdown();
	MH_DisableHook(NULL);
	MH_RemoveHook(NULL);
	MH_Uninitialize();
	FreeConsole();
	FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
	return 0;
}

DWORD WINAPI Glow(LPVOID lpReserved)
{
	DWORD gameClient = (uintptr_t)GetModuleHandle("client.dll");
	uintptr_t glowObj = *(uintptr_t*)(gameClient + dwGlowObjectManager);

	while (true)
	{
		DWORD localPlayer = *(uintptr_t*)(gameClient + dwLocalPlayer);
		if (bGlow)
		{
			int localTeam = *(int*)(dwLocalPlayer + m_iTeamNum);
			for (int i = 0; i < 32; i++)
			{
				uintptr_t entity = *(uintptr_t*)(gameClient + dwEntityList + i * 0x10);
				if (entity != NULL)
				{
					int entityTeam = *(int*)(entity + m_iTeamNum);
					int glowIndex = *(int*)(entity + m_iGlowIndex);

					if (localTeam == entityTeam)
					{
						*(float*)(glowObj + ((glowIndex * 0x38) + 0x4)) = teamColor.Value.x;
						*(float*)(glowObj + ((glowIndex * 0x38) + 0x8)) = teamColor.Value.y;
						*(float*)(glowObj + ((glowIndex * 0x38) + 0xC)) = teamColor.Value.z;
						*(float*)(glowObj + ((glowIndex * 0x38) + 0x10)) = teamColor.Value.w;
					}
					else
					{
						*(float*)(glowObj + ((glowIndex * 0x38) + 0x4)) = enemyColor.Value.x;
						*(float*)(glowObj + ((glowIndex * 0x38) + 0x8)) = enemyColor.Value.y;
						*(float*)(glowObj + ((glowIndex * 0x38) + 0xC)) = enemyColor.Value.z;
						*(float*)(glowObj + ((glowIndex * 0x38) + 0x10)) = enemyColor.Value.w;
					}
					*(bool*)(glowObj + ((glowIndex * 0x38) + 0x24)) = true;
					*(bool*)(glowObj + ((glowIndex * 0x38) + 0x25)) = false;
				}
			}
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, 0, Init, (LPTHREAD_START_ROUTINE)hModule, 0, nullptr);
		CreateThread(nullptr, 0, Glow, (LPTHREAD_START_ROUTINE)hModule, 0, nullptr);
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}
