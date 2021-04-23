#pragma once
#include "utils.h"

#include <string>
#include <map>

/// <summary>
/// Configuration manager
/// TODO: implement batch saving, now it saves to disk every time a variable is set,
/// TODO: make mor reusable
/// </summary>
namespace ets2dc_config {
	/// <summary>
	/// Initialize configuration manager
	/// </summary>
	/// <param name="path">Path to the config file, defaults to user_documents file\\project name</param>
	/// <param name="file">File name</param>
	void init(std::wstring path = ets2dc_utils::getProjectDataFolder(), std::wstring file = L"\\ETS2DataCapture.conf");

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
}