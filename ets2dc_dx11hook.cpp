#include "globals.h"

#include "ets2dc_dx11hook.h"

#include "kiero/kiero.h"
#include "ETS2Hook.h"

namespace ets2dc_dx11hook
{
    namespace {
        static ETS2Hook* hook;
        static bool init_hook = false;

        static Present oPresent = NULL;
        static CreateTexture2D oCreateTexture2D = NULL;
        static Draw oDraw = NULL;
        static ClearDepthStencilView oClearDepthStencilView = NULL;
        static ClearRenderTargetView oClearRenderTargetView = NULL;
    }

    #pragma region Hook functions
    void __stdcall hkClearRenderTargetView(ID3D11DeviceContext* context, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4])
    {
        /// PLOGV << "hkClearRenderTargetView called";

        if (hook != nullptr)
        {
            return hook->ClearRenderTargetViewHook(oClearRenderTargetView, context, pRenderTargetView, ColorRGBA);
        }

        return oClearRenderTargetView(context, pRenderTargetView, ColorRGBA);
    }

    /// <summary>
    /// DX11 Present Hook function
    /// </summary>
    HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
    {
        if (hook != nullptr)
        {
            return hook->PresentHook(oPresent, pSwapChain, SyncInterval, Flags);
        }

        return oPresent(pSwapChain, SyncInterval, Flags);
    }    

    long __stdcall hkCreateTexture2D(ID3D11Device* device, D3D11_TEXTURE2D_DESC* pDesc, D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
    {
        if (hook != nullptr && hook->Initialized)
        {
            return hook->CreateTexture2DHook(oCreateTexture2D, device, pDesc, pInitialData, ppTexture2D);
        }

        return oCreateTexture2D(device, pDesc, pInitialData, ppTexture2D);
    }

    // Old DX11 Draw    
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

    void __stdcall hkClearDepthStencilView(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil)
    {
        if (hook != nullptr && hook->Initialized)
        {
            hook->ClearDepthStencilViewHook(oClearDepthStencilView, deviceContext, pDepthStencilView, ClearFlags, Depth, Stencil);
        } 
        else 
        {
            oClearDepthStencilView(deviceContext, pDepthStencilView, ClearFlags, Depth, Stencil);
        }
    }
#pragma endregion

    void init()
    {
        PLOGI << "Hooking DX11";
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
            kiero::Status::Enum status;

#define register_hook(id, method) status = kiero::bind(id, (void**)&o##method, hk##method); if(status != kiero::Status::Success) { PLOGE << "DX11 hook for "#method" failed."; } else { PLOGV << "DX11 hook for "#method" suceeded."; }
            register_hook(8, Present)
            register_hook(23, CreateTexture2D)
            register_hook(74, Draw)            
            register_hook(111, ClearRenderTargetView)
            register_hook(114, ClearDepthStencilView)
#undef register_hook

            PLOGI << "Instantiating ETS2 Hook";
            hook = new ETS2Hook();
        }
    }

    void shutdown()
    {
        if (hook != nullptr) {            
            delete hook;
            PLOGI << "ETS2 Hook destroyed";
        }

        if (init_hook) {
            PLOGI << "Removing DX11 hooks";
            kiero::unbind(8);
            kiero::unbind(23);
            kiero::unbind(74);
            kiero::unbind(111);
            kiero::unbind(114);            
            kiero::shutdown();
        }
    }
}
