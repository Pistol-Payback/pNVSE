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

					if (entry && (entry->type->IsInstancedForm() || entry->type->pIsDynamicForm())) {

						UInt32 objectsToRemove = entry->countDelta;
						while (objectsToRemove > 0) {

							StaticInstance* staticInst = entry->type->LookupStaticInstance();
							Instance* inst = entry->type->pLookupInstance();

							ExtraDataList* xData = nullptr;
							if (entry->extendData) {
								xData = entry->extendData->GetFirstItem();
							}

							if (xData) {

								bool IsEquipped = xData->HasType(kExtraData_Worn);

								SaveSystem::aSaveData.emplace_back(new SaveSystem::SaveDataObj(
									staticInst,
									inst,
									object->refID,
									xData->CreateCopy()
								));

								if (IsEquipped) {

									((Actor*)object)->SilentUnequip(entry->type, xData);

								}

							}
							else {
								SaveSystem::aSaveData.emplace_back(new SaveSystem::SaveDataObj(
									staticInst,
									inst,
									object->refID,
									xData
								));
							}

							entry->Remove(xData, 1);
							objectsToRemove--;

						}

					}


				}
	
			} else if (object->baseForm->IsInstancedForm() || object->baseForm->pIsDynamicForm()) {

				ExtraDataList* xData = &object->extraDataList;

				Instance* inst = object->baseForm->pLookupInstance();
				StaticInstance* staticInst = object->baseForm->LookupStaticInstance();

				if (object->parentCell) {

						TESObjectCELL* cell = object->parentCell;
						TESWorldSpace* worldSpace = cell->worldSpace;

						UInt32 cellRefID = worldSpace ? worldSpace->refID : cell->refID;

						if (xData) {
							xData = xData->CreateCopy();
						}

						SaveSystem::aSaveData.emplace_back(new SaveSystem::SaveDataWorldObj(
							staticInst,
							inst,
							cellRefID,
							xData,
							object->posX,
							object->posY,
							object->posZ,
							object->rotX,
							object->rotY,
							object->rotZ
						));


				}

				object->DeleteReference(); //I don't think you need to delete the reference bruh. It's just for clearing the cell buffer.
				object = nullptr;

			}


		}

		return object;


	}

}