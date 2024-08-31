#pragma once
#include "WeaponSmith.h"
#include "KitLoader.h"

namespace SaveSystem {

	class InstanceSaveManager;
	class InstanceLoadManager;
	struct SaveData;

	struct SpawnQueue {

		TESForm* baseForm;
		TESForm* location;
		ExtraDataList* xData;
		SInt32 countDelta;
		float x, y, z;
		float xR, yR, zR;

		SpawnQueue(TESForm* baseForm, TESForm* location = nullptr, ExtraDataList* xData = nullptr, SInt32 countDelta = 1,
			float x = 0, float y = 0, float z = 0,
			float xR = 0, float yR = 0, float zR = 0) :
			baseForm(baseForm), location(location), xData(xData),
			countDelta(countDelta), x(x), y(y), z(z), xR(xR), yR(yR), zR(zR) {}

		SpawnQueue(const SpawnQueue& other) = default;
		SpawnQueue(SpawnQueue&& other) noexcept = default;
		SpawnQueue& operator=(const SpawnQueue& other) = default;
		SpawnQueue& operator=(SpawnQueue&& other) noexcept = default;
		~SpawnQueue() = default;

	};

	extern std::vector<SpawnQueue> queueToSpawn;
	extern std::vector<TESObjectREFR*> queueToUpdate3d;

	void LoadGameCallback(void*);
	void SaveGameCallback(void*);

	//Old system used to place objects really early, and we had to update their 3d after the game loop started. 
	// This would cause a crash in JIPs DoQueuedReferenceHook when leaving cells after reloading a save in an interior where objects had been placed. 
	// -Crashed in doErase part of the function.
	void ExecuteUpdate3dQueue();
	void ExecuteSpawnQueue();

	class SaveLoadManager {
	public:

		SaveLoadManager(const SaveLoadManager&) = delete;
		SaveLoadManager& operator=(const SaveLoadManager&) = delete;


		static SaveLoadManager& getInstance() {
			static SaveLoadManager instance;
			return instance;
		}

		static InstanceLoadManager loadManager;
		static InstanceSaveManager saveManager;


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
		static char* ReadRecordString();
		static void SkipRecordString();

		static float ReadRecordFloat();
		static bool (*ResolveRefID)(UInt32 refID, UInt32* outRefID);
		static UInt32(*ReadRecordData)(void* buf, UInt32 length);
		static bool (*GetNextRecordInfo)(UInt32* type, UInt32* version, UInt32* length);
		static void (*SkipNBytes)(UInt32 byteNum);

		static void (*WriteRecord8)(UInt8 inData);
		static void (*WriteRecord16)(UInt16 inData);
		static void (*WriteRecord32)(UInt32 inData);

		static void WriteRecordFloat(float value);
		static void WriteRecordString(std::string value);
		static void WriteRecordString(const char* value);

		static bool (*WriteRecordData)(const void* buf, UInt32 length);
		static bool (*OpenRecord)(UInt32 type, UInt32 version);

		static void logOperation(const std::string& operation);
		static void clearLog();

	};

	struct SaveDataLink {

		UInt32 parentID;
		Instance* instance = nullptr;

		SaveDataLink(TESForm* parent, Instance* instance) : parentID(parent->refID), instance(instance) {
			forceQueueToSave(parent);
		}

		bool operator==(const SaveDataLink& other) const {
			return parentID == other.parentID;
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

		SaveDataEntry(TESForm* parent, Instance* parentInst = nullptr, Instance* self = nullptr, ExtraDataList* xData = nullptr, SInt32 countDelta = 1)
		: SaveDataREFR(parent, parentInst, self), xData(xData), countDelta(countDelta){
			type = 1;
		}

		SInt32 countDelta;
		ExtraDataList* xData; //Free extra data, or reuse it

	};

	struct SaveDataWorldREFR : SaveDataEntry {


		SaveDataWorldREFR(UInt32 worldRefID, TESForm* parent, Instance* parentInst = nullptr, Instance* self = nullptr, ExtraDataList* xData = nullptr, SInt32 countDelta = 1,
			float x = 0, float y = 0, float z = 0,
			float xR = 0, float yR = 0, float zR = 0
		) :
			SaveDataEntry(parent, parentInst, self, xData, countDelta),
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

	struct InstanceComparator {
		bool operator()(const Instance* lhs, const Instance* rhs) const {
			if (!lhs || !rhs) return lhs < rhs;  // Handle potential null pointers
			return lhs->baseInstance->extendedType < rhs->baseInstance->extendedType;
		}
	};

	class InstanceSaveManager : public SaveData {

	private:

		struct SaveQueue {

			typedef UInt32 RefID;
			typedef UInt32 VIndex;
			std::unordered_map<RefID, VIndex> searchLocation;
			std::vector<RefID> locations;
			//std::unordered_map<ExtendedBaseType*, WorldReferences> combinedMap;
			std::unordered_map<ExtendedBaseType*, std::unordered_set<SaveDataREFR*>> dynamicsRefs;

			using InstancesToSave = std::unordered_map<Instance*, std::unordered_set<SaveDataREFR*>>;
			using BaseTypesToSave = std::unordered_map<ExtendedBaseType*, InstancesToSave>;
			using ExtDataToSave = std::map<UInt32, BaseTypesToSave>;

			ExtDataToSave instanceRefs;

			//All string EditorID locations
			TESObjectREFR* queueToSave(TESObjectREFR* thisObj);
			bool queueToSaveRefDirect(TESObjectREFR* thisObj);

			bool queueToSave(TESForm* thisObj);

			//For instances that save regardless
			void queueToSave(ExtendedBaseType* thisObj);

			//Store instances in blocks
			bool queueToSave(Instance* thisObj, bool forceQueue);
			void queueToSaveWEAP(Instance_WEAP* thisObj);
			void queueToSaveAkimbo(Instance_Akimbo* thisObj);

			void queueToSaveRef(Instance* instance, SaveDataREFR* worldRef);		//World reference derived from instances
			void queueToSaveRef(ExtendedBaseType* base, SaveDataREFR* worldRef);	//Dynamics created with Devkit

			bool isQueueToSave(Instance* instance);
			void queueAllWithSaveBehavior();

			void queueAuxValue(const AuxValue& value);

		};

	public:

		SaveQueue queue;

		InstanceSaveManager() = default;

		//Saves all refs associated with a baseform/instance
		void SaveRefsList(const std::unordered_set<SaveDataREFR*>& refList);

		//Deduces Type to cast down to
		void SaveBase(ExtendedBaseType* thisObj);
		void SaveInstance(Instance* thisObj);

		void Save(StaticInstance* thisObj);
		void Save(StaticInstance_Akimbo* thisObj);

		void Save(Kit::KitData* thisObj);
		void SaveUninstaller(Script* thisObj);
		void Save(Instance* thisObj);
		void Save(Instance_WEAP* thisObj);
		void Save(Instance_Akimbo* thisObj);
		void SaveLifecycle(LifecycleManager& thisObj);

		void Save(const SaveDataREFR& thisObj);
		void Save(const SaveDataEntry& thisObj);
		void Save(const SaveDataWorldREFR& thisObj);
		void RestoreEntryRefs(const SaveDataEntry& thisObj);
		void RestoreWorldRefs(const SaveDataWorldREFR& thisObj);

		void Save(TESForm* thisObj);
		void Save(TESObjectREFR* thisObj);
		void SaveExtraData(ExtraDataList* xDataList);

		void SaveLink(const SaveDataLink& thisObj);
		void SaveLinkType(UInt8 type, const UInt32* ID32, const UInt16* ID16 = nullptr);

		void SaveFormTraits(TESForm* form);
		void SaveAuxVector(const AuxVector& auxVector);
		void SaveAuxValue(const AuxValue& value);

		UInt32* GetSaveIndex(UInt32 thisObj);

		void saveAll();

		void reset();

	};

	struct LoadDataLink : SaveData {

		TESForm* parent = nullptr;
		UInt16 instID = InvalidInstID;	//Invalid
		UInt32 refID = 0;
		UInt8 saveType = 0;

		LoadDataLink(TESForm* parent, UInt32 refID, UInt16 instID, UInt8 saveType) : parent(parent), refID(refID), instID(instID), saveType(saveType){}

	};

	struct LoadDataREFR {

		TESForm* baseform; //Self
		LoadDataLink location;

		LoadDataREFR(TESForm* baseform, LoadDataLink location) : baseform(baseform), location(location) {}

	};

	struct LoadDataEntry : LoadDataREFR {

		LoadDataEntry(TESForm* baseform, LoadDataLink location, ExtraDataList* xData = nullptr, SInt32 countDelta = 1)
			: LoadDataREFR(baseform, location), xData(xData), countDelta(countDelta) {}

		SInt32 countDelta;
		ExtraDataList* xData;

	};

	struct LoadDataWorldREFR : LoadDataEntry {

		LoadDataWorldREFR(TESForm* baseform, LoadDataLink location, ExtraDataList* xData = nullptr, SInt32 countDelta = 1, UInt32 worldRefID = 0,
			float x = 0, float y = 0, float z = 0,
			float xR = 0, float yR = 0, float zR = 0
		) :
			LoadDataEntry(baseform, location, xData, countDelta),
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

		FailedAkimboLink(Instance_Akimbo* weap, LoadDataLink left, LoadDataLink right) : weap(weap), left(left), right(right) {}

		Instance_Akimbo* weap;
		LoadDataLink left;
		LoadDataLink right;
	};

	class InstanceLoadManager : public SaveData {

	private:

		TESForm* replacement = nullptr;

		std::unordered_map<UInt32, Kit::KitData*> reverseKitDataMap;
		std::unordered_map<UInt32, TESForm*> baseLocations; //Locations where REFR are, either containers or world spaces.
		std::unordered_map<UInt32, TESObjectREFR*> refLocations; //Objects to be placed in the world.

		//Instances that rely on other instances, not used anymore
		std::vector<FailedWeaponLink*> failedLinks_WEAP;
		std::vector<FailedAkimboLink*> failedLinks_Akimbo;
		//std::vector<Instance*> reconstructEvent;

		std::vector<LoadDataWorldREFR*> worldRefs;
		std::vector<LoadDataEntry*> entries;
		std::vector<LoadDataREFR*> linkRefs; //Not used atm

	public:

		InstanceLoadManager() = default;

		void LoadStaticLocations(UInt32 index);

		//ESPs
			void LoadESPData();

		//Kit data
			Kit::KitData* LoadKitData();
			UInt32* resolveModIndex(UInt32 oldIndex);

		//Instances

			Instance* LoadInstance(ExtendedBaseType* baseInstance, UInt32 modIndex);

			Instance* LoadInstanceData(StaticInstance* baseInstance, UInt32 modIndex);
			Instance_WEAP* LoadInstanceDataWEAP(StaticInstance_WEAP* baseInstance, UInt32 modIndex);
			Instance_Akimbo* LoadInstanceDataAkimbo(StaticInstance_Akimbo* baseInstance, UInt32 modIndex);
			LifecycleManager* LoadLifecycle();

			void					LoadInstanceLinksWEAP();
			//void					LoadInstanceLinksAkimbo();

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
			void LoadLifecycle_Skip();

		//Utility

			TESForm* LoadFromIndex(UInt32 index);
			TESObjectREFR* LoadFromRefID(UInt32 refID);

			void clearInstances();
			void loadAll();
			void reset();
			//void ReconstructEvent();
			bool ReconstructEvent(Instance* inst);
			bool ReconstructEvent(Instance* inst, TESForm* attachment);

			//For situations where we are unsure what the link data could be.
			TESForm* deduceLinkToForm(LoadDataLink* link);
			
	};

}