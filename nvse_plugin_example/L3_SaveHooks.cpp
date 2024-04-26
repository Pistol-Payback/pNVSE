#pragma once
#include "SaveSystem.h"

namespace Hooks
{

	TESObjectREFR* __cdecl SaveData(void* objectPtr)
	{

		//return CdeclCall<TESObjectREFR*>(0x04839C0, objectPtr);

		TESObjectREFR* object = CdeclCall<TESObjectREFR*>(0x04839C0, objectPtr);
		//return object;

		if (object && object->IsReference()) {

			ContChangesEntryList* entryList = object->GetContainerChangesList();

			if (entryList) {

				ContChangesEntry* entry;

				for (auto iter = entryList->Begin(); !iter.End(); ++iter) {

					entry = iter.Get();

					if (entry && entry->type->typeID == 40 && entry->type->IsInstancedForm()) {

						UInt32 objectsToRemove = entry->countDelta;
						while (objectsToRemove > 0) {
							/*
							TESForm* form = entry->type->GetStaticParent();
							ContChangesEntry* newEntry = entry->Create(form->refID, 1, entry->extendData);
							newEntry->Cleanup();
							//entryList->Replace(entry, newEntry);

							ExtraDataList* xData = entry->extendData->GetFirstItem();
							bool IsEquipped = xData->HasType(kExtraData_Worn);

							SaveSystem::aSaveData.emplace_back(new SaveSystem::SaveDataObj(
								form->refID,
								entry->type->GetInstanceID(),
								object->refID,
								xData->CreateCopy()
							));

							if (IsEquipped) {

								((Actor*)object)->SilentUnequip(entry->type, xData);

							}

							entry->Remove(entry->extendData->GetFirstItem(), 1);
																		*/
							objectsToRemove--;
						}

					}


				}
	
			} else if (object->baseForm->typeID == 40 && object->baseForm->IsInstancedForm()) {

				ExtraDataList* xData = &object->extraDataList;
				TESForm* form = object->baseForm->GetStaticParent();
				if (form) {

					if (object->parentCell) {

						TESObjectCELL* cell = object->parentCell;
						TESWorldSpace* worldSpace = cell->worldSpace;

						UInt32 cellRefID = worldSpace ? worldSpace->refID : cell->refID;

						SaveSystem::aSaveData.emplace_back(new SaveSystem::SaveDataWorldObj(
							form->refID,
							object->baseForm->GetInstanceID(),
							cellRefID,
							xData->CreateCopy(),
							object->posX,
							object->posY,
							object->posZ,
							object->rotX,
							object->rotY,
							object->rotZ
						));

					}

				}
				object->DeleteReference();
				object = nullptr;

			}


		}

		return object;


	}

}