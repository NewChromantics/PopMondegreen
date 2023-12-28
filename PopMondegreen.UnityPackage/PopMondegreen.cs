using system;
using System.Runtime.InteropServices;		// required for DllImport
//using System;								// requred for IntPtr
using System.Text;
using System.Collections.Generic;


/// <summary>
///	Low level interface
/// </summary>
public static class PopMondegreen
{
//	need to place editor defines first as we can be in mac editor, but windows target.
#if UNITY_EDITOR_WIN
	private const string PluginName = "PopMondegreen";	//	PopMondegreen.dll
#elif UNITY_EDITOR_OSX
	private const string PluginName = "PopMondegreen";	//	libPopMondegreen.dylib
#elif UNITY_STANDALONE_OSX
	private const string PluginName = "PopMondegreen";	//	libPopMondegreen.dylib
#elif UNITY_WSA
	private const string PluginName = "PopMondegreen.Uwp.dll";	//	PopPopMondegreen.Uwp.dll
#elif UNITY_IPHONE
	private const string PluginName = "__Internal";
#else
	private const string PluginName = "PopMondegreen";
#endif
	[DllImport(PluginName, CallingConvention = CallingConvention.Cdecl)]
	private static extern int	PopMondegreen_GetVersionThousand();

	//	returns decoder instance id, 0 on error.
	[DllImport(PluginName, CallingConvention = CallingConvention.Cdecl)]
	private static extern int	PopMondegreen_CreateInstance(byte[] OptionsJson, [In, Out] byte[] ErrorBuffer, Int32 ErrorBufferLength);

	[DllImport(PluginName, CallingConvention = CallingConvention.Cdecl)]
	private static extern void	PopMondegreen_FreeInstance(int Instance);

	[DllImport(PluginName, CallingConvention = CallingConvention.Cdecl)]
	private static extern int	PopMondegreen_PushData(int Instance,int TimestampMs,float[] SampleData,int SampleCount,int ChannelCount,int SampleHz,byte[] SampleMeta);

	[DllImport(PluginName, CallingConvention = CallingConvention.Cdecl)]
	private static extern int	PopMondegreen_PushEndOfStream(int Instance);

	[DllImport(PluginName, CallingConvention = CallingConvention.Cdecl)]
	private static extern void	PopMondegreen_PopData(int Instance,byte[] JsonBuffer,int JsonBufferSize);


	
	static public string		GetVersion()
	{
		var Version = PopMondegreen_GetVersionThousand();
		var Major = (Version / (1000 * 1000));
		var Minor = (Version / 1000) % 1000;
		var Patch = (Version) % 1000;
		return $"{Major}.{Minor}.{Patch}";
	}


}
