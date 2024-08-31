#pragma once
#include "SaveSystem.h"

namespace Hooks
{

	TESForm* __cdecl SaveData(void* refID)
	{

		//return CdeclCall<TESObjectREFR*>(0x04839C0, refID);
		TESForm* form = CdeclCall<TESForm*>(0x04839C0, refID);
		//TESForm* form = LookupFormByRefID(refID);
		if (!form) {
			return nullptr;
		}

		if (!form->IsReference()) {
			if (form->pIsDynamicForm()) {
				return nullptr;
			}
			return form;
		}

		TESObjectREFR* ref = (TESObjectREFR*)form;
		ref = SaveSystem::SaveLoadManager::saveManager.queue.queueToSave(ref);

		return ref;


	}

	int __fastcall SetPersistentHook(TESForm* form) { //Not used

		int result = ThisStdCall<int>(0x044DDC0, form);

		if (form->IsReference()) {

			if (Instance* inst = ((TESObjectREFR*)form)->baseForm->pLookupInstance()) {

				if (inst->lifecycle.isPolicyEnabled(LifecycleManager::Timed)) {
					newlyCreatedReferences.push_back((TESObjectREFR*)form);
				}

			}
			else if (ExtendedBaseType* staticForm = ((TESObjectREFR*)form)->baseForm->LookupStaticInstance()) {

				if (staticForm->baseLifecycle.isPolicyEnabled(LifecycleManager::Timed)) {
					newlyCreatedReferences.push_back((TESObjectREFR*)form);
				}

			}

		}

		return result;

	}

}