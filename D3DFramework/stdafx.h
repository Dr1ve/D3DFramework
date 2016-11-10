#pragma once

#include <clocale>
#include <ctime>

#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <map>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <D3D11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>

#pragma comment(lib, "d3d11.lib")
#ifdef _DEBUG
#	pragma comment(lib, "d3dx11d.lib")
#else
#	pragma comment(lib, "d3dx11.lib")
#endif