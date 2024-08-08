#pragma once
#include "Actor_Ext.h"
#include "EventHandlers.h"
#include "ppNVSE.h"
#include "WeaponSmith.h"

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
	if (ContChangesEntry* entry = this->GetContainerChangesEntry(item)) {
		this->EquipItemAlt(entry, 0, 1);
	}

	return true;

}

bool Actor::SilentEquip(ContChangesEntry* entry) {

	this->QueueToSkipGroup(kAnimGroup_Equip);
	if (entry) {
		this->EquipItemAlt(entry, 0, 1);
	}

	return true;

}

__declspec(naked) void __fastcall DoFireWeaponEx(TESObjectREFR* refr, int, TESObjectWEAP* weapon)
{
	__asm
	{
		mov		edx, [ecx + 0x68]
		push	dword ptr[edx + 0x118]
		and dword ptr[edx + 0x118], 0
		push	dword ptr[edx + 0x114]
		and dword ptr[edx + 0x114], 0
		push	edx
		push	ecx
		mov		ecx, [esp + 0x14]
		__asm mov eax, 0x523150 __asm call eax
		pop		eax
		pop		dword ptr[eax + 0x114]
		pop		dword ptr[eax + 0x118]
		mov		ecx, [eax + 0x3D4]
		test	ecx, ecx
		jz		done
		mov		byte ptr[ecx + 3], 1
		done:
		retn	4
	}
}


TESObjectREFR* Actor::ReplaceInvObject(TESForm* form, InventoryRef* replace, UInt32 count, bool copy) {

	ExtraDataList* xData = replace->data.xData;
	ExtraDataList* xDataCopy = nullptr;
	bool doEquip = 0;
	bool weaponOut = 0;

	TESObjectREFR* result= nullptr;

	if (xData && copy) {
		xDataCopy = ExtraDataList::CopyItemData(xData, 0);
	}

	if (xData && xData->HasType(kExtraData_Worn)) {

		BaseProcess* baseprocess = (BaseProcess* )this->baseProcess;
		if (baseprocess && g_interfaceManager->currentMode == 1) {
			weaponOut = true;
		}

		this->UnequipItem(replace->data.type, count, xData, 1, 0, 0);
		doEquip = 1;

	}

	this->RemoveItem(replace->data.type, xData, count, 0, 0, nullptr, 0, 0, 1, 0);

	this->AddItemAlt(form, count, 1);
	ContChangesEntry* entry = GetContainerChangesEntry(form);
	ContChangesEntryList* testlist = GetContainerChangesList();

	if (copy && entry && entry->extendData) {
		xData = entry->extendData->GetFirstItem();
		ExtraDataList::CopyItemData(xDataCopy, 1, xData);
	}
	else {
		xData = nullptr;
	}

	if (doEquip) {

		if (weaponOut) {
			this->SilentEquip(entry);
		}
		else {
			//this->EquipItem(form, entry->countDelta, xData, 1, 0, 0);
			this->EquipItemAlt(entry, 0, 1);
		}

	}

	return InventoryRef::InventoryRefCreateEntry(this, form, count, xData);

}

namespace Hooks
{

	int __fastcall OnEquipAlt(TESObjectREFR* equipper, void* edx, TESForm* item, SInt32 count, ExtraDataList* xData, int noMessage, bool lockEquipment, bool playsound)
	{

		if (!onEquipAltEvent.handlers.empty()) {

			double skip = 0;
			AuxVector filter{ static_cast<double>(item->typeID),  item, equipper };
			for (auto it = onEquipAltEvent.handlers.begin(); it != onEquipAltEvent.handlers.end(); ++it) {

				if (it->CompareFilters(filter)) {

					ArrayElementL scriptReturn;
					TESObjectREFR* invItem = InventoryRef::InventoryRefCreateEntry(equipper, item, count, xData);
					g_scriptInterface->CallFunction(it->script, nullptr, nullptr, &scriptReturn, 2, invItem, equipper);

					if (scriptReturn.IsValid()) {

						if (scriptReturn.GetType() == 2) {	//Form

							TESForm* form = scriptReturn.Form();
							InventoryRef* invRef = InventoryRef::InventoryRefGetForID(form->refID); //Check if is Inventory ref.

							if (invRef) {

								item = invRef->data.type; //Equip this inv ref instead
								xData = invRef->data.xData;
								count = invRef->GetCount();

							}
							else if (form->typeID == item->typeID) { //Same type

								float health = 100.0F;
								ExtraHealth* xHealth;
								if (xData && (xHealth = (ExtraHealth*)(xData)->GetByType(kXData_ExtraHealth))) {
									InventoryRef* tempRef = InventoryRef::InventoryRefGetForID(invItem->refID);
									health = tempRef->data.entry->GetHealthPercent();
								}
								health *= 0.01F;
								equipper->AddItemAlt(form, count, health);
								equipper->RemoveItem(item, xData, count, 0, 0, nullptr, 0, 0, 1, 0);
								item = form;
								xData = equipper->GetContainerChangesEntry(item)->extendData->GetFirstItem();

							}
							else { //Not same type, console error
								Console_Print("Error in script %s, OnEquipAlt passed a baseform not of the same type. Required type: %d, Got:", it->script->GetEditorID(), item->typeID, form->typeID);
							}

						}
						else if (scriptReturn.GetType() == 1) {	//Num
							skip = scriptReturn.Number();
						}

					}

				}


			}

			if (skip != 0) {
				return 0;
			}

		}
		const char* EDITORID = item->GetEditorID();
		return ThisStdCall<int>(0x088C830, equipper, item, count, xData, noMessage, lockEquipment, playsound);

	}

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

/*
TESObjectREFR* Actor::ReplaceInvObject(TESForm* form, InventoryRef* replace, UInt32 count, bool copy) {

	//Console_Print("ReplaceInvObject");
	ExtraDataList* xData = replace->data.xData;
	ExtraDataList* xDataCopy = nullptr;
	bool doEquip = 0;
	bool weaponOut = 0;
	float health = 100.0F;
	TESObjectREFR* result;

	if (xData && copy) {

		//if (replace->GetCount() > count) {
			//xDataCopy = replace->SplitFromStack(count);
		//}
		//else {
			//xDataCopy = xData->CreateCopy();
		//}

		ExtraHealth* xHealth;
		if (xData && (xHealth = (ExtraHealth*)(xData)->GetByType(kXData_ExtraHealth))) {
			health = replace->data.entry->GetHealthPercent();
		}
		health *= 0.01F;

	}

	if (xData && xData->HasType(kExtraData_Worn)) {

		BaseProcess* baseprocess = (BaseProcess*)this->baseProcess;
		if (baseprocess) {

			//if (xData->HasType(kExtraData_CannotWear))
				//xData->RemoveByType(kExtraData_CannotWear);

			//SilentUnequip(replace->data.type, xData);
			if (g_interfaceManager->currentMode == 1) {
				weaponOut = true;
				//this->UnequipItem(replace->data.type, 1, xData, 1, 0, 0);
			}

		}
		//else{
		this->UnequipItem(replace->data.type, count, xData, 1, 0, 0);
		//}
		doEquip = 1;

	}

	if (doEquip) {

		//if (xDataCopy && xDataCopy->HasType(kExtraData_Worn))
			//xDataCopy->RemoveByType(kExtraData_Worn);

		//Console_Print("Additem...........................1");
		//this->AddItem(form, xDataCopy, count);				//This was giving me bugs for some reason.

		this->AddItemAlt(form, count, health);

		result = InventoryRef::InventoryRefCreateEntry(this, form, count, xData);
		InventoryRef* invItem = InventoryRef::InventoryRefGetForID(result->refID);
		invItem->data.xData = xData->CreateCopy();

		if (weaponOut) {
			this->SilentEquip(form, invItem->data.xData);
		}
		else {
			this->EquipItem(form, count, invItem->data.xData, 1, 0, 0);
		}

	}
	else {
		//Console_Print("Additem...........................2");
		this->AddItemAlt(form, count, health);
		result = InventoryRef::InventoryRefCreateEntry(this, form, count, xData);
		InventoryRef* invItem = InventoryRef::InventoryRefGetForID(result->refID);

		//this->AddItem(form, xDataCopy, count);
	}

	invItem->data.xData = xData->CreateCopy();
	this->RemoveItem(replace->data.type, xData, count, 0, 0, nullptr, 0, 0, 1, 0);

	return result;

}*/