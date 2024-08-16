#pragma once
#include "ppNVSE.h"

struct AnimDataLock;

struct AnimLockManager {

	struct Hash {
		size_t operator()(const AnimDataLock& adl) const;
		size_t hash_cstr(const char* str) const;
	};

	static std::unordered_set<AnimDataLock, Hash> allData; //Contains all AnimDataLock
	//static void UpdateAllAnims();

	static const AnimDataLock* getAnimLockData(UInt32 refID, const char* animName);
	static const AnimDataLock* findAnimLockData(UInt32 refID, const char* animName, bool deepSearch = 0);
	static bool hasAnimLockData(UInt32 refID, const char* animName, bool deepSearch = 0);
	
};

enum class OverrideType : UInt8 {
	Priority = 0x01,
	Rotation = 0x02,
	Translation = 0x04,
	Scale = 0x08
};

class BlockOverwrite {

private:
	struct Overrides {

		OverrideType type;
		union {
			int priority;
			NiQuaternion rotations;
			NiPoint3 translation;
			float scale;
		};

		Overrides(OverrideType type) : type(type) {}

	};
	UInt8 types = 0; // Bitmask of all override types present
	void addOrUpdateOverride(const Overrides& newOverride);

public:

	std::vector<Overrides> overrides;
	bool hasType(OverrideType oType) const;
	void removeOverride(OverrideType oType);
	void setPriority(int priority);
	void setRotation(const NiQuaternion& rotation);
	void setTranslation(const NiPoint3& translation);
	void setScale(float scale);
	void UpdateBlockOverride(NiControllerSequence::ControlledBlock* controlledBlock);

};

struct AnimDataLock {

	AnimDataLock(UInt32 refID, const char* animPath)
		: refID(refID), animPath(animPath) {
	}

	void destroy() const {
		AnimLockManager::allData.erase(*this);
	}

	void destroyAll() {
		AnimLockManager::allData.clear();
	}

	bool operator==(const AnimDataLock& other) const {
		if (refID != other.refID) return false;
		if (animPath == nullptr || other.animPath == nullptr) {
			return animPath == other.animPath;
		}
		return strcmp(animPath, other.animPath) == 0;
	}

	void SetPriority(const char* controlledBlock, UInt8 priority) const;

	/* Currently not working
	void SetRotation(const char* controlledBlock, float x, float y, float z);
	void SetScale(const char* controlledBlock, float scale);
	void SetTranslation(const char* controlledBlock, float x, float y, float z);
	*/

	void removeOverride(const char* block) const;
	void removeOverride(const char* block, OverrideType type) const;

	bool hasOverride(const char* block);
	bool hasOverride(const char* block, OverrideType type);

	void UpdateAllBlocks(NiControllerSequence* destAnim) const;
	bool IsAnimSequencePlaying(NiControllerSequence* destAnim) const;
	void AddAllCurrentAnimsToCache() const;

	UInt32 refID;
	const char* animPath;

	mutable std::unordered_set<NiControllerSequence*> animCache = {};
	mutable std::unordered_map<std::string, BlockOverwrite> blockOverwrites;

};