#include "PopMondegreen.h"
#include "InstanceManager.hpp"
#include "Listener.hpp"
#include <iostream>


namespace Soy
{
	void	StringToBuffer(const char* Source,char* Buffer,size_t BufferSize);
}


namespace PopMondegreen
{
	InstanceManager_t<Listener_t>		ListenerInstanceManager;

	constexpr int	VersionMajor = VERSION_MAJOR;
	constexpr int	VersionMinor = VERSION_MINOR;
	constexpr int	VersionPatch = VERSION_PATCH;
}


__export int32_t PopMondegreen_GetVersionThousand()
{
	int32_t Version = 0;
	Version += PopMondegreen::VersionMajor;
	Version *= 1000;

	Version += PopMondegreen::VersionMinor;
	Version *= 1000;

	Version += PopMondegreen::VersionPatch;

	return Version;
}


__export int32_t PopMondegreen_CreateInstance(const char* _OptionsJson, char* ErrorBuffer, int32_t ErrorBufferSize)
{
	try
	{
		if ( !_OptionsJson )
			_OptionsJson = "{}";
		std::string_view OptionsJson( _OptionsJson, std::strlen(_OptionsJson) );
		
		auto Instance = PopMondegreen::ListenerInstanceManager.Alloc(OptionsJson);
		return Instance;
	}
	catch (std::exception& e)
	{
		Soy::StringToBuffer( e.what(), ErrorBuffer, ErrorBufferSize );
		return PopMondegreen_NullInstance;
	}
}

__export void PopMondegreen_DestroyInstance(int32_t Instance)
{
	try
	{
		PopMondegreen::ListenerInstanceManager.Free(Instance);
	}
	catch (std::exception& e)
	{
		std::cerr << __FUNCTION__ << " exception; " << e.what() << std::endl;
	}
}


void Soy::StringToBuffer(const char* Source,char* Buffer,size_t BufferSize)
{
	//	copy into nothing, is safe
	if ( Buffer == nullptr )
		return;
	if ( BufferSize == 0 )
		return;
	
	int Len = 0;
	for ( Len=0;	Source && Len<BufferSize-1;	Len++ )
	{
		if ( Source[Len] == '\0' )
			break;
		Buffer[Len] = Source[Len];
	}
	Buffer[std::min<ssize_t>(Len,BufferSize-1)] = '\0';
}
