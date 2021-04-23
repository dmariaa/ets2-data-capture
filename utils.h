#pragma once
#include <cstdio>
#include <string>
#include <vector>

#include <ShlObj_core.h>
#include <Shlwapi.h>

/// <summary>
/// Configuration keys
/// TODO: Also normalize configuration default values?
/// </summary>
namespace ets2dc_config_keys {
	constexpr const char* consecutive_frames = "ConsecutiveFrames";
	constexpr const char* image_file_name = "ImageFileName";
	constexpr const char* image_folder = "ImageFolder";
	constexpr const char* image_file_format = "ImagefileFormat";
	constexpr const char* log_level = "LogLevel";
	constexpr const char* seconds_between_captures = "SecondsBetweenCaptures";
}

/// <summary>
/// Some util functions
/// </summary>
namespace ets2dc_utils
{
	constexpr wchar_t PATH_SEPARATOR[] = L"\\";
	constexpr wchar_t appFolder[] = L"ETS2DataCapture";

	static std::vector<std::string> logLevels = { "none", "fatal", "error", "warning", "info", "debug", "verbose" };

	/// <summary>
	/// Gets log level as integer from string
	/// Valid values none, fatal, error, warning, info, debug, verbose
	/// </summary>
	/// <param name="logLevel">One of valid values</param>
	/// <returns>log level as integer</returns>
	inline int logLevelFromString(std::string logLevel)
	{
		auto it = std::find(logLevels.begin(), logLevels.end(), logLevel);
		if (it == logLevels.end()) return -1;
		return (int)std::distance(logLevels.begin(), it);
	}

	/// <summary>
	/// Returns windows user Documents folder
	/// </summary>
	/// <returns>Windows User Documents path</returns>
	inline std::wstring getDocumentsFolder()
	{
		PWSTR documentsPath;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &documentsPath);
		std::wstring dp(documentsPath);
		CoTaskMemFree(documentsPath);
		return dp;
	}

	/// <summary>
	/// Returns project data folder, as a string.
	/// Creates folder if needed.
	/// </summary>
	/// <returns>Project data folder</returns>
	inline std::wstring getProjectDataFolder()
	{
		std::wstring projectData = getDocumentsFolder() + PATH_SEPARATOR + appFolder + PATH_SEPARATOR;
		if(!PathIsDirectoryW(projectData.c_str()))
		{
			CreateDirectory(projectData.c_str(), NULL);
		}
		return projectData;
	}
}