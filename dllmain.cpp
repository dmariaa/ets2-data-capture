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
#include "ets2dc_telemetry.h"

ETS2Hook* hook;

// Create the type of function that we will hook
typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

/// <summary>
/// DX11 Present Hook function
/// </summary>

HRESULT __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    hook->Init(pSwapChain);    
    hook->Present(pSwapChain);

    return oPresent(pSwapChain, SyncInterval, Flags);
}

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
        const std::wstring logFile = ets2dc_utils::getProjectDataFolder() + L"ets2-data-capture.log";
        const std::string logLevel = ets2dc_config::get("LogLevel", "error");
        int level = ets2dc_utils::logLevelFromString(logLevel);

        static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(logFile.c_str());

        plog::init(plog::Severity(level), &fileAppender).addAppender(AppLog::appLog);
        log_initialized = true;

        PLOGI << L"***** Log initialized ******";
    }

    scs_result_t result = ets2dc_telemetry::init(version, params);
    if (result != SCS_RESULT_ok) {
        return result;
    }

    	
    PLOGI << "Initializing hook";
    hook = new ETS2Hook();

    PLOGI << "Hooking DX11 Present";
    bool init_hook = false;
    do
    {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
        {
            kiero::bind(8, (void**)&oPresent, hkPresent11);
            init_hook = true;
        }
    } while (!init_hook);
    PLOGI << "DX11 Present successfully hooked";

    return SCS_RESULT_ok;    
}

/// <summary>
/// Global shutdown callback for Euro Truck Simulator 2 Telemetry SDK
/// </summary>
SCSAPI_VOID scs_telemetry_shutdown(void)
{
    // Any cleanup needed. The registrations will be removed automatically
    // so there is no need to do that manually.
    delete hook;

    PLOGI << "Removing DX11 Present hook";
    kiero::unbind(8);
    kiero::shutdown();
    
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

