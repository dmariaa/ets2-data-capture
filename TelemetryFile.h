#pragma once

#include "ets2dc_telemetry.h"

#define SEPARATOR ';'

class TelemetryFile
{
	std::wofstream file;
public:
	TelemetryFile(const wchar_t* fileName);
	~TelemetryFile();

	void Save(const std::wstring snapshot_name, ets2dc_telemetry::telemetry_state telemetry);
};

