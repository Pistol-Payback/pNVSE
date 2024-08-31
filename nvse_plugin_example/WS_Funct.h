#pragma once
#include "WeaponSmith.h"

static ParamInfo kParamsSetWeapAttachments[3] =
{
	{"form", kParamType_AnyForm, 0},
	{"string", kParamType_String, 0},
	{"form", kParamType_AnyForm, 1}
};

static ParamInfo kParams_ReplaceItem[3] =
{
	{"form", kParamType_AnyForm, 0},
	{"int", kParamType_Integer, 1},
	{"int", kParamType_Integer, 1}
};

DEFINE_COMMAND_ALT_PLUGIN(ReplaceItemInInventory, ReplaceItem, "Replaces an inventory object", true, kParams_ReplaceItem);
bool Cmd_ReplaceItemInInventory_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32* refResult = (UInt32*)result;
	TESForm* form = NULL;
	UInt32 copy = 0;
	UInt32 count = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &copy, &count)) {

		//Console_Print("Cmd_ReplaceItemInInventory_Execute");
		
		InventoryRef* invRef = InventoryRef::InventoryRefGetForID(thisObj->refID);

		if (!invRef) {
			return true;
		}
		if ((form->typeID != invRef->data.type->typeID)) {
			return true;
		}

		SInt32 maxCount = invRef->GetCount();

		if (maxCount == 0) {
			return true;
		}

		if (count == 0) {
			count = maxCount;
		}
		else {
			count = min(count, maxCount);
		}
		TESObjectREFR* replacement = ((Actor*)invRef->containerRef)->ReplaceInvObject(form, invRef, count, copy);
		if (replacement) {
			*refResult = replacement->refID;
		}

	}
	return true;
}

DEFINE_COMMAND_PLUGIN(MarkAsStaticForm, "Creates a new weapon instance aka new baseform for a weapon", false, kParams_OneForm);
bool Cmd_MarkAsStaticForm_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (form) {
			form->MarkAsStaticForm(scriptObj->GetModIndexAlt());
		}

	}

	return true;

}

static ParamInfo kParams_CreateWeaponInstance[4] =
{
	{	"form",	kParamType_AnyForm,	0	},
	{	"string", kParamType_String, 0	},
	{	"form",	kParamType_AnyForm,	1	},
	{	"form",	kParamType_AnyForm,	1	}
};

static ParamInfo kParams_CreateAkimboInstance[5] =
{
	{	"form",	kParamType_AnyForm,	0	},
	{	"form",	kParamType_AnyForm,	0	},
	{	"string", kParamType_String, 0	},
	{	"form",	kParamType_AnyForm,	1	},
	{	"form",	kParamType_AnyForm,	1	}
};

DEFINE_COMMAND_ALT_PLUGIN(CreateFormInstance, CreateFormInst, "Creates a new weapon instance aka new baseform for a weapon", false, kParams_CreateWeaponInstance);
bool Cmd_CreateFormInstance_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;

	Script* reconstruct = nullptr;
	Script* deconstruct = nullptr;

	TESForm* form = NULL;
	char key[0x50];

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &key, &reconstruct, &deconstruct)) {

			if (key[0] != '\0') {

				form = form->CreateInst(key, scriptObj->GetModIndexAlt());
				if (!form) {
					return true;
				}

				*refResult = form->refID;

				AuxVector filter = Event::EvaluateEventArgAux(1, 1, AuxValue(key));

				if (reconstruct) {
					Console_Print("reconstructer set");
					Event eEvent('1', reconstruct, filter);
					onInstanceReconstructEvent.AddEvent(eEvent);
				}

				if (deconstruct) {
					Console_Print("deconstruct set");
					Event eEvent('1', deconstruct, filter);
					onInstanceDeconstructEvent.AddEvent(eEvent);
				}

			}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN_EXP(GetAkimboWeapons, "Gets base akimbo instance", false, kNVSEParams_OneForm_OneNumber);
bool Cmd_GetAkimboWeapons_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	TESForm* form = NULL;
	UInt32 bRightLeft = false;
	Instance* akimbo = nullptr;
	InventoryRef* invRef = nullptr;


	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		form = eval.GetNthArg(0)->GetTESForm();
		bRightLeft = eval.GetNthArg(1)->GetInt();
		TESForm* baseForm = form->IsReference() ? ((TESObjectREFR*)form)->baseForm : form;

		if (baseForm->IsStaticForm(222)) {
			eval.Error("Error in, GetAkimboWeapons requires an akimbo instance, not a baseform.");
		}

		akimbo = baseForm->pLookupInstance();

		if (akimbo && akimbo->baseInstance->extendedType == 222) {

			if (bRightLeft) {
				*refResult = ((Instance_Akimbo*)akimbo)->left;
			}
			else {
				*refResult = ((Instance_Akimbo*)akimbo)->right;
			}

		}

	}
	return true;

}

DEFINE_COMMAND_PLUGIN_EXP(GetAkimboWeaponsAlt, "Gets base akimbo instance", false, kNVSEParams_ThreeForms);
bool Cmd_GetAkimboWeaponsAlt_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	TESForm* form = NULL;
	Instance* akimbo = nullptr;
	InventoryRef* invRef = nullptr;

	ScriptLocal* akimboLeft;
	ScriptLocal* akimboRight;;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		form = eval.GetNthArg(0)->GetTESForm();
		akimboLeft = eval.GetNthArg(1)->GetScriptVar();
		akimboRight = eval.GetNthArg(2)->GetScriptVar();

		TESForm* baseForm = form->IsReference() ? ((TESObjectREFR*)form)->baseForm : form;

		if (baseForm->IsStaticForm(222)) {
			eval.Error("Error in, GetAkimboWeaponsAlt requires an akimbo instance, not a baseform.");
		}

		akimbo = baseForm->pLookupInstance();

		if (akimbo && akimbo->baseInstance->extendedType == 222) {

			akimboLeft->formId = ((Instance_Akimbo*)akimbo)->left;
			akimboRight->formId = ((Instance_Akimbo*)akimbo)->right;

		}

	}
	return true;

}

DEFINE_COMMAND_PLUGIN(GetStaticAkimbo, "Gets a static akimbo baseform", false, kParams_TwoForms);
bool Cmd_GetStaticAkimbo_Execute(COMMAND_ARGS) {

	*result = 0;
	UInt32* refResult = (UInt32*)result;

	TESForm* leftForm = NULL;
	TESForm* rightForm = NULL;

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &rightForm, &leftForm)) {
		return true;
	}

	if (rightForm && leftForm) {
		leftForm = leftForm->IsReference() ? ((TESObjectREFR*)leftForm)->baseForm : leftForm;
		rightForm = rightForm->IsReference() ? ((TESObjectREFR*)rightForm)->baseForm : rightForm;
		StaticInstance_Akimbo* akimbo = StaticInstance_Akimbo::LookupAkimboSet(leftForm, rightForm);
		if (akimbo) {
			*refResult = akimbo->parent->refID;
			Console_Print("GetStaticAkimbo >> %d (%s) (%s)", akimbo->parent->refID, GetFullName(akimbo->parent), akimbo->parent->GetEditorID());
		}
	}

	return true;

}

DEFINE_COMMAND_PLUGIN(HasAkimboSet, "Gets a static akimbo baseform", false, kParams_TwoForms);
bool Cmd_HasAkimboSet_Execute(COMMAND_ARGS) {

	*result = 0;
	UInt32* refResult = (UInt32*)result;

	TESForm* leftForm = NULL;
	TESForm* rightForm = NULL;

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &rightForm, &leftForm)) {
		return true;
	}

	if (rightForm && leftForm) {
		leftForm = leftForm->GetBaseObject();
		rightForm = rightForm->GetBaseObject();
		StaticInstance_Akimbo* akimbo = StaticInstance_Akimbo::LookupAkimboSet(leftForm, rightForm);
		if (akimbo) {
			*result = 1;
		}
	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN(IsAkimboForm, IsAkimbo, "Checks if instance is an akimbo", false, kParams_OneForm);
bool Cmd_IsAkimboForm_Execute(COMMAND_ARGS) {

	*result = 0;
	TESForm* form = nullptr;

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &form) || !form) {
		return true;
	}

	TESForm* baseForm = form->IsReference() ? ((TESObjectREFR*)form)->baseForm : form;
	auto staticInstance = baseForm->LookupExtendedBase();
	if (staticInstance && staticInstance->extendedType == 222) {
		*result = 1;
	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN(CreateAkimboInstance, CreateAkimbo, "Creates a new akimbo instance", false, kParams_CreateAkimboInstance);
bool Cmd_CreateAkimboInstance_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;

	Script* reconstruct = nullptr;
	Script* deconstruct = nullptr;

	TESForm* leftForm = NULL;
	TESForm* rightForm = NULL;

	TESObjectREFR* leftFormRef = NULL;
	TESObjectREFR* rightFormRef = NULL;

	char key[0x50];

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &rightForm, &leftForm, &key, &reconstruct, &deconstruct)) {

		if (key[0] != '\0') {

			if (!leftForm->IsReference() || !rightForm->IsReference()) {
				return true;
			}

			rightFormRef = (TESObjectREFR*)rightForm;
			leftFormRef = (TESObjectREFR*)leftForm;

			StaticInstance_Akimbo* akimbo = StaticInstance_Akimbo::LookupAkimboSet(rightFormRef->GetBaseObject(), leftFormRef->GetBaseObject());
			if (!akimbo) {
				return true;
			}

			TESForm* form = akimbo->newInstance(rightFormRef, leftFormRef, scriptObj->GetModIndexAlt(), key);

			*refResult = form->refID;

			AuxVector filter = Event::EvaluateEventArgAux(1, 1, AuxValue(key));

			if (reconstruct) {
				Console_Print("reconstructer set");
				Event eEvent('1', reconstruct, filter);
				onInstanceReconstructEvent.AddEvent(eEvent);
			}

			if (deconstruct) {
				Console_Print("deconstruct set");
				Event eEvent('1', deconstruct, filter);
				onInstanceDeconstructEvent.AddEvent(eEvent);
			}

		}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN(GetWeaponAmmoType, "Gets the current ammo inside the gun", true, 0);
bool Cmd_GetWeaponAmmoType_Execute(COMMAND_ARGS) {

	*result = 0;
	UInt32* refResult = (UInt32*)result;

	ExtraAmmo* xData = (ExtraAmmo*)thisObj->extraDataList.GetByType(kExtraData_Ammo);
	if (xData) {
		*refResult = xData->ammo->refID;
	}

	return true;

}

DEFINE_COMMAND_PLUGIN(SetWeaponAmmoType, "Sets the current ammo inside the gun", true, kParams_OneForm);
bool Cmd_SetWeaponAmmoType_Execute(COMMAND_ARGS) {

	*result = 0;
	TESForm* form = NULL;

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &form) || !form || form->typeID != kFormType_TESAmmo) {
		return true;
	}

	ExtraAmmo* xData = (ExtraAmmo*)thisObj->extraDataList.GetByType(kExtraData_Ammo);
	if (xData) {
		xData->ammo = static_cast<TESAmmo*>(form);
	}
	else{
		thisObj->extraDataList.Add(ExtraAmmo::Create(static_cast<TESAmmo*>(form), 0));
	}

	return true;

}

DEFINE_COMMAND_PLUGIN(GetWeaponAmmoCount, "Gets the current ammo count inside the gun", true, 0);
bool Cmd_GetWeaponAmmoCount_Execute(COMMAND_ARGS) {

	*result = 0;

	ExtraAmmo* xData = (ExtraAmmo*)thisObj->extraDataList.GetByType(kExtraData_Ammo);
	if (xData) {
		*result = xData->count;
	}

	return true;

}

DEFINE_COMMAND_PLUGIN(SetWeaponAmmoCount, "Sets the current ammo count inside the gun", true, kParams_OneInt);
bool Cmd_SetWeaponAmmoCount_Execute(COMMAND_ARGS) {

	*result = 0;
	SInt32 count;

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &count)) {
		return true;
	}

	ExtraAmmo* xData = (ExtraAmmo*)thisObj->extraDataList.GetByType(kExtraData_Ammo);
	if (xData) {
		xData->count = count;
		if (count < 0) {
			thisObj->extraDataList.RemoveByType(kExtraData_Ammo);
		}
	}

	return true;

}

DEFINE_COMMAND_PLUGIN(FindWeaponAmmo, "Searches an actor for a matching ammo", true, kParams_OneForm_OneOptionalForm);
bool Cmd_FindWeaponAmmo_Execute(COMMAND_ARGS) {

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	TESForm* weapon = NULL;
	TESForm* ammoForm = NULL;

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &weapon, &ammoForm) || !weapon) {
		return true;
	}

	TESForm* baseForm = weapon->IsReference() ? ((TESObjectREFR*)weapon)->baseForm : weapon;
	TESObjectWEAP* weaponBase = static_cast<TESObjectWEAP*>(baseForm);

	if (ammoForm && ammoForm->typeID == kFormType_TESAmmo) {
		ammoForm = thisObj->CycleAmmoType(weaponBase, static_cast<TESAmmo*>(ammoForm));
		if (ammoForm) {
			*refResult = ammoForm->refID;
		}
	}
	else {
		ammoForm = thisObj->FindAmmoType(weaponBase).first;
		if (ammoForm) {
			*refResult = ammoForm->refID;
		}
	}

	return true;

}

DEFINE_COMMAND_PLUGIN_EXP(GetInventoryFromList, "Gets all the inventory items from a list", true, kNVSEParams_OneBasicType);
bool Cmd_GetInventoryFromList_Execute(COMMAND_ARGS) {

	*result = 0;
	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		if (!thisObj) {
			Console_Print("GetInventoryFromList requires a reference");
			return true;
		}
		PluginScriptToken* arg = eval.GetNthArg(0);
		NVSEArrayVarInterface::Array* resultArr = g_arrInterface->CreateArray(nullptr, 0, scriptObj);

		switch (arg->GetType()) {
		case kTokenType_Form:
		case kTokenType_RefVar:
		{
			TESForm* valueRef = arg->GetTESForm();
			if (valueRef->typeID == kFormType_BGSListForm) {
				BGSListForm* list = static_cast<BGSListForm*>(valueRef);
				for (auto iter = list->list.Head(); iter; iter = iter->next) {
					TESForm* entry = static_cast<TESForm*>(iter->data);
					if (entry) {
						UInt32 count = thisObj->GetItemCount(entry);
						if (count > 0) {
							g_arrInterface->AppendElement(resultArr, ArrayElementL(entry));
						}
					}
				}
			}
			break;
		}
		case kTokenType_Array:
		case kTokenType_ArrayVar:
		{
			NVSEArrayVarInterface::Array* valueArr = arg->GetArrayVar();
			const auto& arrData = ArrayData(valueArr, true);
			for (UInt32 idx = 0; idx < arrData.size; idx++) {
				if (arrData.vals[idx].type == NVSEArrayVarInterface::kType_Form) {
					if (auto entry = arrData.vals[idx].Form()) {
						UInt32 count = thisObj->GetItemCount(entry);
						if (count > 0) {
							g_arrInterface->AppendElement(resultArr, ArrayElementL(entry));
						}
					}
				}
			}
			break;
		}
		}

		g_arrInterface->AssignCommandResult(resultArr, result);

	}

	return true;

}

DEFINE_COMMAND_PLUGIN_EXP(LoadWeaponWithAmmo, "Loads a weapon, optionally using a specific actor, with an optional specific ammo and count", true, kNVSEParams_OneForm_OneOptionalForm_OneOptionalInt);
bool Cmd_LoadWeaponWithAmmo_Execute(COMMAND_ARGS) {

	*result = 0;
	TESForm* ammoForm = nullptr;
	TESForm* weapon = nullptr;
	SInt32 ammoCount = -1;

	*result = 0;
	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs()) {
		weapon = eval.GetNthArg(0)->GetTESForm();

		if (!weapon) {
			return true;
		}

		if (eval.NumArgs() > 1) {
			ammoForm = eval.GetNthArg(1)->GetTESForm();
			if (eval.NumArgs() > 2) {
				ammoCount = eval.GetNthArg(2)->GetInt();
			}
		}

		TESObjectREFR* weaponRef = static_cast<TESObjectREFR*>(weapon);
		TESObjectWEAP* weaponBase = static_cast<TESObjectWEAP*>(weaponRef->baseForm);
		ExtraAmmo* xData = static_cast<ExtraAmmo*>(weaponRef->extraDataList.GetByType(kExtraData_Ammo));

		if (thisObj && IS_ACTOR(thisObj) && weaponBase) {
			UInt32 itemCount = 0;

			if (!ammoForm || ammoForm->typeID != kFormType_TESAmmo) {
				std::pair<TESAmmo*, UInt32> ammoPair = thisObj->FindAmmoType(weaponBase);
				ammoForm = ammoPair.first;
				itemCount = ammoPair.second;
				if (!ammoForm) {
					return true; // No suitable ammo found
				}
			}
			else {
				itemCount = thisObj->GetItemCount(ammoForm);
				if (itemCount < 1) {
					std::pair<TESAmmo*, UInt32> ammoPair = thisObj->FindAmmoType(weaponBase);
					ammoForm = ammoPair.first;
					itemCount = ammoPair.second;
					if (!ammoForm) {
						return true; // Actor has no ammo of this type
					}
				}
			}

			UInt8 clipSize = weaponBase->GetModdedClipSize(weaponRef->GetWeaponModFlags());
			UInt8 neededRounds = clipSize;

			if (xData && xData->ammo == ammoForm) {
				neededRounds -= xData->count;  // Subtract current loaded rounds
			}
			else if (xData) {
				thisObj->AddItem(xData->ammo, nullptr, xData->count); // Return existing ammo back to inventory
			}

			if (neededRounds == 0) {
				*result = 1; // Weapon already full
				return true;
			}

			UInt32 ammoToRemove = ammoCount;
			if (ammoCount == -1 || itemCount < ammoCount) { //Fill ammo
				ammoToRemove = min(neededRounds, itemCount);
				ammoCount = min(clipSize, itemCount);
			}

			if (ammoToRemove > 0) {
				thisObj->RemoveItem(ammoForm, nullptr, ammoToRemove, true, false, nullptr, 0, 0, true, false);
			}
		}

		if (!ammoForm || ammoForm->typeID != kFormType_TESAmmo) {
			return true;
		}

		if (xData) {
			xData->ammo = static_cast<TESAmmo*>(ammoForm);
			xData->count = ammoCount;
		}
		else {
			weaponRef->extraDataList.Add(ExtraAmmo::Create(static_cast<TESAmmo*>(ammoForm), ammoCount));
		}
	}
	*result = 1; // Successfully loaded weapon
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(DeleteAkimboInstance, DeleteAkimbo, "Deletes akimbos, faster than regular DeleteInstance function", false, kParams_OneForm_OneOptionalInt);
bool Cmd_DeleteAkimboInstance_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESForm* form = NULL;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		TESForm* baseForm = form->IsReference() ? ((TESObjectREFR*)form)->baseForm : form;

		if (baseForm->IsInstancedForm()) {

			Instance* rInstance = baseForm->LookupInstance(40);
			if (rInstance) {

				BGSSaveLoadGame* saveGame = BGSSaveLoadGame::GetSingleton();
				BGSSaveLoadChangesMap* saveChanges = saveGame->changesMap;

				if (saveChanges) {

					NiTMapBase<UInt32, BGSFormChange*>* formChangeMap = &saveChanges->BGSFormChangeMap;

					for (auto it = formChangeMap->Begin(); it; ++it) {

						UInt32 refID = it.Key();

						if (refID) {

							TESForm* object = LookupFormByRefID(refID);

							if (object && object->IsReference()) {

								TESObjectREFR* refObject = (TESObjectREFR*)object;

								if (refObject && refObject->baseForm == rInstance->clone) {

									refObject->DeleteReference();

								}

							}

						}

					}

				}

				rInstance->baseInstance->aInstances.remove(rInstance->InstID);
				delete rInstance;

			}

		}

	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN(DeleteFormInstance, DeleteFormInst, "Creates a new weapon instance aka new baseform for a weapon", false, kParams_OneForm_OneOptionalInt);
bool Cmd_DeleteFormInstance_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESForm* form = NULL;
	UInt32 replace = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &replace)) {

		TESForm* baseForm = form->IsReference() ? ((TESObjectREFR*)form)->baseForm : form;

		if (baseForm->IsInstancedForm()) {

			Instance* rInstance = form->pLookupInstance();
			if (rInstance) {

				BGSSaveLoadGame* saveGame = BGSSaveLoadGame::GetSingleton();
				BGSSaveLoadChangesMap* formChangeMap = saveGame->changesMap;

				if (formChangeMap) {

					NiTMapBase<UInt32, BGSFormChange*>* formChangePointerMap = &formChangeMap->BGSFormChangeMap;

					for (auto it = formChangePointerMap->Begin(); it; ++it) {

							UInt32 someFormPtr = it.Key();

							if (someFormPtr) {
								TESForm* object = LookupFormByRefID(someFormPtr);
								
								if (object && object->IsReference()) {
									TESObjectREFR* refObject = (TESObjectREFR*)object;
									if (refObject && refObject->baseForm == rInstance->clone) {

										if (replace) {

											if (rInstance->baseInstance->extendedType <= 120) {
												refObject->baseForm = rInstance->baseInstance->parent;
											}
											else if (rInstance->baseInstance->extendedType == 222) {

												if (((StaticInstance_Akimbo*)rInstance->baseInstance)->right) {
													refObject->baseForm = ((StaticInstance_Akimbo*)rInstance->baseInstance)->right;
												}
												else if (((StaticInstance_Akimbo*)rInstance->baseInstance)->left) {
													refObject->baseForm = ((StaticInstance_Akimbo*)rInstance->baseInstance)->left;
												}
												else {
													refObject->DeleteReference();
												}

											}

										}
										else {

											refObject->DeleteReference();

										}

									}
									else {

										ContChangesEntryList* entryList = refObject->GetContainerChangesList();

										if (entryList) {

											ContChangesEntry* entry;

											for (auto iter = entryList->Begin(); !iter.End(); ++iter) {

												entry = iter.Get();

												if (entry && (entry->type == rInstance->clone)) {

													UInt32 objectsToRemove = entry->countDelta;
													while (objectsToRemove > 0) {

														ExtraDataList* xData = nullptr;
														ExtraDataList* xDataSave = nullptr;
														if (entry->extendData) {
															xData = entry->extendData->GetFirstItem();
															if (xData) {
																xDataSave = xData->CreateCopy();
																if (xData->HasType(kExtraData_Worn)) {
																	(static_cast<Actor*>(refObject))->UnequipItem(entry->type, 1, xData, 1, 0, 0);
																}
															}
														}

														entry->Remove(xData, 1);
														objectsToRemove--;

													}

												}


											}

										}
									}

								}

							}

					}

				}

				rInstance->destroy();

			}

		}

	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN(SetWeaponMod, AttachWeaponMod, "Set a attachment in a slot", false, kParamsSetWeapAttachments);
bool Cmd_SetWeaponMod_Execute(COMMAND_ARGS) {
	*result = 0;
	TESForm* form = NULL;
	UInt32 InstID = 0;
	TESForm* rAttachment = NULL;
	TESForm* parent = nullptr; //Used for event handler
	char sSlot[MAX_PATH];

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &form, &sSlot, &rAttachment) || !form) {
		return true;
	}

	TESForm* baseForm = form->IsReference() ? ((TESObjectREFR*)form)->baseForm : form;
	StaticInstance_WEAP* staticInstance = nullptr;
	std::unordered_map<std::string, UInt32>* attachments = nullptr;

	if (baseForm->IsStaticForm()) {
		staticInstance = static_cast<StaticInstance_WEAP*>(baseForm->LookupStaticInstance());
		attachments = &staticInstance->aBaseAttachments;
	}
	else if (baseForm->IsInstancedForm(40)) {
		Instance_WEAP* instanceWEAP = (Instance_WEAP*)baseForm->pLookupInstance();
		attachments = &instanceWEAP->aAttachments;
		staticInstance = static_cast<StaticInstance_WEAP*>(instanceWEAP->baseInstance);
	}

	if (attachments && staticInstance && !staticInstance->aAllAttachments.empty()) {
		auto it = attachments->find(sSlot);
		bool attachmentExists = it != attachments->end();

		if (rAttachment) {
			// Attach or replace
			if (attachmentExists) {
				onDetachWeapModEvent.DispatchEvent(LookupFormByRefID(it->second), baseForm);
			}
			onAttachWeapModEvent.DispatchEvent(rAttachment, baseForm);
			(*attachments)[sSlot] = rAttachment->refID;
		}
		else if (attachmentExists) {
			// Detach
			onDetachWeapModEvent.DispatchEvent(LookupFormByRefID(it->second), baseForm);
			attachments->erase(it);
		}
		return true;
	}

	//vanilla
	if (form->IsReference() && (!staticInstance || staticInstance->aAllAttachments.empty())) {

		int slotIndex = std::stoi(sSlot);
		if (slotIndex < 0) {
			UInt8 currentFlags = static_cast<TESObjectREFR*>(form)->GetWeaponModFlags();
			UInt8 flagToRemove = 1 << (abs(slotIndex) - 1);
			UInt8 updatedFlags = currentFlags & ~flagToRemove;
			static_cast<TESObjectREFR*>(form)->SetWeaponModFlags(updatedFlags);
		}
		else if (slotIndex == 0) {
			static_cast<TESObjectREFR*>(form)->SetWeaponModFlags(0);
		}
		else {
			UInt8 currentFlags = static_cast<TESObjectREFR*>(form)->GetWeaponModFlags();
			UInt8 updatedFlags = currentFlags | (1 << (slotIndex - 1));
			static_cast<TESObjectREFR*>(form)->SetWeaponModFlags(updatedFlags);
		}
		return true;

	}

	return true;
}

DEFINE_COMMAND_PLUGIN(GetAllAttachedWeaponMods, "Gets all mods currently attached to the weapon", false, kParams_OneForm);
bool Cmd_GetAllAttachedWeaponMods_Execute(COMMAND_ARGS) {

	*result = 0;
	TESForm* ref = nullptr;

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &ref) || !ref) {
		return true;
	}

	TESForm* baseForm = ref->IsBaseForm() ? ref : static_cast<TESObjectREFR*>(ref)->baseForm;
	std::unordered_map<std::string, UInt32>* attachmentMap = nullptr;
	StaticInstance_WEAP* staticWeapon = nullptr;

	if (baseForm->IsStaticForm()) {
		staticWeapon = static_cast<StaticInstance_WEAP*>(StaticLinker[kFormType_TESObjectWEAP][baseForm->refID]);
		attachmentMap = &staticWeapon->aBaseAttachments;
	}
	else if (baseForm->IsInstancedForm(40)) {
		Instance_WEAP* instanceWEAP = static_cast<Instance_WEAP*>(InstanceLinker[kFormType_TESObjectWEAP][baseForm->refID]);
		attachmentMap = &instanceWEAP->aAttachments;
		staticWeapon = static_cast<StaticInstance_WEAP*>(instanceWEAP->baseInstance);
	}

	auto aAttachments = g_arrInterface->CreateStringMap(nullptr, nullptr, 0, scriptObj);

	// Check for extended attachment system
	if (staticWeapon && !staticWeapon->aAllAttachments.empty()) {
		for (const auto& pair : *attachmentMap) {
			g_arrInterface->SetElement(aAttachments, ArrayElementL(pair.first.c_str()), ArrayElementL(LookupFormByRefID(pair.second)));
		}
		g_arrInterface->AssignCommandResult(aAttachments, result);
		return true;
	}

	// Fallback to vanilla system
	if (!ref->IsBaseForm() && (staticWeapon == nullptr || staticWeapon->aAllAttachments.empty())) {

		UInt8 iFlags = static_cast<TESObjectREFR*>(ref)->GetWeaponModFlags();

		for (UInt8 iSlot = 1; iSlot <= 3; ++iSlot) {
			if (iFlags & (1 << (iSlot - 1))) {
				TESObjectIMOD* pItemMod = static_cast<TESObjectWEAP*>(baseForm)->GetItemMod(iSlot);
				if (pItemMod) {
					std::string key = std::to_string(iSlot);
					g_arrInterface->SetElement(aAttachments, ArrayElementL(key.c_str()), ArrayElementL(pItemMod));
				}
			}
		}

	}

	g_arrInterface->AssignCommandResult(aAttachments, result);
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponMod, GetAttachedWeaponMod, "Gets the mod in a specific slot", false, kParams_OneForm_OneString);
bool Cmd_GetWeaponMod_Execute(COMMAND_ARGS) {
	*result = 0;
	TESForm* ref = nullptr;
	UInt32* refResult = (UInt32*)result;
	char sSlot[MAX_PATH] = "";

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &ref, &sSlot) || !ref) {
		return true;
	}

	TESForm* baseForm = ref->IsBaseForm() ? ref : static_cast<TESObjectREFR*>(ref)->baseForm;

	if (baseForm->IsStaticForm()) {
		StaticInstance_WEAP* staticInstance = static_cast<StaticInstance_WEAP*>(StaticLinker[kFormType_TESObjectWEAP][baseForm->refID]);
		auto it = staticInstance->aBaseAttachments.find(sSlot);
		if (it != staticInstance->aBaseAttachments.end()) {
			*refResult = it->second;
			return true;
		}
	}
	else if (baseForm->IsInstancedForm(40)) {
		Instance_WEAP* instanceWEAP = static_cast<Instance_WEAP*>(InstanceLinker[kFormType_TESObjectWEAP][baseForm->refID]);
		auto it = instanceWEAP->aAttachments.find(sSlot);
		if (it != instanceWEAP->aAttachments.end()) {
			*refResult = it->second;
			return true;
		}
	}

	if (!ref->IsBaseForm() && sSlot[1] == '\0' && sSlot[0] >= '1' && sSlot[0] <= '3') {
		int slotIndex = sSlot[0] - '0';														//subtraction of '0' converts to integer value.
		UInt8 weaponModFlags = static_cast<TESObjectREFR*>(ref)->GetWeaponModFlags();
		if (weaponModFlags & (1 << (slotIndex - 1))) {
			TESObjectIMOD* pItemMod = static_cast<TESObjectWEAP*>(baseForm)->GetItemMod(slotIndex);
			if (pItemMod) {
				*refResult = pItemMod->refID;
				return true;
			}
		}
	}

	return true;
}

DEFINE_COMMAND_PLUGIN(GetAllWeaponMods, "Gets all mods in a specific slot", false, kParams_OneForm_OneOptionalString);
bool Cmd_GetAllWeaponMods_Execute(COMMAND_ARGS) {
	*result = 0;
	TESForm* ref = nullptr;
	char sSlot[MAX_PATH] = "";

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &ref, &sSlot) || !ref) {
		return true;
	}

	TESForm* baseForm = ref->IsBaseForm() ? ref : static_cast<TESObjectREFR*>(ref)->baseForm;
	StaticInstance_WEAP* staticInst = nullptr;
	std::unordered_map<std::string, std::vector<UInt32>>* attachmentMap = nullptr;

	if (baseForm->IsStaticForm()) {
		staticInst = static_cast<StaticInstance_WEAP*>(StaticLinker[kFormType_TESObjectWEAP][baseForm->refID]);
	}
	else if (baseForm->IsInstancedForm(40)) {
		staticInst = static_cast<StaticInstance_WEAP*>(InstanceLinker[kFormType_TESObjectWEAP][baseForm->refID]->baseInstance);
	}

	attachmentMap = staticInst ? &staticInst->aAllAttachments : nullptr;

	NVSEArrayVarInterface::Array* resultArray = g_arrInterface->CreateStringMap(nullptr, nullptr, 0, scriptObj);

	// Handle no attachments or not using the extended system
	if (!attachmentMap || attachmentMap->empty()) {
		int iSlot = max(1, min(3, atoi(sSlot)));
		for (; iSlot <= 3; ++iSlot) {
			TESObjectIMOD* pItemMod = static_cast<TESObjectWEAP*>(baseForm)->GetItemMod(iSlot);
			if (pItemMod) {
				NVSEArrayVarInterface::Array* subArray = g_arrInterface->CreateArray(nullptr, 0, scriptObj);
				g_arrInterface->AppendElement(subArray, ArrayElementL(pItemMod));
				g_arrInterface->SetElement(resultArray, ArrayElementL(std::to_string(iSlot).c_str()), ArrayElementL(subArray));
			}
		}
		g_arrInterface->AssignCommandResult(resultArray, result);
		return true;
	}

	// Return all attachments if sSlot is omitted
	if (sSlot[0] == '\0') {
		for (const auto& pair : *attachmentMap) {
			NVSEArrayVarInterface::Array* subArray = g_arrInterface->CreateArray(nullptr, 0, scriptObj);
			for (UInt32 refID : pair.second) {
				g_arrInterface->AppendElement(subArray, ArrayElementL(LookupFormByRefID(refID)));
			}
			g_arrInterface->SetElement(resultArray, ArrayElementL(pair.first.c_str()), ArrayElementL(subArray));
		}
	}
	else { // Return attachments for a specific slot
		auto it = attachmentMap->find(sSlot);
		if (it != attachmentMap->end()) {
			NVSEArrayVarInterface::Array* subArray = g_arrInterface->CreateArray(nullptr, 0, scriptObj);
			for (UInt32 refID : it->second) {
				g_arrInterface->AppendElement(subArray, ArrayElementL(LookupFormByRefID(refID)));
			}
			g_arrInterface->SetElement(resultArray, ArrayElementL(sSlot), ArrayElementL(subArray));
		}
	}

	g_arrInterface->AssignCommandResult(resultArray, result);
	return true;
}

DEFINE_COMMAND_PLUGIN(HasExtendedWeaponMods, "Gets all mods in a specific slot", false, kParams_OneForm);
bool Cmd_HasExtendedWeaponMods_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* ref = NULL;

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &ref))
		return true;

	if (!ref)
		return true;

	TESForm* baseForm = ref->IsBaseForm() ? ref : (static_cast<TESObjectREFR*>(ref))->baseForm;
	*result = baseForm->HasExtendedMods();

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(IsFormInstance, IsInstance, "Gets the weapon instance when passed a ref", false, kParams_OneForm);
bool Cmd_IsFormInstance_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form) {

			*result = form->IsInstancedForm();

		}

	}
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetStaticForm, GSF, "Gets the true baseform", false, kParams_OneForm);
bool Cmd_GetStaticForm_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	UInt32* refResult = (UInt32*)result;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (!form) {
			return true;
		}

		form = form->IsReference() ? ((TESObjectREFR*)form)->baseForm : form;

		if (form->IsInstancedForm()) {

			form = form->GetStaticParent();
			if (form) {
				*refResult = form->refID;
			}

		}
		else {

			*refResult = form->refID;
		}

	}
	return true;

}

DEFINE_COMMAND_ALT_PLUGIN(GetBaseInstance, GBI, "Acts like GetBaseForm or GetBaseObject", true, nullptr);
bool Cmd_GetBaseInstance_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj && thisObj->baseForm) {
		*refResult = thisObj->baseForm->refID;
		if (IsConsoleMode())
			Console_Print("GetBaseInstance >> %08x (%s) (%s)", thisObj->baseForm->refID, GetFullName(thisObj->baseForm), thisObj->baseForm->GetEditorID());
	}
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetFormInstanceID, GetFormInstID, "Gets the ID for a Modifier when passed its clone", false, kParams_OneForm);
bool Cmd_GetFormInstanceID_Execute(COMMAND_ARGS)
{
	*result = -1;
	TESForm* clone = NULL;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &clone)) {

		if (clone->IsReference()) {
			clone = ((TESObjectREFR*)clone)->baseForm;
		}

		if (clone->IsInstancedForm()) {
			*result = InstanceLinker[clone->typeID][clone->refID]->InstID;
		}

	}

	return true;
}

DEFINE_COMMAND_PLUGIN_EXP(GetObjectInstances, "Gets all Instances for a static form.", false, kNVSEParams_OneForm_OneOptionalInt);
bool Cmd_GetObjectInstances_Execute(COMMAND_ARGS) {

	*result = 0;
	TESForm* form = NULL;
	UInt32 index = 0;

	PluginExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (!eval.ExtractArgs()) return true;

	form = eval.GetNthArg(0)->GetTESForm();
	if (!form) return true;

	form = form->GetBaseObject();
	if (!form || !form->IsStaticForm()) return true;

	InstanceVector* instVect = &form->LookupStaticInstance()->aInstances;
	if (!instVect || instVect->empty()) return true;

	NVSEArrayVar* resultArray = g_arrInterface->CreateArray(nullptr, 0, scriptObj);

	if (eval.NumArgs() > 1) {
		index = eval.GetNthArg(1)->GetInt();
		if (index < instVect->size()) {
			Instance* inst = (*instVect)[index];
			g_arrInterface->AppendElement(resultArray, ArrayElementL(inst->clone));
		}
	}
	else {
		for (Instance* inst : *instVect) {
			g_arrInterface->AppendElement(resultArray, ArrayElementL(inst->clone));
		}
	}

	g_arrInterface->AssignCommandResult(resultArray, result);
	return true;
}

DEFINE_COMMAND_PLUGIN_EXP(GetAllBaseInstances, "Gets Instances all instances with optional filters.", false, kNVSE_OneOptionalString_OneOptionalInt_OneOptionalForm);
bool Cmd_GetAllBaseInstances_Execute(COMMAND_ARGS) {

	*result = 0;
	const char* key = nullptr;
	UInt32 type = 0;
	TESForm* form = nullptr;

	PluginExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (!eval.ExtractArgs()) return true;

	key = eval.GetNthArg(0)->GetString();
	if (eval.NumArgs() > 1) type = eval.GetNthArg(1)->GetInt();
	if (eval.NumArgs() > 2) form = eval.GetNthArg(2)->GetTESForm();

	auto aResult = g_arrInterface->CreateArray(nullptr, 0, scriptObj); // Create an empty array

	switch (eval.NumArgs()) {
	case 3: // Form, type, and key are specified
		if (form) {
			form = form->GetBaseObject();
			if (form->IsStaticForm()) {
				auto& instances = form->LookupStaticInstance()->aInstances;
				if (!instances.empty()) {
					for (Instance* inst : instances) {
						if (inst && inst->key == key) {
							g_arrInterface->AppendElement(aResult, ArrayElementL(inst->clone));
						}
					}
				}
			}
		}
		break;

	case 2: // Type and key are specified
		for (auto& [refID, inst] : InstanceLinker[type]) {
			if (inst->key == key) {
				g_arrInterface->AppendElement(aResult, ArrayElementL(inst->clone));
			}
		}
		break;

	case 1: // Only key is specified
		for (const auto& [type, instanceList] : InstanceLinker) {
			for (const auto& [refID, inst] : instanceList) {
				if (inst->key == key) {
					g_arrInterface->AppendElement(aResult, ArrayElementL(inst->clone));
				}
			}
		}
		break;

	default: // No arguments specified, return all instances
		for (const auto& [type, instanceList] : InstanceLinker) {
			for (const auto& [refID, inst] : instanceList) {
				g_arrInterface->AppendElement(aResult, ArrayElementL(inst->clone));
			}
		}
		break;
	}

	g_arrInterface->AssignCommandResult(aResult, result); // Return array
	return true;
}

DEFINE_COMMAND_PLUGIN(IsStaticForm, "Checks if a ref is registered.", false, kParams_OneForm);
bool Cmd_IsStaticForm_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form->IsStaticForm()) {
			*result = 1;
		}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN(SetOnInstanceReconstruct, "When instances is loaded in", false, kParams_Event_OneStringF);
bool Cmd_SetOnInstanceReconstruct_Execute(COMMAND_ARGS)
{
	SInt32 priority = 1;
	Script* script = nullptr;
	char key[0x50];

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &priority, &script, &key))
	{
		return true;
	}

	AuxVector filter;
	if (key[0] != '\0') {
		filter = Event::EvaluateEventArgAux(1, 1, AuxValue(key));
	}
	else {
		filter = Event::EvaluateEventArgAux(1, 1, AuxValue());
	}

	Event eEvent(priority, script, filter);

	if (priority != 0)
	{
		onInstanceReconstructEvent.AddEvent(eEvent);
	}
	else
	{
		onInstanceReconstructEvent.RemoveEvent(eEvent);
	}

	return true;
}

DEFINE_COMMAND_PLUGIN(SetOnInstanceDeconstruct, "When instances is loaded in", false, kParams_Event_OneStringF);
bool Cmd_SetOnInstanceDeconstruct_Execute(COMMAND_ARGS)
{
	SInt32 priority = 1;
	Script* script = nullptr;
	char key[0x50];

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &priority, &script, &key))
	{
		return true;
	}

	AuxVector filter;
	if (key[0] != '\0') {
		filter = Event::EvaluateEventArgAux(1, 1, AuxValue(key));
	}
	else {
		filter = Event::EvaluateEventArgAux(1, 1, AuxValue());
	}

	Event eEvent(priority, script, filter);

	if (priority != 0)
	{
		onInstanceDeconstructEvent.AddEvent(eEvent);
	}
	else
	{
		onInstanceDeconstructEvent.RemoveEvent(eEvent);
	}

	return true;
}

DEFINE_COMMAND_PLUGIN_EXP(SetOnAttachWeaponMod, "Dispatch when a weapon mod is attached", false, kNVSE_Event_OneNumber_TwoFormsF);
bool Cmd_SetOnAttachWeaponMod_Execute(COMMAND_ARGS)
{
	SInt32 priority = 1;
	Script* script = nullptr;

	PluginScriptToken* arg1 = nullptr;
	PluginScriptToken* arg2 = nullptr;
	UInt32 order1 = 0;
	UInt32 order2 = 0;

	bool runOnReconstruct = 0;

	//&priority, &script, &runOnReconstruct, &order1, &arg1, &order2, &arg2

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{

		priority = eval.GetNthArg(0)->GetFloat();
		script = (Script*)eval.GetNthArg(1)->GetTESForm();
		runOnReconstruct = eval.GetNthArg(2)->GetBool();

		if (eval.NumArgs() > 3) {
			order1 = StringUtils::ToUInt32(eval.GetNthArg(3)->GetString());
			arg1 = eval.GetNthArg(4);
			if (eval.NumArgs() > 5) {
				order2 = StringUtils::ToUInt32(eval.GetNthArg(5)->GetString());
				arg2 = eval.GetNthArg(6);
			}
		}

		AuxVector filter = Event::EvaluateEventArg(2, order1, arg1, order2, arg2);
		Event eEvent(priority, script, filter);

		if (priority != 0)
		{

			onAttachWeapModEvent.AddEvent(eEvent);
			if (runOnReconstruct) {
				onAttachWeapModReconstructEvent.AddEvent(eEvent);
			}
		}
		else
		{
			onAttachWeapModEvent.RemoveEvent(eEvent);
			onAttachWeapModReconstructEvent.RemoveEvent(eEvent);
		}

	}

	return true;
}

DEFINE_COMMAND_PLUGIN_EXP(SetOnDetachWeaponMod, "When instances is loaded in", false, kNVSE_Event_OneNumber_TwoFormsF);
bool Cmd_SetOnDetachWeaponMod_Execute(COMMAND_ARGS)
{
	SInt32 priority;
	Script* script;

	PluginScriptToken* arg1 = nullptr;
	PluginScriptToken* arg2 = nullptr;
	UInt32 order1 = 0;
	UInt32 order2 = 0;

	bool runOnDeconstruct = 0;

	//&priority, &script, &runOnDeconstruct, &order1, &arg1, &order2, &arg2

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{

		priority = eval.GetNthArg(0)->GetFloat();
		script = (Script*)eval.GetNthArg(1)->GetTESForm();
		runOnDeconstruct = eval.GetNthArg(2)->GetBool();

		if (eval.NumArgs() > 3) {
			order1 = StringUtils::ToUInt32(eval.GetNthArg(3)->GetString());
			arg1 = eval.GetNthArg(4);
			if (eval.NumArgs() > 5) {
				order2 = StringUtils::ToUInt32(eval.GetNthArg(5)->GetString());
				arg2 = eval.GetNthArg(6);
			}
		}

		AuxVector filter = Event::EvaluateEventArg(2, order1, arg1, order2, arg2);
		Event eEvent(priority, script, filter);

		if (priority != 0)
		{

			onDetachWeapModEvent.AddEvent(eEvent);
			if (runOnDeconstruct) {
				onDetachWeapModDeconstructEvent.AddEvent(eEvent);
			}

		}
		else
		{
			onDetachWeapModEvent.RemoveEvent(eEvent);
			onDetachWeapModDeconstructEvent.RemoveEvent(eEvent);
		}

	}

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetFormInstanceKey, GetWeapInstKey, "Gets the key to a weapon instance", false, kParams_OneForm);
bool Cmd_GetFormInstanceKey_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	const char* sResult = nullptr;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form && form->IsInstancedForm()) {

			sResult = InstanceLinker[form->typeID][form->refID]->key.c_str();

		}

		AssignString(PASS_COMMAND_ARGS, sResult);

	}
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetFormInstance, GetInst, "Gets the clone form the instanceID and a baseform", false, kParams_OneForm_OneInt);
bool Cmd_GetFormInstance_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	UInt32* refResult = (UInt32*)result;

	UInt32 instID = 0;
	Instance* inst;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &instID)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form && form->IsStaticForm()) {

			inst = form->LookupInstanceByID(instID);
			if (inst) {
				*refResult = inst->clone->refID;
			}

		}

	}
	return true;
}

//........................................................................................................................................

void UnpackTraitArguments(PluginExpressionEvaluator& eval, SInt32& index, TESForm*& formBase, const char*& trait, ExtendedBaseType*& linkedObj, const char*& sSlot, UInt8& priority) {

	UInt8 argCount = 0;
	UInt8 type = eval.GetNthArg(argCount)->GetType();
	if (type == kTokenType_Number || type == kTokenType_NumericVar) {
		index = eval.GetNthArg(argCount)->GetInt();
		++argCount;
	}

	formBase = eval.GetNthArg(argCount)->GetTESForm();
	++argCount;
	trait = eval.GetNthArg(argCount)->GetString();
	++argCount;

	int numArgs = eval.NumArgs() - 1;
	if (numArgs >= argCount) {
		TESForm* linked = eval.GetNthArg(argCount)->GetTESForm();
		if (!linked) {
			return;
		}
		linkedObj = linked->LookupExtendedBase();
		if (!linkedObj) {
			return;
		}
		++argCount;
		if (numArgs >= argCount) {
			sSlot = eval.GetNthArg(argCount)->GetString();
			++argCount;
			if (numArgs >= argCount) {
				priority = eval.GetNthArg(argCount)->GetInt();
			}
		}
	}
}

DEFINE_COMMAND_PLUGIN_EXP(GetBaseTraitType, "Gets the var type", false, kNVSEParams_OneFormOrNumber_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber);
bool Cmd_GetBaseTraitType_Execute(COMMAND_ARGS)
{

	*result = -2;
	UInt32* refResult = (UInt32*)result;
	SInt32 index = 0;
	TESForm* formBase = NULL;
	const char* trait = NULL;

	ExtendedBaseType* linkedObj = nullptr;
	const char* sSlot = "null";
	UInt8 priority = 0;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{

		UnpackTraitArguments(eval, index, formBase, trait, linkedObj, sSlot, priority);

		if (!formBase) {
			return true;
		}

		if (ExtendedBaseType* baseObj = formBase->LookupExtendedBase()) {

			const AuxVector* aux = baseObj->GetTrait(trait, linkedObj, sSlot, priority);

			if (!aux || index >= aux->size()) {
				*result = -2;
				return true;
			}

			*result = (*aux)[index].type;

		}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN_EXP(GetBaseTrait, "Gets the var", false, kNVSEParams_OneFormOrNumber_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber);
bool Cmd_GetBaseTrait_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	SInt32 index = 0;
	TESForm* baseForm = NULL;
	TESForm* linkedForm = NULL;
	const char* trait = NULL;

	ExtendedBaseType* linkedObj = nullptr;
	const char* sSlot = "null";
	UInt8 priority = 0;


	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{

		UnpackTraitArguments(eval, index, baseForm, trait, linkedObj, sSlot, priority);

		if (!baseForm) {
			return true;
		}

		ExtendedBaseType* baseObj = baseForm->LookupExtendedBase();
		if (!baseObj) {
			return true;
		}

		const AuxVector* aux = baseObj->GetTrait(trait, linkedObj, sSlot, priority);

		if (!aux || index < 0 || index >= aux->size()) {
			return true;
		}

		if ((*aux)[index].type != -1 ) {
			eval.SetExpectedReturnType((CommandReturnType)(*aux)[index].type);

			switch ((*aux)[index].type) {
			case kRetnType_Default:
				*result = (*aux)[index].num;
				break;
			case kRetnType_Form:
				*refResult = (*aux)[index].refID;
				break;
			case kRetnType_String:
				AssignString(PASS_COMMAND_ARGS, (*aux)[index].str);
				break;
			case kRetnType_Array:
				g_arrInterface->AssignCommandResult((*aux)[index].CopyToNVSEArray(scriptObj), result);
				break;
			}

		}

	}

	return true;

}

void UnpackTraitArguments(PluginExpressionEvaluator& eval, PluginScriptToken*& value, SInt32& index, TESForm*& baseForm, const char*& trait, ExtendedBaseType*& linkedObj, const char*& sSlot, UInt8& priority, Script*& scriptObj) {

	UInt8 argCount = 0;
	value = eval.GetNthArg(argCount);
	++argCount;

	UInt8 type = eval.GetNthArg(argCount)->GetType();
	if (type == kTokenType_Number || type == kTokenType_NumericVar) {
		index = eval.GetNthArg(argCount)->GetInt();
		++argCount;
	}

	baseForm = eval.GetNthArg(argCount)->GetTESForm();
	++argCount;
	trait = eval.GetNthArg(argCount)->GetString();
	++argCount;

	int numArgs = eval.NumArgs() - 1;
	if (numArgs >= argCount) {
		TESForm* linked = eval.GetNthArg(argCount)->GetTESForm();
		if (!linked) {
			return;
		}
		linkedObj = linked->getExtendedBase(scriptObj->GetModIndexAlt());
		if (!linkedObj) {
			return;
		}
		++argCount;
		if (numArgs >= argCount) {
			sSlot = eval.GetNthArg(argCount)->GetString();
			++argCount;
			if (numArgs >= argCount) {
				priority = eval.GetNthArg(argCount)->GetInt();
			}
		}
	}
}


DEFINE_COMMAND_PLUGIN_EXP(SetBaseTrait, "Set the var", false, kNVSEParams_OneBasicType_OneFormOrNumber_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber);
bool Cmd_SetBaseTrait_Execute(COMMAND_ARGS) {

	*result = 0;

	//BasicType value;
	SInt32 index = 0;
	ExtendedBaseType* linkedObj = NULL;
	TESForm* baseForm = NULL;
	const char* trait = NULL;

	const char* sSlot = "null";
	UInt8 priority = 0;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		PluginScriptToken* value;
		UnpackTraitArguments(eval, value, index, baseForm, trait, linkedObj, sSlot, priority, scriptObj);

		if (!baseForm) {
			return true;
		}

		ExtendedBaseType* baseObj = baseForm->getExtendedBase(scriptObj->GetModIndexAlt());
		if (!baseObj) {
			return true;
		}

		AuxVector* aux = nullptr;

		aux = baseObj->SetTrait(trait, linkedObj, sSlot, priority);

		if (!aux) {
			Console_Print("Error: Unable to get trait");
			return true;
		}

		switch (value->GetType()) {
		case kTokenType_Number:
		case kTokenType_NumericVar:
		{
			double valueFlt = value->GetFloat(); // Store the float value
			aux->AddValue(index, valueFlt);
			break;
		}
		case kTokenType_Form:
		case kTokenType_RefVar:
		{
			TESForm* valueRef = value->GetTESForm();
			if (valueRef) {
				aux->AddValue(index, valueRef->refID);
			}
			break;
		}
		case kTokenType_String:
		case kTokenType_StringVar:
		{
			const char* valueStr = value->GetString();
			if (valueStr) {
				aux->AddValue(index, valueStr);
			}
			break;
		}
		case kTokenType_Array:
		case kTokenType_ArrayVar:
		{
			NVSEArrayVarInterface::Array* valueArr = value->GetArrayVar();
			if (valueArr) {
				aux->AddValue(index, valueArr);
			}
			break;
		}
		}

	}

	return true;

}

void UnpackTraitArguments(PluginExpressionEvaluator& eval, TESForm*& baseForm, const char*& trait, ExtendedBaseType*& linkedObj, const char*& sSlot, UInt8& priority) {

	baseForm = eval.GetNthArg(0)->GetTESForm();
	trait = eval.GetNthArg(1)->GetString();

	int numArgs = eval.NumArgs();
	if (numArgs >= 3) {
		TESForm* linked = eval.GetNthArg(2)->GetTESForm();
		if (!linked) {
			return;
		}
		linkedObj = linked->LookupExtendedBase();
		if (!linkedObj) {
			return;
		}
		if (numArgs >= 4) {
			sSlot = eval.GetNthArg(3)->GetString();
			if (numArgs >= 5) {
				priority = eval.GetNthArg(4)->GetInt();
			}
		}
	}
}

DEFINE_COMMAND_PLUGIN_EXP(GetBaseTraitListSize, "Gets the var effect", false, kNVSEParams_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber);
bool Cmd_GetBaseTraitListSize_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESForm* baseForm = NULL;
	const char* trait = NULL;

	ExtendedBaseType* linkedObj = nullptr;
	const char* sSlot = "null";
	UInt8 priority = 0;


	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		UnpackTraitArguments(eval, baseForm, trait, linkedObj, sSlot, priority);

		if (!baseForm) {
			return true;
		}

		if (ExtendedBaseType* baseObj = baseForm->LookupExtendedBase()) {

			const AuxVector* aux = baseObj->GetTrait(trait, linkedObj, sSlot, priority);
			if (aux) {
				*result = aux->size();
			}

		}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN_EXP(HasBaseTrait, "Checks to see if the trait exists", false, kNVSEParams_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber);
bool Cmd_HasBaseTrait_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	TESForm* baseForm = NULL;
	const char* trait = NULL;

	ExtendedBaseType* linkedObj = nullptr;
	const char* sSlot = "null";
	UInt8 priority = 0;


	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{

		UnpackTraitArguments(eval, baseForm, trait, linkedObj, sSlot, priority);

		if (!baseForm) {
			return true;
		}

		if (ExtendedBaseType* baseObj = baseForm->LookupExtendedBase()) {

			const AuxVector* aux = baseObj->GetTrait(trait, linkedObj, sSlot, priority);
			if (aux) {
				*result = 1;
			}

		}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN_EXP(SetOnEquipAlt, "When an inventory object is equipped", false, EventParams3_MultiType);
bool Cmd_SetOnEquipAlt_Execute(COMMAND_ARGS)
{

	SInt32 priority = 1;
	Script* script = nullptr;

	PluginScriptToken* arg1 = nullptr;	//Type
	PluginScriptToken* arg2 = nullptr;	//Equipped
	PluginScriptToken* arg3 = nullptr;	//Equipper
	UInt32 order1 = 0;
	UInt32 order2 = 0;
	UInt32 order3 = 0;

	//&priority, &script, &order1, &arg1, &order2, &arg2, &order3, &arg3

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		priority = eval.GetNthArg(0)->GetFloat();
		script = (Script*)eval.GetNthArg(1)->GetTESForm();

		PluginScriptToken* temp = eval.GetNthArg(2);
		if (temp) {
			order1 = StringUtils::ToUInt32(temp->GetString());
		}
		temp = eval.GetNthArg(4);
		if (temp) {
			order2 = StringUtils::ToUInt32(temp->GetString());
		}
		temp = eval.GetNthArg(6);
		if (temp) {
			order3 = StringUtils::ToUInt32(temp->GetString());
		}

		arg1 = eval.GetNthArg(3);
		arg2 = eval.GetNthArg(5);
		arg3 = eval.GetNthArg(7);

		AuxVector filter = Event::EvaluateEventArg(3, order1, arg1, order2, arg2, order3, arg3);
		Event eEvent(priority, script, filter);

		if (priority != 0)
		{

			onEquipAltEvent.AddEvent(eEvent);

		}
		else
		{
			onEquipAltEvent.RemoveEvent(eEvent);

		}

	}

	return true;
}