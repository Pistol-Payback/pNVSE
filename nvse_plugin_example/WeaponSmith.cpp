#pragma once
#include "WeaponSmith.h"

std::unordered_map<UInt32, std::unordered_map<UInt32, StaticInstance*>> StaticLinker;		//This links the baseform to its extra data.
std::unordered_map<UInt32, std::unordered_map<UInt32, Instance*>> InstanceLinker;					//This links cloned baseforms to its WeapInst that contains attachment data.
std::unordered_map<UInt32, std::unordered_map<UInt32, TESRefr*>> TESRefLinker;

std::unordered_map<UInt32, std::unordered_map<UInt32, StaticInstance_Akimbo*>> AkimboSets; //First UInt32 is the right weapon, and second UInt32 is the left

std::vector<ExpirationTimer*> lifecycleTimer;
std::vector<TESObjectREFR*> newlyCreatedReferences;
//std::unordered_map<std::pair<TESObjectREFR*, TESForm*>, UInt32> lifecycleLookup;

std::unordered_set<UInt32> DynamicallyCreatedForms;

//std::unordered_map<UInt32, StaticInstance*> StaticLinker;					//This links the baseform to its extra data.
//std::unordered_map<UInt32, Instance*> InstanceLinker;								//This links cloned baseforms to its WeapInst that contains attachment data.

//std::vector<UInt32> Instance_WEAP::aCloneRecycle;						//Deleting the created clones would be better, but I'm not sure what issues deleting a baseform would cause.

UInt32 InstanceInterface::cloneCount = 0;

//std::vector<std::vector<WeaponArgValue>> WeaponArgValue::NVSEArrayVector;

//std::vector<UInt32> aUsedClones;
//std::vector<UInt32> aClones;		//Deleting the created clones would be better, but I'm not sure what issues deleting a baseform would cause.

void LifecycleManager::addLifecycleTimer(TESObjectREFR* worldRef, TESForm* baseForm, float time, ExtraDataList* xDataList, bool insideWorldRef, bool set) {

	UInt8 type = insideWorldRef ? kExtraData_ContainerChanges : kExtraData_TimeLeft;

	auto it = std::find_if(lifecycleTimer.begin(), lifecycleTimer.end(),
		[worldRef, baseForm](ExpirationTimer* timer) {
			return timer->ref == worldRef && timer->baseform == baseForm;
		});

	if (it == lifecycleTimer.end()) {
		lifecycleTimer.emplace_back(new ExpirationTimer(worldRef, baseForm, type, &worldRef->extraDataList));
	}

	ExtraTimeLeft* xData = (ExtraTimeLeft*)xDataList->GetByType(kExtraData_TimeLeft);
	if (xData) {
		if (set) {
			xData->time = time;
		}
	}
	else{
		xData = ExtraTimeLeft::Create(time);
		worldRef->extraDataList.Add(xData);
	}

}

TESForm* TESForm::GetStaticParent() const {

	auto iter = InstanceLinker[typeID].find(refID);
	if (iter != InstanceLinker[typeID].end()) {
		return iter->second->baseInstance->parent;
	}

	return nullptr;
}

TESForm* TESForm::GetStaticParent(UInt32 parentTypeFilter) const {

	auto iter = InstanceLinker[typeID].find(refID);
	if (iter != InstanceLinker[typeID].end() && iter->second->baseInstance->extendedType == parentTypeFilter) {
		return iter->second->baseInstance->parent;
	}

	return nullptr;
}

bool TESForm::IsInstancedForm() const {
	return InstanceLinker[this->typeID].find(this->refID) != InstanceLinker[this->typeID].end();
}

bool TESForm::IsInstancedForm(UInt32 typeFilter) const {

	auto iter = InstanceLinker[typeID].find(refID);
	if (iter != InstanceLinker[typeID].end() && iter->second->baseInstance->extendedType == typeFilter) {
		return true;
	}
	return false;
}

UInt32 TESForm::GetInstanceID() const {

	if (this->IsInstancedForm()) {
		return InstanceLinker[this->typeID][this->refID]->InstID;
	}

	return 0;

}

bool TESForm::IsStaticForm(UInt32 typeFilter) const {
	return StaticLinker[typeID].find(refID) != StaticLinker[typeID].end();

		auto iter = StaticLinker[typeID].find(refID);
		if (iter != StaticLinker[typeID].end() && iter->second->extendedType == typeFilter) {
			return true;
		}
		return false;

}
bool TESForm::IsStaticForm() const {
	return StaticLinker[typeID].find(refID) != StaticLinker[typeID].end();
}


bool TESForm::HasExtendedMods() const {

	const StaticInstance_WEAP* staticInst = nullptr;

	if (this->IsStaticForm(40)) {
		staticInst = static_cast<StaticInstance_WEAP*>(StaticLinker[40][this->refID]);
	}
	else if (this->IsInstancedForm(40)) {
		staticInst = static_cast<StaticInstance_WEAP*>(InstanceLinker[40][this->refID]->baseInstance);
	}

	return staticInst && !staticInst->aAllAttachments.empty();

}

bool TESForm::pIsDynamicForm() const {
	return DynamicallyCreatedForms.find(refID) != DynamicallyCreatedForms.end();
}

TESInstance* TESForm::pLookupInstance() const {
	auto iter = InstanceLinker[typeID].find(refID);
	if (iter != InstanceLinker[typeID].end()) {
		return (TESInstance*)iter->second;
	}
	return nullptr;
}

Instance* TESForm::LookupInstance(UInt32 type) const {
	auto iter = InstanceLinker[type].find(refID);
	if (iter != InstanceLinker[type].end()) {
		return (Instance*)iter->second;
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

StaticInstance* TESForm::LookupExtendedBase() const {

	if (Instance* inst = this->LookupInstance(this->typeID)) {
		return inst->baseInstance;
	}
	auto iter = StaticLinker[typeID].find(refID);
	if (iter != StaticLinker[typeID].end()) {
		return iter->second;
	}

	return nullptr;
}

//Lookup or create
StaticInstance* TESForm::getExtendedBase(UInt32 kitIndex) {

	if (Instance* inst = this->LookupInstance(this->typeID)) {
		return inst->baseInstance;
	}
	auto iter = StaticLinker[typeID].find(refID);
	if (iter != StaticLinker[typeID].end()) {
		return iter->second;
	}

	return this->MarkAsStaticForm(kitIndex);
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

	auto it = StaticLinker[this->typeID].find(this->refID);
	if (it == StaticLinker[this->typeID].end() && !this->IsInstancedForm()) {
		switch (this->typeID) {
		case 40:
			return new StaticInstance_WEAP(this, kitIndex);
			break;
		default:
			return new StaticInstance(this, kitIndex, this->typeID);
			break;
		}
	}
	return it->second;
}

TESForm* TESForm::CreateInst(std::string key, UInt32 modIndex) {

	TESForm* baseForm = this->IsReference() ? ((TESObjectREFR*)this)->baseForm : this;

	StaticInstance* staticInstance = baseForm->LookupExtendedBase();

	staticInstance = baseForm->MarkAsStaticForm(modIndex);

	if (!staticInstance) { //Do not create modifiers of other modifiers. 
		return nullptr;
	}

	Instance* NewInst = staticInstance->newInstance(key, modIndex);

	return NewInst->clone;

}

StaticInstance_Akimbo* StaticInstance_Akimbo::LookupAkimboSet(TESForm* left, TESForm* right) {

	auto iter = AkimboSets.find(left->refID);
	if (iter != AkimboSets.end()) {

		auto iterSub = iter->second.find(right->refID);
		if (iterSub != iter->second.end()) {
			return iterSub->second;
		}

	}
	return nullptr;

}

//..............................................................................................................................................................

void Instance::destroy() {
	this->baseInstance->aInstances.remove(this->InstID);
	delete this;
}

TESForm* StaticInstance::createInstance(std::string key) {

	std::string editorID = "Inst" + std::to_string(InstanceInterface::cloneCount);
	if (TESForm* clone = TESForm::CreateNewForm(this->parent, true, editorID.c_str())) {
		return clone;
	}

	return nullptr;

}

//Used for akimbos, if you use this function, make sure to clean up the old form if it's not being used.
//Shouldn't be used at runtime.
void StaticInstance::setParent(TESForm* form, bool doFree) {

	if (this->parent) { //Remove old parent from the map  -potential to leak memory
		auto typeIt = StaticLinker.find(this->parent->typeID);
		if (typeIt != StaticLinker.end()) {
			auto refIt = typeIt->second.find(this->parent->refID); 
			if (refIt != typeIt->second.end()) {
				if (refIt->second == this) {
					typeIt->second.erase(refIt);
				}
			}
			if (typeIt->second.empty()) {
				StaticLinker.erase(typeIt);
			}
		}
		if (doFree) {
			this->parent->Destroy(true);
		}
	}
	this->parent = form; //Add new parent to the map
	if (this->parent) {
		StaticLinker[this->parent->typeID][this->parent->refID] = this;
	}

}

Instance* StaticInstance::loadInstance(UInt32 instID, UInt32 modIndex, const std::string& key, LifecycleManager* lifecycle) {

	TESForm* clone = this->createInstance(key);
	TESInstance* NewInst = new TESInstance(instID, this, clone, modIndex, key, std::move(*lifecycle));
	return NewInst;

}

Instance* StaticInstance::newInstance(const std::string& key, UInt32 modIndex) {

	TESForm* clone = this->createInstance(key);
	return new TESInstance(this, clone, modIndex, key);

}


Instance* StaticInstance_WEAP::loadInstance(UInt32 instID, UInt32 modIndex, const std::string& key, LifecycleManager* lifecycle) {

	TESForm* clone = this->createInstance(key);
	Instance_WEAP* NewInst = new Instance_WEAP(instID, this, clone, modIndex, key, std::move(*lifecycle), this->aBaseAttachments);
	return NewInst;

}
Instance* StaticInstance_WEAP::newInstance(const std::string& key, UInt32 modIndex) {

	TESForm* clone = this->createInstance(key);
	return new Instance_WEAP(this, clone, modIndex, key, this->aBaseAttachments);

}

Instance* StaticInstance_Akimbo::newInstance(const std::string& key, UInt32 modIndex) {

	TESForm* clone = this->createInstance(key);
	Instance_Akimbo* NewInst = new Instance_Akimbo(this, clone, modIndex, key, nullptr, nullptr);
	return NewInst;

}


Instance_Akimbo* StaticInstance_Akimbo::loadInstance(UInt16 InstID, UInt32 modIndex, TESObjectREFR* right, TESObjectREFR* left, const std::string& key, LifecycleManager* lifecycle) {

	if (!PluginFunctions::kNVSE) {
		return nullptr;
	}

	TESObjectWEAP* leftWeap = static_cast<TESObjectWEAP*>(left->baseForm);
	TESObjectWEAP* rightWeap = static_cast<TESObjectWEAP*>(right->baseForm);

	if (!leftWeap || !rightWeap) {
		return nullptr;
	}

	UInt32 currentKitIndex = 0;
	std::string editorID = "AkimboInst" + std::to_string(InstanceInterface::cloneCount);

	Console_Print("Loading akimbo with: %s", key.c_str());

	TESObjectWEAP* clone = (TESObjectWEAP*)TESForm::CreateNewForm(rightWeap, true, editorID.c_str());
	PluginFunctions::CopyAnimationsToForm(leftWeap, clone);
	PluginFunctions::CopyAnimationsToForm(this->parent, clone);

	TESFullName* fullName = clone->GetFullName();

	std::string nameLeft = leftWeap->GetFullName() ? leftWeap->GetFullName()->name.m_data : "left";
	std::string nameRight = rightWeap->GetFullName() ? rightWeap->GetFullName()->name.m_data : "right";
	std::string akimboName = nameLeft + " & " + nameRight;
	clone->GetFullName()->name.Set(akimboName.c_str());

	clone->weight.weight = leftWeap->weight.weight + rightWeap->weight.weight;
	clone->value.value = leftWeap->value.value + rightWeap->value.value;
	clone->strRequired = leftWeap->strRequired + rightWeap->strRequired;
	clone->clipRounds.clipRounds = leftWeap->clipRounds.clipRounds + rightWeap->clipRounds.clipRounds;

	Instance_Akimbo* NewInst = new Instance_Akimbo(InstID, this, clone, modIndex, key, std::move(*lifecycle), left, right);
	return NewInst;

}

TESForm* StaticInstance_Akimbo::newInstance(TESObjectREFR* right, TESObjectREFR* left, UInt32 modIndex, const std::string& key) {

	if (!PluginFunctions::kNVSE) {
		return nullptr;
	}

	TESObjectWEAP* leftWeap = (TESObjectWEAP*)left->baseForm;
	TESObjectWEAP* rightWeap = (TESObjectWEAP*)right->baseForm;

	if (!leftWeap || !rightWeap) {
		return nullptr;
	}
	
	UInt32 currentKitIndex = 0;
	std::string editorID = "AkimboInst" + std::to_string(InstanceInterface::cloneCount);

	TESObjectWEAP* clone = (TESObjectWEAP*)TESForm::CreateNewForm(rightWeap, true, editorID.c_str());
	PluginFunctions::CopyAnimationsToForm(leftWeap, clone);
	PluginFunctions::CopyAnimationsToForm(this->parent, clone);

	TESFullName* fullName = clone->GetFullName();

	std::string nameLeft = leftWeap->GetFullName() ? leftWeap->GetFullName()->name.m_data : "left";
	std::string nameRight = rightWeap->GetFullName() ? rightWeap->GetFullName()->name.m_data : "right";
	std::string akimboName = nameLeft + " & " + nameRight;
	clone->GetFullName()->name.Set(akimboName.c_str());

	clone->weight.weight = leftWeap->weight.weight + rightWeap->weight.weight;
	clone->value.value = leftWeap->value.value + rightWeap->value.value;
	clone->strRequired = leftWeap->strRequired + rightWeap->strRequired;
	clone->clipRounds.clipRounds = leftWeap->clipRounds.clipRounds + rightWeap->clipRounds.clipRounds;

	TESObjectREFR* rightRef = TESObjectREFR::Create(false);
	TESObjectREFR* leftRef = TESObjectREFR::Create(false);

	rightRef->baseForm = rightWeap;
	ExtraDataList::CopyItemData(&right->extraDataList, 0, &rightRef->extraDataList);
	rightRef->SetRefID(GetNextFreeFormID(), true);
	rightRef->extraDataList.RemoveByType(kXData_ExtraCount);

	leftRef->baseForm = leftWeap;
	ExtraDataList::CopyItemData(&left->extraDataList, 0, &leftRef->extraDataList);
	leftRef->SetRefID(GetNextFreeFormID(), true);
	leftRef->extraDataList.RemoveByType(kXData_ExtraCount);

	Instance_Akimbo* NewInst = new Instance_Akimbo(this, clone, modIndex, key, leftRef, rightRef);

	return NewInst->clone;
}

//..........................................................................................................................................

AuxVector* ExtendedBaseType::GetLinkedTrait(const std::string& trait, ExtendedBaseType* linkedObj, const std::string& sSlot) {

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

	return nullptr;

};

AuxVector* ExtendedBaseType::SetLinkedTrait(const std::string& trait, ExtendedBaseType* linkedObj, const std::string& sSlot) {

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

	return nullptr;
};

//................................................................................................................................................................

AuxVector* ExtendedBaseType::GetBaseTrait(const std::string& sTrait) {

	auto it = this->traits.find(sTrait);
	if (it != this->traits.end()) {
		return &(it->second);
	}
	return nullptr;
}

AuxVector* ExtendedBaseType::SetBaseTrait(const std::string& sTrait) {
	return &this->traits[sTrait];
}

void ExtendedBaseType::EraseBaseTrait(const std::string& sTrait) {

	this->traits.erase(sTrait);

}

AuxVector* ExtendedBaseType::GetTrait(const std::string& trait, ExtendedBaseType* linkedObj, const std::string& sSlot, UInt8 priorityFlag) {

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

	return nullptr;

}

AuxVector* ExtendedBaseType::SetTrait(const std::string& trait, ExtendedBaseType* linkedObj, const std::string& sSlot, UInt8 priorityFlag) {

	if (!linkedObj || linkedObj == this) {
		return this->SetBaseTrait(trait); //Set base trait
	}

	if (priorityFlag == 1) { //Set link trait if it exists.
		if (AuxVector* aux = this->GetLinkedTrait(trait, linkedObj, sSlot)) {
			return aux;
		}
		return this->SetBaseTrait(trait);
	}

	//Set linked trait and create it if it doesn't exists
	if (AuxVector* aux = this->SetLinkedTrait(trait, linkedObj, sSlot)) {
		return aux;
	}
	return this->SetBaseTrait(trait); //Fallback

}

void ExtendedBaseType::MarkAsEdit(UInt32 modIndex) {
	if (std::find(edits.begin(), edits.end(), modIndex) == edits.end()) {
		edits.push_back(modIndex);
	}
}

void ExtendedBaseType::clearInstances() {
	for (Instance* instance : aInstances) {
		if (instance) {
			AuxVector filter{ instance->key.c_str() };
			for (auto it = onInstanceDeconstructEvent.handlers.begin(); it != onInstanceDeconstructEvent.handlers.end(); ++it) {
				if (it->CompareFilters(filter)) {
					g_scriptInterface->CallFunction(it->script, nullptr, nullptr, nullptr, 1, instance->clone);
				}
			}
			delete instance;
		}
	}
	aInstances.clear();
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


