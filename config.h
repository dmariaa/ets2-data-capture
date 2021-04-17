#pragma once

#include <map>
#include <sstream>
#include <fstream>
#include <regex>
#include <string>
#include <Windows.h>
#include <winerror.h>
#include <ShlObj_core.h>
#include <Shlwapi.h>

#pragma comment (lib, "shlwapi.lib")
#pragma warning( disable: 4984 )

#define APP_DIR L"\\ETS2DataCapture\\"
#define CONFIG_FILE L"ETS2DataCapture.conf"
#define CONFIG_REGEX "^([^=]*)=(.*)$"

class Config {
	std::wstring file_name;
	std::wstring file_path;
	std::map<std::string, std::string> config_options;

	void initDefaultPaths() {
		if (file_path.empty() || !PathIsDirectoryW(file_path.c_str())) {
			PWSTR documentsPath;
			HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &documentsPath);
			file_path = documentsPath;
			file_path += APP_DIR;
			CreateDirectory(file_path.c_str(), NULL);
			CoTaskMemFree(documentsPath);
		}

		file_name = CONFIG_FILE;
	}

public:
	Config() {
		initDefaultPaths();
	}

	Config(std::wstring path) : file_path(path) {
		initDefaultPaths();
	}

	HRESULT load() {
		std::ifstream ifs;
		ifs.open(file_path + file_name, std::ifstream::in);

		std::regex expression(CONFIG_REGEX);
		std::cmatch cm;

		for (std::string line; std::getline(ifs, line); ) {
			std::string id, val;
			bool error = true;

			if (std::regex_match(line.c_str(), cm, expression)) {
				if (cm.size() == 3) {
					id = cm[1];
					val = cm[2];
					error = false;
				}
			}

			if (error) continue;
			if (id[0] == '#') continue;

			config_options[id] = val;
		}

		ifs.close();
		return S_OK;
	}

	HRESULT save() {
		std::ofstream ofs;
		ofs.open(file_path + file_name, std::ofstream::out | std::ofstream::trunc);

		for (const auto& config_option : config_options) {
			ofs << config_option.first << "=" << config_option.second << std::endl;
		}

		ofs.close();
		return S_OK;
	}

	template<typename T> 
	T get(std::string key, const T &default_value)
	{
		auto config_option = config_options.find(key);

		if (config_option == config_options.end()) {
			return default_value;
		}

		if constexpr(std::is_integral<T>::value) {
			return atoi(config_option->second.c_str());
		}
		else if constexpr(std::is_floating_point<T>::value) {
			return atof(config_option->second.c_str());
		}
		else if constexpr(std::is_base_of<std::string, T>::value) {
			return config_option->second;
		}
		else if constexpr (std::is_base_of<std::wstring, T>::value) {
			std::wstring w(config_option->second.begin(), config_option->second.end());
			return w;
		}

		return default_value;
	}

	void set(std::string key, std::string value) 
	{
		config_options[key] = value;
	}
};