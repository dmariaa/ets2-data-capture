// Needed to generate obj to be able to test module
#include "config.h"

#include <locale>
#include <codecvt>
#include <fstream>
#include <regex>
#include <winerror.h>
#include <ShlObj_core.h>
#include <Shlwapi.h>
#pragma comment (lib, "shlwapi.lib")
#pragma warning( disable: 4984 )

namespace ets2dc_config {
#pragma region this should be private to ets2dc_config namespace
	namespace {
		static constexpr char config_regex[] = "^([^=]*)=(.*)$";

		static bool initialized = false;
		static bool dirty = false;
		static std::wstring config_file;
		static std::map<std::string, std::string> config_options;

		void load()
		{
			config_options = std::map<std::string, std::string>();
			std::ifstream ifs;
			ifs.open(config_file, std::ifstream::in);

			std::regex expression(config_regex);
			std::cmatch cm;

			for (std::string line; std::getline(ifs, line); ) {
				std::string id;
				std::string val;
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
		}

		void save() {
			std::ofstream ofs;
			ofs.open(config_file, std::ofstream::out | std::ofstream::trunc);

			for (const auto& config_option : config_options) {
				ofs << config_option.first << "=" << config_option.second << std::endl;
			}

			ofs.close();
		}

		template<typename T> T get(std::string key, const T& default_value)
		{
			auto config_option = config_options.find(key);

			if (config_option == config_options.end()) {
				return default_value;
			}

			std::string value = config_option->second;

			if constexpr (std::is_integral<T>::value) {
				return std::stoi(value);
			}
			else if constexpr (std::is_same<float, T>::value) {
				return std::stof(value);
			}
			else if constexpr (std::is_same<double, T>::value) {
				return std::stod(value);
			}
			else if constexpr (std::is_base_of<std::string, T>::value) {
				return value;
			}
			else if constexpr (std::is_base_of<std::wstring, T>::value) {
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv1;
				// std::wstring w(config_option->second.begin(), config_option->second.end());
				std::wstring v = conv1.from_bytes(value.data());
				return v;
			}

			return default_value;
		}

		template<typename T> void set(std::string key, T value) {
			std::string string_value;
			if constexpr (std::is_same<int, T>::value ||
				std::is_same<float, T>::value ||
				std::is_same<double, T >::value) {

				string_value = std::to_string(value);
			}
			else if constexpr (std::is_base_of<std::string, T>::value) {
				string_value = value;
			}
			else if constexpr (std::is_base_of<std::wstring, T>::value) {
				std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
				string_value = conv1.to_bytes(value);
			}

			dirty = true;
			config_options[key] = string_value;
			save();
		}
	}
#pragma endregion

	void init(std::wstring path, std::wstring file) {
		config_file = path + file;
		load();
	}

	void set(std::string key, std::string value)
	{
		set<std::string>(key, value);
	}

	void set(std::string key, std::wstring value)
	{
		set<std::wstring>(key, value);
	}

	void set(std::string key, int value)
	{
		set<int>(key, value);
	}

	void set(std::string key, float value)
	{
		set<float>(key, value);
	}

	void set(std::string key, double value)
	{
		set<double>(key, value);
	}

	bool is_dirty()
	{
		bool d = dirty;
		dirty = false;
		return d;
	}

	std::string get(std::string key, std::string default_value)
	{
		return get<std::string>(key, default_value);
	}

	std::wstring get(std::string key, std::wstring default_value)
	{
		return get<std::wstring>(key, default_value);
	}

	int get(std::string key, int default_value)
	{
		return get<int>(key, default_value);
	}

	float get(std::string key, float default_value)
	{
		return get<float>(key, default_value);
	}

	double get(std::string key, double default_value)
	{
		return get<double>(key, default_value);
	}
}