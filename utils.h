#pragma once
#include <cstdio>
#include <string>
#include <vector>
#include <chrono>
#include <date/date.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <ShlObj_core.h>
#include <Shlwapi.h>

#include <d3d11.h>

/// <summary>
/// Some util functions
/// </summary>
namespace ets2dc_utils
{
	static std::vector<std::string> logLevels = { "none", "fatal", "error", "warning", "info", "debug", "verbose" };

	/// <summary>
	/// Gets log level as integer from string
	/// Valid values none, fatal, error, warning, info, debug, verbose
	/// </summary>
	/// <param name="logLevel">One of valid values</param>
	/// <returns>log level as integer</returns>
	int LogLevelFromString(std::string logLevel);

	/// <summary>
	/// Creates a folder if it does not exist
	/// </summary>
	/// <param name="folder"></param>
	void CreateFolderIfNotExists(std::wstring folder);

	/// <summary>
	/// Returns windows user Documents folder
	/// </summary>
	/// <returns>Windows User Documents path</returns>
	std::wstring GetDocumentsFolder();

	/// <summary>
	/// Returns project data folder, as a string.
	/// Creates folder if needed.
	/// </summary>
	/// <returns>Project data folder</returns>
	std::wstring GetProjectDataFolder();

	/// <summary>
	/// Generates error description from hresult
	/// Extracted from https://github.com/microsoft/DirectXTex
	/// </summary>
	/// <param name="hr"></param>
	/// <returns></returns>
	const wchar_t* GetErrorDesc(HRESULT hr);

	/// <summary>
	/// Dumps texture data to string for debugging
	/// </summary>
	/// <param name="texture"></param>
	/// <returns></returns>
	const std::wstring DumptTextureData(ID3D11Texture2D* texture);
}

struct snapshot_stats {
	std::chrono::system_clock::time_point start;
	std::chrono::system_clock::time_point snapshot_time;

	unsigned session = 0;
	unsigned frame = 0;
	unsigned total_frames = 0;
	unsigned snapshot = 0;
	unsigned total_snapshots = 0;

	std::wstring documentsFolder;
	std::wstring projectFolder;

	std::wstring formatted(std::wstring format)
	{
		auto unixTime = std::chrono::duration_cast<std::chrono::milliseconds>(snapshot_time.time_since_epoch()).count();
		auto dp = date::floor<date::days>(snapshot_time);
		auto ymd = date::year_month_day{ dp };
		auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(snapshot_time - dp));

		return fmt::format(format,
			fmt::arg(L"session", session),
			fmt::arg(L"frame", frame),
			fmt::arg(L"snapshot", snapshot),
			fmt::arg(L"totalFramesCaptured", total_frames),
			fmt::arg(L"totalSnapshotsTaken", total_snapshots),
			fmt::arg(L"timestamp", unixTime),
			fmt::arg(L"year", (int)ymd.year()),
			fmt::arg(L"month", (unsigned)ymd.month()),
			fmt::arg(L"day", (unsigned)ymd.day()),
			fmt::arg(L"hour", time.hours().count()),
			fmt::arg(L"minute", time.minutes().count()),
			fmt::arg(L"second", time.seconds().count()),
			fmt::arg(L"millisecond", time.subseconds().count()),
			fmt::arg(L"documentsFolder", documentsFolder),
			fmt::arg(L"projectFolder", projectFolder));
	}

	snapshot_stats() {
		documentsFolder = ets2dc_utils::GetDocumentsFolder();
		projectFolder = ets2dc_utils::GetProjectDataFolder();
	}
};