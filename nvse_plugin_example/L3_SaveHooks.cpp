#pragma once
#include "SaveSystem.h"

namespace Hooks
{

	TESForm* __cdecl SaveData(void* refID)
	{

		//return CdeclCall<TESObjectREFR*>(0x04839C0, refID);
		TESForm* form = CdeclCall<TESForm*>(0x04839C0, refID);
		//TESForm* form = LookupFormByRefID(refID);
		if (!form || !form->IsReference()) {
			return form;
		}

		TESObjectREFR* ref = (TESObjectREFR*)form;
		ref = SaveSystem::SaveLoadManager::saveManager.queueToSave(ref);

		return ref;


	}

}