#pragma once

#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <date/date.h>
#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "utils.h"
#include "TextureBuffer.h"

#include <plog/Log.h>
#include <plog/Logger.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include "ets2dc_telemetry.h"