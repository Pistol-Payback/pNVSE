#pragma once
#include "WeaponSmith.h"

namespace SaveSystem {

	struct SpawnQueue {

		SpawnQueue(
			TESForm* baseForm,
			TESObjectCELL* location = nullptr,
			ExtraDataList* xData = nullptr,
			float x = 0,
			float y = 0,
			float z = 0,
			float xR = 0,
			float yR = 0,
			float zR = 0
		) :
			baseForm(baseForm),
			location(location),
			xData(xData),
			x(x),
			y(y),
			z(z),
			xR(xR),
			yR(yR),
			zR(zR)
		{
		}

		TESForm* baseForm;
		TESObjectCELL* location;
		ExtraDataList* xData;
		float x;
		float y;
		float z;
		float xR;
		float yR;
		float zR;

	};

	struct SaveData {

		SaveData(){
		
		}

		static UInt8(*ReadRecord8)();
		static UInt32(*ReadRecord32)();

		static float ReadRecordFloat();
		static bool (*ResolveRefID)(UInt32 refID, UInt32* outRefID);
		static UInt32(*ReadRecordData)(void* buf, UInt32 length);
		static bool (*GetNextRecordInfo)(UInt32* type, UInt32* version, UInt32* length);
		static void (*SkipNBytes)(UInt32 byteNum);

		static void (*WriteRecord8)(UInt8 inData);
		static void (*WriteRecord32)(UInt32 inData);

		static void WriteRecordFloat(float value);

		static bool (*WriteRecordData)(const void* buf, UInt32 length);
		static bool (*OpenRecord)(UInt32 type, UInt32 version);

	};

	struct SaveDataObj : SaveData{

		SaveDataObj(
			StaticInstance* staticInst = nullptr,
			Instance* inst = nullptr,
			UInt32 location = 0,
			ExtraDataList* xData = nullptr
		) :
			staticInst(staticInst),
			inst(inst),
			location(location),
			xData(xData)
		{
		}

		ExtraDataList* xData;
		StaticInstance* staticInst;
		Instance* inst;
		UInt32 location;

	};

	struct SaveDataWorldObj : SaveDataObj {

		SaveDataWorldObj(
			StaticInstance* staticInst = nullptr,
			Instance* inst = nullptr,
			UInt32 location = 0,
			ExtraDataList* xData = nullptr,
			float x = 0,
			float y = 0,
			float z = 0,
			float xR = 0,
			float yR = 0,
			float zR = 0
		) :
			SaveDataObj(staticInst, inst, location, xData),
			x(x),
			y(y),
			z(z),
			xR(xR),
			yR(yR),
			zR(zR)
		{
		}

		float x;
		float y;
		float z;
		float xR;
		float yR;
		float zR;

	};

	extern std::vector<SaveData*> aSaveData;
	extern std::vector<SpawnQueue> queueToSpawn;

	void SaveGameCallback(void*);
	void LoadGameCallback(void*);
}