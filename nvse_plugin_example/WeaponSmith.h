#pragma once
#include "ppNVSE.h"

extern std::vector<UInt32> aCloneRebuild; //For loading
extern std::unordered_map<UInt32, WeapInstBase*> BaseExtraData;
extern std::unordered_map<UInt32, WeapInst*> WeapInstList;

extern std::vector<UInt32> aUsedClones;
extern std::vector<UInt32> aClones;

class WeapInst {
public:

	WeapInst(
		TESForm* Parent = nullptr,
		UInt8 refID = 0,
		TESForm* Clone = nullptr,
		const std::unordered_map<std::string, UInt32>& aAttachments = std::unordered_map<std::string, UInt32>())
		: Parent(Parent),
		refID(refID),
		Clone(Clone),
		aAttachments(aAttachments) {
	}

	TESForm* Parent;						//The true baseform
	UInt8 refID;	//== the number of Weapon Instances for a specific baseform. Created via the ParentInstCounter.
	TESForm* Clone;							//Dynamic baseform
	std::unordered_map<std::string, UInt32> aAttachments;

};

class WeapInstBase {
public:

	WeapInstBase(
		TESForm* Parent = nullptr,
		const std::vector<WeapInst*>& aInstances = std::vector<WeapInst*>(),
		const std::unordered_map<std::string, UInt32>& aBaseAttachments = std::unordered_map<std::string, UInt32>())
		: Parent(Parent),
		aInstances(aInstances),
		aBaseAttachments(aBaseAttachments) {
	}

	TESForm* Parent;						//The true baseform
	std::vector<WeapInst*> aInstances;		//Save dependent

	std::unordered_map<std::string, UInt32> aBaseAttachments;
	//bool IsAkimbo;

};