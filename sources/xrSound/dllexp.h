#pragma once

#ifdef XRSOUND_STATIC
#define XRSOUND_API
#elif defined XRSOUND_EXPORTS
#define XRSOUND_API __declspec(dllexport)
#else
#define XRSOUND_API __declspec(dllimport)
#endif
