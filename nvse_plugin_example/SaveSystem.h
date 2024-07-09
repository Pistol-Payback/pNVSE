#pragma once
#include "WeaponSmith.h"

namespace SaveSystem {

	class InstanceSaveManager;
	struct SaveData;

	struct SpawnQueue {

		SpawnQueue(
			TESForm* baseForm, TESObjectCELL* location = nullptr, ExtraDataList* xData = nullptr,
			float x = 0, float y = 0, float z = 0,
			float xR = 0, float yR = 0, float zR = 0
		) :
			baseForm(baseForm), location(location), xData(xData),
			x(x), y(y), z(z),
			xR(xR), yR(yR), zR(zR)
		{
		}

		TESForm* baseForm;
		TESObjectCELL* location;
		ExtraDataList* xData;
		float x, y, z;
		float xR, yR, zR;

	};

	extern std::vector<SpawnQueue> queueToSpawn;

	void SaveGameCallback(void*);
	//void LoadGameCallback(void*);

	class SaveLoadManager {
	public:

		SaveLoadManager(const SaveLoadManager&) = delete;
		SaveLoadManager& operator=(const SaveLoadManager&) = delete;


		static SaveLoadManager& getInstance() {
			static SaveLoadManager instance;
			return instance;
		}

		static InstanceSaveManager saveManager;

		static void reset();

	private:
		SaveLoadManager() {}
	};

	struct SaveData_Debug {

		SaveData_Debug() {}
		~SaveData_Debug() {}

		static void (*WriteRecord8)(UInt8 inData);
		static void (*WriteRecord16)(UInt16 inData);
		static void (*WriteRecord32)(UInt32 inData);
		static void (*WriteRecordFloat)(float value);
		static bool (*WriteRecordData)(const void* buf, UInt32 length);

	};

	struct SaveData {

		enum : UInt16 { InvalidInstID = 65535 };

		SaveData(){}
		~SaveData() {}

		static UInt8(*ReadRecord8)();
		static UInt16(*ReadRecord16)();
		static UInt32(*ReadRecord32)();

		static float ReadRecordFloat();
		static bool (*ResolveRefID)(UInt32 refID, UInt32* outRefID);
		static UInt32(*ReadRecordData)(void* buf, UInt32 length);
		static bool (*GetNextRecordInfo)(UInt32* type, UInt32* version, UInt32* length);
		static void (*SkipNBytes)(UInt32 byteNum);

		static void (*WriteRecord8)(UInt8 inData);
		static void (*WriteRecord16)(UInt16 inData);
		static void (*WriteRecord32)(UInt32 inData);

		static void WriteRecordFloat(float value);

		static bool (*WriteRecordData)(const void* buf, UInt32 length);
		static bool (*OpenRecord)(UInt32 type, UInt32 version);

		static void logOperation(const std::string& operation);
		static void clearLog();

	};

	struct SaveDataLink {

		TESForm* parent = nullptr;
		Instance* instance = nullptr;

		SaveDataLink(TESForm* parent, Instance* instance) : parent(parent), instance(instance) {
			forceQueueToSave(parent);
		}

		bool operator==(const SaveDataLink& other) const {
			return parent == other.parent;
		}

		void forceQueueToSave(TESForm* form);

	};

	struct SaveDataREFR {

		UInt8 type;
		Instance* instance; //Self
		SaveDataLink location;

		SaveDataREFR(TESForm* parent, Instance* parentInst = nullptr, Instance* self = nullptr) : location(parent, parentInst), instance(self){
			type = 0;
		}

	};

	struct SaveDataEntry : SaveDataREFR {

		SaveDataEntry(TESForm* parent, Instance* parentInst = nullptr, Instance* self = nullptr, ExtraDataList* xData = nullptr)
		: SaveDataREFR(parent, parentInst, self), xData(xData) {
			type = 1;
		}

		ExtraDataList* xData;

	};

	struct SaveDataWorldREFR : SaveDataEntry {


		SaveDataWorldREFR(UInt32 worldRefID, TESForm* parent, Instance* parentInst = nullptr, Instance* self = nullptr, ExtraDataList* xData = nullptr,
			float x = 0, float y = 0, float z = 0,
			float xR = 0, float yR = 0, float zR = 0
		) :
			SaveDataEntry(parent, parentInst, self, xData),
			x(x), y(y), z(z),
			xR(xR), yR(yR), zR(zR),
			worldRefID(worldRefID)
		{
			type = 2;
		}

		UInt32 worldRefID; //Self
		float x, y, z;
		float xR, yR, zR;

	};

	class InstanceSaveManager : public SaveData {

	private:
		std::unordered_map<TESForm*, UInt32> searchLocation;
		std::vector<TESForm*> locations;
		//std::unordered_map<ExtendedBaseType*, WorldReferences> combinedMap;
		std::unordered_map<ExtendedBaseType*, std::unordered_map<Instance*, std::unordered_set<SaveDataREFR*>>> instanceRefs;
		std::unordered_map<ExtendedBaseType*, std::unordered_set<SaveDataREFR*>> dynamicsRefs;

	public:
		InstanceSaveManager() = default;

		//All string EditorID locations
		TESObjectREFR* queueToSave(TESObjectREFR* thisObj);
		void queueToSave(TESForm* thisObj);

		//For instances that save regardless
		void queueToSave(ExtendedBaseType* thisObj);

		//Store instances in blocks
		void queueToSave(Instance* thisObj, bool forceQueue);
		void queueToSave(Instance_WEAP* thisObj);
		void queueToSave(Instance_Akimbo* thisObj);

		void queueToSaveRef(Instance* instance, SaveDataREFR* worldRef);		//World reference derived from instances
		void queueToSaveRef(ExtendedBaseType* base, SaveDataREFR* worldRef);	//Dynamics created with Devkit

		//Saves all refs associated with a baseform/instance
		void SaveRefsList(const std::unordered_set<SaveDataREFR*>& refList);

		//Deduces Type to cast down to
		void SaveBase(ExtendedBaseType* thisObj);
		void SaveInstance(Instance* thisObj);

		void Save(StaticInstance* thisObj);
		void Save(StaticInstance_Akimbo* thisObj);

		void Save(Instance* thisObj);
		void Save(Instance_WEAP* thisObj);
		void Save(Instance_Akimbo* thisObj);

		void Save(const SaveDataREFR& thisObj);
		void Save(const SaveDataEntry& thisObj);
		void RestorEntryRefs(const SaveDataEntry& thisObj);

		void Save(const SaveDataWorldREFR& thisObj);
		void Save(TESForm* thisObj);
		void Save(TESObjectREFR* thisObj);
		void SaveExtraData(ExtraDataList* xDataList);

		void SaveLink(const SaveDataLink& thisObj);
		UInt32 GetSaveIndex(TESForm* thisObj);

		bool isQueueToSave(Instance* instance);

		void queueAllWithSaveBehavior();
		void saveAll();

		void reset();

	};

	struct LoadDataLink : SaveData {

		TESForm* parent = nullptr;
		UInt16 instID = InvalidInstID;	//Invalid
		UInt32 refID = 0;

		LoadDataLink(TESForm* parent, UInt32 refID, UInt16 instID) : parent(parent), refID(refID), instID(instID) {}

	};

	struct LoadDataREFR {

		TESForm* baseform; //Self
		LoadDataLink location;

		LoadDataREFR(TESForm* baseform, LoadDataLink location) : baseform(baseform), location(location) {}

	};

	struct LoadDataEntry : LoadDataREFR {

		LoadDataEntry(TESForm* baseform, LoadDataLink location, ExtraDataList* xData = nullptr)
			: LoadDataREFR(baseform, location), xData(xData) {}

		ExtraDataList* xData;

	};

	struct LoadDataWorldREFR : LoadDataEntry {

		LoadDataWorldREFR(TESForm* baseform, LoadDataLink location, ExtraDataList* xData = nullptr, UInt32 worldRefID = 0,
			float x = 0, float y = 0, float z = 0,
			float xR = 0, float yR = 0, float zR = 0
		) :
			LoadDataEntry(baseform, location, xData),
			x(x), y(y), z(z),
			xR(xR), yR(yR), zR(zR),
			worldRefID(worldRefID)
		{
		}

		UInt32 worldRefID; //lookup worldID for linking
		float x, y, z;
		float xR, yR, zR;

	};

	struct FailedWeaponLink {

		FailedWeaponLink(Instance_WEAP* weap, std::string slot, LoadDataLink link) : weap(weap), slot(slot), attachment(link){}

		Instance_WEAP* weap;
		std::string slot;
		LoadDataLink attachment;
	};

	struct FailedAkimboLink {

		FailedAkimboLink(Instance_Akimbo* weap, UInt32 leftID, UInt32 rightID) : weap(weap), leftID(leftID), rightID(rightID) {}

		Instance_Akimbo* weap;
		UInt32 leftID;
		UInt32 rightID;
	};

	class InstanceLoadManager : public SaveData {

	private:

		std::unordered_map<UInt32, TESForm*> baseLocations;
		std::unordered_map<UInt32, TESObjectREFR*> refLocations;

		//Instances that rely on other instances
		std::vector<FailedWeaponLink*> failedLinks_WEAP;
		std::vector<FailedAkimboLink*> failedLinks_Akimbo;

		std::vector<LoadDataWorldREFR*> worldRefs;
		std::vector<LoadDataEntry*> entries;
		std::vector<LoadDataREFR*> linkRefs; //Not used

	public:

		InstanceLoadManager() = default;

		void LoadStaticLocations(UInt32 index);

		//Instances

			Instance* LoadInstance(ExtendedBaseType* baseInstance);

			Instance* LoadInstanceData(StaticInstance* baseInstance);
			Instance_WEAP* LoadInstanceDataWEAP(StaticInstance_WEAP* baseInstance);
			Instance_Akimbo* LoadInstanceDataAkimbo(StaticInstance_Akimbo* baseInstance);

			void					LoadInstanceLinksWEAP();
			void					LoadInstanceLinksAkimbo();

		//Statics

			ExtendedBaseType*		LoadBase(UInt32 extendedType);
			StaticInstance*			LoadStatic();
			StaticInstance_Akimbo*	LoadStaticAkimbo();

		//Refs

			void LoadRefsList(TESForm* baseform);
			void LoadRef(TESForm* baseform);
			void LoadEntryRef(TESForm* baseform);
			void LoadWorldRef(TESForm* baseform);

		//Extra Data

			ExtraDataList* LoadExtraDataList();

		//Placement
			void PlaceWorldRefs();
			void PlaceEntryRefs();
			void LinkRefs(); //Not used

			LoadDataLink LoadLink();

		//Skips

			void LoadInstance_Skip(UInt32 extendedType);
			void LoadInstanceData_Skip();
			void LoadInstanceDataWEAP_Skip();
			void LoadInstanceDataAkimbo_Skip();
			void LoadRefsList_Skip();
			void LoadRef_Skip();
			void LoadEntryRef_Skip();
			void LoadWorldRef_Skip();
			void LoadExtraDataList_Skip();
			void LoadLink_Skip();

		//Utility

			TESForm* LoadFromIndex(UInt32 index);
			TESObjectREFR* LoadFromRefID(UInt32 refID);

			void loadAll();
			void reset();

			//For situations where we are unsure what the link data could be.
			TESForm* deduceLinkToForm(LoadDataLink* link);

	};

}