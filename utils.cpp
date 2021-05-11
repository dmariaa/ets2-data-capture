#include "utils.h"

namespace ets2dc_utils {

	namespace 
	{
		constexpr wchar_t PATH_SEPARATOR[] = L"\\";
		constexpr wchar_t appFolder[] = L"ETS2DataCapture";
	}

	int logLevelFromString(std::string logLevel)
	{
		auto it = std::find(logLevels.begin(), logLevels.end(), logLevel);
		if (it == logLevels.end()) return -1;
		return (int)std::distance(logLevels.begin(), it);
	}

	std::wstring getDocumentsFolder()
	{
		PWSTR documentsPath;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &documentsPath);
		std::wstring dp(documentsPath);
		CoTaskMemFree(documentsPath);
		return dp;
	}

	std::wstring getProjectDataFolder()
	{
		std::wstring projectData = getDocumentsFolder() + PATH_SEPARATOR + appFolder + PATH_SEPARATOR;
		if (!PathIsDirectoryW(projectData.c_str()))
		{
			CreateDirectory(projectData.c_str(), NULL);
		}
		return projectData;
	}

	const wchar_t* GetErrorDesc(HRESULT hr)
	{
		static wchar_t desc[1024] = {};

		LPWSTR errorText = nullptr;

		DWORD result = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr,
			static_cast<DWORD>(hr),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&errorText), 0, nullptr);

		*desc = 0;

		if (result > 0 && errorText)
		{
			swprintf_s(desc, L": %ls", errorText);

			size_t len = wcslen(desc);
			if (len >= 2)
			{
				desc[len - 2] = 0;
				desc[len - 1] = 0;
			}

			if (errorText)
				LocalFree(errorText);
		}

		return desc;
	}

	const std::wstring DumptTextureData(ID3D11Texture2D* texture)
	{
		assert(texture != nullptr);

		std::wstringstream s(L"");

		D3D11_TEXTURE2D_DESC pDesc;
		texture->GetDesc(&pDesc);

		s << "---------------------------------------------------" << std::endl
			<< "Texture: " << std::hex << texture << std::endl
			<< "  width: " << std::dec << pDesc.Width << std::endl
			<< "  height: " << pDesc.Height << std::endl
			<< "  usage: " << pDesc.Usage << std::endl
			<< "  format: " << pDesc.Format << std::endl
			<< "  binding: " << pDesc.BindFlags << std::endl
			<< "  array size: " << pDesc.ArraySize << std::endl
			<< "  cpu access flags: " << pDesc.CPUAccessFlags << std::endl
			<< "  mip levels: " << pDesc.MipLevels << std::endl
			<< "----------------------------------------------------";

		return s.str();
	}
}
