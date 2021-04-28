#include "ETS2Hook.h"

#include <wrl/client.h>
#include <wincodec.h>
#include <ShlObj_core.h>
#include <hidusage.h>
#include <strsafe.h>
#include <chrono>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include "config.h"
#include "ets2dc_imgui.h"
#include "ets2dc_telemetry.h"

#define MAX_CCH 256


using namespace DirectX;
using namespace Microsoft::WRL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const wchar_t* ETS2Hook::fmtFile = L"{imageFolder}\\{fileName}.{fileFormat}";
static const wchar_t* defaultFileName = L"snapshot-{unix-timestamp}-{snapshot}-{frame}";

WNDPROC ETS2Hook::ETS2_hWndProc = nullptr;
ETS2Hook* ETS2Hook::pThis = nullptr;

ETS2Hook::ETS2Hook() : capturing(false), inputCaptured(false), queue(100)
{
	imageFolder = ets2dc_config::get(ets2dc_config_keys::image_folder, L"images");
	imageFileFormat = ets2dc_config::get(ets2dc_config_keys::image_file_format, L"jpg");
	imageFileName = ets2dc_config::get(ets2dc_config_keys::image_file_name, L"file{date}{time}");
	consecutiveFramesCapture = ets2dc_config::get(ets2dc_config_keys::consecutive_frames, 3);
	secondsBetweenCaptures = ets2dc_config::get(ets2dc_config_keys::seconds_between_captures, 1);

	PLOGI << "Configuration -------------------------------";
	PLOGI << "Image folder: " << imageFolder;
	PLOGI << "Image file Format: " << imageFileFormat;
	PLOGI << "Image file Name: " << imageFileName;
	PLOGI << "Consecutive frames: " << consecutiveFramesCapture;
	PLOGI << "Seconds Between Captures: " << secondsBetweenCaptures;
	PLOGI << "----------------------------------------------";

	appSettings = new AppSettings();
}

ETS2Hook::~ETS2Hook()
{
	shutdownImGui();
	shutdownInput();

	delete appSettings;
}

HRESULT ETS2Hook::Init(IDXGISwapChain* pSwapChain)
{
	HRESULT hr = S_OK;

	if (pDevice == nullptr) {
		hr = pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice);

		if (SUCCEEDED(hr)) {
			pDevice->GetImmediateContext(&pDeviceContext);
			initImGui(pSwapChain, pDevice, pDeviceContext);
			initInput();

			// Test thread
			std::thread(&ETS2Hook::saveBuffer, this).detach();
		}
	}

	return hr;
}

#pragma region ImGui Stuff
void ETS2Hook::initImGui(IDXGISwapChain* pSwapChain, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	DXGI_SWAP_CHAIN_DESC desc;
	pSwapChain->GetDesc(&desc);
	outputWindow = desc.OutputWindow;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();	
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	io.Fonts->AddFontDefault();

	ImGui_ImplWin32_Init(desc.OutputWindow);
	ImGui_ImplDX11_Init(pDevice, pDeviceContext);

    ImGui::StyleColorsDark();
}


void ETS2Hook::renderImGui()
{
	// std::cout << "Drawing ImGui window" << std::endl;
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDrawCursor = true;
	io.WantCaptureMouse = true;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Here goes our own ImGui drawing code
	appSettings->Draw("ETS2DataCapture settings");
		
	if (ImGui::Begin("ETS2DataCapture settings")) {
		ImGui::SameLine();
		ImGui::Text("Total frames captured: %d", stats.total_frames);
		ImGui::End();
	}

	if (appSettings->hasChanged())
	{
		PLOGD << "Settings changed";
		updateSettings();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ETS2Hook::shutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
#pragma endregion

#pragma region Input handling stuff
void ETS2Hook::initInput()
{
	std::cout << "Hooking ETS2 Window Proc" << std::endl;
	pThis = this;
	ETS2_hWndProc = (WNDPROC)SetWindowLongPtr(outputWindow, GWLP_WNDPROC, (LONG_PTR)hWndProcWrapper);

	// mouseHookHandle = SetWindowsHookEx(WH_MOUSE_LL, mouseHookWrapper, NULL, 0);
}

void ETS2Hook::shutdownInput()
{
	std::cout << "Unhooking ETS2 window input" << std::endl;	
	SetWindowLongPtr(outputWindow, GWLP_WNDPROC, (LONG_PTR)ETS2_hWndProc);
	pThis = nullptr;

	// UnhookWindowsHookEx(mouseHookHandle);	
}

LRESULT CALLBACK ETS2Hook::hWndProcWrapper(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (pThis != nullptr) {
		return pThis->hWndProc(hWnd, uMsg, wParam, lParam);
	}
	else if (ETS2_hWndProc != nullptr) {
		return CallWindowProc(ETS2_hWndProc, hWnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);

}

LRESULT CALLBACK ETS2Hook::hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_KEYDOWN: {
			if (wParam == 0xDC && GetKeyState(VK_CONTROL) & 0x8000) {
				std::wcout << "Setting drawEnabled to " << !imGuiDrawEnabled << std::endl;
				imGuiDrawEnabled = !imGuiDrawEnabled;
			}
			break;
		}
		case WM_INPUT: {
			HRAWINPUT rawInput = (HRAWINPUT)lParam;
			UINT dwSize = 0;
			if (GetRawInputData((HRAWINPUT)rawInput, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER)) == -1) {
				break;
			}

			LPBYTE lpb = new BYTE[dwSize];
			if (lpb == nullptr) {
				break;
			}

			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
				std::cout << "GetRawInputData does not return correct size !" << std::endl;
			}

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE) {
				if (raw->data.mouse.usButtonFlags)
					processRawMouse(raw->data.mouse);
				/*
				TCHAR szTempOutput[MAX_CCH];
				HRESULT hResult = StringCchPrintf(szTempOutput, MAX_CCH,
					TEXT("Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\r\n"),
					raw->data.mouse.usFlags,
					raw->data.mouse.ulButtons,
					raw->data.mouse.usButtonFlags,
					raw->data.mouse.usButtonData,
					raw->data.mouse.ulRawButtons,
					raw->data.mouse.lLastX,
					raw->data.mouse.lLastY,
					raw->data.mouse.ulExtraInformation);
				std::wcout << std::dec << szTempOutput << std::endl;
				*/
			}

			delete[]lpb;
			break;
		}
	}

	if (imGuiDrawEnabled) {
		return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);		
	}
	else {
		return CallWindowProc(ETS2_hWndProc, hWnd, uMsg, wParam, lParam);
	}	
}
#pragma endregion

#pragma region MouseHook stuff
LRESULT ETS2Hook::mouseHookWrapper(int nCode, WPARAM wParam, LPARAM lParam)
{
	return pThis->mouseHook(nCode, wParam, lParam);
}
LRESULT ETS2Hook::mouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0) {
		return CallNextHookEx(mouseHookHandle, nCode, wParam, lParam);
	}

	if (imGuiDrawEnabled) {
		std::cout << "mousehook called, imGuiDrawEnabled: " << imGuiDrawEnabled << std::endl;
		return 1;
	}

	return CallNextHookEx(mouseHookHandle, nCode, wParam, lParam);
}
#pragma endregion

#pragma region Raw Input stuff
void ETS2Hook::getRawMouseDevice()
{
	UINT numDevices;
	GetRegisteredRawInputDevices(NULL, &numDevices, sizeof(RAWINPUTDEVICE));

	RAWINPUTDEVICE* devices = new RAWINPUTDEVICE[numDevices];
	GetRegisteredRawInputDevices(devices, &numDevices, sizeof(RAWINPUTDEVICE));
	
	for (UINT i = 0; i < numDevices; i++) {
		if (devices[i].usUsagePage == HID_USAGE_PAGE_GENERIC && devices[i].usUsage == HID_USAGE_GENERIC_MOUSE) {
			std::cout << std::hex << "Mouse device found flags: " << devices[i].dwFlags << std::endl;
			ETS2RidMouse.usUsagePage = devices[i].usUsagePage;
			ETS2RidMouse.usUsage = devices[i].usUsage;
			ETS2RidMouse.dwFlags = devices[i].dwFlags;
			ETS2RidMouse.hwndTarget = devices[i].hwndTarget;
		}
	}

	delete[] devices;
}

void ETS2Hook::startRawInputCapture()
{
	if (inputCaptured) {
		// Already captured
		return;
	}

	if (ETS2RidMouse.usUsage != HID_USAGE_GENERIC_MOUSE) {
		getRawMouseDevice();
	}

	RAWINPUTDEVICE rid;
	rid.usUsagePage = HID_USAGE_PAGE_GENERIC;          // HID_USAGE_PAGE_GENERIC
	rid.usUsage = HID_USAGE_GENERIC_MOUSE;              // HID_USAGE_GENERIC_MOUSE
	rid.dwFlags = RIDEV_NOLEGACY;						// adds mouse and also ignores legacy mouse messages
	rid.hwndTarget = outputWindow;

	if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE) {
		wchar_t buf[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
		std::cout << buf << std::endl;
	}
	else {
		std::cout << "Raw input mouse device registered" << std::endl;
		inputCaptured = true;
	}
}

void ETS2Hook::endRawInputCapture()
{
	if (inputCaptured) {
		RegisterRawInputDevices(&ETS2RidMouse, 1, sizeof(ETS2RidMouse));
		inputCaptured = false;
	}	
}
void ETS2Hook::processRawMouse(RAWMOUSE rawMouse)
{
	ImGuiIO& io = ImGui::GetIO();

	switch (rawMouse.usButtonFlags) 
	{
		case RI_MOUSE_LEFT_BUTTON_DOWN:
			io.MouseDown[0] = true;
			break;
		case RI_MOUSE_RIGHT_BUTTON_DOWN:
			io.MouseDown[1] = true;
			break;
		case RI_MOUSE_MIDDLE_BUTTON_DOWN:
			io.MouseDown[2] = true;
			break;
		case RI_MOUSE_LEFT_BUTTON_UP:
			io.MouseDown[0] = false;
			break;
		case RI_MOUSE_RIGHT_BUTTON_UP:
			io.MouseDown[1] = false;
			break;
		case RI_MOUSE_MIDDLE_BUTTON_UP:
			io.MouseDown[2] = false;
			break;
		default:
			// Nothing to do
			break;
	}
}
#pragma endregion


/// <summary>
/// Takes a screenshot from the current swapChain
/// </summary>
/// <param name="pSwapChain"></param>
/// <param name="fileName">L"C:\\Users\\david\\Documents\\ETS2_CAPTURE\\captures\\screenshot.jpg"</param>
/// <returns></returns>
HRESULT ETS2Hook::TakeScreenshot1(IDXGISwapChain* pSwapChain, const wchar_t* fileName)
{
	ID3D11Texture2D* pScreenshotTexture;
	HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pScreenshotTexture);
	if (FAILED(hr) || pScreenshotTexture == NULL) 
	{
		PLOGE << "Error capturing buffer for screenshot to " << fileName;
	}
	else
	{
		SaveWICTextureToFile(pDeviceContext, pScreenshotTexture, GUID_ContainerFormatPng, fileName);
	}

	return hr;
}

HRESULT ETS2Hook::TakeScreenshot2(IDXGISwapChain* pSwapChain, const wchar_t* fileName)
{
	HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pScreenshotTexture);

	if (FAILED(hr) || pScreenshotTexture == NULL) {
		PLOGE << "Error capturing buffer for screenshot to " << fileName;
	}
	else
	{		
		SnapshotData* data = new SnapshotData();
		DirectX::ScratchImage* scratchImage = new DirectX::ScratchImage();
		DirectX::CaptureTexture(pDevice, pDeviceContext, pScreenshotTexture, *scratchImage);
		
		// const Image* image = img.GetImage(0, 0, 0);
		// DirectX::Blob blob;
		// DirectX::SaveToWICMemory(*image, WIC_FLAGS_NONE, GUID_ContainerFormatPng, blob, 
		// &GUID_WICPixelFormat24bppBGR);

		data->image = scratchImage;
		data->frame_stats = stats;
		data->fileName = std::wstring(fileName);
		queue.enqueue(data);

		// img.Release();

	}

	return hr;
}

void ETS2Hook::saveBuffer()
{
	SnapshotData* data;

	while (true) {
		if (capturing) {
			if (queue.try_dequeue(data)) {
				const Image* image = data->image->GetImage(0, 0, 0);
				DirectX::SaveToWICFile(*image, WIC_FLAGS_NONE, GUID_ContainerFormatPng, data->fileName.c_str(), &GUID_WICPixelFormat24bppBGR);
				PLOGD << "Dequeed frame: " << data->fileName;
				
				delete data;
			}
		}
	}
}


void ETS2Hook::updateSettings()
{
	consecutiveFramesCapture = appSettings->consecutiveFrames; 
	ets2dc_config::set(ets2dc_config_keys::consecutive_frames, consecutiveFramesCapture);
	secondsBetweenCaptures = appSettings->secondsBetweenSnapshots;
	ets2dc_config::set(ets2dc_config_keys::seconds_between_captures, secondsBetweenCaptures);	
	capturing = appSettings->isCapturing;

	plog::get()->setMaxSeverity(plog::Severity(appSettings->currentLogLevel));
	PLOGD << "Capturing set to: " << capturing;
}

HRESULT ETS2Hook::Present(IDXGISwapChain* swapChain)
{
	static std::chrono::high_resolution_clock::time_point timer = std::chrono::high_resolution_clock::now();
	static std::wstring fileName;

	// auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - timer);
	std::chrono::duration<float> delta = std::chrono::high_resolution_clock::now() - timer;

	if (capturing && !ets2dc_telemetry::output_paused) {
		if (delta.count() >= (1.0f / consecutiveFramesCapture)) {			
			stats.snapshot_time = std::chrono::system_clock::now();

			try {
				fileName = stats.formatted(imageFileName);
			}
			catch (const std::exception& e) {
				PLOGE << "Error trying to format file name variables: " << e.what();
				fileName = stats.formatted(defaultFileName);
			}

			std::wstring imageFile = fmt::format(fmtFile,
				fmt::arg(L"imageFolder", imageFolder),
				fmt::arg(L"fileName", fileName),
				fmt::arg(L"fileFormat", imageFileFormat));

			TakeScreenshot2(swapChain, imageFile.c_str());

			PLOGD << L"Saved ETS2 snapshot to " << fileName << " : " << delta.count();
			stats.frame++;
			stats.total_frames++;

			timer = std::chrono::high_resolution_clock::now();
		}		

		PLOGV << "Time elapsed: " << delta.count();
	}
	else {
		if (imGuiDrawEnabled) {
			startRawInputCapture();

			static time_t timer2 = clock();
			if ((clock() - timer2) / CLOCKS_PER_SEC >= 5) {
				PLOGV << "Rendering ImGui - output_paused: " << ets2dc_telemetry::output_paused;
				timer2 = clock();
			}

			renderImGui();
			stats.frame = 0;
		}
		else {
			endRawInputCapture();
		}
	}

	return S_OK;
}