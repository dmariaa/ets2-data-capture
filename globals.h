#pragma once

#include <cstdio>
#include <ctime>
#include <string>
#include <iostream>
#include <sstream>

#include <Windows.h>

#include <d3d11.h>
#include <ScreenGrab.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DirectXTK.lib")

//#define DIRECTINPUT_VERSION 0x0800
//#include <dinput.h>
//#pragma comment(lib, "dinput8.lib")
//#pragma comment(lib, "dxguid.lib")

#include "utils.h"

#include <plog/Log.h>
#include <plog/Logger.h>
#include <plog/Initializers/RollingFileInitializer.h>
