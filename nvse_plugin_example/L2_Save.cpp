#pragma once
#include "SaveSystem.h"

namespace SaveSystem {

	void SaveCollectedData() {

		//Console_Print("SaveCollectedData");

		UInt32 TotalLength = aSaveData.size();
		//Console_Print("SaveGameCallback size %d", TotalLength);
		WriteRecord32(TotalLength);

		TESForm* rWeapon = nullptr;
		TESObjectREFR* form = nullptr;

		for (auto it = aSaveData.begin(); it != aSaveData.end(); ++it) {

			SaveData* saveData = *it;
			WeapInst* Instance = LookupFormByRefID(saveData->baseRefID)->LookupModifierByID(saveData->instID);
			if (Instance) {
				rWeapon = Instance->Clone; //Add this
				if (saveData->location) {
					form = (TESObjectREFR*)LookupFormByRefID(saveData->location);
				}
			}

			if (rWeapon && form) {

				WriteRecord32(saveData->baseRefID);
				WriteRecord8(saveData->instID);
				WriteRecord32(saveData->location);

				if (saveData->location) {

					WriteRecord8(saveData->equipped);

					ExtraDataList* xData = saveData->xData;
					BSExtraData* mData = xData->m_data;

					while (mData) {
						WriteRecord8(mData->type);
						//SaveExtraData(mData);
						mData = mData->next;
					}

					if (saveData->xData->HasType(kExtraData_Worn)) {
						saveData->xData->RemoveByType(kExtraData_Worn);
					}
					form->AddItem(rWeapon, saveData->xData, 1);

					if (saveData->equipped) {
						//SilentEquip((Actor*)form, rWeapon, saveData->xData);
					}

				}
				else {
					WriteRecord32(saveData->x);
					WriteRecord32(saveData->y);
					WriteRecord32(saveData->z);
					WriteRecord32(saveData->xR);
					WriteRecord32(saveData->yR);
					WriteRecord32(saveData->zR);
				}
			}

			delete saveData;
		}
		aSaveData.clear();
	}

	void SaveGameCallback(void*)
	{
		//WriteRecord(1, 2, "W", 2);
		//Console_Print("SaveGameCallback");

		const char* EditorID;

		OpenRecord(1, 2);

		WriteRecord32(static_cast<UInt32>(BaseExtraData.size()));	//Save the count of base forms that have Weapon Instances.

		for (auto it = BaseExtraData.begin(); it != BaseExtraData.end(); ++it) {

			WeapInstBase* Base = it->second;

			//Console_Print("Saving Weapon: %s", LookupFormByRefID(it->first)->GetEditorID());
			//Console_Print("RefID %d", it->first);	//Load the count of baseforms.
			WriteRecord32(it->first);

			//Console_Print("Saving Instance array size: %d", static_cast<uint8_t>(Base->aInstances.size()));
			WriteRecord8(static_cast<uint8_t>(Base->aInstances.size()));	//Save the count of instances iterated over.

			for (auto& rInstance : Base->aInstances) {

				if (rInstance) {

					WriteRecord8(static_cast<uint8_t>(rInstance->aAttachments.size()));	//Save attachment size.
					//Console_Print("Saving Attachment size: %d", static_cast<uint8_t>(rInstance->aAttachments.size()));

					for (auto slot = rInstance->aAttachments.begin(); slot != rInstance->aAttachments.end(); ++slot) {

						const char* sSlot = slot->first.c_str();
						UInt32 rAttachment = slot->second;
						size_t length;
						if (rAttachment) {

							const auto& sSlot = slot->first;
							const char* sSlotCStr = sSlot.c_str();

							//Console_Print("Save Slot %s", sSlotCStr);
							auto length = sSlot.size();
							//Console_Print("Size %d", length);

							WriteRecord32(length);
							WriteRecordData(sSlotCStr, length);

							TESForm* attachment = LookupFormByRefID(rAttachment);
							EditorID = attachment->GetEditorID();
							//Console_Print("Save EditorID %s", EditorID);

							length = strlen(EditorID);
							//Console_Print("Size %d", length);
							WriteRecord32(length);
							WriteRecordData(EditorID, length);
						}
					}

				}

			}


		}

		SaveCollectedData();

	}

	void SaveExtraData(BSExtraData* extraData)
	{

		switch (extraData->type) {
		case kXData_ExtraWorn: {

			ExtraWorn* xData = (ExtraWorn*)extraData;
			if (xData)
			{
				WriteRecord8(kXData_ExtraWorn);
				xData->Create();
			}
		}
							 break;
		case kXData_ExtraOwnership: {
			ExtraOwnership* xData = (ExtraOwnership*)extraData;
			if (xData)
			{
				WriteRecord8(kXData_ExtraOwnership);
				xData->owner;
				xData->Create();
			}
		}
								  break;
		case kXData_ExtraCount: {

			ExtraCount* xData = (ExtraCount*)extraData;
			if (xData)
			{
				WriteRecord8(kXData_ExtraCount);
				xData->count;
				xData->Create();
			}
		}
							  break;
		case kXData_ExtraHealth: {

			ExtraHealth* xData = (ExtraHealth*)extraData;
			if (xData)
			{
				WriteRecord8(kXData_ExtraHealth);
				xData->health;
				xData->Create();
			}
		}
							   break;
		case kXData_ExtraCannotWear: {

			ExtraCannotWear* xData = (ExtraCannotWear*)extraData;
			if (xData)
			{
				WriteRecord8(kXData_ExtraCannotWear);
				xData->Create();
			}
		}
								   break;
		case kXData_ExtraHotkey: {

			ExtraHotkey* xData = (ExtraHotkey*)extraData;
			if (xData)
			{
				WriteRecord8(kXData_ExtraHotkey);
				xData->index;
				xData->Create();
			}
		}
							   break;
		default:
			// Handle default case
			break;
		}
	}
}