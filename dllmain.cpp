// dllmain.cpp : Defines the entry point for the DLL application.
#include <d3d11.h>
#include <Windows.h>
#include <iostream>
#include <wrl/client.h>
#include <wincodec.h>

#include "kiero/kiero.h"
#include "log.h"
#include "ETS2Hook.h"

ETS2Hook* hook;

// Create the type of function that we will hook
typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    hook->Init(pSwapChain);
    hook->Present(pSwapChain);

    return oPresent(pSwapChain, SyncInterval, Flags);
}

int DLLMainThread() {    
    std::cout << "DLL Injected" << std::endl;

    std::cout << "Hooking DX11 Present" << std::endl;

    bool init_hook = false;
    do
    {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
        {
            kiero::bind(8, (void**)&oPresent, hkPresent11);
            init_hook = true;
        }
    } while (!init_hook);

    std::cout << "DX11 Present successfully hooked" << std::endl;

    return TRUE;
}

BOOL WINAPI DllMain( HINSTANCE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    DEBUG_INIT;

    hook = new ETS2Hook();

    DisableThreadLibraryCalls(hModule);

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DLLMainThread, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

