#pragma once

#include "OscDataElemStruct.h"
#include "OscMessageStruct.h"
#include "SsiFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class EStreamSampletype : uint8
{
	UNDEF = 0,
	CHAR = 1,
	UCHAR = 2,
	SHORT = 3,
	USHORT = 4,
	INT = 5,
	UINT = 6,
	LONG = 7,
	ULONG = 8,
	FLOAT = 9,
	DOUBLE = 10,
	LDOUBLE = 11,
	STRUCT = 12,
	IMAGE = 13,
	BOOL = 14,
};

int32 GetStreamSampletypeSize(EStreamSampletype type);

UCLASS()
class SSI_API USsiFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /// Add a boolean value to an OSC message.
    UFUNCTION(BlueprintPure, Category="SSI|Streams", meta=(AutoCreateRefTerm = "input"))
    static void StreamPushBool(const TArray<FOscDataElemStruct> & input, bool Value, TArray<FOscDataElemStruct> & output);

    /// Add a floating point value to an OSC message.
    UFUNCTION(BlueprintPure, Category="SSI|Streams", meta=(AutoCreateRefTerm = "input"))
    static void StreamPushFloat(const TArray<FOscDataElemStruct> & input, float Value, TArray<FOscDataElemStruct> & output);

    /// Add a integer value to an OSC message.
    UFUNCTION(BlueprintPure, Category="SSI|Streams", meta=(AutoCreateRefTerm = "input"))
    static void StreamPushInt(const TArray<FOscDataElemStruct> & input, int32 Value, TArray<FOscDataElemStruct> & output);

    /// Add a string value to an OSC message.
    UFUNCTION(BlueprintPure, Category="SSI|Streams", meta=(AutoCreateRefTerm = "input"))
    static void StreamPushString(const TArray<FOscDataElemStruct> & input, FName Value, TArray<FOscDataElemStruct> & output);

    /// Add a blob to an OSC message.
    UFUNCTION(BlueprintPure, Category="SSI|Streams", meta=(AutoCreateRefTerm = "input"))
    static void StreamPushBlob(const TArray<FOscDataElemStruct> & input, const TArray<uint8> & Value, TArray<FOscDataElemStruct> & output);

	UFUNCTION(BlueprintPure, Category="SSI|Events", meta=(AutoCreateRefTerm = "input"))
	static void EventPushFloat(const TArray<FOscDataElemStruct> & input, FName Key, float Value, TArray<FOscDataElemStruct> & output);

    /**
     *  @brief Send an SSI event.
     *  @param Address OSC address.
     *  @param Data result of successive PushFloat/Int/String/etc.
     *  @param TargetIndex index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
	 *
	 * sender_name and event_name are unused in the SSI SocketEventReader.
     */
    UFUNCTION(BlueprintCallable, Category="SSI|Events", meta=(AutoCreateRefTerm = "Data"))
    static void SendEvent(/*const FString sender_name, const FString event_name, */const TArray<FOscDataElemStruct> & Data, int32 TargetIndex, int32 timestamp = -1, int32 duration = 0, int32 state = 0);

	/**
	*  @brief Send an SSI event.
	*  @param Address OSC address.
	*  @param Data result of successive PushFloat/Int/String/etc.
	*  @param TargetIndex index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
	*
	* sender_name and event_name are unused in the SSI SocketEventReader.
	*/
	UFUNCTION(BlueprintCallable, Category="SSI|Events", meta = (AutoCreateRefTerm="Data"))
	static void SendMessage(/*const FString sender_name, const FString event_name, */const FString message, int32 TargetIndex, int32 timestamp = -1, int32 duration = 0);

	/**
	*  @brief Sends a batch of stream samples.
	*
	*  @param data
	*  @param TargetIndex index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
	*  @param stream_name	A string identifier for the stream.
	*  @param timestamp
	*  @param samplerate	Sample rate in Hz.
	*  @param num			The sample count in this batch.
	*  @param dimension		Number of dimensions per sample. E.g. for a position in 3D space this would be 3.
	*  @param bytes			Bytes per dimension. E.g. if the 3D position would be stored as floats, this would be 4, because a float is 4 bytes long.
	*  @param type			All of the dimensions in a sample have to have this type.
	*
	*  @note Infers the SSI parameters bytes, dimension and type from the data array and the num parameter.
	*/
	UFUNCTION(BlueprintCallable, Category="SSI|Streams", meta=(AutoCreateRefTerm = "Data"))
	static void SendSamples(FString stream_name, const TArray<FOscDataElemStruct> & data, int32 TargetIndex, int32 timestamp, float samplerate, int32 num);

	/**
	*  @brief Sends a batch of stream samples.
	*
	*  @param data
	*  @param TargetIndex index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
	*  @param stream_name	A string identifier for the stream.
	*  @param timestamp
	*  @param samplerate	Sample rate in Hz.
	*  @param num			The sample count in this batch.
	*  @param dimension		Number of dimensions per sample. E.g. for a position in 3D space this would be 3.
	*  @param bytes			Bytes per dimension. E.g. if the 3D position would be stored as floats, this would be 4, because a float is 4 bytes long.
	*  @param type			All of the dimensions in a sample have to have this type.
	*
	*  With this method, the sample type, dimension and sample size can be specified explicitly. This allows to send more complex structures as streams.
	*/
	UFUNCTION(BlueprintCallable, Category="SSI|Streams", meta=(AutoCreateRefTerm="Data"))
	static void SendSamplesEx(FString stream_name, const TArray<FOscDataElemStruct> & data, int32 TargetIndex, int32 timestamp, float samplerate, int32 num, int32 dimension, int32 bytes, EStreamSampletype type);

    /**
     *  @brief Add Ip:Port to the available OSC send targets.
     *  @param IpPort "ip:port". e.g. "192.168.0.1:7777"
     *  @return The created TargetIndex to pass to the SendOsc function.
     *
     *  Use this function to add target at runtime. Generally, it is best
     *  to define your targets in the OSC plugin settings.
     *
     *  @see SendOsc
     */
    UFUNCTION(BlueprintCallable, Category="SSI")
    static int32 AddSendOscTarget(FString IpPort = "127.0.0.1:8000");

	UFUNCTION(BlueprintPure, Category = "SSI|Math")
	static void RotatorToQuat(FRotator const R, FQuat& quaternion);
};
