#pragma once
#include "utils.h"

#include <string>
#include <map>

/// <summary>
/// Configuration manager
/// </summary>
namespace ets2dc_config {
	/// <summary>
	/// Configuration keys
	/// TODO: Also normalize configuration default values?
	/// </summary>
	namespace keys {
		constexpr const char* consecutive_frames = "ConsecutiveFrames";
		constexpr const char* image_file_name = "ImageFileName";
		constexpr const char* image_folder = "ImageFolder";
		constexpr const char* image_file_format = "ImagefileFormat";
		constexpr const char* log_level = "LogLevel";
		constexpr const char* seconds_between_captures = "SecondsBetweenCaptures";
		constexpr const char* capture_depth = "CaptureDepth";
		constexpr const char* capture_telemtry = "CaptureTelemetry";
		constexpr const char* last_session = "LastSession";
	}

	namespace default_values {
		constexpr const int last_session = 0;
		constexpr const int consecutive_frames = 10;
		constexpr const int seconds_between_captures = 1;
		constexpr const wchar_t* image_file_name = L"capture-{frame:010d}";
		constexpr const wchar_t* image_folder = L"{projectFolder}\\data\\{year:04d}{month:02d}{day:02d}-{session:06d}\\";
		constexpr const wchar_t* image_file_format = L"bmp";
		constexpr const char* log_level = "debug";
		constexpr const bool capture_depth = true;
		constexpr const bool capture_telemtry = true;

	}

	/// <summary>
	/// Initialize configuration manager
	/// </summary>
	/// <param name="path">Path to the config file, defaults to user_documents file\\project name</param>
	/// <param name="file">File name</param>
	void init(std::wstring path = ets2dc_utils::GetProjectDataFolder(), std::wstring file = L"\\ETS2DataCapture.conf");

	/// <summary>
	/// Value setters
	/// </summary>
	void set(std::string, std::string value);
	void set(std::string, std::wstring value);
	void set(std::string, int value);
	void set(std::string, float value);
	void set(std::string, double value);

	/// <summary>
	/// Value getters
	/// </summary>
	std::string get(std::string key, std::string default_value);	
	std::wstring get(std::string key, std::wstring default_value);
	int get(std::string key, int default_value);
	float get(std::string key, float default_value);
	double get(std::string key, double default_value);

	/// <summary>
	/// Has configuration changed since last is_dirty read?
	/// Absolutly NOT thread safe, not even module safe
	/// first reader resets it
	/// </summary>
	bool is_dirty();

	/// <summary>
	/// Allow batch saving
	/// </summary>
	void set_auto_save(bool value);
	void save();

	void begin_save_session();
	void end_save_session();
}