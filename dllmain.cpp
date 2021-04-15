// dllmain.cpp : Defines the entry point for the DLL application.
#include <d3d11.h>
#include <Windows.h>
#include <iostream>
#include <wrl/client.h>
#include <wincodec.h>

#include "ScreenGrab.h"
#include "log.h"
#include "kiero/kiero.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DirectXTK.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

bool captured = false;
ID3D11Device* pDevice;
ID3D11DeviceContext* pDeviceContext;

void ScreenCapture(IDXGISwapChain* pSwapChain) {
    ID3D11Texture2D* pSurface = nullptr;
    HRESULT hr;

    if (pDevice == nullptr) {
        hr = pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice);

        if (FAILED(hr)) {
            std::cout << "Error getting device" << std::endl;
        }

        pDevice->GetImmediateContext(&pDeviceContext);
    }
    else {
        hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pSurface);

        if (FAILED(hr) || pSurface == nullptr) {
            std::cout << "Error capturing buffer for screenshot" << std::endl;
        }        

        if (!captured) {
            hr = SaveWICTextureToFile(pDeviceContext, pSurface, GUID_ContainerFormatJpeg, L"C:\\Users\\david\\Documents\\ETS2_CAPTURE\\captures\\screenshot.jpg");
            DX::ThrowIfFailed(hr);
            captured = true;
        }
    }
}

// Create the type of function that we will hook
typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    ScreenCapture(pSwapChain);
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

