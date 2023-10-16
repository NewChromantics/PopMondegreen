#pragma once

/*
	gr: this header should be C
	it probably isn't strict C at the moment, but no classes, namespaces etc.
*/

//	constant for invalid instance numbers, to avoid use of magic-number 0 around code bases
enum { PopMondegreen_NullInstance=0 };

#include <stdbool.h>
#include <stdint.h>

#if !defined(__export)

#if defined(_MSC_VER)
#define __export			extern "C" __declspec(dllexport)
#else
#define __export			extern "C"
#endif

#endif


__export int32_t			PopMondegreen_GetVersionThousand();	//	1.23.456 = 100230456


//	All options are optional
//	returns an instance id. 0/PopMondegreen_NullInstance on error.
__export int32_t			PopMondegreen_CreateInstance(const char* OptionsJson, char* ErrorBuffer, int32_t ErrorBufferSize);
__export void				PopMondegreen_FreeInstance(int32_t Instance);


