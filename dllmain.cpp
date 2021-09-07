// dllmain.cpp : Defines the entry point for the DLL application.
#include "globals.h"

#include <d3d11.h>
#include <Windows.h>
#include <iostream>
#include <wrl/client.h>
#include <wincodec.h>
#include <plog/Formatters/TxtFormatter.h>

#include "scs_sdk_1_12/include/scssdk_telemetry.h"
#include "scs_sdk_1_12/include/eurotrucks2/scssdk_eut2.h"
#include "scs_sdk_1_12/include/eurotrucks2/scssdk_telemetry_eut2.h"

#include "kiero/kiero.h"

#include "ETS2Hook.h"

#include "config.h"
#include "ets2dc_imgui.h"
#include "ets2dc_dx11hook.h"

static bool log_initialized = false;

/// <summary>
/// Global shutdown callback for Euro Truck Simulator 2 Telemetry SDK
/// </summary>
SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t* const params)
{
    if (!log_initialized) {
        // Best place to do this?
        ets2dc_config::init();

        // Init logging
        const std::wstring logFile = ets2dc_utils::GetProjectDataFolder() + L"ets2-data-capture.log";
        const std::string logLevel = ets2dc_config::get("LogLevel", "error");
        int level = ets2dc_utils::LogLevelFromString(logLevel);

        static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(logFile.c_str());

        plog::init(plog::Severity(level), &fileAppender).addAppender(AppLog::appLog);
        log_initialized = true;

        PLOGI << L"***** Log initialized ******";
    }

    scs_result_t result = ets2dc_telemetry::init(version, params);
    if (result != SCS_RESULT_ok) {
        return result;
    }

    ets2dc_dx11hook::init();
    	    
    return SCS_RESULT_ok;    
}

/// <summary>
/// Global shutdown callback for Euro Truck Simulator 2 Telemetry SDK
/// </summary>
SCSAPI_VOID scs_telemetry_shutdown(void)
{
    // Any cleanup needed. The registrations will be removed automatically
    // so there is no need to do that manually.
    ets2dc_dx11hook::shutdown();
    // FreeConsole();
    PLOGI << "DLL Unloaded";
}

/// <summary>
/// DLL entry
/// </summary>
/// <param name="version"></param>
/// <param name="params"></param>
/// <returns></returns>
BOOL WINAPI DllMain( HINSTANCE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        // do whatever needed when the DLL is detached
    }

    return TRUE;
}

