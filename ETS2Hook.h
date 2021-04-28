#pragma once
#include "globals.h"

#include "readerwriterqueue/readerwriterqueue.h"

#include <sstream>

class Config;
class AppSettings;

class SnapshotData 
{
public:
	DirectX::ScratchImage* image;
	snapshot_stats frame_stats;	
	std::wstring fileName;

	~SnapshotData()
	{
		image->Release();
	}
};

/// <summary>
/// Hook class for Euro Truck Simulator 2
/// </summary>
class ETS2Hook
{
	static const wchar_t* fmtFile;

	// Pointers to useful objects
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;	
	ID3D11Texture2D* pScreenshotTexture;
	HWND outputWindow;

	bool simulate;
	bool inputCaptured;
	std::atomic<bool> capturing;
	snapshot_stats stats;	


	// Configuration vars
	int consecutiveFramesCapture;
	int secondsBetweenCaptures; 	
	std::wstring imageFolder;
	std::wstring imageFileFormat;
	std::wstring imageFileName;	
	std::wstring formatString;

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
	HRESULT TakeScreenshot1(IDXGISwapChain* swapChain, const wchar_t* fileName);
	HRESULT TakeScreenshot2(IDXGISwapChain* swapChain, const wchar_t* fileName);

	// Settings change update
	void updateSettings();
	AppSettings* appSettings;

	// Writing thread
	moodycamel::ReaderWriterQueue<SnapshotData*> queue;
	std::atomic_bool captureThreadRunning;
	void saveBuffer();

public:
	ETS2Hook();	
	~ETS2Hook();

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
	HRESULT Present(IDXGISwapChain* swapChain);	
};

