#pragma once
#include "SaveSystem.h"

namespace SaveSystem {

	void SaveExtraData(ExtraDataList* xDataList)
	{
		if (xDataList) {
			BSExtraData* extraData = xDataList->m_data;

			while (extraData) {

				switch (extraData->type) {
				case kXData_ExtraOwnership: {
					ExtraOwnership* xData = (ExtraOwnership*)extraData;
					if (xData)
					{
						SaveData::WriteRecord8(extraData->type);
						SaveData::WriteRecord32(xData->owner->refID);
					}
				}
										  break;
				case kXData_ExtraCount: {

					ExtraCount* xData = (ExtraCount*)extraData;
					if (xData)
					{
						SaveData::WriteRecord8(extraData->type);
						SaveData::WriteRecord32(xData->count);
					}
				}
									  break;
				case kXData_ExtraHealth: {

					ExtraHealth* xData = (ExtraHealth*)extraData;
					if (xData)
					{
						SaveData::WriteRecord8(extraData->type);
						SaveData::WriteRecordFloat(xData->health);
					}
				}
									   break;
				case kXData_ExtraHotkey: {

					ExtraHotkey* xData = (ExtraHotkey*)extraData;
					if (xData)
					{
						SaveData::WriteRecord8(extraData->type);
						SaveData::WriteRecord8(xData->index);
					}
				}
									   break;
				case kXData_ExtraWorn: {

					ExtraWorn* xData = (ExtraWorn*)extraData;
					if (xData)
					{
						SaveData::WriteRecord8(extraData->type);
					}
				}
									   break;
				default:
					break;
				}

				extraData = extraData->next;

			}

		}

		SaveData::WriteRecord8(kXData_ExtraUnknown00);

	}

	void SaveCollectedData() {

		UInt32 TotalSize = aSaveData.size();
		SaveData::WriteRecord32(TotalSize);

		TESForm* object = nullptr;
		TESForm* rLocation = nullptr;

		for (auto it = aSaveData.begin(); it != aSaveData.end(); ++it) {

			SaveDataObj* saveData = (SaveDataObj*)*it;
			if (saveData->location) {
				rLocation = LookupFormByRefID(saveData->location);
			}

			if (rLocation) {

				UInt32 length;

				if (saveData->staticInst) {

					object = saveData->staticInst->parent;
					SaveData::WriteRecord8(0);									//Was dynamic

					const char* EditorID = object->GetEditorID();
					length = strlen(EditorID);
					SaveData::WriteRecord32(length);
					SaveData::WriteRecordData(EditorID, length);

				}
				else if (saveData->inst){

					object = saveData->inst->clone;
					SaveData::WriteRecord8(1);			//Was an instance

					const char* EditorID = saveData->inst->baseInstance->parent->GetEditorID();
					length = strlen(EditorID);
					SaveData::WriteRecord32(length);
					SaveData::WriteRecordData(EditorID, length);

					SaveData::WriteRecord8(saveData->inst->InstID);

				}
				else {
					Console_Print("Error, instance and static instance both invalid");
				}

				SaveData::WriteRecord32(saveData->location);

				ExtraDataList* xData = saveData->xData;
				SaveExtraData(xData);

				if (rLocation->typeID != 57 && rLocation->typeID != 65) {

					SaveData::WriteRecord8(rLocation->typeID); //Location type

					if (saveData->xData && saveData->xData->HasType(kExtraData_Worn)) {
						saveData->xData->RemoveByType(kExtraData_Worn);
						((TESObjectREFR*)rLocation)->AddItem(object, saveData->xData, 1);
						((Actor*)rLocation)->SilentEquip(object, saveData->xData);
					}
					else {
						((TESObjectREFR*)rLocation)->AddItem(object, saveData->xData, 1);
					}

				}
				else if (rLocation->typeID == 57 || rLocation->typeID == 65){

					SaveData::WriteRecord8(rLocation->typeID); //Location type

					SaveDataWorldObj* saveDataWorld = (SaveDataWorldObj*)saveData;

					SaveData::WriteRecordFloat(saveDataWorld->x);
					SaveData::WriteRecordFloat(saveDataWorld->y);
					SaveData::WriteRecordFloat(saveDataWorld->z);

					SaveData::WriteRecordFloat(saveDataWorld->xR);
					SaveData::WriteRecordFloat(saveDataWorld->yR);
					SaveData::WriteRecordFloat(saveDataWorld->zR);
					
					TESObjectREFR* placeref = object->PlaceAtCell(rLocation, saveDataWorld->x, saveDataWorld->y, saveDataWorld->z, saveDataWorld->xR, saveDataWorld->yR, saveDataWorld->zR);
					if (xData) {
						placeref->extraDataList = *xData;
					}

				}
			}

			delete saveData;
		}
		aSaveData.clear();
	}

	void SaveWeaponInstances(StaticInstance* Base) {

		for (Instance* instIt : Base->aInstances) {

			Instance_WEAP* rInstance = (Instance_WEAP*)instIt;

			if (rInstance) {

				UInt32 length = rInstance->key.length();

				SaveData::WriteRecord32(length);
				SaveData::WriteRecordData(rInstance->key.c_str(), length);

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
						const char* EditorID = attachment->GetEditorID();

						length = strlen(EditorID);
						SaveData::WriteRecord32(length);
						SaveData::WriteRecordData(EditorID, length);

					}
				}

			}

		}

	}

	void SaveInstances(StaticInstance* Base) {

		for (Instance* rInstance : Base->aInstances) {

			if (rInstance) {

				UInt32 length = rInstance->key.length();

				SaveData::WriteRecord32(length);
				SaveData::WriteRecordData(rInstance->key.c_str(), length);

			}

		}

	}

	void SaveGameCallback(void*)
	{

		SaveData::OpenRecord(1, 2);

		SaveData::WriteRecord32(static_cast<UInt32>(StaticLinker[40].size()));	//Save the count of base forms that have Static Instances.

		for (auto it = StaticLinker[40].begin(); it != StaticLinker[40].end(); ++it) {

			StaticInstance* Base = it->second;

			TESForm* parent = LookupFormByRefID(it->first);

			const char* EditorID = parent->GetEditorID();
			UInt32 length = strlen(EditorID);
			SaveData::WriteRecord32(length);
			SaveData::WriteRecordData(EditorID, length);

			SaveData::WriteRecord8(static_cast<uint8_t>(Base->aInstances.size()));	//Save the count of instances iterated over.
			SaveData::WriteRecord8(Base->parent->typeID);

			switch (Base->parent->typeID) {
			case 40:
				SaveWeaponInstances(Base);
				break;
			case 103:
				SaveInstances(Base);
				break;
			default:
				SaveInstances(Base);
				break;
			}

		}

		SaveCollectedData();

	}

}