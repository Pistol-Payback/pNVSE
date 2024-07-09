#pragma once
#include "SaveSystem.h"
#include <fstream>
#include <filesystem>

namespace SaveSystem {

	std::vector<SpawnQueue> queueToSpawn;

	UInt8(*SaveData::ReadRecord8)();
	UInt16(*SaveData::ReadRecord16)();
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
	void (*SaveData::WriteRecord16)(UInt16 inData);
	void (*SaveData::WriteRecord32)(UInt32 inData);
	bool (*SaveData::WriteRecordData)(const void* buf, UInt32 length);

	void (*SaveData_Debug::WriteRecord8)(UInt8 inData);
	void (*SaveData_Debug::WriteRecord16)(UInt16 inData);
	void (*SaveData_Debug::WriteRecord32)(UInt32 inData);
	bool (*SaveData_Debug::WriteRecordData)(const void* buf, UInt32 length);
	void (*SaveData_Debug::WriteRecordFloat)(float value);

	void SaveData::WriteRecordFloat(float value)
	{
		WriteRecord32(*reinterpret_cast<int*>(&value));
	}

	bool (*SaveData::OpenRecord)(UInt32 type, UInt32 version);

	void SaveData::logOperation(const std::string& operation) {
		std::string sFile = (GetFalloutDirectory() + "pNVSE_SaveSystemLog.log");
		std::ofstream logFile(sFile, std::ios::app);
		if (logFile.is_open()) {
			logFile << operation << std::endl;
		}
		logFile.close();
	}

	void SaveData::clearLog() {
		std::string sFile = GetFalloutDirectory() + "pNVSE_SaveSystemLog.log";
		std::ofstream logFile(sFile, std::ios::trunc); // Open in truncate mode to clear the file
		if (logFile.is_open()) {
			logFile.close();  // Close the file immediately after opening it in truncate mode
		}
		else {
			// Optionally handle the error if the file could not be opened
			std::cerr << "Failed to open log file for clearing: " << sFile << std::endl;
		}
	}

	// Wrapper functions
	void WriteRecord8(UInt8 inData) {
		SaveData_Debug::WriteRecord8(inData);
		SaveData::logOperation("8-bit data: " + std::to_string(inData));
	}

	void WriteRecord16(UInt16 inData) {
		SaveData_Debug::WriteRecord16(inData);
		SaveData::logOperation("16-bit data: " + std::to_string(inData));
	}

	void WriteRecord32(UInt32 inData) {
		SaveData_Debug::WriteRecord32(inData);
		SaveData::logOperation("32-bit data: " + std::to_string(inData));
	}

	bool WriteRecordData(const void* buf, UInt32 length) {

		bool result = SaveData_Debug::WriteRecordData(buf, length);

		const char* charBuf = static_cast<const char*>(buf);
		std::string dataString(charBuf, length);
		SaveData::logOperation("data: '" + dataString + "' " + (result ? "success" : "failure"));

		return result;
	}

	void SaveWeaponInst(const NVSEInterface* nvse, PluginHandle& pluginHandle)
	{

		NVSESerializationInterface* serialization = (NVSESerializationInterface*)nvse->QueryInterface(kInterface_Serialization);

		bool debug = 1;

		if (debug) { //Wrapper functions

			SaveData_Debug::WriteRecordData = serialization->WriteRecordData;
			SaveData_Debug::WriteRecord32 = serialization->WriteRecord32;
			SaveData_Debug::WriteRecord16 = serialization->WriteRecord16;
			SaveData_Debug::WriteRecord8 = serialization->WriteRecord8;

			SaveData::WriteRecordData = WriteRecordData;
			SaveData::WriteRecord32 = WriteRecord32;
			SaveData::WriteRecord16 = WriteRecord16;
			SaveData::WriteRecord8 = WriteRecord8;

		}
		else {
			SaveData::WriteRecordData = serialization->WriteRecordData;
			SaveData::WriteRecord32 = serialization->WriteRecord32;
			SaveData::WriteRecord16 = serialization->WriteRecord16;
			SaveData::WriteRecord8 = serialization->WriteRecord8;
		}

		SaveData::ResolveRefID = serialization->ResolveRefID;
		SaveData::ReadRecordData = serialization->ReadRecordData;
		SaveData::ReadRecord32 = serialization->ReadRecord32;
		SaveData::ReadRecord16 = serialization->ReadRecord16;
		SaveData::ReadRecord8 = serialization->ReadRecord8;
		SaveData::GetNextRecordInfo = serialization->GetNextRecordInfo;
		SaveData::SkipNBytes = serialization->SkipNBytes;
		SaveData::OpenRecord = serialization->OpenRecord;

		//serialization->SetLoadCallback(pluginHandle, LoadGameCallback);
		serialization->SetSaveCallback(pluginHandle, SaveGameCallback);

	}

}