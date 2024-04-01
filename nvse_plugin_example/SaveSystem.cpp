#pragma once
#include "SaveSystem.h"

namespace SaveSystem {

	std::vector<SaveData*> aSaveData;

	UInt8(*ReadRecord8)();
	UInt32(*ReadRecord32)();

	float ReadRecordFloat() {
		int intValue = ReadRecord32();
		return *reinterpret_cast<float*>(&intValue);
	}

	bool (*ResolveRefID)(UInt32 refID, UInt32* outRefID);
	UInt32(*ReadRecordData)(void* buf, UInt32 length);
	bool (*GetNextRecordInfo)(UInt32* type, UInt32* version, UInt32* length);
	void (*SkipNBytes)(UInt32 byteNum);

	void (*WriteRecord8)(UInt8 inData);
	void (*WriteRecord32)(UInt32 inData);

	extern void WriteRecordFloat(float value)
	{
		WriteRecord32(*reinterpret_cast<int*>(&value));
	}

	bool (*WriteRecordData)(const void* buf, UInt32 length);
	bool (*OpenRecord)(UInt32 type, UInt32 version);

	void SaveWeaponInst(const NVSEInterface* nvse, PluginHandle& pluginHandle)
	{

		NVSESerializationInterface* serialization = (NVSESerializationInterface*)nvse->QueryInterface(kInterface_Serialization);

		WriteRecordData = serialization->WriteRecordData;
		WriteRecord32 = serialization->WriteRecord32;
		WriteRecord8 = serialization->WriteRecord8;
		ResolveRefID = serialization->ResolveRefID;

		ReadRecordData = serialization->ReadRecordData;
		ReadRecord32 = serialization->ReadRecord32;
		ReadRecord8 = serialization->ReadRecord8;
		GetNextRecordInfo = serialization->GetNextRecordInfo;
		SkipNBytes = serialization->SkipNBytes;
		OpenRecord = serialization->OpenRecord;

		serialization->SetLoadCallback(pluginHandle, LoadGameCallback);
		serialization->SetSaveCallback(pluginHandle, SaveGameCallback);

	}

}