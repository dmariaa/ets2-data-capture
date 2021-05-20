#include "ETS2Hook.h"

#include <wrl/client.h>
#include <wincodec.h>
#include <ShlObj_core.h>
#include <hidusage.h>
#include <strsafe.h>
#include <chrono>
#include <string>
#include <locale>
#include <codecvt>


#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include "config.h"
#include "ets2dc_imgui.h"
#include "ets2dc_telemetry.h"
#include "TelemetryFile.h"

#define MAX_CCH 256

using namespace Microsoft::WRL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WNDPROC ETS2Hook::ETS2_hWndProc = nullptr;
ETS2Hook* ETS2Hook::pThis = nullptr;

ETS2Hook::ETS2Hook() : Initialized(false), capturing(false), simulate(false), inputCaptured(false), queue(100), telemetryFile(nullptr)
{
	imageFolder = ets2dc_config::get(ets2dc_config::keys::image_folder, ets2dc_config::default_values::image_folder);
	imageFileFormat = ets2dc_config::get(ets2dc_config::keys::image_file_format, ets2dc_config::default_values::image_file_format);
	imageFileName = ets2dc_config::get(ets2dc_config::keys::image_file_name, ets2dc_config::default_values::image_file_name);
	consecutiveFramesCapture = ets2dc_config::get(ets2dc_config::keys::consecutive_frames, ets2dc_config::default_values::consecutive_frames);
	secondsBetweenCaptures = ets2dc_config::get(ets2dc_config::keys::seconds_between_captures, ets2dc_config::default_values::seconds_between_captures);
	stats.session = ets2dc_config::get(ets2dc_config::keys::last_session, ets2dc_config::default_values::last_session);

	PLOGI << "Configuration -------------------------------";
	PLOGI << "Image folder: " << imageFolder;
	PLOGI << "Image file Format: " << imageFileFormat;
	PLOGI << "Image file Name: " << imageFileName;
	PLOGI << "Consecutive frames: " << consecutiveFramesCapture;
	PLOGI << "Seconds Between Captures: " << secondsBetweenCaptures;
	PLOGI << "Last session recorded: " << stats.session;
	PLOGI << "----------------------------------------------";

	appSettings = new AppSettings();
}

ETS2Hook::~ETS2Hook()
{
	capturing = false;
	captureThreadRunning = false;

	CloseTelemetryFile();

	shutdownImGui();
	shutdownInput();

	pDeviceContext->Release();
	pDevice->Release();

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

			Initialized = true;
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
		ImGui::Separator();		
		ImGui::Text("Total frames captured since start: %d", stats.total_frames);
		ImGui::Text("Current session: %d", stats.session);
		ImGui::Text("Current frame: %d", stats.frame);
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

void ETS2Hook::updateSettings()
{
	consecutiveFramesCapture = appSettings->consecutiveFrames;
	secondsBetweenCaptures = appSettings->secondsBetweenSnapshots;
	captureDepth = appSettings->captureDepth;
	captureTelemetry = appSettings->captureTelemetry;
	simulate = appSettings->simulate;

	// Capturing has stopped, increment session
	if (!capturing && capturing != appSettings->isCapturing)
	{
		startCapture = true;
	}

	capturing = appSettings->isCapturing;
	PLOGD << "Capturing state set to: " << capturing;

	plog::get()->setMaxSeverity(plog::Severity(appSettings->currentLogLevel));
	PLOGI << "Log severity level set to: " << appSettings->currentLogLevel;
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

#pragma region Snapshotting stuff
/// <summary>
/// Takes a screenshot from the current swapChain
/// </summary>
/// <param name="pSwapChain"></param>
/// <param name="fileName">L"C:\\Users\\david\\Documents\\ETS2_CAPTURE\\captures\\screenshot.jpg"</param>
/// <returns></returns>
HRESULT ETS2Hook::TakeScreenshot1(IDXGISwapChain* pSwapChain)
{
	ID3D11Texture2D* pScreenshotTexture;
	HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pScreenshotTexture);
	std::wstring fileName = GenerateFileName();
	
	if (FAILED(hr) || pScreenshotTexture == NULL) 
	{
		PLOGE << "Error capturing buffer for screenshot to " << fileName;
	}
	else
	{
		std::string fname;
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
		fname = conv1.to_bytes(fileName);

		TextureBuffer* texture = new TextureBuffer(pDevice, pDeviceContext, pScreenshotTexture);
		texture->save(fname.c_str());
	}
		
	return hr;
}

/// <summary>
/// Takes a screenshot from the current SwapChain.
/// Uses a producer-consumer queue to save the file in background, different thread
/// </summary>
/// <param name="pSwapChain"></param>
/// <param name="fileName"></param>
/// <returns></returns>
HRESULT ETS2Hook::TakeScreenshot2()
{
	HRESULT hr = S_OK;
	bool capture = false;
	std::wstring file_name = GenerateFileName();

	SnapshotData* data = new SnapshotData();
	data->fileName = file_name;
	data->frame_stats = stats;

	if (!simulate) 
	{
		if (pScreenshotTexture != nullptr)
		{
			PLOGV << "Screenshot texture: \n" << ets2dc_utils::DumptTextureData(pScreenshotTexture);
			TextureBuffer* buffer = new TextureBuffer(pDevice, pDeviceContext, pScreenshotTexture);

			if (buffer->type == TextureBuffer::Type::ScreenShot)
			{
				data->image = buffer;
				capture = true;
			}
		}

		if (pDepthTexture != nullptr && captureDepth)
		{
			PLOGV << "Depth texture: \n" << ets2dc_utils::DumptTextureData(pDepthTexture);
			TextureBuffer* buffer = new TextureBuffer(pDevice, pDeviceContext, pDepthTexture);

			if (buffer->type == TextureBuffer::Type::Depth)
			{
				data->depth = buffer;
				capture = true;
			}
		}

		if (captureTelemetry)
		{
			// data->telemetry = new ets2dc_telemetry::telemetry_state(ets2dc_telemetry::telemetry);
			telemetryFile->Save(snapshotId, ets2dc_telemetry::telemetry);
			capture = true;
		}

		if (capture)
		{
			queue.enqueue(data);
		}
		else {
			hr = E_FAIL;
		}
	}
		
	PLOGD << L"Saved ETS2 snapshot to: " << data->fileName;
	PLOGD << "Saved telemetry data: " << ets2dc_telemetry::telemetry.truck_placement.position << ';' << ets2dc_telemetry::telemetry.truck_placement.orientation << '\n';
	return hr;
}

void ETS2Hook::saveBuffer()
{
	SnapshotData* data;
	HRESULT result;

	while (captureThreadRunning) {
		if (capturing) {
			if (queue.try_dequeue(data)) {
				std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
				std::string fname;
				fname = conv1.to_bytes(data->fileName);

				if (data->image != nullptr)
				{
					data->image->save(fname.c_str());
				}

				if (data->depth != nullptr)
				{
					data->depth->save(fname.c_str());
				}

				PLOGD << "Dequeed frame: " << data->fileName;
				delete data;
			}
		}
	}
}
#pragma endregion

#pragma region DirectX 11 Hooks
void ETS2Hook::DrawHook(Draw oDraw, ID3D11DeviceContext* context, UINT VertexCount, UINT StartVertexLocation)
{
	PLOGV << "Draw called";
	oDraw(context, VertexCount, StartVertexLocation);

	if (capturing && !ets2dc_telemetry::output_paused)
	{
		ID3D11RenderTargetView* renderTargetView = nullptr;
		ID3D11DepthStencilView* depthStencilView = nullptr;
		context->OMGetRenderTargets(1, &renderTargetView, &depthStencilView);

		if (depthStencilView != nullptr && renderTargetView != nullptr)
		{
			ID3D11Resource* res = nullptr;
			renderTargetView->GetResource(&res);

			if (res != nullptr)
			{
				pScreenshotTexture = nullptr;
				res->QueryInterface(&pScreenshotTexture);
				res->Release();
			}

			renderTargetView->Release();

			res = nullptr;
			depthStencilView->GetResource(&res);

			if (res != nullptr)
			{
				pDepthTexture = nullptr;
				res->QueryInterface(&pDepthTexture);
				res->Release();
			}

			depthStencilView->Release();
		}
	}
}

HRESULT ETS2Hook::CreateTexture2DHook(CreateTexture2D oCreateTexture2D, ID3D11Device* device, D3D11_TEXTURE2D_DESC* pDesc, D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
{
	PLOGV << "Create Texture 2D called";

	HRESULT result = oCreateTexture2D(device, pDesc, pInitialData, ppTexture2D);

	if (!FAILED(result) && *ppTexture2D != nullptr && pDesc->BindFlags == D3D11_BIND_DEPTH_STENCIL)
	{
		PLOGD << "---------------------------------------------------" << std::endl
			<< "Depth texture created " << std::endl
			<< ets2dc_utils::DumptTextureData(*ppTexture2D);
	}

	return result;
}

HRESULT ETS2Hook::PresentHook(Present oPresent, IDXGISwapChain* swapChain, UINT SyncInterval, UINT Flags)
{
	// Only needed in this function
	typedef std::chrono::high_resolution_clock clock;
	typedef std::chrono::duration<float> duration;

	static clock::time_point timer = clock::now();

	static float accumDelta = 0;

	duration delta = clock::now() - timer;

	HRESULT result;

	if (capturing && !ets2dc_telemetry::output_paused) {
		endRawInputCapture();

		if (startCapture)
		{
			stats.snapshot_time = std::chrono::system_clock::now();
			stats.session++;
			stats.frame = 0;			
			GenerateFileFormatString();
			InitTelemetryFile();

			startCapture = false;
		}

		accumDelta += delta.count();

		if (accumDelta >= (1.0f / consecutiveFramesCapture)) {
			stats.snapshot_time = std::chrono::system_clock::now();
			stats.frame++;
			stats.total_frames++;

			TakeScreenshot2();
			accumDelta = 0.0f;
		}

		PLOGV << "Time elapsed: " << accumDelta;

		result = oPresent(swapChain, SyncInterval, Flags);
	}
	else {
		accumDelta = 0.0f;

		if (imGuiDrawEnabled && ets2dc_telemetry::output_paused) {
			startRawInputCapture();
			renderImGui();
		}
		else {
			endRawInputCapture();
		}

		result = oPresent(swapChain, SyncInterval, Flags);
	}

	timer = std::chrono::high_resolution_clock::now();
	return result;
}
#pragma endregion

#pragma region Other usefull functions
void ETS2Hook::GenerateFileFormatString()
{
	std::wstring folderName = stats.formatted(imageFolder);	
	ets2dc_utils::CreateFolderIfNotExists(folderName);	
	formatString = folderName + imageFileName;
}

const std::wstring ETS2Hook::GenerateFileName()
{
	std::wstring fileName;

	try {
		fileName = stats.formatted(formatString);
		snapshotId = stats.formatted(imageFileName);
	}
	catch (const std::exception& e) {
		PLOGE << "Error trying to format file name variables: " << e.what();
		std::wstring name(ets2dc_config::default_values::image_folder);
		name += ets2dc_config::default_values::image_file_name;

		fileName = stats.formatted(name);
		snapshotId = stats.formatted(ets2dc_config::default_values::image_file_name);
	}

	return fileName;
}

void ETS2Hook::InitTelemetryFile()
{
	CloseTelemetryFile();
	std::wstring folderName = stats.formatted(imageFolder);
	std::wstring telemetryFileName = folderName + L"telemetry.txt";
	telemetryFile = new TelemetryFile(telemetryFileName.c_str());
}

void ETS2Hook::CloseTelemetryFile()
{
	delete telemetryFile;
}
#pragma endregion