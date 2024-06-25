#pragma once
#include <thread>
#include "SaveSystem.h"

namespace SaveSystem {

	ExtraDataList* LoadExtraDataList()
	{

		BSExtraData* xData = nullptr;
		ExtraDataList* xDataList = ExtraDataList::Create();

		UInt8 type = SaveData::ReadRecord8();

		if (type == kXData_ExtraUnknown00) {
			return xDataList = nullptr;
		}

		while (type != kXData_ExtraUnknown00) {

			switch (type) {
			case kXData_ExtraWorn:
			{
				xData = ExtraWorn::Create();
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraOwnership:
			{
				UInt32 refID = SaveData::ReadRecord32();
				if (SaveData::ResolveRefID(refID, &refID) && refID) {
					xData = ExtraOwnership::Create();
					((ExtraOwnership*)xData)->owner = LookupFormByRefID(refID);
					xDataList->Add(xData);
				}

			}
			break;
			case kXData_ExtraCount:
			{
				UInt32 iCount = SaveData::ReadRecord32();
				xData = ExtraCount::Create();
				((ExtraCount*)xData)->count = iCount;
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraHealth:
			{
				float fHealth = SaveData::ReadRecordFloat();
				xData = ExtraHealth::Create(fHealth);
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraCannotWear:
			{
				xData = ExtraCannotWear::Create();
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraHotkey:
			{
				UInt8 iIndex = SaveData::ReadRecord8();
				xData = ExtraHotkey::Create(iIndex);
				xDataList->Add(xData);
			}
			break;
			default:
				break;
			}
			type = SaveData::ReadRecord8();

		}

		return xDataList;

	}

	void SkipExtraDataList()
	{

		UInt8 type = SaveData::ReadRecord8();

		while (type != kXData_ExtraUnknown00) {

			SaveData::SkipNBytes(1);

		}

	}

	void LoadCollectedData() {

		UInt32 TotalSize = SaveData::ReadRecord32();
		TESForm* rLocation = nullptr;

		while (TotalSize > 0) {

			UInt8 type = SaveData::ReadRecord8();			//0 = Is Instace dynamic, 1 = is an instance

			UInt32 length = SaveData::ReadRecord32();
			char* EditorID = new char[length + 1];
			SaveData::ReadRecordData(EditorID, length);
			EditorID[length] = '\0';

			if (TESForm * form = LookupEditorID<TESForm*>(EditorID)) {

				if (type) {	//Was an instance

					UInt8 instID = SaveData::ReadRecord8();		//Get instance using baseRefID and instID
					Instance* inst = form->LookupInstanceByID(instID);
					if (inst) {
						form = inst->clone;
					}
					else {
						Console_Print("uninstalled from the game: %s", EditorID);
					}

				}

				UInt32 location = SaveData::ReadRecord32();

				if (SaveData::ResolveRefID(location, &location) && location && form) {

					rLocation = LookupFormByRefID(location);

					ExtraDataList* xData = LoadExtraDataList();

					UInt8 locationCellType = SaveData::ReadRecord8();

					if (!rLocation || rLocation->typeID != locationCellType) { //Location does not exist anymore.

						if (locationCellType == 57 || locationCellType == 65) {
							SaveData::SkipNBytes(24);	//Skip x,y,z and rot for world pos.
						}
						continue;

					}

					if (rLocation->typeID != 57 && rLocation->typeID != 65) {

						if (xData && xData->HasType(kExtraData_Worn)) {
							xData->RemoveByType(kExtraData_Worn);
							((Actor*)rLocation)->AddItem(form, xData, 1);
							((Actor*)rLocation)->SilentEquip(form, xData);
						}
						else {
							((TESObjectREFR*)rLocation)->AddItem(form, xData, 1);
						}

					}
					else {	//World Object

						float x = SaveData::ReadRecordFloat();
						float y = SaveData::ReadRecordFloat();
						float z = SaveData::ReadRecordFloat();
						float xR = SaveData::ReadRecordFloat();
						float yR = SaveData::ReadRecordFloat();
						float zR = SaveData::ReadRecordFloat();

						queueToSpawn.emplace_back(
							form,
							(TESObjectCELL*)rLocation,
							xData,
							x, y, z,
							xR, yR, zR
							);

					}

				}
				else {	//Location does not exist anymore.

					SkipExtraDataList();

					UInt8 locationCellType = SaveData::ReadRecord8();

					if (locationCellType == 57 || locationCellType == 65) {
						SaveData::SkipNBytes(24);
					}

				}

			}
			else {

				if (type) {
					SaveData::SkipNBytes(1);
				}

				SaveData::SkipNBytes(4);

				SkipExtraDataList();
				UInt8 locationCellType = SaveData::ReadRecord8();

				if (locationCellType == 57 || locationCellType == 65) {
					SaveData::SkipNBytes(24);
				}

			}

			--TotalSize;

		}
	}

	bool ClearAllWeapData() {

		//std::vector<std::thread> threads;

		for (auto it = StaticLinker[40].begin(); it != StaticLinker[40].end(); ++it) {

			StaticInstance* Base = (StaticInstance*)it->second;

			for (auto& rInstance : Base->aInstances) {

				if (rInstance) {

					AuxVector filter{ rInstance->key.c_str() };
					for (auto it = onInstanceDeconstructEvent.handlers.begin(); it != onInstanceDeconstructEvent.handlers.end(); ++it) {

						if (it->CompareFilters(filter)) {
							g_scriptInterface->CallFunction(it->script, nullptr, nullptr, nullptr, 1, rInstance->clone);
						}

					}

					//Iterate through attachments for deconstructors.
					delete rInstance;

				}

			}
			Base->aInstances.clear(); // Restore instance data when loading

		}

		InstanceInterface::cloneCount = 0;
		return true;
	}

	void LoadInstances(StaticInstance* staticInst) {

		UInt32 length = SaveData::ReadRecord32();
		char* key = new char[length + 1];
		SaveData::ReadRecordData(key, length);
		key[length] = '\0';

		staticInst->create(key);

	}

	void LoadWeaponInstances(StaticInstance* staticInst) {

		UInt32 length = SaveData::ReadRecord32();
		char* key = new char[length + 1];
		SaveData::ReadRecordData(key, length);
		key[length] = '\0';

		Instance_WEAP* rInstance = (Instance_WEAP*)staticInst->create(key);

		if (rInstance) {

			//std::unordered_map<std::string, UInt32> aAttachments = {};
			UInt8 i = SaveData::ReadRecord8();		//Re-Apply attachments to WeapInst
			while (i > 0) {

				length = SaveData::ReadRecord32();
				char* sSlot = new char[length + 1];
				SaveData::ReadRecordData(sSlot, length);
				sSlot[length] = '\0';

				length = SaveData::ReadRecord32();
				char* EditorID = new char[length + 1];
				SaveData::ReadRecordData(EditorID, length);
				EditorID[length] = '\0';
				TESForm* rAttachment = LookupEditorID<TESForm*>(EditorID); //LookupEditorID of Attachment

				if (rAttachment) {

					rInstance->aAttachments[sSlot] = rAttachment->refID;

					AuxVector filterOnAttach{ rAttachment->refID, rInstance->baseInstance->parent->refID };
					for (auto it = onAttachWeapModReconstructEvent.handlers.begin(); it != onAttachWeapModReconstructEvent.handlers.end(); ++it) {
						if (it->CompareFilters(filterOnAttach)) {
							g_scriptInterface->CallFunction(it->script, nullptr, nullptr, nullptr, 2, rAttachment, rInstance->clone);
						}
					}

				}

				--i;

			}

			AuxVector filter{ key };
			for (auto it = onInstanceReconstructEvent.handlers.begin(); it != onInstanceReconstructEvent.handlers.end(); ++it) {

				if (it->CompareFilters(filter)) {

					ArrayElementL scriptReturn;
					g_scriptInterface->CallFunction(it->script, nullptr, nullptr, &scriptReturn, 1, rInstance->clone);
					if (scriptReturn.IsValid()) {
						rInstance->baseInstance->aInstances.markForDelete(rInstance->InstID);
					}

				}

			}

		}
		else {//Handle unregistered forms

			Console_Print("Skip Record: %s", staticInst->parent->GetTheName());
			UInt8 i = SaveData::ReadRecord8();		//Re-Apply attachments to WeapInst

			while (i > 0) {

				SaveData::SkipNBytes(SaveData::ReadRecord32());	//Skip slot

				SaveData::SkipNBytes(SaveData::ReadRecord32());	//Skip attachment editorID

				--i;

			}

		}

	}

	void SkipInstances() {

		SaveData::SkipNBytes(SaveData::ReadRecord32());	//Key

	}

	void SkipWeaponInstances() {

		SaveData::SkipNBytes(SaveData::ReadRecord32());	//Key

		UInt8 i = SaveData::ReadRecord8();		//Re-Apply attachments to WeapInst

		while (i > 0) {

			SaveData::SkipNBytes(SaveData::ReadRecord32());	//Skip slot

			SaveData::SkipNBytes(SaveData::ReadRecord32());	//Skip attachment editorID

			--i;

		}

	}

	void LoadGameCallback(void*)
	{

		UInt32 type, version, length;
		UInt32 iFinish = 0;

		while (SaveData::GetNextRecordInfo(&type, &version, &length)) {

			ClearAllWeapData();

			UInt32 StaticInstListSize = SaveData::ReadRecord32();

			TESForm* form = NULL;
			UInt32 length = 0;

			while (StaticInstListSize > 0) {

				length = SaveData::ReadRecord32();
				char* EditorID = new char[length + 1];
				SaveData::ReadRecordData(EditorID, length);
				EditorID[length] = '\0';

				UInt8 FormInstanceSize = SaveData::ReadRecord8();					//The instance count
				UInt8 type = SaveData::ReadRecord8();

				form = LookupEditorID<TESForm*>(EditorID);

				if (form && form->typeID == type) {

					StaticInstance* staticInst = form->LookupStaticInstance();

					while (FormInstanceSize > 0) {

						if (staticInst) {

							switch (type) {
							case 40:
								LoadWeaponInstances(staticInst);
								break;
							case 103:
								LoadInstances(staticInst);
								break;
							default:
								LoadInstances(staticInst);
								break;
							}

						}
						else {

							switch (type) {
							case 40:
								SkipWeaponInstances();
								break;
							case 103:
								SkipInstances();
								break;
							default:
								SkipInstances();
								break;
							}

						}

						--FormInstanceSize;

					}

				}
				else {

					while (FormInstanceSize > 0) {

						switch (type) {
						case 40:
							SkipWeaponInstances();
							break;
						case 103:
							SkipInstances();
							break;
						default:
							SkipInstances();
							break;
						}

						--FormInstanceSize;

					}

				}
				--StaticInstListSize;

			}

			LoadCollectedData();

		}

	}

}