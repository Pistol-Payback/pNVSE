#pragma once
#include "WeaponSmith.h"

std::unordered_map<UInt32, StaticInstance_WEAP*> StaticInstance_WEAP::Linker;		//This links the baseform to its extra data.
std::unordered_map<UInt32, Instance_WEAP*> Instance_WEAP::Linker;					//This links cloned baseforms to its WeapInst that contains attachment data.

//std::unordered_map<UInt32, StaticInstance*> StaticInstance::Linker;					//This links the baseform to its extra data.
//std::unordered_map<UInt32, Instance*> Instance::Linker;								//This links cloned baseforms to its WeapInst that contains attachment data.

//std::vector<UInt32> Instance_WEAP::aCloneRecycle;						//Deleting the created clones would be better, but I'm not sure what issues deleting a baseform would cause.

EventHandler onInstanceReconstructEvent;
EventHandler onInstanceDeconstructEvent;

EventHandler onAttachWeapModEvent;
EventHandler onAttachWeapModReconstructEvent;

EventHandler onDetachWeapModEvent;
EventHandler onDetachWeapModDeconstructEvent;

UInt32 InstanceInterface::cloneCount = 0;

//std::vector<UInt32> aUsedClones;
//std::vector<UInt32> aClones;		//Deleting the created clones would be better, but I'm not sure what issues deleting a baseform would cause.

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

TESForm* TESForm::GetStaticParent() {

	if (Instance_WEAP::Linker.find(this->refID) != Instance_WEAP::Linker.end()) {
		//Console_Print("Is Static Parent");
		return Instance_WEAP::Linker[this->refID]->baseInstance->parent;
	}
}

bool TESForm::IsInstancedForm() {
	if (Instance_WEAP::Linker.find(this->refID) != Instance_WEAP::Linker.end()) {
			return true;
	}
}

UInt32 TESForm::GetInstanceID() {

	if (this->IsInstancedForm()) {
		return Instance_WEAP::Linker[this->refID]->InstID;
	}

	if (this->IsReference()) {
		Script* PistolGetModifierID = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolGetModifierID");
		ArrayElementL scriptReturn;
		g_scriptInterface->CallFunction(PistolGetModifierID, (TESObjectREFR*)this, nullptr, &scriptReturn, 0);
		return static_cast<UInt32>(scriptReturn.Number());
	}
	return 0;

}

bool TESForm::MarkAsStaticForm() {

	UInt32 refID = this->refID;
	if (StaticInstance_WEAP::Linker.find(refID) == StaticInstance_WEAP::Linker.end()) {

		new StaticInstance_WEAP(this);
		return true;
	}
	return false;
}

bool TESForm::IsStaticForm() {

	if (StaticInstance_WEAP::Linker.find(this->refID) != StaticInstance_WEAP::Linker.end()) {
		return true;
	}
	return false;
}

Instance_WEAP* TESForm::LookupInstanceByID(UInt32 InstID) {

	if (StaticInstance_WEAP::Linker.find(this->refID) != StaticInstance_WEAP::Linker.end() && InstID < StaticInstance_WEAP::Linker[this->refID]->aInstances.size()) {
		return StaticInstance_WEAP::Linker[this->refID]->aInstances[InstID];
	}
	else {
		return nullptr;
	}

}

StaticInstance_WEAP* TESForm::LookupStaticInstance() {

	if (StaticInstance_WEAP::Linker.find(this->refID) != StaticInstance_WEAP::Linker.end()) {
		return StaticInstance_WEAP::Linker[this->refID];
	}
	else {
		return nullptr;
	}

}

UInt32 TESForm::CreateInst(std::string key) {

	TESForm* form = this;
	TESObjectREFR* formRef = nullptr;
	if (form->IsReference()) {
		formRef = (TESObjectREFR*)this;
	}

	UInt32 cloneRefID = 0;

	if (formRef) {

		form = formRef->baseForm;

		cloneRefID = formRef->GetInstanceID();
		if (cloneRefID > 0) {
			return form->LookupInstanceByID(cloneRefID)->clone->refID;
		}

		if (!form->IsStaticForm()) {	//Make parent static, if it's not static already.
			form->MarkAsStaticForm();
		}

	}

	if (!form->IsStaticForm() || form->IsInstancedForm()) { //Do not create modifiers of other modifiers. 
		return 0;
	}

	//UInt32 modIndex = 13;

	Instance_WEAP* NewInst = Instance_WEAP::create(StaticInstance_WEAP::Linker[form->refID], key);
	cloneRefID = NewInst->clone->refID;

	return cloneRefID;

}

void Instance_WEAP::destroy() {
	if (this->baseInstance) {
		auto it = std::find(this->baseInstance->aInstances.begin(), this->baseInstance->aInstances.end(), this);
		if (it != this->baseInstance->aInstances.end()) {
			this->baseInstance->aInstances.erase(it);
		}
	}
	delete this;
}

Instance_WEAP* Instance_WEAP::create(StaticInstance_WEAP* staticForm, std::string key) {

	TESForm* clone = nullptr;
	UInt32 modIndex = 13;
	UInt32 cloneRefID = 0;

	if (!staticForm) {
		throw std::runtime_error("Instance constructor failed.");
	}

	//if (!Instance_WEAP::aCloneRecycle.empty()) {
		//cloneRefID = Instance_WEAP::aCloneRecycle.back();
		//clone = LookupFormByRefID(cloneRefID);
		///clone->CopyFromAlt(staticForm->parent);
		//Instance_WEAP::aCloneRecycle.pop_back();
	//}
	//if {
		clone = staticForm->parent->CloneForm(0);
		if (clone) {
			cloneRefID = GetNextFreeFormID(GetFirstFormIDForModIndex(modIndex));
			if (cloneRefID && cloneRefID >> 24 == modIndex) {
				clone->SetRefID(cloneRefID, true);
				std::string editorID = "Inst" + std::to_string(InstanceInterface::cloneCount);
				clone->SetEditorID(editorID.c_str());
			}
			else {
				throw std::runtime_error("Instance constructor failed.");
			}
		}
	//}
	if (clone) {
		Console_Print("Making new instance with: %s", key.c_str());
		return new Instance_WEAP(staticForm, clone, staticForm->aInstances.size(), key, staticForm->aBaseAttachments);
	}
	else {
		throw std::runtime_error("Instance constructor failed.");
	}
}

