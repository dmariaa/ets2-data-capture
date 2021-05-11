#include "ets2dc_telemetry.h"

#include <plog/Log.h>

bool ets2dc_telemetry::output_paused = true;

SCSAPI_RESULT ets2dc_telemetry::init(const scs_u32_t version, const scs_telemetry_init_params_t* const params)
{
    if (version != SCS_TELEMETRY_VERSION_1_00) {
        PLOGI << "Telemetry version not supported";
        return SCS_RESULT_unsupported;
    }

    const scs_telemetry_init_params_v100_t* const version_params = static_cast<const scs_telemetry_init_params_v100_t*>(params);
    PLOGI.printf("Game '%s' %u.%u", version_params->common.game_id, SCS_GET_MAJOR_VERSION(version_params->common.game_version), SCS_GET_MINOR_VERSION(version_params->common.game_version));

	if (strcmp(version_params->common.game_id, SCS_GAME_ID_EUT2) == 0) {

		// Below the minimum version there might be some missing features (only minor change) or
		// incompatible values (major change).

		const scs_u32_t MINIMAL_VERSION = SCS_TELEMETRY_EUT2_GAME_VERSION_1_00;
		if (version_params->common.game_version < MINIMAL_VERSION) {
			PLOGI << "WARNING: Too old version of the game, some features might behave incorrectly";
		}

		// Future versions are fine as long the major version is not changed.

		const scs_u32_t IMPLEMENTED_VERSION = SCS_TELEMETRY_EUT2_GAME_VERSION_CURRENT;
		if (SCS_GET_MAJOR_VERSION(version_params->common.game_version) > SCS_GET_MAJOR_VERSION(IMPLEMENTED_VERSION)) {
			PLOGI << "WARNING: Too new major version of the game, some features might behave incorrectly";
		}
	}
	else if (strcmp(version_params->common.game_id, SCS_GAME_ID_ATS) == 0) {

		// Below the minimum version there might be some missing features (only minor change) or
		// incompatible values (major change).

		const scs_u32_t MINIMAL_VERSION = SCS_TELEMETRY_ATS_GAME_VERSION_1_00;
		if (version_params->common.game_version < MINIMAL_VERSION) {
			PLOGI << "WARNING: Too old version of the game, some features might behave incorrectly";
		}

		// Future versions are fine as long the major version is not changed.

		const scs_u32_t IMPLEMENTED_VERSION = SCS_TELEMETRY_ATS_GAME_VERSION_CURRENT;
		if (SCS_GET_MAJOR_VERSION(version_params->common.game_version) > SCS_GET_MAJOR_VERSION(IMPLEMENTED_VERSION)) {
			PLOGI << "WARNING: Too new major version of the game, some features might behave incorrectly";
		}
	}
	else {
		PLOGI << "WARNING: Unsupported game, some features or values might behave incorrectly";
	}

	const bool events_registered =
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_start, ets2dc_telemetry::telemetry_frame_start, NULL) == SCS_RESULT_ok) &&
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_end, ets2dc_telemetry::telemetry_frame_end, NULL) == SCS_RESULT_ok) &&
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_paused, ets2dc_telemetry::telemetry_pause, NULL) == SCS_RESULT_ok) &&
		(version_params->register_for_event(SCS_TELEMETRY_EVENT_started, ets2dc_telemetry::telemetry_pause, NULL) == SCS_RESULT_ok)
		;
	if (!events_registered) {

		// Registrations created by unsuccessfull initialization are
		// cleared automatically so we can simply exit.

		version_params->common.log(SCS_LOG_TYPE_error, "Unable to register event callbacks");
		return SCS_RESULT_generic_error;
	}

	return SCS_RESULT_ok;
}

SCSAPI_VOID ets2dc_telemetry::telemetry_frame_start(const scs_event_t UNUSED(event), const void* const event_info, const scs_context_t UNUSED(context))
{
	// PLOGV << "Telemetry frame start called ";
}

SCSAPI_VOID ets2dc_telemetry::telemetry_frame_end(const scs_event_t UNUSED(event), const void* const UNUSED(event_info), const scs_context_t UNUSED(context))
{
	// PLOGV << "Telemetry frame end called";
}

SCSAPI_VOID ets2dc_telemetry::telemetry_pause(const scs_event_t event, const void* const UNUSED(event_info), const scs_context_t UNUSED(context))
{
	ets2dc_telemetry::output_paused = (event == SCS_TELEMETRY_EVENT_paused);

	if (ets2dc_telemetry::output_paused) {
		PLOGD << "Telemetry paused";
	}
	else {
		PLOGD << "Telemetry unpaused";
	}
}

