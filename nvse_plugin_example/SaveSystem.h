#pragma once
#include "WeaponSmith.h"

namespace SaveSystem {

	extern UInt8(*ReadRecord8)();
	extern UInt32(*ReadRecord32)();

	extern float ReadRecordFloat();
	extern bool (*ResolveRefID)(UInt32 refID, UInt32* outRefID);
	extern UInt32(*ReadRecordData)(void* buf, UInt32 length);
	extern bool (*GetNextRecordInfo)(UInt32* type, UInt32* version, UInt32* length);
	extern void (*SkipNBytes)(UInt32 byteNum);

	extern void (*WriteRecord8)(UInt8 inData);
	extern void (*WriteRecord32)(UInt32 inData);

	extern void WriteRecordFloat(float value);

	extern bool (*WriteRecordData)(const void* buf, UInt32 length);
	extern bool (*OpenRecord)(UInt32 type, UInt32 version);

	class SaveData {
	public:
		SaveData(
			UInt32 baseRefID = 0,
			UInt8 instID = 0,
			UInt32 location = 0,
			bool equipped = 0,
			ExtraDataList* xData = nullptr,
			float x = 0,
			float y = 0,
			float z = 0,
			float xR = 0,
			float yR = 0,
			float zR = 0) :
			baseRefID(baseRefID),
			instID(instID),
			location(location),
			equipped(equipped),
			xData(xData),
			x(x),
			y(y),
			z(z),
			xR(xR),
			yR(yR),
			zR(zR)
		{
		}

		ExtraDataList* xData;
		UInt32 baseRefID;
		UInt8 instID;
		UInt32 location;

		bool equipped;
		float x;
		float y;
		float z;
		float xR;
		float yR;
		float zR;
	};

	extern std::vector<SaveData*> aSaveData;

	void SaveGameCallback(void*);
	void LoadGameCallback(void*);
}