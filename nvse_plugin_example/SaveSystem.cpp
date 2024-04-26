#pragma once
#include "SaveSystem.h"

namespace SaveSystem {

	std::vector<SaveData*> aSaveData;
	std::vector<SpawnQueue> queueToSpawn;

	UInt8(*SaveData::ReadRecord8)();
	UInt32(*SaveData::ReadRecord32)();

	float SaveData::ReadRecordFloat() {
		int intValue = ReadRecord32();
		return *reinterpret_cast<float*>(&intValue);
	}

	bool (*SaveData::ResolveRefID)(UInt32 refID, UInt32* outRefID);
	UInt32(*SaveData::ReadRecordData)(void* buf, UInt32 length);
	bool (*SaveData::GetNextRecordInfo)(UInt32* type, UInt32* version, UInt32* length);
	void (*SaveData::SkipNBytes)(UInt32 byteNum);

	void (*SaveData::WriteRecord8)(UInt8 inData);
	void (*SaveData::WriteRecord32)(UInt32 inData);

	void SaveData::WriteRecordFloat(float value)
	{
		WriteRecord32(*reinterpret_cast<int*>(&value));
	}

	bool (*SaveData::WriteRecordData)(const void* buf, UInt32 length);
	bool (*SaveData::OpenRecord)(UInt32 type, UInt32 version);

	void SaveWeaponInst(const NVSEInterface* nvse, PluginHandle& pluginHandle)
	{

		NVSESerializationInterface* serialization = (NVSESerializationInterface*)nvse->QueryInterface(kInterface_Serialization);

		SaveData::WriteRecordData = serialization->WriteRecordData;
		SaveData::WriteRecord32 = serialization->WriteRecord32;
		SaveData::WriteRecord8 = serialization->WriteRecord8;
		SaveData::ResolveRefID = serialization->ResolveRefID;

		SaveData::ReadRecordData = serialization->ReadRecordData;
		SaveData::ReadRecord32 = serialization->ReadRecord32;
		SaveData::ReadRecord8 = serialization->ReadRecord8;
		SaveData::GetNextRecordInfo = serialization->GetNextRecordInfo;
		SaveData::SkipNBytes = serialization->SkipNBytes;
		SaveData::OpenRecord = serialization->OpenRecord;

		serialization->SetLoadCallback(pluginHandle, LoadGameCallback);
		serialization->SetSaveCallback(pluginHandle, SaveGameCallback);

	}

}