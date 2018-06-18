#include "SsiPrivatePCH.h"
#include "SsiSettings.h"
#include "SsiFunctionLibrary.h"
#include "oscpack/osc/OscOutboundPacketStream.h"
#include <vector>

void USsiFunctionLibrary::StreamPushBool(const TArray<FOscDataElemStruct> & input, bool Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetBool(Value);
    output.Add(elem);
}

void USsiFunctionLibrary::StreamPushFloat(const TArray<FOscDataElemStruct> & input, float Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetFloat(Value);
    output.Add(elem);
}

void USsiFunctionLibrary::StreamPushInt(const TArray<FOscDataElemStruct> & input, int32 Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetInt(Value);
    output.Add(elem);
}

void USsiFunctionLibrary::StreamPushString(const TArray<FOscDataElemStruct> & input, FName Value, TArray<FOscDataElemStruct> & output)
{
    if(Value.GetDisplayNameEntry()->IsWide())
    {
        const auto tmp = Value.GetPlainNameString();
        UE_LOG(LogSSI, Error, TEXT("Invalid string argument \"%s\": ASCII only"), *tmp);
        return;
    }

    output = input;
    FOscDataElemStruct elem;
    elem.SetString(Value);
    output.Add(elem);
}

void USsiFunctionLibrary::StreamPushBlob(const TArray<FOscDataElemStruct> & input, const TArray<uint8> & Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetBlob(Value);
    output.Add(elem);
}

void USsiFunctionLibrary::EventPushFloat(const TArray<FOscDataElemStruct> & input, FName Key, float Value, TArray<FOscDataElemStruct> & output)
{
	StreamPushString(input, Key, output);
	StreamPushFloat(output, Value, output);
}

namespace
{
    bool isValidAddress(const FName & Address)
    {
        if(!Address.IsValid())
        {
            UE_LOG(LogSSI, Error, TEXT("Empty OSC address"));
            return false;
        }

        if(Address.GetDisplayNameEntry()->IsWide())
        {
            const auto tmp = Address.GetPlainNameString();
            UE_LOG(LogSSI, Error, TEXT("Invalid OSC address \"%s\": ASCII only"), *tmp);
            return false;
        }

        if(Address.GetPlainANSIString()[0] != '/')
        {
            const auto tmp = Address.GetPlainNameString();
            UE_LOG(LogSSI, Error, TEXT("Invalid OSC address \"%s\": must start with '/'"), *tmp);
            return false;
        }

        return true;
    }

    void appendMessage(osc::OutboundPacketStream & output, FName Address, const TArray<FOscDataElemStruct> & Data)
    {
        output << osc::BeginMessage(Address.GetPlainANSIString());
        if(output.State() != osc::SUCCESS)
        {
            return;
        }

        for(const auto & elem : Data)
        {
            if(elem.IsFloat())
            {
                output << (float)elem.AsFloatValue();
            }
            else if(elem.IsInt())
            {
                output << (int32)elem.AsIntValue();
            }
            else if(elem.IsBool())
            {
                output << elem.AsBoolValue();
            }
            else if(elem.IsString())
            {
                output << elem.AsStringValue().GetPlainANSIString();
            }
            else if(elem.IsBlob())
            {
                const TArray<uint8> & value = elem.AsBlobValue();
                output << osc::Blob(value.GetData(), value.Num());
            }

            if(output.State() != osc::SUCCESS)
            {
                return;
            }
        }
        output << osc::EndMessage;
    }

    TArray<uint8> GlobalBuffer(TArray<uint8>(), 1024);

	FName GetStreamSampletypeEnumAsString(EStreamSampletype EnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EStreamSampletype"), true);
		if (!EnumPtr) return FName("Invalid");

		return EnumPtr->GetNameByValue((int64)EnumValue); // for EnumValue == VE_Dance returns "VE_Dance"
	}
}

void USsiFunctionLibrary::SendEvent(/*const FString sender_name, const FString event_name, */const TArray<FOscDataElemStruct> & Data, int32 TargetIndex, int32 timestamp, int32 duration, int32 state)
{
    static_assert(sizeof(uint8) == sizeof(char), "Cannot cast uint8 to char");

	if (Data.Num() % 2 != 0)
	{
		UE_LOG(LogSSI, Error, TEXT("Data is expected to contain Key/Value pairs but contains an uneven number (\"%d\") of elements."), Data.Num());
		return;
	}

	osc::int32 const n_events = Data.Num() / 2;

    osc::OutboundPacketStream output((char *)GlobalBuffer.GetData(), GlobalBuffer.Max());
    check(reinterpret_cast<const void *>(GlobalBuffer.GetData()) == reinterpret_cast<const void *>(output.Data()));

	output << osc::BeginMessage("/evnt");
	output << ""; // TCHAR_TO_ANSI(*sender_name); // Is unused in socket event reader
	output << ""; // TCHAR_TO_ANSI(*event_name); // Is unused in socket event reader
	output << timestamp;
	output << duration;
	output << state;
	output << n_events; // number of string/float pairs.

	for (int i = 0; i < Data.Num();)
	{
		FOscDataElemStruct const& key = Data[i++];
		if (!key.IsString())
		{
			UE_LOG(LogSSI, Error, TEXT("Keys of Data are expected to be strings but \"%d\"-th element wasn't."), i);
			return;
		}
		output << key.AsStringValue().GetPlainANSIString();

		FOscDataElemStruct const& value = Data[i++];
		if (!value.IsFloat())
		{
			UE_LOG(LogSSI, Error, TEXT("Values of Data are expected to be floats but \"%d\"-th element wasn't."), i);
			return;
		}
		output << (float)value.AsFloatValue();
	}

	output << osc::EndMessage;

	if (output.State() != osc::SUCCESS)
	{
		return;
	}

    if(output.State() == osc::OUT_OF_BUFFER_MEMORY_ERROR)
    {
        GlobalBuffer.Reserve(GlobalBuffer.Max() * 2);  // not enough memory: double the size
        SendEvent(/*sender_name, event_name, */Data, TargetIndex, timestamp, duration, state);  // try again
        return;
    }

    if(output.State() == osc::SUCCESS)
    {
        GetMutableDefault<USsiSettings>()->Send(GlobalBuffer.GetData(), output.Size(), TargetIndex);
    }
    else
    {
        UE_LOG(LogSSI, Error, TEXT("SSI Send Event Error: %s"), osc::errorString(output.State()));
    }
}

void USsiFunctionLibrary::SendMessage(/*const FString sender_name, const FString event_name, */const FString message, int32 TargetIndex, int32 timestamp, int32 duration)
{
	static_assert(sizeof(uint8) == sizeof(char), "Cannot cast uint8 to char");

	osc::OutboundPacketStream output((char *)GlobalBuffer.GetData(), GlobalBuffer.Max());
	check(reinterpret_cast<const void *>(GlobalBuffer.GetData()) == reinterpret_cast<const void *>(output.Data()));

	output << osc::BeginMessage("/text");
	output << ""; // TCHAR_TO_ANSI(*sender_name); // Is unused in socket event reader
	output << ""; // TCHAR_TO_ANSI(*event_name); // Is unused in socket event reader
	output << timestamp;
	output << duration;
	output << TCHAR_TO_ANSI(*message);

	output << osc::EndMessage;

	if (output.State() != osc::SUCCESS)
	{
		return;
	}

	if (output.State() == osc::OUT_OF_BUFFER_MEMORY_ERROR)
	{
		GlobalBuffer.Reserve(GlobalBuffer.Max() * 2);  // not enough memory: double the size
		SendMessage(/*sender_name, event_name, */message, TargetIndex, timestamp, duration);  // try again
		return;
	}

	if (output.State() == osc::SUCCESS)
	{
		GetMutableDefault<USsiSettings>()->Send(GlobalBuffer.GetData(), output.Size(), TargetIndex);
	}
	else
	{
		UE_LOG(LogSSI, Error, TEXT("SSI Send Message Error: %s"), osc::errorString(output.State()));
	}
}

void USsiFunctionLibrary::SendSamples(const TArray<FOscDataElemStruct> & data, int32 TargetIndex, FString id, int32 timestamp, float samplerate, int32 num, int32 dimension, int32 bytes, EStreamSampletype type)
{
	static_assert(sizeof(uint8) == sizeof(char), "Cannot cast uint8 to char");

	osc::OutboundPacketStream output((char *)GlobalBuffer.GetData(), GlobalBuffer.Max());
	check(reinterpret_cast<const void *>(GlobalBuffer.GetData()) == reinterpret_cast<const void *>(output.Data()));

	output << osc::BeginMessage("/strm");
	output << TCHAR_TO_ANSI(*id);
	output << timestamp;
	output << samplerate;
	output << num;
	output << dimension;
	output << bytes;
	output << (int8)type;

	if (data.Num() != num * dimension)
	{
		UE_LOG(LogSSI, Error, TEXT("Number of elements (%d) doesn't match num * dimension (%d * %d)"), data.Num(), num, dimension);
		return;
	}

	std::vector<BYTE> blob_data(num * dimension * bytes);
	
	int32 idxWrongElementType = -1;
	for (int i = 0; i < data.Num() && idxWrongElementType < 0; ++i)
	{
		const FOscDataElemStruct & elem = data[i];

		switch (type)
		{
			case EStreamSampletype::FLOAT:
			{
				if (!elem.IsFloat())
				{
					idxWrongElementType = i;
					break;
				}
				float* const dst = reinterpret_cast<float* const>(blob_data.data()) + i;
				*dst = (float)elem.AsFloatValue();
				break;
			}
			default:
			{
				UE_LOG(LogSSI, Error, TEXT("Sample type %s not supported, yet."), *GetStreamSampletypeEnumAsString(type).ToString());
				return;
			}
		}
	}

	if (idxWrongElementType >= 0)
	{
		UE_LOG(LogSSI, Error, TEXT("All elements were expected to be %s but %d-th element wasn't."), *GetStreamSampletypeEnumAsString(type).ToString(), idxWrongElementType);
		return;
	}

	output << osc::Blob(blob_data.data(), blob_data.size());
	output << osc::EndMessage;

	if (output.State() != osc::SUCCESS)
	{
		return;
	}

	if (output.State() == osc::OUT_OF_BUFFER_MEMORY_ERROR)
	{
		GlobalBuffer.Reserve(GlobalBuffer.Max() * 2);  // not enough memory: double the size
		SendSamples(data, TargetIndex, id, timestamp, samplerate, num, dimension, bytes, type);  // try again
		return;
	}

	if (output.State() == osc::SUCCESS)
	{
		GetMutableDefault<USsiSettings>()->Send(GlobalBuffer.GetData(), output.Size(), TargetIndex);
	}
	else
	{
		UE_LOG(LogSSI, Error, TEXT("SSI Send Message Error: %s"), osc::errorString(output.State()));
	}
}

int32 USsiFunctionLibrary::AddSendOscTarget(FString IpPort)
{
    return GetMutableDefault<USsiSettings>()->GetOrAddSendTarget(IpPort);
}
