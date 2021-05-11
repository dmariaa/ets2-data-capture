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

ETS2Hook* hook = nullptr;

static CreateTexture2D oCreateTexture2D = NULL;
long __stdcall hkCreateTexure2D(ID3D11Device* device, D3D11_TEXTURE2D_DESC* pDesc, D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
{
    if (hook != nullptr && hook->Initialized)
    {
        return hook->CreateTexture2DHook(oCreateTexture2D, device, pDesc, pInitialData, ppTexture2D);
    }

    return oCreateTexture2D(device, pDesc, pInitialData, ppTexture2D);
}

// Old DX11 Draw
static Draw oDraw = NULL;
void __stdcall hkDraw(ID3D11DeviceContext* deviceContext, UINT VertexCount, UINT StartVertexLocation)
{
    if (hook != nullptr && hook->Initialized)
    {
        hook->DrawHook(oDraw, deviceContext, VertexCount, StartVertexLocation);
    }
    else 
    {
        oDraw(deviceContext, VertexCount, StartVertexLocation);
    }
}

// Old DX11 Present
static Present oPresent = NULL;

/// <summary>
/// DX11 Present Hook function
/// </summary>
HRESULT __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if(hook != nullptr) 
    { 
        hook->Init(pSwapChain);
        return hook->PresentHook(oPresent, pSwapChain, SyncInterval, Flags);
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}

static bool log_initialized = false;
static bool init_hook = false;

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
    	
    PLOGI << "Hooking DX11 Present";    
    do
    {
        kiero::Status::Enum status = kiero::init(kiero::RenderType::D3D11);
        
        if (status == kiero::Status::NotSupportedError) {
            PLOGE << "Kiero DX11 Not supported, check kiero.h, DX11 not hooked";
            break;
        }

        if (status == kiero::Status::Success)
        {
            init_hook = true;
        }
    } while (!init_hook);

    if (init_hook)
    {
        kiero::Status::Enum status = kiero::bind(8, (void**)&oPresent, hkPresent11);

        if (status != kiero::Status::Success) {
            PLOGE << "DX11 present not hooked.";
        }
        else {
            PLOGI << "DX11 Present successfully hooked";
        }

        status = kiero::bind(23, (void**)&oCreateTexture2D, hkCreateTexure2D);

        if (status != kiero::Status::Success) {
            PLOGE << "DX11 CreateTexture2D not hooked.";
        }
        else {
            PLOGI << "DX11 CreateTexture2D successfully hooked";
        }

        status = kiero::bind(74, (void**)&oDraw, hkDraw);

        if (status != kiero::Status::Success) {
            PLOGE << "DX11 Draw not hooked.";
        }
        else {
            PLOGI << "DX11 Draw successfully hooked";
        }

        PLOGI << "Instantiating ETS2 Hook";
        hook = new ETS2Hook();
    }
    
    return SCS_RESULT_ok;    
}

/// <summary>
/// Global shutdown callback for Euro Truck Simulator 2 Telemetry SDK
/// </summary>
SCSAPI_VOID scs_telemetry_shutdown(void)
{
    // Any cleanup needed. The registrations will be removed automatically
    // so there is no need to do that manually.
    if (hook != nullptr) {
        delete hook;
    }
    
    if (init_hook) {
        PLOGI << "Removing DX11 Present hook";
        kiero::unbind(8);
        kiero::unbind(23);
        kiero::unbind(74);
        kiero::shutdown();
    }
    
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

