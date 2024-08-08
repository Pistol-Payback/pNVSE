#pragma once
#include "SaveSystem.h"

namespace SaveSystem {

	InstanceLoadManager SaveLoadManager::loadManager;

	void LoadGameCallback(void*)
	{
		UInt32 type, version, length;

		SaveLoadManager::loadManager.clearInstances();

		while (SaveData::GetNextRecordInfo(&type, &version, &length)) {
			SaveLoadManager::loadManager.loadAll();
		}
	}

	void InstanceLoadManager::reset() {

		baseLocations.clear();
		refLocations.clear();

		failedLinks_WEAP.clear();
		failedLinks_Akimbo.clear();
		//reconstructEvent.clear();

		worldRefs.clear();
		entries.clear();
		linkRefs.clear();

	}

	void InstanceLoadManager::LoadStaticLocations(UInt32 index) {

		char* EditorID = ReadRecordString();
		UInt8 type = SaveData::ReadRecord8();

		TESForm* form = LookupEditorID<TESForm*>(EditorID);
		if (form && type == form->typeID) {
			baseLocations[index] = form;
		}
		else {
			baseLocations[index] = nullptr;
		}


	}

	//Used for ref links
	TESObjectREFR* InstanceLoadManager::LoadFromRefID(UInt32 refID) {
		auto iter = refLocations.find(refID);
		if (iter != refLocations.end()) {
			return iter->second;
		}
		return nullptr;
	}

	//Used for baseform links
	TESForm* InstanceLoadManager::LoadFromIndex(UInt32 index) {
		auto iter = baseLocations.find(index);
		if (iter != baseLocations.end()) {
			return iter->second;
		}
		return nullptr;
	}

	ExtendedBaseType* InstanceLoadManager::LoadBase(UInt32 extendedType) {
		switch (extendedType) {
		case 222:
			return LoadStaticAkimbo();
			break;
		default:
			return LoadStatic();
			break;
		}
	}

	StaticInstance* InstanceLoadManager::LoadStatic() {

		LoadDataLink link = LoadLink();
		TESForm* form = deduceLinkToForm(&link);
		if (form && form->IsStaticForm()) {
			return form->LookupStaticInstance();
		}
		else {
			replacement = form;
		}
		return nullptr;

	}

	StaticInstance_Akimbo* InstanceLoadManager::LoadStaticAkimbo() {

		LoadDataLink right = LoadLink();
		LoadDataLink left = LoadLink();
		TESForm* rightForm = deduceLinkToForm(&right);
		TESForm* leftForm = deduceLinkToForm(&left);
		if (rightForm && leftForm) {
			return StaticInstance_Akimbo::LookupAkimboSet(rightForm, leftForm);
		}
		else if (rightForm) {
			replacement = rightForm;
		}
		else if (leftForm) {
			replacement = leftForm;
		}

		return nullptr;

	}

	LoadDataLink InstanceLoadManager::LoadLink() {

		TESForm* parent = nullptr;
		UInt32 refID = 0;
		UInt32 instID = InvalidInstID;

		UInt8 saveType = ReadRecord8();

		if (saveType == 1) { //vanilla
			refID = ReadRecord32();
		}
		else if (saveType == 2) {  //baseform vanilla
			refID = ReadRecord32();
		}
		else if (saveType == 3){ //ref dynamic
			UInt32 index = ReadRecord32();
			parent = LoadFromIndex(index);
			refID = ReadRecord32();
		}
		else if (saveType == 4) { //baseform dynamic

			UInt32 index = ReadRecord32();
			parent = LoadFromIndex(index);

		}
		else if (saveType == 5) { //vanilla Instances
			refID = ReadRecord32();
			instID = ReadRecord16();
		}
		else if (saveType == 6) { //instances

			UInt32 index = ReadRecord32();
			parent = LoadFromIndex(index);
			instID = ReadRecord16();
		}

		return LoadDataLink(parent, refID, instID, saveType);

	}

	TESForm* InstanceLoadManager::deduceLinkToForm(LoadDataLink* link) {

		if (link->saveType == 1 || link->saveType == 2) { //Vanilla
			if (ResolveRefID(link->refID, &link->refID) && link->refID) {
				return LookupFormByRefID(link->refID);
			};
		}
		else if (link->saveType == 3) { //ref dynamic
			return LoadFromRefID(link->refID);
		}
		else if (link->saveType == 4) { //baseform dynamic
			return link->parent;
		}
		else if (link->saveType == 5) { //Vanilla instance
			if (SaveData::ResolveRefID(link->refID, &link->refID) && link->refID) {
				TESForm* form = LookupFormByRefID(link->refID);
				return form->LookupInstanceByID(link->instID)->clone;
			};
		}
		else if (link->saveType == 6) { //Is an instance baseform
			return link->parent->LookupInstanceByID(link->instID)->clone;
		}

		return nullptr;

	}

	void InstanceLoadManager::LoadLink_Skip() {

		UInt8 saveType = ReadRecord8();

		if (saveType == 1) { //Vanilla
			SkipNBytes(4);
		}
		else if (saveType == 2) { //baseform Vanilla
			SkipNBytes(4);
		}
		else if (saveType == 3) { //ref dynamic
			SkipNBytes(8);
		}
		else if (saveType == 4) { //baseform Dynamic
			SkipNBytes(4);
		}
		else if (saveType == 5) { //Vanilla Instances
			SkipNBytes(6);
		}
		else if (saveType == 6) { //Instances
			SkipNBytes(6);
		}

	}

	Instance* InstanceLoadManager::LoadInstance(ExtendedBaseType* baseInstance, UInt32 modIndex) {
		switch (baseInstance->extendedType) {
		case 40:
			return LoadInstanceDataWEAP((StaticInstance_WEAP*)baseInstance, modIndex);
			break;
		case 222:
			return LoadInstanceDataAkimbo((StaticInstance_Akimbo*)baseInstance, modIndex);
			break;
		default:
			return LoadInstanceData((StaticInstance*)baseInstance, modIndex);
			break;
		}
	}

	void InstanceLoadManager::LoadInstance_Skip(UInt32 extendedType) {
		switch (extendedType) {
		case 40:
			LoadInstanceDataWEAP_Skip();
			return;
			break;
		case 222:
			LoadInstanceDataAkimbo_Skip();
			return;
			break;
		default:
			LoadInstanceData_Skip();
			return;
			break;
		}
	}

	LifecycleManager* InstanceLoadManager::LoadLifecycle() {

		UInt8 policies = ReadRecord8();
		float lifetime = ReadRecordFloat();

		return new LifecycleManager(policies, lifetime);

	}

	void InstanceLoadManager::LoadLifecycle_Skip() {
		SkipNBytes(5);
	}

	Instance* InstanceLoadManager::LoadInstanceData(StaticInstance* baseInstance, UInt32 modIndex) {

		UInt16 instID = ReadRecord16();
		char* key = ReadRecordString();
		LifecycleManager* lifeCycle = LoadLifecycle();

		Instance* inst = baseInstance->loadInstance(instID, modIndex, key, lifeCycle);

		if (!ReconstructEvent(inst)) {
			inst->destroy();
			return nullptr;
		}

		return inst;

	}

	void InstanceLoadManager::LoadInstanceData_Skip() {

		SkipNBytes(2);
		SkipRecordString();
		LoadLifecycle_Skip();
		return;

	}

	Instance_WEAP* InstanceLoadManager::LoadInstanceDataWEAP(StaticInstance_WEAP* baseInstance, UInt32 modIndex) {

		UInt16 instID = ReadRecord16();
		char* key = ReadRecordString();
		LifecycleManager* lifeCycle = LoadLifecycle();

		Instance_WEAP* inst = (Instance_WEAP*)baseInstance->loadInstance(instID, modIndex, key, lifeCycle);

		//Deduce later

		UInt32 size = ReadRecord32();
		for (UInt32 i = 0; i < size; ++i) {

			char* slot = ReadRecordString();

			LoadDataLink attachment = LoadLink();
			TESForm* form = deduceLinkToForm(&attachment);
			if (form) {
				inst->aAttachments[slot] = form->refID;
			}
			else { //Will happen when attachments are instances and attached to other instances.
				failedLinks_WEAP.emplace_back(new FailedWeaponLink{ inst, slot, attachment });
			}

		}

		if (!ReconstructEvent(inst)) {
			inst->destroy();
			return nullptr;
		}

		return inst;

	}

	void InstanceLoadManager::LoadInstanceDataWEAP_Skip() {

		SkipNBytes(2);
		SkipRecordString();
		LoadLifecycle_Skip();

		UInt32 size = ReadRecord32();
		for (UInt32 i = 0; i < size; ++i) {

			SkipRecordString();
			LoadLink_Skip();

		}

		return;

	}

	Instance_Akimbo* InstanceLoadManager::LoadInstanceDataAkimbo(StaticInstance_Akimbo* baseInstance, UInt32 modIndex) {

		UInt16 instID = ReadRecord16();
		char* key = ReadRecordString();

		LifecycleManager* life = LoadLifecycle();

		LoadDataLink right = LoadLink();
		LoadDataLink left = LoadLink();

		ExtraDataList* xDataRight = LoadExtraDataList();
		ExtraDataList* xDataLeft = LoadExtraDataList();

		if (!PluginFunctions::kNVSE) {
			return nullptr;
		}

		TESObjectWEAP* rightWeap = (TESObjectWEAP*)deduceLinkToForm(&right);
		TESObjectWEAP* leftWeap = (TESObjectWEAP*)deduceLinkToForm(&left);

		if (leftWeap && rightWeap) {

			TESObjectREFR* rightRef = TESObjectREFR::Create(false);
			rightRef->SetRefID(GetNextFreeFormID(), true);
			rightRef->baseForm = rightWeap;

			TESObjectREFR* leftRef = TESObjectREFR::Create(false);
			leftRef->SetRefID(GetNextFreeFormID(), true);
			leftRef->baseForm = leftWeap;


			if (!leftWeap || !rightWeap) {
				return nullptr;
			}

			if (xDataRight) {
				rightRef->extraDataList.CopyFrom(xDataRight, true);
			}
			if (xDataLeft) {
				leftRef->extraDataList.CopyFrom(xDataLeft, true);
			}

			Instance_Akimbo* inst = baseInstance->loadInstance(instID, modIndex, rightRef, leftRef, key, life);
			return inst;
		}



		//Make ordered list
		/*
		if (!rightWeap || !leftWeap) {
			failedLinks_Akimbo.emplace_back(new FailedAkimboLink(inst, left, right));
		}
		*/

		//if (!ReconstructEvent(inst)) {
			//inst->destroy();
			//return nullptr;
		//}

		//return inst;

		return nullptr;

	}

	void InstanceLoadManager::LoadInstanceDataAkimbo_Skip() {

		SkipNBytes(2);
		SkipRecordString();
		LoadLifecycle_Skip();

		LoadLink_Skip();
		LoadLink_Skip();

		LoadExtraDataList_Skip();
		LoadExtraDataList_Skip();

		return;

	}

	void InstanceLoadManager::LoadInstanceLinksWEAP() {

		for (FailedWeaponLink* failedData : failedLinks_WEAP) {
			TESForm* attachment = deduceLinkToForm(&failedData->attachment);
			if (attachment) {
				failedData->weap->aAttachments[failedData->slot] = attachment->refID;
			}
			else {
				TESForm* attachment = deduceLinkToForm(&failedData->attachment);

			}
			delete failedData;
		}

	}

	void InstanceLoadManager::LoadRefsList(TESForm* baseform) {

		UInt32 size = ReadRecord32();
		for (UInt32 i = 0; i < size; ++i) {
			LoadRef(baseform);
		}

	}

	void InstanceLoadManager::LoadRef(TESForm* baseform) {

		UInt8 type = ReadRecord8();
		if (type == 1){	//Is entry
			LoadEntryRef(baseform);
		}
		else if (type == 2) {	//Is World ref
			LoadWorldRef(baseform);
		}

	}

	void InstanceLoadManager::LoadEntryRef(TESForm* baseform) {

		//logOperation("Save SaveDataEntry&");
		LoadDataLink link = LoadLink();
		ExtraDataList* xData = LoadExtraDataList();
		SInt32 count = ReadRecord32();

		entries.emplace_back(new LoadDataEntry(baseform, link, xData, count));

	}

	void InstanceLoadManager::LoadWorldRef(TESForm* baseform) {

		LoadDataLink link = LoadLink();
		ExtraDataList* xData = LoadExtraDataList();
		SInt32 count = ReadRecord32();

		UInt32 worldRefID = ReadRecord32();
		float x = ReadRecordFloat();
		float y = ReadRecordFloat();
		float z = ReadRecordFloat();

		float xR = ReadRecordFloat();
		float yR = ReadRecordFloat();
		float zR = ReadRecordFloat();

		worldRefs.emplace_back(new LoadDataWorldREFR(baseform, link, xData, count, worldRefID, x, y, z, xR, yR, zR));

	}

	void InstanceLoadManager::LoadRefsList_Skip() {
		UInt32 size = SaveData::ReadRecord32();
		for (UInt32 i = 0; i < size; ++i) {
			LoadRef_Skip();
		}
	}

	void InstanceLoadManager::LoadRef_Skip() {
		UInt8 type = SaveData::ReadRecord8();
		if (type == 1) {	//Is entry
			LoadEntryRef_Skip();
		}
		else if (type == 2) {	//Is World ref
			LoadWorldRef_Skip();
		}
	}

	void InstanceLoadManager::LoadEntryRef_Skip() {
		LoadLink_Skip(), LoadExtraDataList_Skip(), SkipNBytes(4);
	}

	void InstanceLoadManager::LoadWorldRef_Skip() {
		LoadLink_Skip(), LoadExtraDataList_Skip();
		SkipNBytes(4 * 8);
	}

	void InstanceLoadManager::PlaceWorldRefs() {

		for (size_t i = 0; i < worldRefs.size(); i++) {
			auto worldRef = worldRefs[i];
			TESForm* location = deduceLinkToForm(&worldRef->location);
			if (location && worldRef->baseform) {
				queueToSpawn.emplace_back(worldRef->baseform, location, worldRef->xData, worldRef->countDelta,
					worldRef->x, worldRef->y, worldRef->z, worldRef->xR, worldRef->yR, worldRef->zR);
			}
		}

		for (auto& ref : worldRefs) {
			delete ref;
		}
		worldRefs.clear();
	}

	void InstanceLoadManager::PlaceEntryRefs() {

		for (LoadDataEntry* ref : entries) {

			TESForm* location = deduceLinkToForm(&ref->location);
			if (!location || !ref->baseform) {
				return;
			}

			if (ref->xData && ref->xData->HasType(kExtraData_Worn)) {
				ref->xData->RemoveByType(kExtraData_Worn);
				((Actor*)location)->AddItem(ref->baseform, ref->xData, ref->countDelta);
				((Actor*)location)->SilentEquip(ref->baseform, ref->xData);
			}
			else {
				((TESObjectREFR*)location)->AddItem(ref->baseform, ref->xData, ref->countDelta);
			}

			delete ref;

		}

	}

	void InstanceLoadManager::clearInstances() {

		InstanceInterface::cloneCount = 0;

		for (auto& typeMap : StaticLinker) {
			for (auto& pair : typeMap.second) {
				StaticInstance* staticInst = pair.second;
				if (staticInst) {
					staticInst->clearInstances();
				}
			}
		}

		for (auto& akimboMap : AkimboSets) {
			for (auto& akimboPair : akimboMap.second) {
				StaticInstance_Akimbo* akimbo = akimboPair.second;
				akimbo->clearInstances();
			}
		}

	}

	bool InstanceLoadManager::ReconstructEvent(Instance* inst) {

		AuxVector filter{ inst->key.c_str() };
		for (auto it = onInstanceReconstructEvent.handlers.begin(); it != onInstanceReconstructEvent.handlers.end(); ++it) {

			if (it->CompareFilters(filter)) {

				ArrayElementL scriptReturn;
				g_scriptInterface->CallFunction(it->script, nullptr, nullptr, &scriptReturn, 1, inst->clone);
				if (scriptReturn.IsValid()) {
					return false;
				}

			}

		}

		return true;

	}
	/*
	void InstanceLoadManager::ReconstructEvent() {

		for (auto inst : reconstructEvent) {

			AuxVector filter{ inst->key.c_str() };
			for (auto it = onInstanceReconstructEvent.handlers.begin(); it != onInstanceReconstructEvent.handlers.end(); ++it) {

				if (it->CompareFilters(filter)) {

					ArrayElementL scriptReturn;
					g_scriptInterface->CallFunction(it->script, nullptr, nullptr, &scriptReturn, 1, inst->clone);
					if (scriptReturn.IsValid()) {
						inst->baseInstance->aInstances.markForDelete(inst->InstID);
						inst->destroy();
					}

				}

			}

		}

	}
	*/

	Kit::KitData* InstanceLoadManager::LoadKitData() {

		std::string name = ReadRecordString();
		UInt32 index = ReadRecord32();
		UInt32 oldVersion = ReadRecord32();
		bool safeUninstall = ReadRecord8();

		Kit::KitData* kit = Kit::GetKitDataByName(name);
		if (!kit) {
			//Run uninstaller
			if (!safeUninstall) {
				//Warning
				Console_Print("%s is not safe to uninstall mid game", name.c_str());
			}
			return nullptr;
		}

		reverseKitDataMap[index] = kit;

		if (!kit->updater.empty() && oldVersion < kit->version) {
			kit->runUpdater(kit->version);
		}

		return kit;

	}

	UInt32* InstanceLoadManager::resolveModIndex(UInt32 oldIndex) {

		auto iter = reverseKitDataMap.find(oldIndex);
		if (iter != reverseKitDataMap.end()) {
			return &iter->second->index;
		}
		return nullptr;
	}

	void InstanceLoadManager::loadAll() {

		UInt32 size = ReadRecord32();

		for (UInt32 i = 0; i < size; ++i) {
			LoadKitData();
		} //Load kit files


		//SaveData::logOperation("Loading Forms:::::::::::::::::::::::::::::::::::::::::");
		size = ReadRecord32();
		for (UInt32 i = 0; i < size; ++i) {
			LoadStaticLocations(i);
		}

		//SaveData::logOperation("Load instanceList::::::::::::::::::::::::::::::::::");
		size = ReadRecord32();
		for (UInt32 i = 0; i < size; ++i) {

			UInt32 extendedType = ReadRecord32();
			UInt32 extSize = ReadRecord32();

			for (UInt32 ext = 0; ext < extSize; ++ext) {

				ExtendedBaseType* baseInstance = LoadBase(extendedType);

				if (baseInstance) {

					UInt32 instSize = ReadRecord32();

					for (UInt32 j = 0; j < instSize; ++j) {

						UInt32 modIndex = ReadRecord32();
						UInt32* newModIndex = resolveModIndex(modIndex);

						if (newModIndex) {
							Instance* inst = LoadInstance(baseInstance, *newModIndex);
							if (inst) {
								LoadRefsList(inst->clone);
							}
							else if (baseInstance->parent && extendedType <= 120) {
								LoadRefsList(baseInstance->parent);
							}
							else
							{
								LoadRefsList_Skip();
							}
						}
						else {
							LoadInstance_Skip(extendedType);
							LoadRefsList_Skip();
						}

					}

					//if worldRefs exist, that also happen to be dirived from dynamics.
					if (extendedType <= 120) {
						LoadRefsList(baseInstance->parent);
					}

				}
				else {

					UInt32 instSize = ReadRecord32();

					for (UInt32 j = 0; j < instSize; ++j) {

						SkipNBytes(4); //Mod index
						LoadInstance_Skip(extendedType);
						if (replacement) {
							LoadRefsList(replacement);
							replacement = nullptr;
						}
						else {
							LoadRefsList_Skip();
						}

					}

					if (extendedType <= 120) {

						if (replacement) {
							LoadRefsList(replacement);
							replacement = nullptr;
						}
						else {
							LoadRefsList_Skip();
						}

					}

				}

			}

		}//All Instances have been built

		LoadInstanceLinksWEAP();
		//LoadInstanceLinksAkimbo();

		//logOperation("Loading dynamic refs::::::::::::::::::::::::::::::::::");
		size = ReadRecord32();

		for (UInt32 i = 0; i < size; ++i) {	//Iterate through left over dynamics

			UInt32 extendedType = ReadRecord32();
			if (extendedType <= 120) { //Only vanilla statics can be placed directly in the world.

				ExtendedBaseType* baseInstance = LoadBase(extendedType);

				if (baseInstance) {
					LoadRefsList(baseInstance->parent); //Dynamic World Refs
				}
				else {
					LoadRefsList_Skip(); //Skip Dynamic World Refs
				}

			}

		}

		PlaceWorldRefs();
		PlaceEntryRefs();

		reset();

	}

	ExtraDataList* InstanceLoadManager::LoadExtraDataList()
	{

		BSExtraData* xData = nullptr;
		ExtraDataList* xDataList = ExtraDataList::Create();

		UInt8 type = SaveData::ReadRecord8();

		if (type == kXData_ExtraUnknown00) {
			return nullptr;
		}

		while (type != kXData_ExtraUnknown00) {

			switch (type) {
			case kXData_ExtraWorn:
			{
				xData = ExtraWorn::Create();
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraOwnership:
			{
				UInt32 refID = SaveData::ReadRecord32();
				if (SaveData::ResolveRefID(refID, &refID) && refID) {
					xData = ExtraOwnership::Create();
					((ExtraOwnership*)xData)->owner = LookupFormByRefID(refID);
					xDataList->Add(xData);
				}

			}
			break;
			case kXData_ExtraCount:
			{
				SInt16 icount = SaveData::ReadRecord16();
				xData = ExtraCount::Create();
				((ExtraCount*)xData)->count = icount;
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraHealth:
			{
				float fHealth = SaveData::ReadRecordFloat();
				xData = ExtraHealth::Create(fHealth);
				xDataList->Add(xData);
			}
			break;
			case kExtraData_TimeLeft: 
			{
				float ftime = SaveData::ReadRecordFloat();
				xData = ExtraTimeLeft::Create(ftime);
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraCannotWear:
			{
				xData = ExtraCannotWear::Create();
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraHotkey:
			{
				UInt8 iIndex = SaveData::ReadRecord8();
				xData = ExtraHotkey::Create(iIndex);
				xDataList->Add(xData);
			}
			break;
			default:
				break;
			}
			type = SaveData::ReadRecord8();

		}

		return xDataList;

	}

	void InstanceLoadManager::LoadExtraDataList_Skip()
	{

		UInt8 type = SaveData::ReadRecord8();

		if (type == kXData_ExtraUnknown00) {
			return;
		}

		while (type != kXData_ExtraUnknown00) {

			switch (type) {
				case kXData_ExtraOwnership:
				case kXData_ExtraHealth:
				case kExtraData_TimeLeft:
				{
					SkipNBytes(4);
				}
				break;
				case kXData_ExtraCount: {
					SkipNBytes(2);
				}
				break;
				case kXData_ExtraHotkey:
				{
					SkipNBytes(1);
				}
				break;
			}
			type = SaveData::ReadRecord8();

		}

		return;

	}


	/*
	ExtraDataList* LoadExtraDataList()
	{

		BSExtraData* xData = nullptr;
		ExtraDataList* xDataList = ExtraDataList::Create();

		UInt8 type = SaveData::ReadRecord8();

		if (type == kXData_ExtraUnknown00) {
			return xDataList = nullptr;
		}

		while (type != kXData_ExtraUnknown00) {

			switch (type) {
			case kXData_ExtraWorn:
			{
				xData = ExtraWorn::Create();
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraOwnership:
			{
				UInt32 refID = SaveData::ReadRecord32();
				if (SaveData::ResolveRefID(refID, &refID) && refID) {
					xData = ExtraOwnership::Create();
					((ExtraOwnership*)xData)->owner = LookupFormByRefID(refID);
					xDataList->Add(xData);
				}

			}
			break;
			case kXData_ExtraCount:
			{
				UInt32 iCount = SaveData::ReadRecord32();
				xData = ExtraCount::Create();
				((ExtraCount*)xData)->count = iCount;
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraHealth:
			{
				float fHealth = SaveData::ReadRecordFloat();
				xData = ExtraHealth::Create(fHealth);
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraCannotWear:
			{
				xData = ExtraCannotWear::Create();
				xDataList->Add(xData);
			}
			break;
			case kXData_ExtraHotkey:
			{
				UInt8 iIndex = SaveData::ReadRecord8();
				xData = ExtraHotkey::Create(iIndex);
				xDataList->Add(xData);
			}
			break;
			default:
				break;
			}
			type = SaveData::ReadRecord8();

		}

		return xDataList;

	}

	void SkipExtraDataList()
	{

		UInt8 type = SaveData::ReadRecord8();

		while (type != kXData_ExtraUnknown00) {

			SaveData::SkipNBytes(1);

		}

	}

	void LoadCollectedData() {

		UInt32 TotalSize = SaveData::ReadRecord32();
		TESForm* rLocation = nullptr;

		while (TotalSize > 0) {

			UInt8 type = SaveData::ReadRecord8();			//0 = Is Instace dynamic, 1 = is an instance

			UInt32 length = SaveData::ReadRecord32();
			char* EditorID = new char[length + 1];
			SaveData::ReadRecordData(EditorID, length);
			EditorID[length] = '\0';

			if (TESForm * form = LookupEditorID<TESForm*>(EditorID)) {

				if (type) {	//Was an instance

					UInt8 instID = SaveData::ReadRecord8();		//Get instance using baseRefID and instID
					Instance* inst = form->LookupInstanceByID(instID);
					if (inst) {
						form = inst->clone;
					}
					else {
						Console_Print("uninstalled from the game: %s", EditorID);
					}

				}

				UInt32 location = SaveData::ReadRecord32();

				if (SaveData::ResolveRefID(location, &location) && location && form) {

					rLocation = LookupFormByRefID(location);

					ExtraDataList* xData = LoadExtraDataList();

					UInt8 locationCellType = SaveData::ReadRecord8();

					if (!rLocation || rLocation->typeID != locationCellType) { //Location does not exist anymore.

						if (locationCellType == 57 || locationCellType == 65) {
							SaveData::SkipNBytes(24);	//Skip x,y,z and rot for world pos.
						}
						continue;

					}

					if (rLocation->typeID != 57 && rLocation->typeID != 65) {

						if (xData && xData->HasType(kExtraData_Worn)) {
							xData->RemoveByType(kExtraData_Worn);
							((Actor*)rLocation)->AddItem(form, xData, 1);
							((Actor*)rLocation)->SilentEquip(form, xData);
						}
						else {
							((TESObjectREFR*)rLocation)->AddItem(form, xData, 1);
						}

					}
					else {	//World Object

						float x = SaveData::ReadRecordFloat();
						float y = SaveData::ReadRecordFloat();
						float z = SaveData::ReadRecordFloat();
						float xR = SaveData::ReadRecordFloat();
						float yR = SaveData::ReadRecordFloat();
						float zR = SaveData::ReadRecordFloat();

						queueToSpawn.emplace_back(
							form,
							(TESObjectCELL*)rLocation,
							xData,
							x, y, z,
							xR, yR, zR
							);

					}

				}
				else {	//Location does not exist anymore.

					SkipExtraDataList();

					UInt8 locationCellType = SaveData::ReadRecord8();

					if (locationCellType == 57 || locationCellType == 65) {
						SaveData::SkipNBytes(24);
					}

				}

			}
			else {

				if (type) {
					SaveData::SkipNBytes(1);
				}

				SaveData::SkipNBytes(4);

				SkipExtraDataList();
				UInt8 locationCellType = SaveData::ReadRecord8();

				if (locationCellType == 57 || locationCellType == 65) {
					SaveData::SkipNBytes(24);
				}

			}

			--TotalSize;

		}
	}

	void LoadInstances(StaticInstance* staticInst) {

		UInt32 length = SaveData::ReadRecord32();
		char* key = new char[length + 1];
		SaveData::ReadRecordData(key, length);
		key[length] = '\0';

		staticInst->newInstance(key);

	}

	void LoadWeaponInstances(StaticInstance* staticInst) {

		UInt32 length = SaveData::ReadRecord32();
		char* key = new char[length + 1];
		SaveData::ReadRecordData(key, length);
		key[length] = '\0';

		Instance_WEAP* rInstance = (Instance_WEAP*)staticInst->newInstance(key);

		if (rInstance) {

			//std::unordered_map<std::string, UInt32> aAttachments = {};
			UInt8 i = SaveData::ReadRecord8();		//Re-Apply attachments to WeapInst
			while (i > 0) {

				length = SaveData::ReadRecord32();
				char* sSlot = new char[length + 1];
				SaveData::ReadRecordData(sSlot, length);
				sSlot[length] = '\0';

				length = SaveData::ReadRecord32();
				char* EditorID = new char[length + 1];
				SaveData::ReadRecordData(EditorID, length);
				EditorID[length] = '\0';
				TESForm* rAttachment = LookupEditorID<TESForm*>(EditorID); //LookupEditorID of Attachment

				if (rAttachment) {

					rInstance->aAttachments[sSlot] = rAttachment->refID;

					AuxVector filterOnAttach{ rAttachment->refID, rInstance->baseInstance->parent->refID };
					for (auto it = onAttachWeapModReconstructEvent.handlers.begin(); it != onAttachWeapModReconstructEvent.handlers.end(); ++it) {
						if (it->CompareFilters(filterOnAttach)) {
							g_scriptInterface->CallFunction(it->script, nullptr, nullptr, nullptr, 2, rAttachment, rInstance->clone);
						}
					}

				}

				--i;

			}

			AuxVector filter{ key };
			for (auto it = onInstanceReconstructEvent.handlers.begin(); it != onInstanceReconstructEvent.handlers.end(); ++it) {

				if (it->CompareFilters(filter)) {

					ArrayElementL scriptReturn;
					g_scriptInterface->CallFunction(it->script, nullptr, nullptr, &scriptReturn, 1, rInstance->clone);
					if (scriptReturn.IsValid()) {
						rInstance->baseInstance->aInstances.markForDelete(rInstance->InstID);
					}

				}

			}

		}
		else {//Handle unregistered forms

			Console_Print("Skip Record: %s", staticInst->parent->GetTheName());
			UInt8 i = SaveData::ReadRecord8();		//Re-Apply attachments to WeapInst

			while (i > 0) {

				SaveData::SkipNBytes(SaveData::ReadRecord32());	//Skip slot

				SaveData::SkipNBytes(SaveData::ReadRecord32());	//Skip attachment editorID

				--i;

			}

		}

	}

	void SkipInstances() {

		SaveData::SkipNBytes(SaveData::ReadRecord32());	//Key

	}

	void SkipWeaponInstances() {

		SaveData::SkipNBytes(SaveData::ReadRecord32());	//Key

		UInt8 i = SaveData::ReadRecord8();		//Re-Apply attachments to WeapInst

		while (i > 0) {

			SaveData::SkipNBytes(SaveData::ReadRecord32());	//Skip slot

			SaveData::SkipNBytes(SaveData::ReadRecord32());	//Skip attachment editorID

			--i;

		}

	}

	void LoadGameCallback(void*)
	{

		UInt32 type, version, length;
		UInt32 iFinish = 0;

		while (SaveData::GetNextRecordInfo(&type, &version, &length)) {

			ClearAllWeapData();

			UInt32 StaticInstListSize = SaveData::ReadRecord32();

			TESForm* form = NULL;
			UInt32 length = 0;

			while (StaticInstListSize > 0) {

				length = SaveData::ReadRecord32();
				char* EditorID = new char[length + 1];
				SaveData::ReadRecordData(EditorID, length);
				EditorID[length] = '\0';

				UInt8 FormInstanceSize = SaveData::ReadRecord8();					//The instance count
				UInt8 type = SaveData::ReadRecord8();

				form = LookupEditorID<TESForm*>(EditorID);

				if (form && form->typeID == type) {

					StaticInstance* staticInst = form->LookupStaticInstance();

					while (FormInstanceSize > 0) {

						if (staticInst) {

							switch (type) {
							case 40:
								LoadWeaponInstances(staticInst);
								break;
							case 103:
								LoadInstances(staticInst);
								break;
							default:
								LoadInstances(staticInst);
								break;
							}

						}
						else {

							switch (type) {
							case 40:
								SkipWeaponInstances();
								break;
							case 103:
								SkipInstances();
								break;
							default:
								SkipInstances();
								break;
							}

						}

						--FormInstanceSize;

					}

				}
				else {

					while (FormInstanceSize > 0) {

						switch (type) {
						case 40:
							SkipWeaponInstances();
							break;
						case 103:
							SkipInstances();
							break;
						default:
							SkipInstances();
							break;
						}

						--FormInstanceSize;

					}

				}
				--StaticInstListSize;

			}

			LoadCollectedData();

		}

	}
	*/
}