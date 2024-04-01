#pragma once
#include "SaveSystem.h"

namespace SaveSystem {

	void RebuildClones() {

		Script* PistolWeaponEditorCloneRebuild = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolWeaponEditorCloneRebuild");
		ArrayElementL scriptReturn;
		UInt32 cloneRefID;

		for (auto it = aCloneRebuild.begin(); it != aCloneRebuild.end(); ++it) {
			cloneRefID = *it;
			//g_scriptInterface->CallFunction(PistolWeaponEditorCloneRebuild, nullptr, nullptr, &scriptReturn, 1, LookupFormByRefID(cloneRefID));
		}
		aCloneRebuild.clear();

	}

	void LoadCollectedData() {

		//Console_Print("LoadCollectedData");

		UInt32 Totallength = ReadRecord32();
		TESObjectREFR* form = nullptr;
		//Console_Print("LoadCollectedDataTotal %d", Totallength);
		while (Totallength > 0) {

			UInt32 baseRefID = ReadRecord32();

			if (ResolveRefID(baseRefID, &baseRefID) && baseRefID) {

				UInt8 instID = ReadRecord8();		//Get instance using baseRefID and instID

				//Console_Print("LoadCollectedData Weapon Base %s", LookupFormByRefID(baseRefID)->GetEditorID());

				TESForm* rWeapon = LookupFormByRefID(baseRefID)->LookupModifierByID(instID)->Clone; //Add this
				//Console_Print("LoadCollectedData Weapon Clone %s", rWeapon->GetEditorID());

				float health = ReadRecordFloat();

				//Console_Print("LoadCollectedData Health %f", health);

				UInt32 location = ReadRecord32();

				if (ResolveRefID(location, &location) && location) {

					form = (TESObjectREFR*)LookupFormByRefID(location);

					if (form) {
						bool bequipped = ReadRecord8();
						//Console_Print("LoadCollectedData Equipped %d", bequipped);
						//Add item to location using all values above.

						form->AddItemAlt(rWeapon, 1, health, bequipped);
						if (bequipped) {
							//QueueToSkipGroup((Actor*)form, kAnimGroup_Equip);
						}

					}
					else {
						float x = ReadRecord32();	//Place in world
						float y = ReadRecord32();
						float z = ReadRecord32();
						float xR = ReadRecord32();
						float yR = ReadRecord32();
						float zR = ReadRecord32();
					}
				}
			}

			--Totallength;

		}
	}

	bool ClearAllWeapData() {

		//deletes every weapon instances that may have been created in other saves when loading a new save.

		for (auto it = BaseExtraData.begin(); it != BaseExtraData.end(); ++it) {

			WeapInstBase* Base = it->second;
			for (auto& rInstance : Base->aInstances) {
				if (rInstance) {
					//delete rInstance->Clone;
					delete rInstance;
				}
			}
			Base->aInstances.clear(); //Restore instance data when loading.
		}

		aClones.clear();
		aClones = std::move(aUsedClones);
		aUsedClones.clear();

		WeapInstList.clear();

		return true;
	}

	void LoadGameCallback(void*)
	{

		UInt32 type, version, length;
		UInt32 iFinish = 0;

		while (GetNextRecordInfo(&type, &version, &length)) {

			//SkipNBytes(length);
			//Console_Print("SkipBytes %d", length);	//Load the count of baseforms.

			ClearAllWeapData();

			UInt32 aBaseForms = ReadRecord32();
			//Console_Print("LoadGameCallbackTotal %d", aBaseForms);	//Load the count of baseforms.

			TESForm* form = NULL;
			TESForm* rAttachment = NULL;
			UInt32 length = 0;

			while (aBaseForms > 0) {

				//Console_Print("Next block: %d", aBaseForms);	//Load the count of baseforms.
				UInt32 refID = ReadRecord32();
				//Console_Print("RefID %d", refID);	//Load the count of baseforms.
				if (ResolveRefID(refID, &refID) && refID) {

					form = LookupFormByRefID(refID);

					if (!form) {
						Console_Print("Form Is Not valid");	//Load the count of baseforms.
						break;
					}

					//Console_Print("LoadGameCallbackTotal %s", form->GetEditorID());	//Load the count of baseforms.

					UInt8 BaseFormInstances = ReadRecord8();					//The instance count
					//Console_Print("Saving Instance array size: %d", BaseFormInstances);

					while (BaseFormInstances > 0) {

						UInt32 cloneRefID = form->CreateInst();	//Re-Create the instance
						//Console_Print("InstID %d", cloneRefID);	//Total attachments on WI.
						WeapInst* rInstance = WeapInstList[cloneRefID];											//Lookup the WeapInstBase that was just created by CreateInst

						UInt8 i = ReadRecord8();		//Re-Apply attachments to WeapInst

						//Console_Print("Total Attachments %d", i);	//Total attachments on WI.
						while (i > 0) {

							length = ReadRecord32();
							//Console_Print("sSlot size %d", length);	//Total attachments on WI.
							char* sSlot = new char[length + 1];
							ReadRecordData(sSlot, length);
							sSlot[length] = '\0';
							//Console_Print("Loading Slot %s", sSlot);

							length = ReadRecord32();
							//Console_Print("Attachments size %d", length);	//Total attachments on WI.
							char* EditorID = new char[length + 1];
							ReadRecordData(EditorID, length);
							EditorID[length] = '\0';
							//Console_Print("Loading Attachments %s", EditorID);
							rAttachment = ((TESForm * (__cdecl*)(char*))(0x483A00))(EditorID); //LookupEditorID of Attachment

							if (rAttachment)
								rInstance->aAttachments[sSlot] = rAttachment->refID;

							--i;

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