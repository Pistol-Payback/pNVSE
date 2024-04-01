#pragma once
#include "SaveSystem.h"

namespace Hooks
{

	int __fastcall HookActorInventorySave(TESObjectREFR* rContainer, void* edx, int a2)
	{

		if (rContainer) {
			ContChangesEntryList* entryList = rContainer->GetContainerChangesList();

			if (entryList) {
				ContChangesEntry* entry;
				for (auto iter = entryList->Begin(); !iter.End(); ++iter) {

					entry = iter.Get();
					if (entry && entry->type->typeID == 40 && entry->type->IsModifierForm()) {

						UInt32 objectsToRemove = entry->countDelta;
						while (objectsToRemove > 0) {

							TESForm* form = (entry->type->GetModifierParent());
							ContChangesEntry* newEntry = entry->Create(form->refID, 1, entry->extendData);
							newEntry->Cleanup();
							//entryList->Replace(entry, newEntry);

							ExtraDataList* xData = entry->extendData->GetFirstItem();
							bool IsEquipped = xData->HasType(kExtraData_Worn);

							SaveSystem::SaveData* saveData = new SaveSystem::SaveData(form->refID, 0, rContainer->refID, IsEquipped, xData->CreateCopy());
							SaveSystem::aSaveData.push_back(saveData);

							if (IsEquipped) {

								((Actor*)rContainer)->SilentUnequip(entry->type, xData);

							}

							entry->Remove(entry->extendData->GetFirstItem(), 1);

							objectsToRemove--;
						}

					}

				}
			}

		}

		ThisStdCall<int>(0x0562230, rContainer, a2);

		return 0;

	}
}