#pragma once

#include <Windows.h>

#include <d3d9.h>
#include <d3dx9.h>

#include "../deps/kiero/kiero.h"
#include "../deps/kiero/minhook/include/MinHook.h"

#include "../deps/imgui/imgui.h"
#include "../deps/imgui/imgui_impl_win32.h"
#include "../deps/imgui/imgui_impl_dx9.h"

#include "offsets.h"

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

#define WINDOW_NAME "csinternal123"

typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9);