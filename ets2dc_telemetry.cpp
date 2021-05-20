#include "ets2dc_telemetry.h"

#include <plog/Log.h>

bool ets2dc_telemetry::output_paused = true;
ets2dc_telemetry::telemetry_state ets2dc_telemetry::telemetry;

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

	// Register channels
#define register_channel(name, index, type, field) version_params->register_for_channel(SCS_TELEMETRY_##name, index, SCS_VALUE_TYPE_##type, SCS_TELEMETRY_CHANNEL_FLAG_no_value, telemetry_store_##type, &telemetry.field);

	register_channel(TRUCK_CHANNEL_world_placement, SCS_U32_NIL, fplacement, truck_placement);
	register_channel(TRUCK_CHANNEL_local_linear_velocity, SCS_U32_NIL, fvector, linear_valocity);
	register_channel(TRUCK_CHANNEL_local_angular_velocity, SCS_U32_NIL, fvector, angular_velocity);
	register_channel(TRUCK_CHANNEL_local_linear_acceleration, SCS_U32_NIL, fvector, linear_acceleration);
	register_channel(TRUCK_CHANNEL_local_angular_acceleration, SCS_U32_NIL, fvector, angular_acceleration);
	register_channel(CHANNEL_local_scale, SCS_U32_NIL, float, local_scale);

#undef register_channel

	return SCS_RESULT_ok;
}

SCSAPI_VOID ets2dc_telemetry::telemetry_frame_start(const scs_event_t UNUSED(event), const void* const event_info, const scs_context_t UNUSED(context))
{
	// PLOGV << "Telemetry frame start called ";
	const struct scs_telemetry_frame_start_t* const info = static_cast<const scs_telemetry_frame_start_t*>(event_info);	
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

SCSAPI_VOID ets2dc_telemetry::telemetry_store_fplacement(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context)
{
	assert(context);
	scs_value_fplacement_t* const storage = static_cast<scs_value_fplacement_t*>(context);

	if (value) {
		assert(value->type == SCS_VALUE_TYPE_fplacement);
		*storage = value->value_fplacement;
	}
	else {
		storage->position.x = 0.0;
		storage->position.y = 0.0;
		storage->position.z = 0.0;
		storage->orientation.heading = 0.0f;
		storage->orientation.pitch = 0.0f;
		storage->orientation.roll = 0.0f;
	}
}

SCSAPI_VOID ets2dc_telemetry::telemetry_store_dplacement(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context)
{
	assert(context);
	scs_value_dplacement_t* const storage = static_cast<scs_value_dplacement_t*>(context);

	if (value) {
		assert(value->type == SCS_VALUE_TYPE_dplacement);
		*storage = value->value_dplacement;
	}
	else {
		storage->position.x = 0.0;
		storage->position.y = 0.0;
		storage->position.z = 0.0;
		storage->orientation.heading = 0.0f;
		storage->orientation.pitch = 0.0f;
		storage->orientation.roll = 0.0f;
	}
}

SCSAPI_VOID ets2dc_telemetry::telemetry_store_float(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context)
{
	assert(context);
	scs_float_t* const storage = static_cast<scs_float_t*>(context);

	if (value) {
		assert(value->type == SCS_VALUE_TYPE_float);
		*storage = value->value_float.value;
	}
	else {
		*storage = 0.0f;
	}
}

SCSAPI_VOID ets2dc_telemetry::telemetry_store_s32(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context)
{
	assert(context);
	scs_s32_t* const storage = static_cast<scs_s32_t*>(context);

	if (value) {
		assert(value->type == SCS_VALUE_TYPE_s32);
		*storage = value->value_s32.value;
	}
	else {
		*storage = 0;
	}
}

SCSAPI_VOID ets2dc_telemetry::telemetry_store_orientation(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context)
{
	assert(context);
	scs_value_euler_t* const storage = static_cast<scs_value_euler_t*>(context);

	if (value) {
		assert(value->type == SCS_VALUE_TYPE_euler);
		*storage = value->value_euler;
	}
	else {
		storage->heading = 0.0f;
		storage->pitch = 0.0f;
		storage->roll = 0.0f;
	}
}

SCSAPI_VOID ets2dc_telemetry::telemetry_store_fvector(const scs_string_t name, const scs_u32_t index, const scs_value_t* const value, const scs_context_t context)
{
	assert(context);
	scs_value_fvector_t* const storage = static_cast<scs_value_fvector_t*>(context);

	if (value) {
		assert(value->type == SCS_VALUE_TYPE_fvector);
		*storage = value->value_fvector;
	}
	else {
		storage->x = 0.0f;
		storage->y = 0.0f;
		storage->z = 0.0f;
	}
}

