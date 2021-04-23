#pragma once

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
	extern bool output_paused;

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
}

