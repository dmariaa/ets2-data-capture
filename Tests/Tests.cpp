#include "pch.h"
#include "CppUnitTest.h"

#include <string>
#include <Shlwapi.h>
#pragma comment (lib, "shlwapi.lib")

#include "../config.h"
// #pragma comment (lib, "config.obj")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(TestConfig)
	{
	public:
		// Config* config = new Config();
		std::wstring file = L"temp_config.cfg";

		TEST_METHOD_INITIALIZE(init)
		{
			WCHAR tempPath[MAX_PATH];
			GetTempPath(MAX_PATH, tempPath);
			ets2dc_config::init(tempPath, file);
		}

		TEST_METHOD_CLEANUP(clean)
		{
			WCHAR tempPath[MAX_PATH];
			GetTempPath(MAX_PATH, tempPath);
			std::wstring configFile = tempPath + file;
			DeleteFile(configFile.c_str());
		}

		TEST_METHOD(TestNonExistingKey)
		{
			Assert::AreEqual(ets2dc_config::get("TestKey", "Default Value").c_str(), "Default Value");
		}

		TEST_METHOD(TestCreateStringKey)
		{
			ets2dc_config::set("TestKey", "Test Value");			
			Assert::AreEqual(ets2dc_config::get("TestKey", "Default Value").c_str(), "Test Value");			
		}

		TEST_METHOD(TestCreateWStringKey)
		{
			ets2dc_config::set("TestKeyWstring", L"Test Value");
			Assert::AreEqual(ets2dc_config::get("TestKeyWstring", L"Default Value").c_str(), L"Test Value");
		}

		TEST_METHOD(TestCreateIntKey)
		{
			ets2dc_config::set("TestKeyInt", 123);
			Assert::AreEqual(ets2dc_config::get("TestKeyInt", 0), 123);
		}

		TEST_METHOD(TestCreateFloatKey)
		{
			ets2dc_config::set("TestKeyFloat", 123.0f);
			Assert::AreEqual(ets2dc_config::get("TestKeyFloat", 0.f), 123.0f);
		}

		TEST_METHOD(TestCreateDoubleKey)
		{
			double d = 123.0;
			ets2dc_config::set("TestKeyDouble", d);
			Assert::AreEqual(ets2dc_config::get("TestKeyDouble", 0.0), d);
		}

		TEST_METHOD(TestLoadSave)
		{
			ets2dc_config::set("TestKey", "Test Value");
			ets2dc_config::set("#TestKey2", "Test Value 2");
			ets2dc_config::set("TestKey3", "Test Value 3");

			Assert::AreEqual(ets2dc_config::get("TestKey", "Default Value").c_str(), "Test Value");
			Assert::AreEqual(ets2dc_config::get("TestKey2", "Default Value").c_str(), "Default Value");
			Assert::AreEqual(ets2dc_config::get("TestKey3", "Default Value").c_str(), "Test Value 3");
		}
	};
}
