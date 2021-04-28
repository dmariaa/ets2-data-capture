#pragma once

#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <date/date.h>
#include <Windows.h>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <DirectXTex.h>
#pragma comment(lib, "DirectXTex.lib")

#include <ScreenGrab.h>
#pragma comment(lib, "DirectXTK.lib")

//#define DIRECTINPUT_VERSION 0x0800
//#include <dinput.h>
//#pragma comment(lib, "dinput8.lib")
//#pragma comment(lib, "dxguid.lib")

#include "utils.h"

#include <plog/Log.h>
#include <plog/Logger.h>
#include <plog/Initializers/RollingFileInitializer.h>
