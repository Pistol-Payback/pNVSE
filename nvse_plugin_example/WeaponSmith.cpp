#pragma once
#include "WeaponSmith.h"

std::unordered_map<UInt32, std::unordered_map<UInt32, StaticInstance*>> StaticLinker;		//This links the baseform to its extra data.
std::unordered_map<UInt32, std::unordered_map<UInt32, Instance*>> InstanceLinker;					//This links cloned baseforms to its WeapInst that contains attachment data.

std::unordered_set<UInt32> DynamicallyCreatedForms;

//std::unordered_map<UInt32, StaticInstance*> StaticLinker;					//This links the baseform to its extra data.
//std::unordered_map<UInt32, Instance*> InstanceLinker;								//This links cloned baseforms to its WeapInst that contains attachment data.

//std::vector<UInt32> Instance_WEAP::aCloneRecycle;						//Deleting the created clones would be better, but I'm not sure what issues deleting a baseform would cause.

UInt32 InstanceInterface::cloneCount = 0;

//std::vector<std::vector<WeaponArgValue>> WeaponArgValue::NVSEArrayVector;

//std::vector<UInt32> aUsedClones;
//std::vector<UInt32> aClones;		//Deleting the created clones would be better, but I'm not sure what issues deleting a baseform would cause.

TESForm* TESForm::GetStaticParent() const {

	if (InstanceLinker[this->typeID].find(this->refID) != InstanceLinker[this->typeID].end()) {
		return InstanceLinker[this->typeID][this->refID]->baseInstance->parent;
	}
	return nullptr;
}

bool TESForm::IsInstancedForm() const {
	return InstanceLinker[this->typeID].find(this->refID) != InstanceLinker[this->typeID].end();
}

UInt32 TESForm::GetInstanceID() const {

	if (this->IsInstancedForm()) {
		return InstanceLinker[this->typeID][this->refID]->InstID;
	}

	return 0;

}

bool TESForm::IsStaticForm() const {
	return StaticLinker[typeID].find(refID) != StaticLinker[typeID].end();
}

bool TESForm::HasExtendedMods() const {

	const StaticInstance_WEAP* staticInst = nullptr;

	if (this->IsStaticForm()) {
		staticInst = static_cast<StaticInstance_WEAP*>(StaticLinker[40][this->refID]);
	}
	else if (this->IsInstancedForm()) {
		staticInst = static_cast<StaticInstance_WEAP*>(InstanceLinker[40][this->refID]->baseInstance);
	}

	return staticInst && !staticInst->aAllAttachments.empty();

}

bool TESForm::pIsDynamicForm() const {
	return DynamicallyCreatedForms.find(refID) != DynamicallyCreatedForms.end();
}

Instance* TESForm::pLookupInstance() const {
	auto iter = InstanceLinker[typeID].find(refID);
	if (iter != InstanceLinker[typeID].end()) {
		return iter->second;
	}
	return nullptr;
}

StaticInstance* TESForm::LookupStaticInstance() const {
	auto iter = StaticLinker[typeID].find(refID);
	if (iter != StaticLinker[typeID].end()) {
		return iter->second;
	}
	return nullptr;
}

Instance* TESForm::LookupInstanceByID(UInt32 InstID) const {

	auto it = StaticLinker[typeID].find(refID);
	if (it != StaticLinker[typeID].end()) {
		StaticInstance* staticInstance = (StaticInstance*)(it->second);
		if (staticInstance && InstID < staticInstance->aInstances.size()) {
			return staticInstance->aInstances[InstID];
		}
	}
	return nullptr;

}

StaticInstance* TESForm::MarkAsStaticForm(UInt32 kitIndex) {

	if (!this->IsInstancedForm() && StaticLinker[this->typeID].find(this->refID) == StaticLinker[this->typeID].end()) {
		switch (this->typeID) {
		case 40:
			return new StaticInstance_WEAP(this, kitIndex);
			break;
		default:
			return new StaticInstance(this, kitIndex);
			break;
		}
	}
	return nullptr;
}

TESForm* TESForm::CreateInst(std::string key) {

	TESForm* baseForm = this->IsReference() ? ((TESObjectREFR*)this)->baseForm : this;
	StaticInstance* staticInstance = nullptr;

	if (baseForm->IsStaticForm()) {
		staticInstance = StaticLinker[baseForm->typeID][baseForm->refID];
	}
	else if (baseForm->IsInstancedForm()) {
		Instance* instanceWEAP = InstanceLinker[baseForm->typeID][baseForm->refID];
		staticInstance = instanceWEAP->baseInstance;
	}

	if (!staticInstance) { //Do not create modifiers of other modifiers. 
		return nullptr;
	}

	Instance* NewInst = staticInstance->create(key);

	return NewInst->clone;

}


//..............................................................................................................................................................

void Instance::destroy() {
	if (this->baseInstance) {
		auto it = std::find(this->baseInstance->aInstances.begin(), this->baseInstance->aInstances.end(), this);
		if (it != this->baseInstance->aInstances.end()) {
			this->baseInstance->aInstances.erase(it);
		}
	}
	delete this;
}

TESForm* createInstance(StaticInstance* staticForm, std::string key) {

	TESForm* clone = nullptr;
	UInt32 cloneRefID = 0;

	if (!staticForm) {
		throw std::runtime_error("Instance constructor failed.");
	}

	clone = staticForm->parent->CloneForm(0);
	if (clone) {

		cloneRefID = GetNextFreeFormID(GetFirstFormIDForModIndex(0));

		clone->SetRefID(cloneRefID, true);
		std::string editorID = "Inst" + std::to_string(InstanceInterface::cloneCount);
		clone->SetEditorID(editorID.c_str());

	}

	if (clone) {
		Console_Print("Making new instance with: %s", key.c_str());
		return clone;
	}
	else {
		throw std::runtime_error("Instance constructor failed.");
	}

}


Instance* StaticInstance::create(std::string key) {

	TESForm* clone = createInstance(this, key);
	return new Instance(this, clone, key);

}

Instance* StaticInstance_WEAP::create(std::string key) {

	TESForm* clone = createInstance(this, key);
	return new Instance_WEAP(this, clone, key, this->aBaseAttachments);

}

//..........................................................................................................................................

AuxVector* StaticInstance::GetLinkedTrait(const std::string& trait, StaticInstance* linkedObj, const std::string& sSlot) {

	if (!linkedObj) {
		return nullptr;
	}

	if (linkedTraits.empty()) {
		return nullptr;
	}

	auto slotIter = linkedTraits.find(sSlot);
	if (slotIter == linkedTraits.end()) {
		return nullptr;
	}

	auto& modOverwrites = slotIter->second;

	auto modsIter = modOverwrites.find(linkedObj);
	if (modsIter == modOverwrites.end()) {
		return nullptr;
	}

	auto& functionOverwrites = modsIter->second;

	auto functionsIter = functionOverwrites.find(trait);
	if (functionsIter == functionOverwrites.end()) {
		return nullptr;
	}

	return &functionsIter->second;

};

AuxVector* StaticInstance::SetLinkedTrait(const std::string& trait, StaticInstance* linkedObj, const std::string& sSlot) {

	if (!linkedObj) {
		return nullptr;
	}

	auto slotIter = linkedTraits.find(sSlot);
	if (slotIter == linkedTraits.end()) {
		return &linkedTraits[sSlot][linkedObj][trait];
	}

	auto& modOverwrites = slotIter->second;

	auto modsIter = modOverwrites.find(linkedObj);
	if (modsIter == modOverwrites.end()) {
		return &modOverwrites[linkedObj][trait];
	}

	auto& functionOverwrites = modsIter->second;

	auto functionsIter = functionOverwrites.find(trait);
	if (functionsIter == functionOverwrites.end()) {
		return &functionOverwrites[trait];
	}

	return &functionsIter->second;

};

//................................................................................................................................................................

AuxVector* StaticInstance::GetBaseTrait(const std::string& sTrait) {

	auto it = this->traits.find(sTrait);
	if (it != this->traits.end()) {
		return &(it->second);
	}
	return nullptr;
}

AuxVector* StaticInstance::SetBaseTrait(const std::string& sTrait) {
	return &this->traits[sTrait];
}

void StaticInstance::EraseBaseTrait(const std::string& sTrait) {

	this->traits.erase(sTrait);

}

AuxVector* StaticInstance::GetTrait(const std::string& trait, StaticInstance* linkedObj, const std::string& sSlot, UInt8 priorityFlag) {

	if (!linkedObj) {
		return this->GetBaseTrait(trait);
	}

	if (AuxVector* aux = this->GetLinkedTrait(trait, linkedObj, sSlot)) {
		return aux;
	}

	if (priorityFlag == 1) {
		return this->GetBaseTrait(trait);	// Return this object base trait
	}
	else if (priorityFlag == 2) {
		return linkedObj->GetBaseTrait(trait);	// Return Linked base trait
	}

}

AuxVector* StaticInstance::SetTrait(const std::string& trait, StaticInstance* linkedObj, const std::string& sSlot, UInt8 priorityFlag) {

	if (!linkedObj || linkedObj == this) {
		return this->SetBaseTrait(trait);
	}

	if (priorityFlag == 1) {
		if (AuxVector* aux = this->GetLinkedTrait(trait, linkedObj, sSlot)) {
			return aux;
		}
		return this->SetBaseTrait(trait);
	}

	if (AuxVector* aux = this->SetLinkedTrait(trait, linkedObj, sSlot)) {
		return aux;
	}
	return this->SetBaseTrait(trait);

}

void StaticInstance::MarkAsEdit(UInt32 modIndex) {
	if (std::find(edits.begin(), edits.end(), modIndex) == edits.end()) {
		edits.push_back(modIndex);
	}
}

InstanceVector::~InstanceVector() {
	for (auto inst : *this) {
		delete inst;  // Properly delete instances to prevent memory leaks
	}
}

UInt32 InstanceVector::add(Instance* newInstance) {
	auto it = std::find(this->begin(), this->end(), nullptr);
	if (it != this->end()) {
		*it = newInstance;
		return std::distance(this->begin(), it);
	}
	else {
		this->push_back(newInstance);
		return this->size() - 1;
	}
}

void InstanceVector::remove(UInt8 instID) {

	if (instID < this->size()) {

		this->operator[](instID) = nullptr;

		if (instID == this->size() - 1) {
			this->pop_back();
		}

	}
}

void InstanceVector::markForDelete(UInt8 instID) {

	if (instID < this->size()) {
		this->operator[](instID) = nullptr;
	}
}

//Not used atm
void InstanceVector::cleanUp() {

	while (!this->empty() && this->back() == nullptr) {
		this->pop_back();
	}

}


