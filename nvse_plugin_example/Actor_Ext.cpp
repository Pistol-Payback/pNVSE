#pragma once
#include "Actor_Ext.h"
#include "ppNVSE.h"

//Animation canceling

bool g_SkipAnimation;
std::vector<Actor*> g_aSkipCurrentAnimation;

//std::vector<BSAnimGroupSequence*> aAnimToSkip; if I ever figure out how to specify an animation path, and not just by group.

std::unordered_map<UInt16, std::vector<Actor*>> animGroupToActorsMap;

bool Actor::QueueToSkipAnimation()
{
	g_aSkipCurrentAnimation.push_back(this);
	g_SkipAnimation = true;
	return true;
}

bool Actor::QueueToSkipGroup(UInt16 GroupID)
{
	animGroupToActorsMap[GroupID].push_back(this);
	return true;
}

bool Actor::SilentUnequip(TESForm* item, ExtraDataList* xData) {

	this->UnequipItem(item, 1, xData, 1, 0, 0);
	if (!this->baseProcess->processLevel) {
		this->QueueToSkipGroup(kAnimGroup_Holster);
	}

	return true;

}

bool Actor::SilentEquip(TESForm* item, ExtraDataList* xData) {

	this->EquipItem(item, 1, xData, 1, 0, 0);
	if (!this->baseProcess->processLevel) {
		this->QueueToSkipGroup(kAnimGroup_Equip);
	}

	return true;

}

bool Actor::ReplaceInvObject(TESForm* form, InventoryRef* replace, UInt32 count, bool copy) {

	ExtraDataList* xData = replace->data.xData;
	ExtraDataList* xDataCopy = nullptr;
	bool doEquip = 0;
	bool weaponOut = 0;

	if (xData->HasType(kExtraData_Worn)) {

		BaseProcess* baseprocess = this->baseProcess;
		if (baseprocess) {
			this->baseProcess->RemoveAllItemsFromQueue();
			weaponOut = baseprocess->IsWeaponOut();
			this->SilentUnequip(replace->data.type, xData);
		}
		else {
			this->UnequipItem(replace->data.type, 1, xData, 1, 0, 0);
		}
		doEquip = 1;

	}
	if (copy) {
		xDataCopy = xData->CreateCopy();
	}

	this->RemoveItem(replace->data.type, xData, count, 0, 0, nullptr, 0, 0, 1, 0);
	this->AddItem(form, xDataCopy, count);

	if (doEquip) {
		if (weaponOut) {
			this->SilentEquip(form, xDataCopy);
		}
		else {
			this->EquipItem(form, 1, xDataCopy, 1, 0, 0);
		}

	}

	return true;

}

namespace Hooks
{

	int __fastcall StopAnimationType(AnimData* animData, void*, BSAnimGroupSequence* destAnim, UInt16 animGroupId, eAnimSequence animSequence)
	{

		const auto baseAnimGroup = static_cast<AnimGroupID>(animGroupId);

		if (!animGroupToActorsMap.empty()) {
			//Console_Print("Ran HookDetourTemp, Not Empty");
			Actor* actor = animData->actor;
			auto it = animGroupToActorsMap.find(baseAnimGroup);

			if (it != animGroupToActorsMap.end()) {
				//Console_Print("Ran HookDetourTemp, Found");
				auto& actorsVector = it->second;
				for (auto it = actorsVector.begin(); it != actorsVector.end(); ++it) {

					if (*it == actor) {

						//Console_Print("Ran HookDetourTemp, Matching");
						actor->QueueToSkipAnimation();
						actorsVector.erase(it);
						if (actorsVector.empty()) {
							animGroupToActorsMap.erase(baseAnimGroup);
						}
						actor->RefreshAnimData();
						break;
					}

				}

			}
		}

		return ThisStdCall<int>(0x04949A0, animData, destAnim, animGroupId, animSequence);

	}

}