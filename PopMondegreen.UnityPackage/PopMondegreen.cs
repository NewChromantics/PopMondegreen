using System;
using System.Runtime.InteropServices;		// required for DllImport
using UnityEngine;


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

//	if we make this public for re-use, give it a name that doesn't suggest this is an API function
	//	make sure we use this! using GetString without all 0's will crash unity as console tries to print it
	static private string GetString(byte[] Ascii)
	{
		if ( Ascii[0] == 0 )
			return null;
		var String = System.Text.ASCIIEncoding.ASCII.GetString(Ascii);
		
		//	clip string as unity doesn't cope well with large terminator strings
		var TerminatorPos = String.IndexOf('\0');
		if (TerminatorPos >= 0)
			String = String.Substring(0, TerminatorPos);
		return String;
	}
	
	static private byte[] GetJsonBytes<T>(T Obj)
	{
		var Json = JsonUtility.ToJson(Obj);
		Json += 0;
		var String = System.Text.ASCIIEncoding.ASCII.GetBytes(Json);
		return String;
	}

	
	static public string		GetVersion()
	{
		var Version = PopMondegreen_GetVersionThousand();
		var Major = (Version / (1000 * 1000));
		var Minor = (Version / 1000) % 1000;
		var Patch = (Version) % 1000;
		return $"{Major}.{Minor}.{Patch}";
	}

	[Serializable]
	public struct DecoderParams
	{
		public string	Name;		//	name of decoder
	}
	
	[Serializable]
	public struct DecoderOutput
	{
		public int		StartTime;
		public int		EndTime;
		public string	Data;
		public string	Error;
		
		public bool		HasError => !String.IsNullOrEmpty(Error);
		public bool		IsValid => StartTime!=-1;
	}

	public class Decoder : IDisposable
	{
		int?		Instance = null;
		
		public Decoder(DecoderParams Params)
		{
			var ParamsJson = GetJsonBytes(Params);
			var ErrorBuffer = new Byte[100];
			Instance = PopMondegreen_CreateInstance(ParamsJson,ErrorBuffer,ErrorBuffer.Length);
			var Error = GetString(ErrorBuffer);
			if ( Instance.Value == 0 )
				throw new Exception($"Failed to allocate decoder; Error={Error}");
		}
		
		~Decoder()
		{
			Dispose();
		}
		
		public void PushData(float[] Samples,int TimestampMs,int ChannelCount,int FrequencyHz)
		{
			PopMondegreen_PushData( Instance.Value, TimestampMs, Samples, Samples.Length, ChannelCount, FrequencyHz, null );
		}
		
		public DecoderOutput? PopData()
		{
			var JsonBuffer = new byte[1024*1024*1];
			PopMondegreen_PopData( Instance.Value, JsonBuffer, JsonBuffer.Length );
			var OutputJson = GetString(JsonBuffer);
			var Output = JsonUtility.FromJson<DecoderOutput>(OutputJson);
			if ( Output.HasError )
				throw new Exception($"Error with decoder {Output.Error}");
			if ( !Output.IsValid )
				return null;
			return Output;
		}
		
		public void	Dispose()
		{
			if ( Instance is int instance )
			{
				PopMondegreen_FreeInstance( instance );
				Instance = null;
			}
		}
	}

}
