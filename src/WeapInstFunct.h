#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <nvse/InventoryRef.h>

using UInt32List = std::list<UInt32>;
extern std::unordered_map<UInt32, UInt32List> s_WeaponInstanceIndex;
std::unordered_map<UInt32, UInt32List> s_WeaponInstanceIndex;

class WeapInst {
public:

	WeapInst() {

		iSize = 0;
		aAttachments = new int[iSize];

	}

	~WeapInst() {
		delete[] aAttachments;
	}

	UInt32 Baseform = 0;
	bool IsAkimbo = 0;
	bool IsEquipped = 0;

	float fHealth = 0.0;
	
	TESForm* Parent = 0;
	float fX = 0.0;
	float fY = 0.0;
	float fZ = 0.0;

	TESForm* refID = 0;

private:
	int* aAttachments;  // Pointer to the array
	int iSize; // Size of the array
};

void AddNewInst(UInt32 originalRefID, UInt32 clonedRefID) {
	// Check if the original form's reference ID exists in the map
	auto it = s_WeaponInstanceIndex.find(originalRefID);
	if (it != s_WeaponInstanceIndex.end()) {
		// If it exists, add the cloned reference ID to the list
		it->second.push_back(clonedRefID);
	}
	else {
		// If it doesn't exist, create a new list and add the cloned reference ID
		UInt32List newList;
		newList.push_back(clonedRefID);
		s_WeaponInstanceIndex[originalRefID] = newList;
	}
}

bool IsInst(UInt32 formID) {
	for (const auto& entry : s_WeaponInstanceIndex) {
		const UInt32List& clonedList = entry.second;
		for (UInt32 clonedID : clonedList) {
			if (clonedID == formID) {
				return true; // Found the formID within the nested cloned reference IDs
			}
		}
	}
	return false; // formID was not found within any nested cloned reference IDs
}

UInt32 GetInstParent(UInt32 formID) {
	for (const auto& entry : s_WeaponInstanceIndex) {
		const UInt32List& clonedList = entry.second;
		for (UInt32 clonedID : clonedList) {
			if (clonedID == formID) {
				return entry.first; // Return the parent ID (the key in the map)
			}
		}
	}
	return 0; // Return 0 if the parent ID is not found (assuming 0 is an invalid ID)
}

size_t CountInstForms(UInt32 refID) {
	size_t count = 0;
	for (const auto& entry : s_WeaponInstanceIndex) {
		const UInt32List& clonedList = entry.second;
		for (UInt32 clonedID : clonedList) {
			count++; // Increment the count when the refID is found in the cloned reference IDs
		}
	}
	return count;
}

std::string AssignInstNumber(const char* originalString, size_t number) {
	std::string result = originalString;
	result += "WI" + std::to_string(number); // Convert number to string and append to the original string
	return result; // Return the resulting string as a const char*
}

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

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponBase, GWB, "Returns the true baseform", false, kParams_OneOptionalForm);
bool Cmd_GetWeaponBase_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		form = form->TryGetREFRParent();			//Check if it's a baseform. If it's not, grab the baseform.
		if (!form) {
			if (!thisObj) return true;
			form = thisObj->baseForm;
			*refResult = form->refID;
		}

		if (IsInst(form->refID)) {
			*refResult = GetInstParent(form->refID);
		}

	}

	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(IsWeapInst, IsWeaponInstance, "Checks if a form is a Weapon Inst", false, kParams_OneOptionalForm);
bool Cmd_IsWeapInst_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESForm* form = NULL;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form)) {

		form = form->TryGetREFRParent();			//Check if it's a baseform. If it's not, grab the baseform.
		if (!form) {
			if (!thisObj) return true;
			form = thisObj->baseForm;
		}

		if (IsInst(form->refID)) {
			*result = 1;
		}

	}

	return true;
}

static ParamInfo kParamsNewWeapInst[2] =
{
	{"form", kParamType_AnyForm, 0},
	{"int", kParamType_Integer, 1}
};

DEFINE_COMMAND_ALT_PLUGIN(NewWeapInst, NewWeaponInstance, "Creates a new weapon instance aka new baseform for a weapon", false, kParamsNewWeapInst);
bool Cmd_NewWeapInst_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32* refResult = (UInt32*)result;
	TESForm* form = NULL;
	UInt32 modIndex = 0;
	if (ExtractArgsEx(EXTRACT_ARGS_EX, &form, &modIndex)) {

		if (!form) {
			if (!thisObj) return true;
			form = thisObj->baseForm;
		}

		TESForm* clonedForm = form->CloneForm(0);
		if (clonedForm)
		{
			UInt32 nextFormId;
			if (modIndex) {
				nextFormId = GetNextFreeFormID(GetFirstFormIDForModIndex(modIndex));
			}
			else {
				modIndex = scriptObj->GetModIndex();
				nextFormId = GetNextFreeFormID(modIndex);
			}
			if (nextFormId) {
				if (nextFormId >> 24 == modIndex)
				{
					clonedForm->SetRefID(nextFormId, true);
					//const char* editorId = form->GetEditorID();
					const char* editorId = form->GetName();
					std::string editorIdNew = AssignInstNumber(editorId, CountInstForms(form->refID));
					clonedForm->SetEditorID(editorIdNew.c_str());
					//WeapInst MyWeap = new WeapInst;
					AddNewInst(form->refID, nextFormId);

					*refResult = clonedForm->refID;
					if (IsConsoleMode())
						Console_Print("Created cloned form: %08x", *refResult);

				}
				else
				{
					Console_Print("ERROR Weapon Instance Failed to create. ESP/ESM is full");
				}
			}

		}

	}
	return true;

}