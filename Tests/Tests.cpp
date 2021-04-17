#include "pch.h"
#include "CppUnitTest.h"

#include "../ETS2DataCapture/config.cpp"

#pragma comment (lib, "config.obj")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(TestConfig)
	{
	public:
		Config* config = new Config();
		
		TEST_METHOD(TestNonExistingKey)
		{			
			Assert::AreEqual(config->get<std::string>("TestKey", "Default Value").c_str(), "Default Value");
		}

		TEST_METHOD(TestCreateKey)
		{
			config->set("TestKey", "Test Value");
			Assert::AreEqual(config->get<std::string>("TestKey", "Default Value").c_str(), "Test Value");
			config->set("TestKeyInt", "123");
			Assert::AreEqual(config->get<int>("TestKeyInt", 0), 123);
		}

		TEST_METHOD(TestLoadSave)
		{
			WCHAR tempPath[MAX_PATH];
			GetTempPath(MAX_PATH, tempPath);

			Config* config = new Config(tempPath);
			config->set("TestKey", "Test Value");
			config->set("#TestKey2", "Test Value 2");
			config->set("TestKey3", "Test Value 3");
			config->save();

			Config* config2 = new Config(tempPath);
			config2->load();
			Assert::AreEqual(config2->get<std::string>("TestKey", "Default Value").c_str(), "Test Value");
			// Assert::AreEqual(config2->get("TestKey2", "Default Value").c_str(), "Test Value 2");
			Assert::AreEqual(config2->get<std::string>("TestKey3", "Default Value").c_str(), "Test Value 3");
		}
	};
}
