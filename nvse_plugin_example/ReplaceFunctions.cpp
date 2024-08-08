#pragma once
#include "ReplaceFunctions.h"
#include "KitLoader.h"

//Instance function hooks

bool Hook_GetBaseObject_Execute(COMMAND_ARGS)
{
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (thisObj && thisObj->baseForm) {

		TESForm* form = thisObj->baseForm;
		*refResult = form->refID;

		if (form->IsInstancedForm()) {

			form = form->GetStaticParent(form->typeID);
			if (form) {
				*refResult = form->refID;

				if (IsConsoleMode())
					Console_Print("GetBaseObject >> %08x (%s) (%s)", form->refID, GetFullName(form), form->GetEditorID());

				return true;

			}

		}

		if (IsConsoleMode())
			Console_Print("GetBaseObject >> %08x (%s) (%s)", form->refID, GetFullName(form), form->GetEditorID());

	}
	return true;

}
/*
bool Hook_ListGetFormIndex_Eval(COMMAND_ARGS_EVAL)
{
	*result = -1;
	BGSListForm* pListForm = NULL;
	TESForm* pForm = NULL;

	if (arg1 && arg2) {
		pListForm = (BGSListForm*)arg1;
		pForm = (TESForm*)arg2;
		if (pForm->GetBaseObject()) {

		}
		SInt32 index = pListForm->GetIndexOf(pForm);
		*result = index;
		if (IsConsoleMode()) {
			Console_Print("Index: %d", index);
		}
	}

	return true;
}

bool Cmd_ListGetFormIndex_Execute(COMMAND_ARGS)
{
	*result = -1;
	BGSListForm* pListForm = NULL;
	TESForm* pForm = NULL;
		if (ExtractArgs(EXTRACT_ARGS, &pListForm, &pForm))
			return Hook_ListGetFormIndex_Eval(thisObj, pListForm, pForm, result);
	return true;
}
*/
bool Hook_IsInList_Execute(COMMAND_ARGS)
{
	*result = -1;
	BGSListForm* pListForm = NULL;

	if (ExtractArgs(EXTRACT_ARGS, &pListForm)) {
		TESForm* baseForm = thisObj->GetBaseObject();
		if (baseForm) {
			SInt32 index = pListForm->GetIndexOf(baseForm);
			*result = index;
			if (IsConsoleMode()) {
				Console_Print("Index: %d", index);
			}
		}
	}
	return true;
}

bool Hook_IsWeaponInList_Execute(COMMAND_ARGS)
{
	*result = -1;
	BGSListForm* pListForm = NULL;

	if (ExtractArgs(EXTRACT_ARGS, &pListForm)) {
		TESObjectWEAP* weap = ((Actor*)thisObj)->GetEquippedWeapon();
		TESForm* baseForm = weap->GetBaseObject();
		if (baseForm) {
			SInt32 index = pListForm->GetIndexOf(baseForm);
			*result = index;
			if (IsConsoleMode()) {
				Console_Print("Index: %d", index);
			}
		}
	}
	return true;
}

//Kit files....................................................................................

//JIP hooks........................................................................

bool Hook_GetSelfModIndex_Execute(COMMAND_ARGS)
{
	*result = scriptObj ? scriptObj->GetModIndexAlt() : 0xFF;
	return true;
}

bool Hook_GetFormMods_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = nullptr;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &form)) return true;
	if (!form && !thisObj) return true;
	if (!form) form = thisObj->baseForm; // Use thisObj's baseForm if form is null.

	if (!form) return true; // Final check for form validity.

	NVSEArrayVarInterface::Array* resultArr = g_arrInterface->CreateArray(nullptr, 0, scriptObj);
	for (ListNode<ModInfo>* iter = form->mods.Head(); iter != nullptr; iter = iter->next) {
		if (iter->data) {
			g_arrInterface->AppendElement(resultArr, ArrayElementL(iter->data->name));
		}
	}

	std::vector<UInt32> edits = form->GetFormEdits(true);
	for (UInt32 edit : edits) {
		g_arrInterface->AppendElement(resultArr, ArrayElementL(edit));
	}

	g_arrInterface->AssignCommandResult(resultArr, result);
	return true;
}

bool Hook_GetModName_Execute(COMMAND_ARGS)
{
	std::string modName;
	UInt32 index, keepExt = 0;
	if (ExtractArgsEx(EXTRACT_ARGS_EX, &index, &keepExt) && (index < 0xFF))
	{
		if (Kit::KitData* kit = Kit::GetKitDataByIndex(index)) {
			modName = kit->name;
			if (!keepExt) {
				size_t lastDot = modName.rfind('.');
				if (lastDot != std::string::npos) {
					modName = modName.substr(0, lastDot); // Remove file extension
				}
			}
		}
	}
	AssignString(PASS_COMMAND_ARGS, modName.c_str());
	return true;
}

//NVSE hooks.........................................................................

bool Hook_GetNthModName_Execute(COMMAND_ARGS)
{
	UInt32 modIdx = 0xFF;
	const char* modName = "";

	if (ExtractArgs(EXTRACT_ARGS, &modIdx)) {
		if (Kit::KitData* kit = Kit::GetKitDataByIndex(modIdx)) {
			modName = kit->name.c_str();
		}
	}

	AssignString(PASS_COMMAND_ARGS, modName);

	return true;
}

bool Hook_GetModIndex_Execute(COMMAND_ARGS)
{
	char modName[512];
	if (!ExtractArgs(EXTRACT_ARGS, &modName))
		return true;

	if (Kit::KitData* kit = Kit::GetKitDataByName(modName)) {
		*result = kit->index;
		if (IsConsoleMode())
			Console_Print("Mod Index: %02X", *result);
	}
	return true;
}

bool Hook_GetSourceModIndex_Execute(COMMAND_ARGS)
{
	TESForm* form = NULL;
	*result = -1;

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &form))
		return true;

	if (!form && thisObj) {
		form = thisObj;
		*result = (UInt8)(form->refID >> 24);
		return true;
	}
	else
	{
		*result = form->GetModIndexAlt();
	}

	return true;
}

bool Hook_IsModLoaded_Execute(COMMAND_ARGS)
{
	char modName[512];
	*result = 0;

	if (!ExtractArgs(EXTRACT_ARGS, &modName))
		return true;

	if (Kit::KitData* kit = Kit::GetKitDataByName(modName)) {
		*result = 1;
	}
	if (IsConsoleMode())
	{
		if (*result)
			Console_Print("Mod Loaded");
		else
			Console_Print("Mod not loaded");
	}

	return true;
}

bool Hook_GetNumLoadedMods_Execute(COMMAND_ARGS)
{
	*result = Kit::loadedKitFiles.size();
	if (IsConsoleMode()) {
		Console_Print("Mods Loaded: %.0f", *result);
	}
	return true;
}

//....................................................................................
