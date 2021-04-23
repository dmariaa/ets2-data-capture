#include "dinput8_hook.h"
#include <Windows.h>
#include <assert.h>
#include <iostream>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "minhook/include/MinHook.h"

#ifdef _UNICODE
# define DINPUT8_HOOK_TEXT(text) L##text
#else
# define DINPUT8_HOOK_TEXT(text) text
#endif

#define DINPUT8_HOOK_ARRAY_SIZE(arr) ((size_t)(sizeof(arr)/sizeof(arr[0])))

static uint150_t* g_methodsTable = NULL;
static bool initialized = false;

dinput8_hook::Status::Enum dinput8_hook::init()
{
	if (initialized) {
		return Status::AlreadyInitializedError;
	}

	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	IDirectInput8* pDirectInput = nullptr;
	if (DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&pDirectInput, NULL) != DI_OK) {
		return Status::UnknownError;
	}

	LPDIRECTINPUTDEVICE8 lpdiMouse;
	if (pDirectInput->CreateDevice(GUID_SysMouse, &lpdiMouse, NULL) != DI_OK) {
		pDirectInput->Release();
		return Status::UnknownError;
	}

	//WNDCLASSEX windowClass;
	//windowClass.cbSize = sizeof(WNDCLASSEX);
	//windowClass.style = CS_HREDRAW | CS_VREDRAW;
	//windowClass.lpfnWndProc = DefWindowProc;
	//windowClass.cbClsExtra = 0;
	//windowClass.cbWndExtra = 0;
	//windowClass.hInstance = GetModuleHandle(NULL);
	//windowClass.hIcon = NULL;
	//windowClass.hCursor = NULL;
	//windowClass.hbrBackground = NULL;
	//windowClass.lpszMenuName = NULL;
	//windowClass.lpszClassName = DINPUT8_HOOK_TEXT("Dinput8Hook");
	//windowClass.hIconSm = NULL;

	//::RegisterClassEx(&windowClass);

	//HWND window = ::CreateWindow(windowClass.lpszClassName, DINPUT8_HOOK_TEXT("Dinput8Hook DirectX Window"), WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

	//HMODULE libDINPUT8;
	//if ((libDINPUT8 = ::GetModuleHandle(DINPUT8_HOOK_TEXT("dinput8.dll"))) == NULL)
	//{
	//	::DestroyWindow(window);
	//	::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
	//	return Status::ModuleNotFoundError;
	//}

	//void* DirectInput8Create;
	//if ((DirectInput8Create = ::GetProcAddress(libDINPUT8, "DirectInput8Create")) == NULL) {
	//	::DestroyWindow(window);
	//	::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
	//	return Status::UnknownError;
	//}

	//std::cout << "DirectInput8Create address: " << std::hex << (uint150_t)DirectInput8Create << std::endl;

	//IDirectInput8* directInput8;
	//HRESULT hr = ((HRESULT(__stdcall*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN))(DirectInput8Create))(windowClass.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&directInput8, NULL);
	//if ( hr != DI_OK)
	//{
	//	::DestroyWindow(window);
	//	::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
	//	return Status::UnknownError;
	//}

	//LPDIRECTINPUTDEVICE8 lpdiMouse;
	//if (directInput8->CreateDevice(GUID_SysMouse, &lpdiMouse, NULL) != DI_OK) {
	//	directInput8->Release();
	//	::DestroyWindow(window);
	//	::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
	//	return Status::UnknownError;
	//}

	DIDEVICEINSTANCE deviceInstance;
	deviceInstance.dwSize = sizeof(deviceInstance);
	lpdiMouse->GetDeviceInfo(&deviceInstance);

	std::cout << "------------------------------" << std::endl;
	std::wcout << "Device: " << deviceInstance.tszInstanceName << std::endl;
	std::cout << "Device pointer: " << std::hex << lpdiMouse << std::endl;
	std::cout << "VTable beginning: " << std::hex << *((uint150_t**)lpdiMouse) << std::endl;
	std::cout << "GetDeviceState beginning: " << std::hex << *(uint150_t *)(*(uint150_t*)lpdiMouse + 9 * sizeof(uint150_t)) << std::endl;
	std::cout << "------------------------------" << std::endl;

	g_methodsTable = (uint150_t*)::calloc(32, sizeof(uint150_t));
	
	if (nullptr != g_methodsTable) {
		::memcpy(g_methodsTable, *(uint150_t**)lpdiMouse, 32 * sizeof(uint150_t));

		const char* const methodsNames[] = { "QueryInterface","AddRef","Release","GetCapabilities","EnumObjects","GetProperty","SetProperty","Acquire",
		"Unacquire","GetDeviceState","GetDeviceData","SetDataFormat","SetEventNotification","SetCooperativeLevel","GetObjectInfo","GetDeviceInfo","RunControlPanel",
		"Initialize","CreateEffect","EnumEffects","GetEffectInfo","GetForceFeedbackState","SendForceFeedbackCommand","EnumCreatedEffectObjects","Escape","Poll",
		"SendDeviceData","EnumEffectsInFile","WriteEffectToFile","BuildActionMap","SetActionMap","GetImageInfo" };

		for (int i = 0; i < 32; i++)
		{
			std::cout << std::hex << "Address of " << methodsNames[i] << ": " << g_methodsTable[i] << std::endl;
		}

		MH_Initialize();

		initialized = true;

		return Status::Success;
	}

	/*
	lpdiMouse->Release();
	lpdiMouse = NULL;

	directInput8->Release();
	directInput8 = NULL;
	*/

	//::DestroyWindow(window);
	//::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

	return Status::UnknownError;
}

void dinput8_hook::shutdown()
{
	MH_DisableHook(MH_ALL_HOOKS);

	::free(g_methodsTable);
	g_methodsTable = NULL;
}

dinput8_hook::Status::Enum dinput8_hook::bind(uint16_t _index, void** _original, void* _function)
{
	assert(_original != NULL && _function != NULL);

	if (initialized) {
		void* target = (void*)g_methodsTable[_index];
		if (MH_CreateHook(target, _function, _original) != MH_OK || MH_EnableHook(target) != MH_OK) {
			return Status::UnknownError;
		}

		return Status::Success;
	}

	return Status::NotInitializedError;
}

void dinput8_hook::unbind(uint16_t _index)
{
	if (initialized) {
		MH_DisableHook((void*)g_methodsTable[_index]);
	}
}

uint150_t* dinput8_hook::getMethodsTable()
{
	return g_methodsTable;
}

bool dinput8_hook::isInitialized()
{
	return initialized;
}


