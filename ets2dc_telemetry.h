#pragma once

#include <iostream>
#include <fstream>

#include "scs_sdk_1_12/include/scssdk_telemetry.h"
#include "scs_sdk_1_12/include/eurotrucks2/scssdk_eut2.h"
#include "scs_sdk_1_12/include/eurotrucks2/scssdk_telemetry_eut2.h"
#include "scs_sdk_1_12/include/amtrucks/scssdk_ats.h"
#include "scs_sdk_1_12/include/amtrucks/scssdk_telemetry_ats.h"

#define UNUSED(x)

/// <summary>
/// Euro Truck Simulator and American Truck Simulator telemetry helper functions
/// </summary>
namespace ets2dc_telemetry {
	struct telemetry_state
	{
		scs_value_fplacement_t  truck_placement;
		scs_value_fvector_t		linear_valocity;		
		scs_value_fvector_t		angular_velocity;		
		scs_value_fvector_t		linear_acceleration;	
		scs_value_fvector_t		angular_acceleration;	
		scs_value_float_t		local_scale;
	};

	extern bool output_paused;
	extern telemetry_state telemetry;

	/// <summary>
	/// Telemetry functions initialization callback
	/// </summary>
	/// <param name="version"></param>
	/// <param name="params"></param>
	/// <returns></returns>
	SCSAPI_RESULT init(const scs_u32_t version, const scs_telemetry_init_params_t* const params);

	/// <summary>
	/// Telemetry frame start callback
	/// </summary>
	/// <param name="UNUSED"></param>
	/// <param name="event_info"></param>
	/// <param name="UNUSED"></param>
	/// <returns></returns>
	SCSAPI_VOID telemetry_frame_start(const scs_event_t UNUSED(event), const void* const event_info, const scs_context_t UNUSED(context));

	/// <summary>
	/// Telemetry frame end callback
	/// </summary>
	/// <param name="UNUSED"></param>
	/// <param name="UNUSED"></param>
	/// <param name="UNUSED"></param>
	/// <returns></returns>
	SCSAPI_VOID telemetry_frame_end(const scs_event_t UNUSED(event), const void* const UNUSED(event_info), const scs_context_t UNUSED(context));

	/// <summary>
	/// Called when game pause state (paused, menus, ...) changes
	/// </summary>
	/// <param name="event"></param>
	/// <param name="UNUSED"></param>
	/// <param name="UNUSED"></param>
	/// <returns></returns>
	SCSAPI_VOID telemetry_pause(const scs_event_t event, const void* const UNUSED(event_info), const scs_context_t UNUSED(context));

	SCSAPI_VOID telemetry_store_fplacement(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context);
	SCSAPI_VOID telemetry_store_dplacement(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context);
	SCSAPI_VOID telemetry_store_float(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context);
	SCSAPI_VOID telemetry_store_s32(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context);
	SCSAPI_VOID telemetry_store_orientation(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context);
	SCSAPI_VOID telemetry_store_fvector(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context);
}


// SCS vector stream operators
inline std::ostream& operator<<(std::ostream& os, scs_value_fvector_t v)
{
	return os << v.x << ';' << v.y << ';' << v.z;
}

inline std::wostream& operator<<(std::wostream& os, scs_value_fvector_t v)
{
	return os << v.x << ';' << v.y << ';' << v.z;
}

// SCS euler stream operators
inline std::ostream& operator<<(std::ostream& os, scs_value_euler_t v)
{
	return os << v.heading << ';' << v.heading << ';' << v.roll;
}

inline std::wostream& operator<<(std::wostream& os, scs_value_euler_t v)
{
	return os << v.heading << ';' << v.heading << ';' << v.roll;
}

// SCS float stream operators
inline std::ostream& operator<<(std::ostream& os, scs_value_float_t v)
{
	return os << v.value;
}

inline std::wostream& operator<<(std::wostream& os, scs_value_float_t v)
{
	return os << v.value;
}

