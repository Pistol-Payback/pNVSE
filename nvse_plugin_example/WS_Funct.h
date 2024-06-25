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

				form = form->CreateInst(key);
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

				AuxVector filter{ rInstance->key.c_str() };
				for (auto it = onInstanceDeconstructEvent.handlers.begin(); it != onInstanceDeconstructEvent.handlers.end(); ++it) {

					if (it->CompareFilters(filter)) {
						g_scriptInterface->CallFunction(it->script, nullptr, nullptr, nullptr, 1, rInstance->clone);
					}

				}

				BGSSaveLoadGame* saveGame = BGSSaveLoadGame::GetSingleton();
				BGSSaveLoadChangesMap* formChangeMap = saveGame->changesMap;

				const char* name;
				bool suck = false;

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
											refObject->baseForm = rInstance->baseInstance->parent;
											//thisObj->Update3D();
										}
										else {
											refObject->DeleteReference();
										}
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
		staticInstance = static_cast<StaticInstance_WEAP*>(StaticLinker[kFormType_TESObjectWEAP][baseForm->refID]);
		attachments = &staticInstance->aBaseAttachments;
	}
	else if (baseForm->IsInstancedForm()) {
		Instance_WEAP* instanceWEAP = static_cast<Instance_WEAP*>(InstanceLinker[kFormType_TESObjectWEAP][baseForm->refID]);
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
	else if (baseForm->IsInstancedForm()) {
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
					g_arrInterface->SetElement(aAttachments, ArrayElementL(key.c_str()), ArrayElementL(pItemMod->refID));
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
	else if (baseForm->IsInstancedForm()) {
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
	else if (baseForm->IsInstancedForm()) {
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

	StaticInstance_WEAP* staticInst = nullptr;

	if (baseForm->IsStaticForm()) {

		staticInst = (StaticInstance_WEAP*)StaticLinker[40][baseForm->refID];
		*result = staticInst->aAllAttachments.size();

	}
	else if (baseForm->IsInstancedForm()) {

		staticInst = (StaticInstance_WEAP*)InstanceLinker[40][baseForm->refID]->baseInstance;
		*result = staticInst->aAllAttachments.size();

	}

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

bool Hook_GetBaseObject_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj && thisObj->baseForm) {

		TESForm* form = thisObj->baseForm;

		if (form->IsInstancedForm()) {

			form = form->GetStaticParent();
			if (form) {
				*refResult = form->refID;
			}

		}

		*refResult = form->refID;

		if (IsConsoleMode())
			Console_Print("GetBaseObject >> %08x (%s) (%s)", form->refID, GetFullName(form), form->GetEditorID());

	}
	return true;

}

namespace Hooks
{
	void CMDPatchHooks()
	{

		CommandInfo* cmdInfo = g_cmdTableInterface.GetByOpcode(0x1403);
		cmdInfo->execute = Hook_GetBaseObject_Execute;

	}
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

DEFINE_COMMAND_ALT_PLUGIN(GetAllFormInstances, GetAllFormInsts, "Gets Instances for a static form.", false, kParams_OneForm_OneOptionalInt);
bool Cmd_GetAllFormInstances_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	UInt32 index = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &index)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form->IsStaticForm()) {

			auto aResult = g_arrInterface->CreateArray(nullptr, 0, scriptObj);	//Create an empty array
			auto* instanceVector = &form->LookupStaticInstance()->aInstances;

			for (auto it = instanceVector->begin(); it != instanceVector->end(); ++it) {
				Instance* inst = *it;
				ArrayElementL rElem = LookupFormByRefID(inst->clone->refID);
				g_arrInterface->AppendElement(aResult, rElem);
			}

			g_arrInterface->AssignCommandResult(aResult, result);	//Return array

		}

	}

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

DEFINE_COMMAND_PLUGIN(SetOnAttachWeaponMod, "Dispatch when a weapon mod is attached", false, kParams_Event_OneForm_TwoFormsF);
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

DEFINE_COMMAND_PLUGIN(SetOnDetachWeaponMod, "When instances is loaded in", false, kParams_Event_OneForm_TwoFormsF);
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

DEFINE_COMMAND_PLUGIN_EXP(GetFormTrait, "Gets the var effect", false, kNVSEParams_OneNumber_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber);
bool Cmd_GetFormTrait_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	SInt32 index = -1;
	TESForm* baseForm = NULL;
	TESForm* linkedForm = NULL;
	const char* trait = NULL;

	StaticInstance* linkedObj = nullptr;
	const char* sSlot = "null";
	UInt8 priority = 0;


	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{

		int numArgs = eval.NumArgs();
		if (numArgs >= 4) {

			TESForm* linkedForm = eval.GetNthArg(3)->GetTESForm();
			if (!linkedForm) {
				Console_Print("Error, linked Form invalid");
				return true;
			}
			linkedObj = linkedForm->LookupStaticInstance();

			if (numArgs >= 5) {
				sSlot = eval.GetNthArg(4)->GetString();
			}
			if (numArgs >= 6) {
				priority = eval.GetNthArg(5)->GetInt();
			}
		}

		index = eval.GetNthArg(0)->GetFloat();
		baseForm = eval.GetNthArg(1)->GetTESForm();
		trait = eval.GetNthArg(2)->GetString();

		if (!baseForm) {
			return true;
		}

		if (StaticInstance* baseObj = baseForm->LookupStaticInstance()) {

			const AuxVector* aux = baseObj->GetTrait(trait, linkedObj, sSlot, priority);

			if (!aux || index < 0 || index >= aux->size()) {
				return true;
			}

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

DEFINE_COMMAND_PLUGIN_EXP(SetFormTrait, "Gets the var effect", false, kNVSEParams_OneBasicType_OneNumber_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber);
bool Cmd_SetFormTrait_Execute(COMMAND_ARGS) {

	*result = 0;

	//BasicType value;
	TESForm* linkedForm = NULL;
	TESForm* baseForm = NULL;
	SInt32 index = -1;
	const char* trait = NULL;

	const char* sSlot = "null";
	UInt8 priority = 0;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{

		int numArgs = eval.NumArgs();
		if (numArgs >= 5) {

			linkedForm = eval.GetNthArg(4)->GetTESForm();

			if (numArgs >= 6) {
				sSlot = eval.GetNthArg(5)->GetString();
			}
			if (numArgs >= 7) {
				priority = eval.GetNthArg(6)->GetInt();
			}
		}

		PluginScriptToken* arg = eval.GetNthArg(0);
		index = eval.GetNthArg(1)->GetFloat();
		baseForm = eval.GetNthArg(2)->GetTESForm();
		trait = eval.GetNthArg(3)->GetString();


		if (!baseForm) {
			return true;
		}

		StaticInstance* baseObj = baseForm->LookupStaticInstance();
		if (!baseObj) {
			Console_Print("Error: object is not static");
			return true;
		}

		AuxVector* aux = nullptr;

		if (linkedForm) {

			StaticInstance* linkedObj = linkedForm->LookupStaticInstance();
			if (!linkedObj) {
				Console_Print("Error, linked form is not static");
				return true;
			}

			aux = baseObj->SetTrait(trait, linkedObj, sSlot, priority);

		}
		else {

			aux = baseObj->SetBaseTrait(trait);

		}

		if (!aux) {
			Console_Print("Error: Unable to get trait");
			return true;
		}

		switch (arg->GetType()) {
		case kTokenType_Number:
		case kTokenType_NumericVar:
		{
			double valueFlt = arg->GetFloat(); // Store the float value
			aux->AddValue(index, valueFlt);
			break;
		}
		case kTokenType_Form:
		case kTokenType_RefVar:
		{
			TESForm* valueRef = arg->GetTESForm();
			if (valueRef) {
				aux->AddValue(index, valueRef->refID);
			}
			break;
		}
		case kTokenType_String:
		case kTokenType_StringVar:
		{
			const char* valueStr = arg->GetString();
			if (valueStr) {
				aux->AddValue(index, valueStr);
			}
			break;
		}
		case kTokenType_Array:
		case kTokenType_ArrayVar:
		{
			NVSEArrayVarInterface::Array* valueArr = arg->GetArrayVar();
			if (valueArr) {
				aux->AddValue(index, valueArr);
			}
			break;
		}
		}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN_EXP(GetFormTraitListSize, "Gets the var effect", false, kNVSEParams_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber);
bool Cmd_GetFormTraitListSize_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	TESForm* baseForm = NULL;
	const char* trait = NULL;

	StaticInstance* linkedObj = nullptr;
	const char* sSlot = "null";
	UInt8 priority = 0;


	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{

		int numArgs = eval.NumArgs();
		if (numArgs >= 3) {

			TESForm* linkedForm = eval.GetNthArg(2)->GetTESForm();
			if (!linkedForm) {
				return true;
			}
			linkedObj = linkedForm->LookupStaticInstance();

			if (numArgs >= 4) {
				sSlot = eval.GetNthArg(3)->GetString();
			}
			if (numArgs >= 5) {
				priority = eval.GetNthArg(4)->GetInt();
			}
		}

		baseForm = eval.GetNthArg(0)->GetTESForm();
		trait = eval.GetNthArg(1)->GetString();

		if (!baseForm) {
			return true;
		}

		if (StaticInstance* baseObj = baseForm->LookupStaticInstance()) {

			const AuxVector* aux = baseObj->GetTrait(trait, linkedObj, sSlot, priority);
			if (aux) {
				*result = aux->size();
			}

		}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN_EXP(HasFormTrait, "Checks to see if the trait exists", false, kNVSEParams_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber);
bool Cmd_HasFormTrait_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	TESForm* baseForm = NULL;
	const char* trait = NULL;

	StaticInstance* linkedObj = nullptr;
	const char* sSlot = "null";
	UInt8 priority = 0;


	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{

		int numArgs = eval.NumArgs();
		if (numArgs >= 3) {

			TESForm* linkedForm = eval.GetNthArg(2)->GetTESForm();
			if (!linkedForm) {
				return true;
			}
			linkedObj = linkedForm->LookupStaticInstance();

			if (numArgs >= 4) {
				sSlot = eval.GetNthArg(3)->GetString();
			}
			if (numArgs >= 5) {
				priority = eval.GetNthArg(4)->GetInt();
			}
		}

		baseForm = eval.GetNthArg(0)->GetTESForm();
		trait = eval.GetNthArg(1)->GetString();

		if (!baseForm) {
			return true;
		}

		if (StaticInstance* baseObj = baseForm->LookupStaticInstance()) {

			const AuxVector* aux = baseObj->GetTrait(trait, linkedObj, sSlot, priority);
			if (aux) {
				*result = 1;
			}

		}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN_EXP(GetFormTraitType, "Gets the var effect", false, kNVSEParams_OneNumber_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber);
bool Cmd_GetFormTraitType_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	SInt32 index = -1;
	TESForm* formBase = NULL;
	const char* trait = NULL;

	StaticInstance* linkedObj = nullptr;
	const char* sSlot = "null";
	UInt8 priority = 0;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		int numArgs = eval.NumArgs();
		if (numArgs >= 4) {
			linkedObj = eval.GetNthArg(3)->GetTESForm()->LookupStaticInstance();
			if (numArgs >= 5) {
				sSlot = eval.GetNthArg(4)->GetString();
			}
			if (numArgs >= 6) {
				priority = eval.GetNthArg(5)->GetInt();
			}
		}

		index = eval.GetNthArg(0)->GetFloat();
		formBase = eval.GetNthArg(1)->GetTESForm();
		trait = eval.GetNthArg(2)->GetString();

		if (!formBase) {
			return true;
		}

		if (StaticInstance* baseObj = formBase->LookupStaticInstance()) {

			const AuxVector* aux = baseObj->GetTrait(trait, linkedObj, sSlot, priority);

			if (!aux || index < 0 || index >= aux->size()) {
				*result = -2;
				return true;
			}

			*result = (*aux)[index].type;

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