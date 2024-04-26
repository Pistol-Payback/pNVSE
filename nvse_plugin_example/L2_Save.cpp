#pragma once
#include "SaveSystem.h"

namespace SaveSystem {

	void SaveExtraData(ExtraDataList* xDataList)
	{

		BSExtraData* extraData = xDataList->m_data;

		while (extraData) {

			SaveData::WriteRecord8(extraData->type);

			switch (extraData->type) {
			case kXData_ExtraOwnership: {
				ExtraOwnership* xData = (ExtraOwnership*)extraData;
				if (xData)
				{
					SaveData::WriteRecord32(xData->owner->refID);
				}
			}
									  break;
			case kXData_ExtraCount: {

				ExtraCount* xData = (ExtraCount*)extraData;
				if (xData)
				{
					SaveData::WriteRecord32(xData->count);
				}
			}
								  break;
			case kXData_ExtraHealth: {

				ExtraHealth* xData = (ExtraHealth*)extraData;
				if (xData)
				{
					SaveData::WriteRecordFloat(xData->health);
				}
			}
								   break;
			case kXData_ExtraHotkey: {

				ExtraHotkey* xData = (ExtraHotkey*)extraData;
				if (xData)
				{
					SaveData::WriteRecord8(xData->index);
				}
			}
								   break;
			default:
				break;
			}

			extraData = extraData->next;

		}

		SaveData::WriteRecord8(kXData_ExtraUnknown00);

	}

	void SaveCollectedData() {

		UInt32 TotalLength = aSaveData.size();
		SaveData::WriteRecord32(TotalLength);

		TESForm* rWeapon = nullptr;
		TESForm* rLocation = nullptr;

		for (auto it = aSaveData.begin(); it != aSaveData.end(); ++it) {

			SaveDataObj* saveData = (SaveDataObj*)*it;
			Instance_WEAP* Instance = LookupFormByRefID(saveData->baseRefID)->LookupInstanceByID(saveData->instID);
			if (Instance) {
				rWeapon = Instance->clone; //Add this
				if (saveData->location) {
					rLocation = LookupFormByRefID(saveData->location);
				}
			}

			if (rWeapon && rLocation) {

				SaveData::WriteRecord32(saveData->baseRefID);
				SaveData::WriteRecord8(saveData->instID);
				SaveData::WriteRecord32(saveData->location);

				ExtraDataList* xData = saveData->xData;
				SaveExtraData(xData);

				if (rLocation->typeID != 57 && rLocation->typeID != 65) {

					if (saveData->xData->HasType(kExtraData_Worn)) {
						saveData->xData->RemoveByType(kExtraData_Worn);
						((TESObjectREFR*)rLocation)->AddItem(rWeapon, saveData->xData, 1);
						((Actor*)rLocation)->SilentEquip(rWeapon, saveData->xData);
					}
					else {
						((TESObjectREFR*)rLocation)->AddItem(rWeapon, saveData->xData, 1);
					}

				}
				else if (rLocation->typeID == 57 || rLocation->typeID == 65){

					Console_Print("Saving world references: %s", rWeapon->GetTheName());
					Console_Print("Saving In Location: %s", rLocation->GetTheName());

					SaveDataWorldObj* saveDataWorld = (SaveDataWorldObj*)saveData;

					SaveData::WriteRecordFloat(saveDataWorld->x);
					SaveData::WriteRecordFloat(saveDataWorld->y);
					SaveData::WriteRecordFloat(saveDataWorld->z);

					SaveData::WriteRecordFloat(saveDataWorld->xR);
					SaveData::WriteRecordFloat(saveDataWorld->yR);
					SaveData::WriteRecordFloat(saveDataWorld->zR);
					
					TESObjectREFR* placeref = rWeapon->PlaceAtCell(rLocation, saveDataWorld->x, saveDataWorld->y, saveDataWorld->z, saveDataWorld->xR, saveDataWorld->yR, saveDataWorld->zR);
					placeref->extraDataList = *xData;

				}
			}

			delete saveData;
		}
		aSaveData.clear();
	}

	void SaveGameCallback(void*)
	{

		const char* EditorID;

		SaveData::OpenRecord(1, 2);

		SaveData::WriteRecord32(static_cast<UInt32>(StaticInstance_WEAP::Linker.size()));	//Save the count of base forms that have Weapon Instances.

		for (auto it = StaticInstance_WEAP::Linker.begin(); it != StaticInstance_WEAP::Linker.end(); ++it) {

			StaticInstance_WEAP* Base = it->second;

			SaveData::WriteRecord32(it->first);
			SaveData::WriteRecord8(static_cast<uint8_t>(Base->aInstances.size()));	//Save the count of instances iterated over.

			for (auto& rInstance : Base->aInstances) {

				if (rInstance) {

					size_t length = rInstance->key.length();

					SaveData::WriteRecord32(length);
					SaveData::WriteRecordData(rInstance->key.c_str(), length);

					Console_Print("Saved Key: %s", rInstance->key.c_str());	//Load the count of baseforms.

					SaveData::WriteRecord8(static_cast<uint8_t>(rInstance->aAttachments.size()));	//Save attachment size.

					for (auto slot = rInstance->aAttachments.begin(); slot != rInstance->aAttachments.end(); ++slot) {

						const char* sSlot = slot->first.c_str();
						UInt32 rAttachment = slot->second;
						if (rAttachment) {

							const auto& sSlot = slot->first;
							const char* sSlotCStr = sSlot.c_str();

							length = sSlot.size();

							SaveData::WriteRecord32(length);
							SaveData::WriteRecordData(sSlotCStr, length);

							TESForm* attachment = LookupFormByRefID(rAttachment);
							EditorID = attachment->GetEditorID();

							length = strlen(EditorID);
							SaveData::WriteRecord32(length);
							SaveData::WriteRecordData(EditorID, length);
						}
					}

				}

			}


		}

		SaveCollectedData();

	}

}