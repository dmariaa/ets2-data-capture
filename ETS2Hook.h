#pragma once
#include "globals.h"
#include "readerwriterqueue/readerwriterqueue.h"
#include <sstream>

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
	// Pointers to useful objects
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pDeviceContext = nullptr;	
	ID3D11Texture2D* pScreenshotTexture = nullptr;
	ID3D11Texture2D* pDepthTexture = nullptr;
	HWND outputWindow;

	bool simulate;
	bool inputCaptured;
	bool startCapture;
	std::atomic<bool> capturing;
	snapshot_stats stats;	

	// Configuration vars
	int consecutiveFramesCapture;
	int secondsBetweenCaptures; 	
	bool captureDepth;
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
	HRESULT PresentHook(Present oPresent, IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags);

	/// <summary>
	/// Called before draw
	/// </summary>
	/// <param name="VertexCount"></param>
	/// <param name="StartVertexLocation"></param>
	void DrawHook(Draw oDraw, ID3D11DeviceContext* context, UINT VertexCount, UINT StartVertexLocation);

	HRESULT CreateTexture2DHook(CreateTexture2D oCreateTexture2D, ID3D11Device* device, D3D11_TEXTURE2D_DESC* pDesc, D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D);
};

