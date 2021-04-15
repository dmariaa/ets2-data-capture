#ifndef __LOG_H__
#define __LOG_H__

#define UG_DEBUG 1
static int UG_DEBUG_INIT = 0;

#define DEBUG_INIT \
			do { if (UG_DEBUG && !UG_DEBUG_INIT) { \
			AllocConsole(); \
			freopen("CONOUT$", "w", stdout); \
			UG_DEBUG_INIT = 1; }} while (0)

#define DEBUG_PRINT(fmt, ...) \
            do { if (UG_DEBUG && UG_DEBUG_INIT) std::cout << fmt << std::endl; } while (0)

#include <exception>

namespace DX
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
            // throw std::exception();
            std::cout << std::system_category().message(hr) << std::endl;
        }
    }
}
#endif __LOG_H__