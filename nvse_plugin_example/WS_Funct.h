#pragma once
//#include <InventoryRef.h>
//#include "SafeWrite.h"
//#include "WeaponSmith.h"
/*
static ParamInfo kParamsSetWeapAttachments[3] =
{
	{"form", kParamType_AnyForm, 0},
	{"string", kParamType_String, 0},
	{"form", kParamType_AnyForm, 0}
};

static ParamInfo kParamsSetWeapBaseAttachments[3] =
{
	{"form", kParamType_AnyForm, 0},
	{"string", kParamType_String, 0},
	{"form", kParamType_AnyForm, 0}
};

static ParamInfo kParams_OneFormOneOptionalString[2] =
{
	{"form", kParamType_AnyForm, 0},
	{"string", kParamType_String, 1}
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
	TESForm* form = NULL;
	UInt32 copy = 0;
	UInt32 count = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &copy, &count)) {

		InventoryRef* invRef = InventoryRefGetForID(thisObj->refID);
		if (!invRef || (form->typeID != invRef->data.type->typeID)) {
			return true;
		}

		SInt32 maxCount = invRef->GetCount();
		if (count == 0) {
			count = maxCount;
		}
		else {
			count = min(count, maxCount);
		}

		((Actor*)invRef->containerRef)->ReplaceInvObject(form, invRef, count, copy);

	}
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(RegWSWeapon, ExtWeaponData, "Creates a new weapon instance aka new baseform for a weapon", false, kParams_OneForm);
bool Cmd_RegWSWeapon_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (form) {
			form->MarkAsStaticForm();
		}

	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN(NewWeaponModifier, NewWeapModifier, "Creates a new weapon instance aka new baseform for a weapon", false, kParams_OneForm);
bool Cmd_NewWeaponModifier_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	UInt32 CloneRefID = 0;
	TESForm* form = NULL;
	TESObjectREFR* formRef = NULL;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (form->IsReference()) {	//Check if form is a ref.

			formRef = static_cast<TESObjectREFR*>(form);
			form = formRef->baseForm;

			if (form && formRef) {

				CloneRefID = formRef->CreateInst();
				*refResult = CloneRefID;

				Script* PistolSetModifierID = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolSetModifierID");
				ArrayElementL idkWhatThisIs;
				g_scriptInterface->CallFunction(PistolSetModifierID, formRef, nullptr, &idkWhatThisIs, 1, WeapInstList[CloneRefID]->refID);

				Console_Print("Created cloned form");

			}
		}
		else if (BaseExtraData[form->refID]->aInstances.empty()){

			Console_Print("Created cloned 0");
			CloneRefID = form->CreateInst();
			*refResult = CloneRefID;

		}

	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN(SetWeaponAttachment, SetWeapMod, "Set a attachment in a slot", false, kParamsSetWeapAttachments);
bool Cmd_SetWeaponAttachment_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESForm* form = NULL;
	UInt32 InstID = 0;
	TESForm* rAttachment = NULL;
	char sSlot[MAX_PATH];

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &sSlot, &rAttachment)) {

		Console_Print("Storing attachment via ppNVSE");

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form->IsStaticForm()) {	//If passed a baseform parent...................................

			InstID = form->GetModifierID();
			auto* attachmentMap = &(form->LookupModifierByID(InstID)->aAttachments);


			(*attachmentMap)[sSlot] = rAttachment->refID;
			Console_Print("Appended to attachment map");
			return true;

		}
		else if (form->IsModifierForm()) { // If passed a modifier clone......................................

			WeapInst* xData = WeapInstList[form->refID];

			xData->aAttachments[sSlot] = rAttachment->refID;;
			Console_Print("Appended to attachment map via modifier");

			return true;
		}
		else {
			Console_Print("Faild to attach to map");
		}

	}

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponAttachments, GetWeapMod, "Gets the attachment array", false, kParams_OneFormOneOptionalString);
bool Cmd_GetWeaponAttachments_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESForm* form = NULL;
	UInt32 InstID = 0;
	char sSlot[MAX_PATH] = "";

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &sSlot)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		auto aAttachments = g_arrInterface->CreateArray(nullptr, 0, scriptObj);
		
		if (form->IsStaticForm()) {	//If passed a baseform parent...................................

			InstID = form->GetModifierID();

			auto* attachmentMap = &(form->LookupModifierByID(InstID)->aAttachments);

			if (sSlot[0] == '\0') { // Return all attachments

				for (const auto& pair : *attachmentMap) {
					UInt32 attachmentId = pair.second;
					ArrayElementR rElem = LookupFormByRefID(attachmentId);
					g_arrInterface->AppendElement(aAttachments, rElem);
				}

				g_arrInterface->AssignCommandResult(aAttachments, result);
			}
			else if (attachmentMap->find(sSlot) != attachmentMap->end()) { // Return slot attachment

				ArrayElementR rElem = LookupFormByRefID((*attachmentMap)[sSlot]);
				g_arrInterface->AppendElement(aAttachments, rElem);

			}
			else {
				Console_Print("Error could not find slot");
			}

		}
		else if (form->IsModifierForm()) { // If passed a modifier clone......................................

			auto* attachmentMap = &WeapInstList[form->refID]->aAttachments;

			if (sSlot[0] == '\0') { // Return all attachments
				for (const auto& pair : *attachmentMap) {
					UInt32 attachmentId = pair.second;
					ArrayElementR rElem = LookupFormByRefID(attachmentId);
					g_arrInterface->AppendElement(aAttachments, rElem);
				}
			}
			else {
				if (attachmentMap->find(sSlot) != attachmentMap->end()) {
					ArrayElementR rElem = LookupFormByRefID((*attachmentMap)[sSlot]);
					g_arrInterface->AppendElement(aAttachments, rElem);
				}
			}
		}

		g_arrInterface->AssignCommandResult(aAttachments, result);

		//Free something with the ArrayElementR, idk I'll do it later.

	}

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponSlots, GetWeapSlots, "Gets the slot array for a ref", false, kParams_OneForm);
bool Cmd_GetWeaponSlots_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESForm* form = NULL;
	UInt32 InstID = 0;
	std::string sSlot = "NULL";


	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		auto aSlots = g_arrInterface->CreateArray(nullptr, 0, scriptObj);

		if (form->IsModifierForm()) {

			auto* attachmentMap = &WeapInstList[form->refID]->aAttachments;

			for (const auto& pair : *attachmentMap) {
				sSlot = pair.first;
				ArrayElementL rElem = sSlot.c_str();
				g_arrInterface->AppendElement(aSlots, rElem);
			}

			g_arrInterface->AssignCommandResult(aSlots, result);

		}
		else {

			InstID = form->GetModifierID();

			if (BaseExtraData.find(form->refID) != BaseExtraData.end()) {	//Find link data

				auto* attachmentMap = &(form->LookupModifierByID(InstID)->aAttachments);

				for (const auto& pair : *attachmentMap) {
					sSlot = pair.first;
					ArrayElementL rElem = sSlot.c_str();
					g_arrInterface->AppendElement(aSlots, rElem);
				}

			}

			g_arrInterface->AssignCommandResult(aSlots, result);

		}
	}
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(IsWeaponModifier, IsModifier, "Gets the weapon instance when passed a ref", false, kParams_OneForm);
bool Cmd_IsWeaponModifier_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form) {

			*result = form->IsModifierForm();

		}

	}
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetBaseWeapon, GBW, "Gets the true baseform of a weapon", false, kParams_OneForm);
bool Cmd_GetBaseWeapon_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	UInt32* refResult = (UInt32*)result;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form && form->IsModifierForm()) {

			form = form->GetModifierParent();
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

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponModifier, GetWeapModifier, "Gets the weapon instance when passed a ref", false, kParams_OneForm);
bool Cmd_GetWeaponModifier_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	UInt32 InstID = 0;
	UInt32* refResult = (UInt32*)result;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (!form->IsBaseForm()) {
			InstID = form->GetModifierID();
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form) {
			form = form->LookupModifierByID(InstID)->Clone;
			if (form)
				*refResult = form->refID;
		}

	}
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponModifierID, GetWeapModifierID, "Gets the ID for a Modifier when passed its clone", false, kParams_OneForm);
bool Cmd_GetWeaponModifierID_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* clone = NULL;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &clone)) {

		if (!clone->IsBaseForm()) {
			clone = ((TESObjectREFR*)clone)->baseForm;
		}

		if (WeapInstList.find(clone->refID) != WeapInstList.end()) {
			*result = WeapInstList[clone->refID]->refID;
		}
		else {
			*result = -1;
		}

	}

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(HasWeaponModifier, HasRefModifier, "Checks if a ref is registered.", true, 0);
bool Cmd_HasWeaponModifier_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;

	form = thisObj->baseForm;

	if (form) {

		if (form->IsStaticForm()) {
			*result = 1;
		}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN(SetWeaponBaseAttachment, "Set a attachment in a base slot", false, kParamsSetWeapBaseAttachments);
bool Cmd_SetWeaponBaseAttachment_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESForm* rAttachment = NULL;
	char sSlot[MAX_PATH];
	TESForm* form = NULL;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &sSlot, &rAttachment)) {
		Console_Print("Storing attachment via ppNVSE");

		if (form && BaseExtraData.find(form->refID) != BaseExtraData.end()) { // If passed a baseform parent...

			BaseExtraData[form->refID]->aBaseAttachments[sSlot] = rAttachment->refID;
			Console_Print("Updating base attachments %s", sSlot);

			return true;
		}
		else {
			Console_Print("Weapon is not registered with WS to attach to map");
		}
	}

	return true;
}
*/