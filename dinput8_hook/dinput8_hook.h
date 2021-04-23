#ifndef __DINPUT8_HOOK__
#define __DINPUT8_HOOK__

#include <stdint.h>

#define DINPUT8_HOOK_VERSION "0.1.0"

#define DINPUT8_HOOK_ARCH_X64 0
#define DINPUT8_HOOK_ARCH_X86 0

#if defined(_M_X64)	
# undef  DINPUT8_HOOK_ARCH_X64
# define DINPUT8_HOOK_ARCH_X64 1
#else
# undef  DINPUT8_HOOK_ARCH_X86
# define DINPUT8_HOOK_ARCH_X86 1
#endif

#if DINPUT8_HOOK_ARCH_X64
typedef uint64_t uint150_t;
#else
typedef uint32_t uint150_t;
#endif

namespace dinput8_hook
{
	struct Status
	{
		enum Enum
		{
			UnknownError = -1,
			NotSupportedError = -2,
			ModuleNotFoundError = -3,

			AlreadyInitializedError = -4,
			NotInitializedError = -5,

			Success = 0,
		};
	};

	Status::Enum init();
	void shutdown();

	Status::Enum bind(uint16_t index, void** original, void* function);
	void unbind(uint16_t index);

	uint150_t* getMethodsTable();
	bool isInitialized();
}

#endif // __DINPUT8_HOOK__