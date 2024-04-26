#pragma once
#include "Actor_Ext.h"
#include "ppNVSE.h"

//Animation canceling

//std::vector<BSAnimGroupSequence*> aAnimToSkip; if I ever figure out how to specify an animation path, and not just by group.

std::vector<QueueAnim> queueToSkipGroup;

bool Actor::QueueToSkipGroup(UInt16 GroupID)
{
	QueueAnim stopGroup;

	if (this->IsPlayer() && !((PlayerCharacter*)this)->bThirdPerson) {
		stopGroup = { GroupID, this, 0.1, 6, 4, 1 };	//stop 4 anim groups
	}
	else {
		stopGroup = { GroupID, this, 0.1, 6, 2, 1 };	//stop 2 anim groups
	}

	queueToSkipGroup.push_back(stopGroup);

	return true;
}

bool Actor::SilentUnequip(TESForm* item, ExtraDataList* xData) {

	this->QueueToSkipGroup(kAnimGroup_Holster);
	this->UnequipItem(item, 1, xData, 1, false, 0);

	return true;

}

bool Actor::SilentEquip(TESForm* item, ExtraDataList* xData) {

	this->QueueToSkipGroup(kAnimGroup_Equip);
	this->EquipItem(item, 1, xData, 1, false, 0);

	return true;

}


bool Actor::ReplaceInvObject(TESForm* form, InventoryRef* replace, UInt32 count, bool copy) {

	//Console_Print("ReplaceInvObject");
	ExtraDataList* xData = replace->data.xData;
	ExtraDataList* xDataCopy = nullptr;
	bool doEquip = 0;
	bool weaponOut = 0;

	if (xData->HasType(kExtraData_Worn)) {

		BaseProcess* baseprocess = this->baseProcess;
		if (baseprocess) {
			weaponOut = baseprocess->IsWeaponOut();

			//if (xData->HasType(kExtraData_CannotWear))
				//xData->RemoveByType(kExtraData_CannotWear);

			//SilentUnequip(form, xData);
			//this->UnequipItem(replace->data.type, 1, xData, 1, 0, 0);

		}
		//else{
			//this->UnequipItem(replace->data.type, 1, xData, 1, 0, 0);
		//}
		doEquip = 1;

	}

	if (copy) {
		xDataCopy = xData->CreateCopy();
	}

	if (doEquip) {

		if (xDataCopy && xDataCopy->HasType(kExtraData_Worn))
			xDataCopy->RemoveByType(kExtraData_Worn);

		//Console_Print("Additem...........................1");
		this->AddItem(form, xDataCopy, count);

		if (weaponOut) {
			this->SilentEquip(form, xDataCopy);
		}
		else {
			this->EquipItem(form, 1, xDataCopy, 1, 0, 0);
		}

	}
	else {
		//Console_Print("Additem...........................2");
		this->AddItem(form, xDataCopy, count);
	}

	this->RemoveItem(replace->data.type, xData, count, 0, 0, nullptr, 0, 0, 1, 0);

	return true;

}

namespace Hooks
{

	int __fastcall StopAnimationType(AnimData* animData, void* edx, UInt16 GroupID, int a3, float a4, int a5)
	{

		if (queueToSkipGroup.empty()) {
			return ThisStdCall<int>(0x0494740, animData, GroupID, a3, a4, a5);
		}

		const auto baseAnimGroup = static_cast<AnimGroupID>(GroupID);

		for (auto it = queueToSkipGroup.begin(); it != queueToSkipGroup.end(); ) {

			if (it->groupId == baseAnimGroup && it->actor == animData->actor) {

				int iReturn = ThisStdCall<int>(0x0494740, animData, GroupID, a3, a4, a5);
				animData->actor->RefreshAnimData();
				//Console_Print("Stopping animation %d", baseAnimGroup);

				it->wait = 0;
				if (--it->iter <= 0) {
					//Console_Print("Erased animation %d", baseAnimGroup);
					it = queueToSkipGroup.erase(it);
				}

				return iReturn;

			}
			else {
				++it;
			}
		}

		return ThisStdCall<int>(0x0494740, animData, GroupID, a3, a4, a5);

	}

	//int __fastcall StopAnimationType(AnimData* animData, void*, BSAnimGroupSequence* destAnim, UInt16 animGroupId, eAnimSequence animSequence)
	//{

		//const auto baseAnimGroup = static_cast<AnimGroupID>(animGroupId);

		/*if (!animGroupToActorsMap.empty()) {
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

			}*/
		//}

		//return ThisStdCall<int>(0x04949A0, animData, destAnim, animGroupId, animSequence);

	//}

	//WriteRelCall(0x494989, (UInt32)StopAnimationType); //Play Animation Group Hook

}