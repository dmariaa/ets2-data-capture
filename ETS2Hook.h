#pragma once
#include "globals.h"
#include "readerwriterqueue/readerwriterqueue.h"
#include <sstream>

#include "ets2dc_dx11hook.h"

class Config;
class AppSettings;
class TelemetryFile;

class SnapshotData 
{
public:
	TextureBuffer* image;
	TextureBuffer* depth;
	ets2dc_telemetry::telemetry_state* telemetry;

	snapshot_stats frame_stats;	
	std::wstring fileName;

	~SnapshotData()
	{
		if (image != nullptr) delete image;
		if (depth != nullptr) delete depth;
		if (telemetry != nullptr) delete telemetry;
	}
};


/// <summary>
/// Hook class for Euro Truck Simulator 2
/// </summary>
class ETS2Hook
{
	typedef std::chrono::duration<float> duration;
	typedef std::chrono::high_resolution_clock clock;

	// Pointers to useful objects
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pDeviceContext = nullptr;	
	
	ID3D11Texture2D* pSwapchainImageTexture = nullptr;

	ID3D11Texture2D *pScreenshotTexture = nullptr, *pLastScreenshotTexture = nullptr;
	ID3D11Texture2D *pDepthTexture = nullptr, *pLastDepthTexture = nullptr;
	
	HWND outputWindow;

	bool simulate;
	bool inputCaptured;
	std::atomic<int> capturing;
	
	snapshot_stats stats;	

	bool IsCapturing();

	// Configuration vars
	int consecutiveFramesCapture;
	int secondsBetweenCaptures; 	
	int captureDepth;
	bool captureTelemetry;
	std::wstring imageFolder;
	std::wstring imageFileFormat;
	std::wstring imageFileName;	
	
	std::wstring formatString;
	std::wstring snapshotId;

	// ImGui Stuff	
	bool imGuiDrawEnabled;
	void initImGui(IDXGISwapChain* pSwapChain, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	void shutdownImGui();
	void renderImGui();

	// Input handling stuff	
	void initInput();
	void shutdownInput();

	static WNDPROC ETS2_hWndProc;
	static ETS2Hook* pThis;
	static LRESULT CALLBACK hWndProcWrapper(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	// Raw input hooking
	RAWINPUTDEVICE ETS2RidMouse;
	void getRawMouseDevice();
	void startRawInputCapture();
	void endRawInputCapture();
	void processRawMouse(RAWMOUSE rawMouse);

	// Mouse hook stuff
	HHOOK mouseHookHandle;
	static LRESULT CALLBACK mouseHookWrapper(int nCode, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK mouseHook(int nCode, WPARAM wParam, LPARAM lParam);

	// Some util functions
	HRESULT TakeScreenshot1(IDXGISwapChain* swapChain);
	HRESULT TakeScreenshot2();
	void CaptureBlock(float deltaTime);
	void PauseBlock(float deltaTime);
	

	const std::wstring GenerateFileName();
	void GenerateFileFormatString();

	// Settings change update
	void updateSettings();
	AppSettings* appSettings;

	// Telemetry file
	TelemetryFile* telemetryFile;
	void InitTelemetryFile();
	void CloseTelemetryFile();

	// Writing thread
	moodycamel::ReaderWriterQueue<SnapshotData*> queue;
	std::atomic_bool captureThreadRunning = true;
	void saveBuffer();

	// To control pipelane stages
	bool frameStart = true;


public:
	ETS2Hook();
	~ETS2Hook();

	bool Initialized;

	/// <summary>
	/// Called to initialize usefull DX11 stuff
	/// </summary>
	/// <param name="swapChain"></param>
	/// <returns></returns>
	HRESULT Init(IDXGISwapChain* swapChain);	

	/// <summary>
	/// Called before DX11 present call
	/// </summary>
	/// <param name="swapChain"></param>
	/// <returns></returns>
	HRESULT PresentHook(ets2dc_dx11hook::Present oPresent, IDXGISwapChain* swapChain, 
		UINT SyncInterval, UINT Flags);

	/// <summary>
	/// Called before draw
	/// </summary>
	/// <param name="VertexCount"></param>
	/// <param name="StartVertexLocation"></param>
	void DrawHook(ets2dc_dx11hook::Draw oDraw, ID3D11DeviceContext* context, 
		UINT VertexCount, UINT StartVertexLocation);
	
	/// <summary>
	/// Called before DX11 CreateTexture2D
	/// </summary>
	/// <param name="oCreateTexture2D"></param>
	/// <param name="device"></param>
	/// <param name="pDesc"></param>
	/// <param name="pInitialData"></param>
	/// <param name="ppTexture2D"></param>
	/// <returns></returns>
	HRESULT CreateTexture2DHook(ets2dc_dx11hook::CreateTexture2D oCreateTexture2D, ID3D11Device* device, 
		D3D11_TEXTURE2D_DESC* pDesc, D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D);

	/// <summary>
	/// Called before DX11 ClearDepthStencilView
	/// </summary>
	/// <param name="oClearDepthStencilView"></param>
	/// <param name="deviceContext"></param>
	/// <param name="pDepthStencilView"></param>
	/// <param name="ClearFlags"></param>
	/// <param name="Depth"></param>
	/// <param name="Stencil"></param>
	void ClearDepthStencilViewHook(ets2dc_dx11hook::ClearDepthStencilView oClearDepthStencilView, ID3D11DeviceContext* deviceContext, 
		ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil);

	/// <summary>
	/// Called when a render view is cleared
	/// </summary>
	/// <param name="oClearRenderTargetView"></param>
	/// <param name="deviceContext"></param>
	/// <param name="pRenderTargetView"></param>
	/// <param name=""></param>
	void ClearRenderTargetViewHook(ets2dc_dx11hook::ClearRenderTargetView oClearRenderTargetView, ID3D11DeviceContext* deviceContext,
		ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4]);
 };

