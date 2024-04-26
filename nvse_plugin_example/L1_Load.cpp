#pragma once
#include <thread>
#include "SaveSystem.h"

namespace SaveSystem {

	ExtraDataList* LoadExtraDataList()
	{

		BSExtraData* xData = nullptr;
		ExtraDataList* xDataList = xDataList->Create();

		UInt8 type = SaveData::ReadRecord8();

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
				xData = ExtraHealth::Create();
				((ExtraHealth*)xData)->health = fHealth;
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
				xData = ExtraHotkey::Create();
				((ExtraHotkey*)xData)->index = iIndex;
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

	void LoadCollectedData() {

		UInt32 Totallength = SaveData::ReadRecord32();
		TESForm* rLocation = nullptr;

		while (Totallength > 0) {

			UInt32 baseRefID = SaveData::ReadRecord32();

			if (SaveData::ResolveRefID(baseRefID, &baseRefID) && baseRefID) {

				UInt8 instID = SaveData::ReadRecord8();		//Get instance using baseRefID and instID

				TESForm* rWeapon = LookupFormByRefID(baseRefID); //Add this

				Instance_WEAP* weap = rWeapon->LookupInstanceByID(instID);
				if (weap) {
					rWeapon = weap->clone;
				}

				UInt32 location = SaveData::ReadRecord32();

				if (SaveData::ResolveRefID(location, &location) && location) {

					rLocation = LookupFormByRefID(location);

					ExtraDataList* xData = LoadExtraDataList();

					if (rLocation->typeID != 57 && rLocation->typeID != 65) {

						if (xData->HasType(kExtraData_Worn)) {
							xData->RemoveByType(kExtraData_Worn);
							((Actor*)rLocation)->AddItem(rWeapon, xData, 1);
							((Actor*)rLocation)->SilentEquip(rWeapon, xData);
						}
						else {
							((TESObjectREFR*)rLocation)->AddItem(rWeapon, xData, 1);
						}

					}
					else {

						float x = SaveData::ReadRecordFloat();
						float y = SaveData::ReadRecordFloat();
						float z = SaveData::ReadRecordFloat();
						float xR = SaveData::ReadRecordFloat();
						float yR = SaveData::ReadRecordFloat();
						float zR = SaveData::ReadRecordFloat();

						queueToSpawn.emplace_back(
							rWeapon,
							(TESObjectCELL*)rLocation,
							xData,
							x, y, z,
							xR, yR, zR
							);

					}
				}
			}

			--Totallength;

		}
	}

	bool ClearAllWeapData() {

		//std::vector<std::thread> threads;

		for (auto it = StaticInstance_WEAP::Linker.begin(); it != StaticInstance_WEAP::Linker.end(); ++it) {

			StaticInstance_WEAP* Base = it->second;

			//threads.emplace_back([Base]() {

				for (auto& rInstance : Base->aInstances) {
					if (rInstance) {

						
						std::vector<void*> filter{ &rInstance->key };
						for (auto it = onInstanceDeconstructEvent.handlers.begin(); it != onInstanceDeconstructEvent.handlers.end(); ++it) {

							if (it->CompareFilters(filter)) {
								g_scriptInterface->CallFunction(it->script, nullptr, nullptr, nullptr, 1, rInstance->clone);
							}

						}

						//Iterate through attachments for deconstructors.

						rInstance->clone->Destroy(0);
						delete rInstance;
					}
				}
				Base->aInstances.clear(); // Restore instance data when loading

			//	});
		}

		//for (std::thread& thread : threads) {
			//thread.join();
		//}
		return true;
	}

	/*
	bool ClearAllWeapData2() {
		std::vector<std::thread> threads;

		// Clear aInstances vectors associated with StaticInstance objects
		for (auto& pair : StaticInstance::Linker) {
			StaticInstance* base = pair.second;
			threads.emplace_back([base]() {
				base->aInstances.clear();
			});
		}

		// Delete Instance objects
		for (auto& pair : Instance::Linker) {
			Instance* instance = &pair.second;
			threads.emplace_back([instance]() {
				delete instance;
			});
		}

		// Wait for all threads to finish
		for (std::thread& thread : threads) {
			thread.join();
		}

		// Clear the Linker maps
		StaticInstance::Linker.clear();
		Instance::Linker.clear();

		return true;
	}
	*/

	void LoadGameCallback(void*)
	{

		UInt32 type, version, length;
		UInt32 iFinish = 0;
		ArrayElementL scriptReturn;

		while (SaveData::GetNextRecordInfo(&type, &version, &length)) {

			ClearAllWeapData();

			UInt32 aBaseForms = SaveData::ReadRecord32();

			TESForm* form = NULL;
			TESForm* rAttachment = NULL;
			UInt32 length = 0;

			while (aBaseForms > 0) {

				UInt32 refID = SaveData::ReadRecord32();
				if (SaveData::ResolveRefID(refID, &refID) && refID) {

					form = LookupFormByRefID(refID);

					if (!form) {
						Console_Print("Form Is Not valid");	//Load the count of baseforms.
						break;
					}

					UInt8 BaseFormInstances = SaveData::ReadRecord8();					//The instance count

					while (BaseFormInstances > 0) {


						length = SaveData::ReadRecord32();
						char* key = new char[length + 1];
						SaveData::ReadRecordData(key, length);
						key[length] = '\0';

						Instance_WEAP* rInstance = Instance_WEAP::create(StaticInstance_WEAP::Linker[form->refID], key);
						//UInt32 cloneRefID = form->CreateInst(key);

						if (rInstance) //Re-Create the instance
						{

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
								rAttachment = ((TESForm * (__cdecl*)(char*))(0x483A00))(EditorID); //LookupEditorID of Attachment

								if (rAttachment)
									rInstance->aAttachments[sSlot] = rAttachment->refID;


								std::vector<void*> filterOnAttach{ rAttachment, rInstance->baseInstance->parent };
								for (auto it = onAttachWeapModReconstructEvent.handlers.begin(); it != onAttachWeapModReconstructEvent.handlers.end(); ++it) {
									if (it->CompareFilters(filterOnAttach)) {
										g_scriptInterface->CallFunction(it->script, nullptr, nullptr, nullptr, 2, rAttachment, rInstance->clone);
									}
								}

								--i;

							}

							std::vector<void*> filter{ &key };
							for (auto it = onInstanceReconstructEvent.handlers.begin(); it != onInstanceReconstructEvent.handlers.end(); ++it) {

								if (it->CompareFilters(filter)) {
									g_scriptInterface->CallFunction(it->script, nullptr, nullptr, &scriptReturn, 1, rInstance->clone);
								}

							}

						}
						else {//Handle unregistered forms

							Console_Print("Skip Record: %s", form->GetTheName());
							UInt8 i = SaveData::ReadRecord8();		//Re-Apply attachments to WeapInst

							while (i > 0) {

								SaveData::SkipNBytes(SaveData::ReadRecord32());	//Skip slot

								SaveData::SkipNBytes(SaveData::ReadRecord32());	//Skip attachment editorID

								--i;

							}

						}
						--BaseFormInstances;

					}

				}
				--aBaseForms;

			}

			LoadCollectedData();

		}

	}

}