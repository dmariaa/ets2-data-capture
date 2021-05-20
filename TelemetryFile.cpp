#include "globals.h"
#include "TelemetryFile.h"

TelemetryFile::TelemetryFile(const wchar_t* fileName)
{
	file.open(fileName);
}

TelemetryFile::~TelemetryFile()
{
	file.flush();
	file.close();
}

void TelemetryFile::Save(const std::wstring snapshot_name, ets2dc_telemetry::telemetry_state telemetry)
{
	file << snapshot_name << SEPARATOR
		 << telemetry.truck_placement.position << SEPARATOR
		 << telemetry.truck_placement.orientation << SEPARATOR
		 << telemetry.linear_valocity << SEPARATOR
		 << telemetry.angular_velocity << SEPARATOR
		 << telemetry.linear_acceleration << SEPARATOR
		 << telemetry.angular_acceleration << SEPARATOR 
		 << telemetry.local_scale << '\n';
}


