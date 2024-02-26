#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <InventoryRef.h>

//Weapon Instance Bases are all the different variations of attachments that can be put on a weapon.
class WeapInst {
public:

	WeapInst(
		UInt8 refID = 0,
		TESForm* Clone = nullptr,
		const std::unordered_map<std::string, UInt32>& aAttachments = std::unordered_map<std::string, UInt32>())
		: refID(refID),
		Clone(Clone),
		aAttachments(aAttachments) {
	}

	UInt8 refID;	//== the number of Weapon Instances for a specific baseform. Created via the ParentInstCounter.
	TESForm* Clone;							//Dynamic baseform
	std::unordered_map<std::string, UInt32> aAttachments;

};

class WeapInstBase {
public:

	WeapInstBase(
		TESForm* Parent = nullptr,
		const std::vector<WeapInst*>& aInstances = std::vector<WeapInst*>())
		: Parent(Parent),
		aInstances(aInstances) {
	}

	TESForm* Parent;						//The true baseform
	std::vector<WeapInst*> aInstances;		//Save dependent
	//bool IsAkimbo;

};

std::unordered_map<UInt32, WeapInstBase*> BaseExtraData;	//This links the baseform to its extra data.
std::unordered_map<UInt32, WeapInst*> WeapInstList;			//This links cloned baseforms to its WeapInst that contains attachment data.

std::vector<UInt32> aUsedClones;
std::vector<UInt32> aClones;		//Deleting the created clones would be better, but I'm not sure what issues deleting a baseform would cause.

//Write Instance data................................................................................................................................

void (*WriteRecord8)(UInt8 inData);
void (*WriteRecord32)(UInt32 inData);
bool (*WriteRecord)(UInt32 type, UInt32 version, const void* buf, UInt32 length);

UInt32 GetFirstFormIDForModIndex(UInt32 modIndex) {
	// Check for valid mod index range (0x00 to 0xFF)
	if (modIndex > 0xFF) {
		Console_Print("Invalid mod index %02X", modIndex);
		return 0;
	}

	// Calculate the first FormID based on the mod index
	UInt32 firstFormID = (modIndex << 24); // Shift the mod index by 24 bits to the left, leaving the index at the begining
	return firstFormID;

}

//Clear all existing data on load.

bool ClearAllWeapData() {

	//deletes every weapon instances that may have been created in other saves when loading a new save.

	auto it = BaseExtraData.begin();
	auto end = BaseExtraData.end();

	for (auto it = BaseExtraData.begin(); it != BaseExtraData.end(); ++it) {

		WeapInstBase* Base = it->second;
		for (auto& rInstance : Base->aInstances) {
			if (rInstance) {
				delete rInstance;
			}
		}
		Base->aInstances.clear(); //Restore instance data when loading.
	}

	aClones.clear();
	aClones = std::move(aUsedClones);
	aUsedClones.clear();

	for (auto& pair : WeapInstList) {

		delete pair.second; // Delete clone links to each WeapInst object

	}
	WeapInstList.clear();

	return true;
}

UInt32 CreateInst(TESForm* form, TESObjectREFR* formRef = nullptr);	// Forward declaration

bool RegisterWeapon(TESForm* form) {

	UInt32 refID = form->refID;
	if (BaseExtraData.find(refID) == BaseExtraData.end()) {
		BaseExtraData[refID] = new WeapInstBase;
		BaseExtraData[refID]->Parent = form;
		CreateInst(form);
		Console_Print("Registered Weapon");
		return true;
	}
	return false;
}

WeapInst* LookupByInstID(UInt32 refID, UInt32 InstID) {

	if (InstID < BaseExtraData[refID]->aInstances.size()) {
		Console_Print("Found baseform, and InstID: %d", InstID);
		return BaseExtraData[refID]->aInstances[InstID];
	}
	else {
		return nullptr;
	}


}

UInt32 CreateInst(TESForm* form, TESObjectREFR* formRef) {

	TESForm* clonedForm = nullptr;
	UInt32 cloneRefID = 0;

	if (formRef != nullptr) {

		form = formRef->baseForm;

		Script* PistolGetModifierID = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolGetModifierID");
		ArrayElementL idkWhatThisIs;
		g_scriptInterface->CallFunction(PistolGetModifierID, formRef, nullptr, &idkWhatThisIs, 0);
		cloneRefID = static_cast<UInt32>(idkWhatThisIs.Number());

		if (cloneRefID > 0) {
			Console_Print("Weapon already has a modifier");
			return LookupByInstID(form->refID, cloneRefID)->Clone->refID;
		}

	}

	if (form == nullptr) {
		Console_Print("ERROR Weapon Instance Failed to create. Form is a nullptr");
		return 0;
	}

	UInt32 modIndex = 13;

	if (BaseExtraData.find(form->refID) == BaseExtraData.end()) {
		RegisterWeapon(form);
	}

	if (!aClones.empty()) {

		cloneRefID = aClones.back();
		clonedForm = LookupFormByRefID(cloneRefID);
		aClones.pop_back();

		aUsedClones.push_back(clonedForm->refID);
		WeapInst* NewInst = new WeapInst();
			NewInst->refID = BaseExtraData[form->refID]->aInstances.size();
			NewInst->Clone = clonedForm;

		BaseExtraData[form->refID]->aInstances.push_back(NewInst);
		WeapInstList[cloneRefID] = NewInst;

	}
	else {

		//Create new clone.
		Console_Print("Creating new clone");
		clonedForm = form->CloneForm(0);
		if (clonedForm)
		{

			cloneRefID = GetNextFreeFormID(GetFirstFormIDForModIndex(modIndex));

			if (cloneRefID) {
				if (cloneRefID >> 24 == modIndex)
				{
					clonedForm->SetRefID(cloneRefID, true);
					std::string editorID = "WS" + std::to_string(aUsedClones.size()); // Convert number to string and append to the original string
					clonedForm->SetEditorID(editorID.c_str());

					aUsedClones.push_back(clonedForm->refID);
					WeapInst* NewInst = new WeapInst();
						NewInst->refID = BaseExtraData[form->refID]->aInstances.size();
						NewInst->Clone = clonedForm;

					BaseExtraData[form->refID]->aInstances.push_back(NewInst);
					WeapInstList[cloneRefID] = NewInst;

				}
				else
				{
					Console_Print("ERROR Weapon Instance Failed to create. ESP/ESM is full");
				}

			}

		}

	}

	return cloneRefID;

}

void SaveGameCallback(void*)
{
	UInt32 Totallength = BaseExtraData.size();
	WriteRecord32(Totallength);

	auto it = BaseExtraData.begin();
	auto end = BaseExtraData.end();
	const char* EditorID;

	for (auto it = BaseExtraData.begin(); it != BaseExtraData.end(); ++it) {

		WeapInstBase* Base = it->second;

		WriteRecord8(static_cast<uint8_t>(Base->aInstances.size()));	//Save the count of instances iterated over.
		for (auto& rInstance : Base->aInstances) {

			if (rInstance) {

				WriteRecord8(static_cast<uint8_t>(rInstance->aAttachments.size()));	//Save the count of attachment slots iterated over.

				for (const auto& rAttachment : rInstance->aAttachments) {
					//if (rAttachment) {
						//TESForm* attachment = LookupFormByRefID(rAttachment);
						//EditorID = attachment->GetEditorID();
						//size_t length = strlen(EditorID);
						//WriteRecord32(length);
						//WriteRecord(1, 2, EditorID, length);
					//}
				}

			}

		}

	}

}

//Read Instance data................................................................................................................................

UInt8(*ReadRecord8)();
UInt32(*ReadRecord32)();
bool	(*ResolveRefID)(UInt32 refID, UInt32* outRefID);
UInt32(*ReadRecordData)(void* buf, UInt32 length);

void LoadGameCallback(void*)
{

	//ClearAllWeapData();

	UInt8 breaker = 0;

	UInt32 Totallength = ReadRecord32();

	char* EditorID;
	TESForm* form = NULL;
	/*
	while (Totallength >= 0) {

		UInt32 cloneRefID = CreateInst(((TESForm * (__cdecl*)(char*))(0x483A00))(EditorID));	//Re-Create the instance
		WeapInst* rInstance = WeapInstList[cloneRefID];											//Lookup the WeapInstBase that was just created by CreateInst

		UInt8 i = ReadRecord8();		//Re-Apply attachments to WeapInst
		void* aAttachments;

		while (i >= 0) {

			UInt32 length = ReadRecord32();
			ReadRecordData(EditorID, length);
			form = ((TESForm * (__cdecl*)(char*))(0x483A00))(EditorID); //LookupEditorID
			
			rInstance->aAttachments.push_back(form->refID);

			i -= 1;

		}

		Totallength -= 1;

	}
	*/

}

//Initialize save/load events

void SaveWeaponInst(const NVSEInterface* nvse, PluginHandle& pluginHandle)
{

	NVSESerializationInterface* serialization = (NVSESerializationInterface*)nvse->QueryInterface(kInterface_Serialization);

	WriteRecord = serialization->WriteRecord;
	WriteRecord32 = serialization->WriteRecord32;
	WriteRecord8 = serialization->WriteRecord8;

	ReadRecordData = serialization->ReadRecordData;
	ReadRecord32 = serialization->ReadRecord32;
	ReadRecord8 = serialization->ReadRecord8;

	serialization->SetLoadCallback(pluginHandle, LoadGameCallback);
	serialization->SetSaveCallback(pluginHandle, SaveGameCallback);

}

//Functions...................................................................................................................
bool	(*CallFunctionAlt)(Script* funcScript, TESObjectREFR* callingObj, UInt8 numArgs, ...);

static ParamInfo kParamsSetWeapAttachments[2] =
{
	{"string", kParamType_String, 0},
	{"form", kParamType_AnyForm, 0}
};

static ParamInfo kParams_OneOptionalString[1] =
{
	{"string", kParamType_String, 1}
};

DEFINE_COMMAND_ALT_PLUGIN(RegWSWeapon, ExtWeaponData, "Creates a new weapon instance aka new baseform for a weapon", false, kParams_OneForm);
bool Cmd_RegWSWeapon_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		if (form) {
			RegisterWeapon(form);
		}

	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN(NewWeaponModifier, NewWeapModifier, "Creates a new weapon instance aka new baseform for a weapon", true, 0);
bool Cmd_NewWeaponModifier_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	UInt32 CloneRefID = 0;
	TESForm* form = nullptr;

		form = thisObj->baseForm;

		if (form) {

			CloneRefID = CreateInst(nullptr, thisObj);
			*refResult = CloneRefID;

			Script* PistolSetModifierID = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolSetModifierID");
			ArrayElementL idkWhatThisIs;
			g_scriptInterface->CallFunction(PistolSetModifierID, thisObj, nullptr, &idkWhatThisIs, 1, WeapInstList[CloneRefID]->refID);

			if (IsConsoleMode())
				Console_Print("Created cloned form: %08x", *refResult);

		}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN(SetWeaponAttachment, SetWeapMod, "Set a attachment in a slot", true, kParamsSetWeapAttachments);
bool Cmd_SetWeaponAttachment_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESForm* form = NULL;
	UInt32 InstID = 0;
	TESForm* rAttachment = NULL;
	char sSlot[MAX_PATH];

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sSlot, &rAttachment)) {

		Console_Print("Storing attachment via ppNVSE");
		form = thisObj->baseForm;

		Script* PistolGetModifierID = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolGetModifierID");
		ArrayElementL idkWhatThisIs;
		g_scriptInterface->CallFunction(PistolGetModifierID, thisObj, nullptr, &idkWhatThisIs, 0);
		InstID = static_cast<UInt32>(idkWhatThisIs.Number());

		if (BaseExtraData.find(form->refID) != BaseExtraData.end()) {	//If passed a baseform parent...................................

			auto* attachmentMap = &(LookupByInstID(form->refID, InstID)->aAttachments);

			//if (attachmentMap->find(sSlot) != attachmentMap->end()) {
				(*attachmentMap)[sSlot] = rAttachment->refID;
				Console_Print("Appended to attachment map");
				return true;
			//}
			//else {
				//Console_Print("SetWeaponAttachment Could not find slot inside attachmentMap");
			//}

		}
		else if (WeapInstList.find(form->refID) != WeapInstList.end()) { // If passed a modifier clone......................................

			WeapInst* xData = WeapInstList[form->refID];
			if (xData->aAttachments.find(sSlot) != xData->aAttachments.end()) {

				xData->aAttachments[sSlot] = rAttachment->refID;;
				Console_Print("Appended to attachment map via modifier");
			}

			return true;
		}
		else {
			Console_Print("Faild to attach to map");
		}

	}

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponAttachments, GetWeapMod, "Gets the attachment array", true, kParams_OneOptionalString);
bool Cmd_GetWeaponAttachments_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESForm* form = NULL;
	UInt32 InstID = 0;
	char sSlot[MAX_PATH] = "";

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sSlot)) {

		Script* PistolGetModifierID = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolGetModifierID");
		ArrayElementL idkWhatThisIs;
		g_scriptInterface->CallFunction(PistolGetModifierID, thisObj, nullptr, &idkWhatThisIs, 0);
		InstID = static_cast<UInt32>(idkWhatThisIs.Number());

		form = thisObj->baseForm;

		if (BaseExtraData.find(form->refID) != BaseExtraData.end()) {	//If passed a baseform parent...................................

			auto* attachmentMap = &(LookupByInstID(form->refID, InstID)->aAttachments);
			auto aAttachments = g_arrInterface->CreateArray(nullptr, 0, scriptObj);

			if (sSlot[0] == '\0') { // Return all attachments

				Console_Print("Slot is NULL, returning all attachments");
				for (const auto& pair : *attachmentMap) {
					UInt32 attachmentId = pair.second;
					ArrayElementR rElem = LookupFormByRefID(attachmentId);
					g_arrInterface->AppendElement(aAttachments, rElem);
				}

				g_arrInterface->AssignCommandResult(aAttachments, result);
				return true;
			}
			else if (attachmentMap->find(sSlot) != attachmentMap->end()) { // Return slot attachment

				Console_Print("Slot is not Null, returning 1 attachment");
				ArrayElementR rElem = LookupFormByRefID((*attachmentMap)[sSlot]);
				g_arrInterface->AppendElement(aAttachments, rElem);

			}
			else {
				Console_Print("Error could not find slot");
			}

			g_arrInterface->AssignCommandResult(aAttachments, result);

		}
		else if (WeapInstList.find(form->refID) != WeapInstList.end()) { // If passed a modifier clone......................................

			WeapInst* clone = WeapInstList[form->refID];
			if (clone->aAttachments.find(sSlot) != clone->aAttachments.end()) {
				*result = clone->aAttachments[sSlot];
			}
			return true;
		}

		//Free something with the ArrayElementR, idk I'll do it later.

	}

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponSlots, GetWeapSlots, "Gets the slot array for a ref", true, 0);
bool Cmd_GetWeaponSlots_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESForm* form = NULL;
	UInt32 InstID = 0;
	std::string sSlot = "NULL";

	Script* PistolGetModifierID = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolGetModifierID");
	ArrayElementL idkWhatThisIs;
	g_scriptInterface->CallFunction(PistolGetModifierID, thisObj, nullptr, &idkWhatThisIs, 0);
	InstID = static_cast<UInt32>(idkWhatThisIs.Number());

	form = thisObj->baseForm;
	auto aSlots = g_arrInterface->CreateArray(nullptr, 0, scriptObj);

	if (BaseExtraData.find(form->refID) != BaseExtraData.end()) {	//Find link data

		auto* attachmentMap = &(LookupByInstID(form->refID, InstID)->aAttachments);

		for (const auto& pair : *attachmentMap) {
			sSlot = pair.first;
			ArrayElementR rElem = sSlot.c_str();
			g_arrInterface->AppendElement(aSlots, rElem);
		}

	}

	g_arrInterface->AssignCommandResult(aSlots, result);

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponModifier, GetWeapModifier, "Gets the weapon instance when passed a ref", true, 0);
bool Cmd_GetWeaponModifier_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	//TESObjectREFR* formRef;
	UInt32 InstID = 0;
	UInt32* refResult = (UInt32*)result;

		Script* PistolGetModifierID = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolGetModifierID");
		ArrayElementR idkWhatThisIs;
		g_scriptInterface->CallFunction(PistolGetModifierID, thisObj, nullptr, &idkWhatThisIs, 0);
		InstID = static_cast<UInt32>(idkWhatThisIs.Number());

		form = thisObj->baseForm;

		if (form) {

			if (BaseExtraData.find(form->refID) != BaseExtraData.end()) {
				*refResult = LookupByInstID(form->refID, InstID)->Clone->refID;

			}
			else {
				Console_Print("No Inst for GetWeapInst");
				*result = -1;
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

		clone = clone->TryGetREFRParent();			//Check if it's a baseform. If it's not, grab the baseform.
		if (!clone) {
			if (!thisObj) return true;
			clone = thisObj->baseForm;
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

		if (BaseExtraData.find(form->refID) != BaseExtraData.end()) {
			*result = 1;
		}

	}

	return true;

}
