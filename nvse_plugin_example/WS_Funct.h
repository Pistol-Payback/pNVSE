#pragma once
#include "WeaponSmith.h"

static ParamInfo kParamsSetWeapAttachments[3] =
{
	{"form", kParamType_AnyForm, 0},
	{"string", kParamType_String, 0},
	{"form", kParamType_AnyForm, 1}
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

static ParamInfo kParams_SetOnAttachWeaponMod[3] =
{
	{"form", kParamType_AnyForm, 0},
	{"int", kParamType_Integer, 1},
	{"int", kParamType_Integer, 1},
};


DEFINE_COMMAND_ALT_PLUGIN(ReplaceItemInInventory, ReplaceItem, "Replaces an inventory object", true, kParams_ReplaceItem);
bool Cmd_ReplaceItemInInventory_Execute(COMMAND_ARGS)
{
	*result = 0;
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

DEFINE_COMMAND_PLUGIN(MarkAsStaticForm, "Creates a new weapon instance aka new baseform for a weapon", false, kParams_OneForm);
bool Cmd_MarkAsStaticForm_Execute(COMMAND_ARGS)
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

static ParamInfo kParams_CreateWeaponInstance[4] =
{
	{	"form",	kParamType_AnyForm,	0	},
	{	"string",	kParamType_String,	0	},
	{	"form",	kParamType_AnyForm,	1	},
	{	"form",	kParamType_AnyForm,	1	}
};

DEFINE_COMMAND_ALT_PLUGIN(CreateWeaponInstance, CreateWeapInst, "Creates a new weapon instance aka new baseform for a weapon", false, kParams_CreateWeaponInstance);
bool Cmd_CreateWeaponInstance_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	UInt32 CloneRefID = 0;

	Script* reconstruct = nullptr;
	Script* deconstruct = nullptr;

	TESForm* form = NULL;
	char key[0x50];

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &key, &reconstruct, &deconstruct)) {

			if (key[0] != '\0') {

				CloneRefID = form->CreateInst(key);
				*refResult = CloneRefID;

				std::vector<void*> filter = Event::EvaluateEventArg(1, '1', &key);

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

DEFINE_COMMAND_ALT_PLUGIN(SetWeaponMod, AttachWeaponMod, "Set a attachment in a slot", false, kParamsSetWeapAttachments);
bool Cmd_SetWeaponMod_Execute(COMMAND_ARGS) {
	*result = 0;
	TESForm* form = NULL;
	UInt32 InstID = 0;
	TESForm* rAttachment = NULL;
	TESForm* parent = nullptr; //Used for event handler
	char sSlot[MAX_PATH];

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &form, &sSlot, &rAttachment)) {
		return true;
	}

	TESForm* baseForm = form->IsReference() ? ((TESObjectREFR*)form)->baseForm : form;

	std::unordered_map<std::string, UInt32>* attachments = nullptr;

	if (baseForm->IsStaticForm()) { // If passed a baseform parent
		attachments = &StaticInstance_WEAP::Linker[baseForm->refID]->aBaseAttachments;
		parent = baseForm;
	}
	else if (baseForm->IsInstancedForm()) { // If passed a modifier clone
		attachments = &Instance_WEAP::Linker[baseForm->refID]->aAttachments;
		parent = baseForm->GetStaticParent();
	}
	else if (form->IsReference()) {

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

	if (!attachments) {
		return true;
	}

	auto it = attachments->find(sSlot);
	bool attachmentExists = it != attachments->end();

	if (rAttachment) {
		// Attach
		if (attachmentExists) {
			onDetachWeapModEvent.DispatchEvent(LookupFormByRefID(it->second), parent);
		}
		onAttachWeapModEvent.DispatchEvent(rAttachment, parent);
		(*attachments)[sSlot] = rAttachment->refID;
	}
	else {
		// Detach
		if (attachmentExists) {
			onDetachWeapModEvent.DispatchEvent(LookupFormByRefID(it->second), parent);
			attachments->erase(it);
		}
	}

	return true;
}

DEFINE_COMMAND_PLUGIN(GetAllEquippedWeaponMods, "Gets the attachment array", false, kParams_OneForm);
bool Cmd_GetAllEquippedWeaponMods_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* ref = NULL;

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &ref)) {
		return true;
	}

	if (!ref)
		return true;

	TESForm* baseForm = ref->IsBaseForm() ? ref : ((TESObjectREFR*)ref)->baseForm;

	std::unordered_map<std::string, UInt32>* attachmentMap = nullptr;
	if (baseForm->IsStaticForm()) { // If passed a baseform parent
		attachmentMap = &(StaticInstance_WEAP::Linker[baseForm->refID]->aBaseAttachments);
	}
	else if (baseForm->IsInstancedForm()) { // If passed a modifier clone
		attachmentMap = &(Instance_WEAP::Linker[baseForm->refID]->aAttachments);
	}
	else if (!ref->IsBaseForm()) {	// Vanilla attachments

		UInt8 iSlot = 1;
		UInt8 iFlags = static_cast<TESObjectREFR*>(ref)->GetWeaponModFlags();
		auto aAttachments = g_arrInterface->CreateStringMap(nullptr, nullptr, 0, scriptObj);

		while (iSlot <= 3) {
			if (iFlags & iSlot) {
				TESObjectIMOD* pItemMod = static_cast<TESObjectWEAP*>(baseForm)->GetItemMod(iSlot);
				if (pItemMod) {
					std::string key = std::to_string(iSlot);
					g_arrInterface->SetElement(aAttachments, ArrayElementL(key.c_str()), ArrayElementL(pItemMod));
				}
			}
			iSlot++;
		}

		g_arrInterface->AssignCommandResult(aAttachments, result);
		return true;
	}

	if (!attachmentMap)
		return true;

	auto aAttachments = g_arrInterface->CreateStringMap(nullptr, nullptr, 0, scriptObj);

	for (const auto& pair : *attachmentMap) {
		g_arrInterface->SetElement(aAttachments, ArrayElementL(pair.first.c_str()), ArrayElementL(LookupFormByRefID(pair.second)));
	}

	g_arrInterface->AssignCommandResult(aAttachments, result);
	return true;
}

DEFINE_COMMAND_PLUGIN(GetWeaponMod, "Gets the mod in a specific slot", false, kParams_OneFormOneOptionalString);
bool Cmd_GetWeaponMod_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* ref = NULL;
	UInt32* refResult = (UInt32*)result;
	char sSlot[MAX_PATH] = "";

	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &ref, &sSlot)) {
		return true;
	}

	if (!ref)
		return true;

	TESForm* baseForm = ref->IsBaseForm() ? ref : ((TESObjectREFR*)ref)->baseForm;

	// If passed a baseform parent
	if (baseForm->IsStaticForm()) {
		auto it = StaticInstance_WEAP::Linker[baseForm->refID]->aBaseAttachments.find(sSlot);
		if (it != StaticInstance_WEAP::Linker[baseForm->refID]->aBaseAttachments.end()) {
			*refResult = it->second;
			return true;
		}
	}
	// If passed a modifier clone
	else if (baseForm->IsInstancedForm()) {
		auto it = Instance_WEAP::Linker[baseForm->refID]->aAttachments.find(sSlot);
		if (it != Instance_WEAP::Linker[baseForm->refID]->aAttachments.end()) {
			*refResult = it->second;
			return true;
		}
	}
	else {	// If passed vanilla

		int iSlot;
		try {
			iSlot = std::stoi(sSlot);
		}
		catch (...) {
			return true;
		}

		if (iSlot < 1 || iSlot > 3) {
			return true;
		}

		if (!ref->IsBaseForm()) {
			UInt8 weaponModFlags = static_cast<TESObjectREFR*>(ref)->GetWeaponModFlags();
			if (weaponModFlags & (1 << (iSlot - 1))) {
				TESObjectIMOD* pItemMod = static_cast<TESObjectWEAP*>(baseForm)->GetItemMod(iSlot);
				if (pItemMod) {
					*refResult = pItemMod->refID;
					return true;
				}
			}
		}

	}

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(IsWeaponInstance, IsInstance, "Gets the weapon instance when passed a ref", false, kParams_OneForm);
bool Cmd_IsWeaponInstance_Execute(COMMAND_ARGS)
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

		if (form && form->IsInstancedForm()) {

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

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponInstanceID, GetWeapInstID, "Gets the ID for a Modifier when passed its clone", false, kParams_OneForm);
bool Cmd_GetWeaponInstanceID_Execute(COMMAND_ARGS)
{
	*result = -1;
	TESForm* clone = NULL;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &clone)) {

		if (clone->IsReference()) {
			clone = ((TESObjectREFR*)clone)->baseForm;
		}

		if (clone->IsInstancedForm()) {
			*result = Instance_WEAP::Linker[clone->refID]->InstID;
		}

	}

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetAllWeaponInstances, GetAllWeapInsts, "Gets Instances for a static form.", false, kParams_OneForm_OneOptionalInt);
bool Cmd_GetAllWeaponInstances_Execute(COMMAND_ARGS)
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
				Instance_WEAP* weap = *it;
				ArrayElementL rElem = LookupFormByRefID(weap->clone->refID);
				g_arrInterface->AppendElement(aResult, rElem);
			}

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

	if (priority != 0)
	{
		std::vector<void*> filter;
		if (key[0] != '\0') {
			filter = Event::EvaluateEventArg(1, '1', &key);
		}
		else {
			filter = Event::EvaluateEventArg(1, '1', nullptr);
		}

		Event eEvent(priority, script, filter);
		onInstanceReconstructEvent.AddEvent(eEvent);
	}
	else
	{
		onInstanceReconstructEvent.RemoveEvent(script);
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

	if (priority != 0)
	{
		std::vector<void*> filter;
		if (key[0] != '\0') {
			filter = Event::EvaluateEventArg(1, '1', &key);
		}
		else {
			filter = Event::EvaluateEventArg(1, '1', nullptr);
		}

		Event eEvent(priority, script, filter);
		onInstanceDeconstructEvent.AddEvent(eEvent);
	}
	else
	{
		onInstanceDeconstructEvent.RemoveEvent(script);
	}

	return true;
}

DEFINE_COMMAND_PLUGIN(SetOnAttachWeaponMod, "Dispatch when a weapon mod is attached", false, kParams_Event_OneForm_TwoFormsF);
bool Cmd_SetOnAttachWeaponMod_Execute(COMMAND_ARGS)
{
	SInt32 priority = 1;
	Script* script = nullptr;

	TESForm* arg1 = nullptr;	//Attachment
	TESForm* arg2 = nullptr;	//Baseform
	char order1 = '0';
	char order2 = '0';

	bool runOnReconstruct = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &priority, &script, &runOnReconstruct, &order1, &arg1, &order2, &arg2))
	{

		if (priority != 0)
		{

			std::vector<void*> filter = Event::EvaluateEventArg(2, order1, arg1, order2, arg2);

			Event eEvent(priority, script, filter);
			onAttachWeapModEvent.AddEvent(eEvent);
			if (runOnReconstruct) {
				Event eEvent(priority, script, filter);
				onAttachWeapModReconstructEvent.AddEvent(eEvent);
			}
		}
		else
		{
			onAttachWeapModEvent.RemoveEvent(script);
			onAttachWeapModReconstructEvent.RemoveEvent(script);
		}

	}

	return true;
}

DEFINE_COMMAND_PLUGIN(SetOnDetachWeaponMod, "When instances is loaded in", false, kParams_Event_OneForm_TwoFormsF);
bool Cmd_SetOnDetachWeaponMod_Execute(COMMAND_ARGS)
{
	SInt32 priority;
	Script* script;

	TESForm* arg1 = nullptr;
	TESForm* arg2 = nullptr;
	char order1;
	char order2;

	bool runOnDeconstruct = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &priority, &script, &runOnDeconstruct, &order1, &arg1, &order2, &arg2))
	{

		if (priority != 0)
		{

			std::vector<void*> filter = Event::EvaluateEventArg(2, order1, arg1, order2, arg2);

			Event eEvent(priority, script, filter);
			onDetachWeapModEvent.AddEvent(eEvent);
			if (runOnDeconstruct) {
				Event eEvent(priority, script, filter);
				onDetachWeapModDeconstructEvent.AddEvent(eEvent);
			}

		}
		else
		{
			onDetachWeapModEvent.RemoveEvent(script);
			onDetachWeapModDeconstructEvent.RemoveEvent(script);
		}

	}

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponInstanceKey, GetWeapInstKey, "Gets the true baseform of a weapon", false, kParams_OneForm);
bool Cmd_GetWeaponInstanceKey_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	const char* sResult = nullptr;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form && form->IsInstancedForm()) {

			sResult = Instance_WEAP::Linker[form->refID]->key.c_str();

		}

		AssignString(PASS_COMMAND_ARGS, sResult);

	}
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponInstance, GetWeapInst, "Gets the true baseform of a weapon", false, kParams_OneForm_OneInt);
bool Cmd_GetWeaponInstance_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	UInt32* refResult = (UInt32*)result;

	UInt32 instID = 0;
	Instance_WEAP* weap;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &instID)) {

		if (!form->IsBaseForm()) {
			form = ((TESObjectREFR*)form)->baseForm;
		}

		if (form && form->IsStaticForm()) {

			weap = form->LookupInstanceByID(instID);

		}

		*refResult = weap->clone->refID;

	}
	return true;
}