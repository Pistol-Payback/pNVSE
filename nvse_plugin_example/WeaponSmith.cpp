#pragma once
#include "WeaponSmith.h"

std::vector<UInt32> aCloneRebuild; //For loading
std::unordered_map<UInt32, WeapInstBase*> BaseExtraData;		//This links the baseform to its extra data.
std::unordered_map<UInt32, WeapInst*> WeapInstList;			//This links cloned baseforms to its WeapInst that contains attachment data.

std::vector<UInt32> aUsedClones;
std::vector<UInt32> aClones;		//Deleting the created clones would be better, but I'm not sure what issues deleting a baseform would cause.

UInt32 GetFirstFormIDForModIndex(UInt32 modIndex) {
	// Check for valid mod index range (0x00 to 0xFF)
	if (modIndex > 0xFF) {
		//Console_Print("Invalid mod index %02X", modIndex);
		return 0;
	}

	// Calculate the first FormID based on the mod index
	UInt32 firstFormID = (modIndex << 24); // Shift the mod index by 24 bits to the left, leaving the index at the begining
	return firstFormID;

}

TESForm* TESForm::GetModifierParent() {

	if (WeapInstList.find(this->refID) != WeapInstList.end()) {
		return WeapInstList[this->refID]->Parent;
	}
}

bool TESForm::IsModifierForm() {
	if (WeapInstList.find(this->refID) != WeapInstList.end()) {
			return true;
	}
}

UInt32 TESForm::GetModifierID() {

	if (this->IsModifierForm()) {
		return WeapInstList[this->refID]->refID;
	}

	TESObjectREFR* ref = dynamic_cast<TESObjectREFR*>(this);
	if (ref) {
		Script* PistolGetModifierID = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolGetModifierID");
		ArrayElementL scriptReturn;
		g_scriptInterface->CallFunction(PistolGetModifierID, ref, nullptr, &scriptReturn, 0);
		return static_cast<UInt32>(scriptReturn.Number());
	}
	return 0;

}

bool TESForm::MarkAsStaticForm() {

	UInt32 refID = this->refID;
	if (BaseExtraData.find(refID) == BaseExtraData.end()) {
		BaseExtraData[refID] = new WeapInstBase;
		BaseExtraData[refID]->Parent = this;
		return true;
	}
	return false;
}

bool TESForm::IsStaticForm() {

	if (BaseExtraData.find(this->refID) != BaseExtraData.end()) {
		return true;
	}
	return false;
}

WeapInst* TESForm::LookupModifierByID(UInt32 InstID) {

	if (BaseExtraData.find(this->refID) != BaseExtraData.end() && InstID < BaseExtraData[refID]->aInstances.size()) {
		return BaseExtraData[refID]->aInstances[InstID];
	}
	else {
		return nullptr;
	}

}

UInt32 TESForm::CreateInst() {

	TESForm* form = this;
	TESObjectREFR* formRef = reinterpret_cast<TESObjectREFR*>(this);

	TESForm* clonedForm = nullptr;
	UInt32 cloneRefID = 0;

	if (formRef) {

		form = formRef->baseForm;

		cloneRefID = formRef->GetModifierID();
		if (cloneRefID > 0) {
			return form->LookupModifierByID(cloneRefID)->Clone->refID;
		}

		if (BaseExtraData.find(form->refID) == BaseExtraData.end()) {	//Make parent static, if it's not static already.
			form->MarkAsStaticForm();
		}

	}

	if (form == nullptr || form->IsModifierForm()) { //Do not create modifiers of other modifiers. 
		return 0;
	}

	UInt32 modIndex = 13;

	if (!aClones.empty()) {

		//Use an old unused clone sitting in memory

		cloneRefID = aClones.back();
		clonedForm = LookupFormByRefID(cloneRefID);
		clonedForm->CopyFrom(form);

		aClones.pop_back();

		aUsedClones.push_back(cloneRefID);
		WeapInst* NewInst = new WeapInst(
			form,
			BaseExtraData[form->refID]->aInstances.size(),  // refID can be used to determine the size
			clonedForm,
			BaseExtraData[form->refID]->aBaseAttachments    // Copy the map
		);

		BaseExtraData[form->refID]->aInstances.push_back(NewInst);
		WeapInstList[cloneRefID] = NewInst;

		aCloneRebuild.push_back(cloneRefID);

	}
	else {

		//Create new clone.

		clonedForm = form->CloneForm(0);
		if (clonedForm)
		{

			cloneRefID = GetFirstFormIDForModIndex(modIndex);

			if (cloneRefID) {
				if (cloneRefID >> 24 == modIndex)
				{
					clonedForm->SetRefID(cloneRefID, true);
					std::string editorID = "D" + std::to_string(aUsedClones.size()); // Convert number to string and append to the original string
					clonedForm->SetEditorID(editorID.c_str());

					aUsedClones.push_back(clonedForm->refID);

					WeapInst* NewInst = new WeapInst(
						form,
						BaseExtraData[form->refID]->aInstances.size(),  // refID can be used to determine the size
						clonedForm,
						BaseExtraData[form->refID]->aBaseAttachments    // Copy the map
					);

					BaseExtraData[form->refID]->aInstances.push_back(NewInst);
					WeapInstList[cloneRefID] = NewInst;

				}
				//else
				//{
					//Console_Print("ERROR Weapon Instance Failed to create. ESP/ESM is full");
				//}

			}

		}

	}

	return cloneRefID;

}
