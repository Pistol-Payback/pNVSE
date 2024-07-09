#pragma once
#include <thread>
#include "SaveSystem.h"

namespace SaveSystem {

	void InstanceLoadManager::reset() {
		baseLocations.clear();
		refLocations.clear();

		failedLinks_WEAP.clear();
		failedLinks_Akimbo.clear();

		worldRefs.clear();
		entries.clear();
		linkRefs.clear();
	}

	void InstanceLoadManager::LoadStaticLocations(UInt32 index) {

		UInt32 length = ReadRecord32();
		char* EditorID = new char[length + 1];
		ReadRecordData(EditorID, length);
		EditorID[length] = '\0';
		UInt8 type = SaveData::ReadRecord8();

		TESForm* form = LookupEditorID<TESForm*>(EditorID);
		if (type == form->typeID) {
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

		LoadDataLink staticInst = LoadLink();
		if (staticInst.parent) {
			return staticInst.parent->LookupStaticInstance();
		}
		return nullptr;

	}

	StaticInstance_Akimbo* InstanceLoadManager::LoadStaticAkimbo() {

		LoadDataLink right = LoadLink();
		LoadDataLink left = LoadLink();
		if (right.parent && left.parent) {
			return StaticInstance_Akimbo::LookupAkimboSet(right.parent, left.parent);
		}
		return nullptr;

	}

	LoadDataLink InstanceLoadManager::LoadLink() {

		TESForm* parent = nullptr;
		UInt32 refID = 0;
		UInt32 instID = InvalidInstID;

		UInt8 saveType = ReadRecord8();

		if (saveType == 0) { //Vanilla
			refID = ReadRecord32();
		}
		else if (saveType == 1){ //ref dynamic
			UInt32 index = ReadRecord32();
			parent = LoadFromIndex(index);
			refID = ReadRecord32();
		}
		else if (saveType == 2){ //baseform Dynamic

			UInt32 index = ReadRecord32();
			parent = LoadFromIndex(index);

		}
		else if (saveType == 3) { //Instances

			UInt32 index = ReadRecord32();
			parent = LoadFromIndex(index);
			instID = ReadRecord16();

		}

		return LoadDataLink(parent, refID, instID);

	}

	void InstanceLoadManager::LoadLink_Skip() {

		UInt8 saveType = ReadRecord8();

		if (saveType == 0) { //Vanilla
			SkipNBytes(4);
		}
		else if (saveType == 1) { //ref dynamic
			SkipNBytes(8);
		}
		else if (saveType == 2) { //baseform Dynamic
			SkipNBytes(4);
		}
		else if (saveType == 3) { //Instances
			SkipNBytes(6);
		}

	}

	Instance* InstanceLoadManager::LoadInstance(ExtendedBaseType* baseInstance) {
		switch (baseInstance->extendedType) {
		case 40:
			return LoadInstanceDataWEAP((StaticInstance_WEAP*)baseInstance);
			break;
		case 222:
			return LoadInstanceDataAkimbo((StaticInstance_Akimbo*)baseInstance);
			break;
		default:
			return LoadInstanceData((StaticInstance*)baseInstance);
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

	Instance* InstanceLoadManager::LoadInstanceData(StaticInstance* baseInstance) {

		UInt16 instID = ReadRecord16();

		UInt32 length = ReadRecord32();
		char* key = new char[length + 1];
		ReadRecordData(key, length);
		key[length] = '\0';

		return baseInstance->loadInstance(instID, key);

	}

	void InstanceLoadManager::LoadInstanceData_Skip() {

		SkipNBytes(2);
		UInt32 length = ReadRecord32();
		SkipNBytes(length);
		return;

	}

	Instance_WEAP* InstanceLoadManager::LoadInstanceDataWEAP(StaticInstance_WEAP* baseInstance) {

		UInt16 instID = ReadRecord16();

		UInt32 length = ReadRecord32();
		char* key = new char[length + 1];
		ReadRecordData(key, length);
		key[length] = '\0';

		Instance_WEAP* inst = (Instance_WEAP*)baseInstance->loadInstance(instID, key);

		//Deduce later

		UInt32 size = ReadRecord32();
		for (UInt32 i = 0; i < size; ++i) {

			length = ReadRecord32();
			char* slot = new char[length + 1];
			SaveData::ReadRecordData(slot, length);
			slot[length] = '\0';

			LoadDataLink attachment = LoadLink();
			TESForm* form = deduceLinkToForm(&attachment);
			if (form) {
				inst->aAttachments[slot] = form->refID;
			}
			else { //Will happen when attachments are instances and attached to other instances.
				failedLinks_WEAP.emplace_back(new FailedWeaponLink{ inst, slot, attachment });
			}

		}

		return inst;

	}

	void InstanceLoadManager::LoadInstanceDataWEAP_Skip() {

		SkipNBytes(2);
		UInt32 length = ReadRecord32();
		SkipNBytes(length);

		UInt32 size = ReadRecord32();
		for (UInt32 i = 0; i < size; ++i) {

			UInt32 length = ReadRecord32();
			SkipNBytes(length);
			LoadLink_Skip();

		}

		return;

	}

	Instance_Akimbo* InstanceLoadManager::LoadInstanceDataAkimbo(StaticInstance_Akimbo* baseInstance) {

		UInt16 leftID = ReadRecord16();	//Fetch static instance using these
		UInt16 rightID = ReadRecord16();

		//Deduce later
			Instance_WEAP* right = (Instance_WEAP*)baseInstance->right->parent->LookupInstanceByID(leftID);
			Instance_WEAP* left = (Instance_WEAP*)baseInstance->left->parent->LookupInstanceByID(rightID);

		UInt16 instID = ReadRecord16();

		UInt32 length = ReadRecord32();
		char* key = new char[length + 1];
		ReadRecordData(key, length);
		key[length] = '\0';

		ExtraDataList* xDataLeft = LoadExtraDataList();
		ExtraDataList* xDataRight = LoadExtraDataList();

		Instance_Akimbo* inst = baseInstance->loadInstance(instID, right, left, xDataRight, xDataLeft, key);

		if (!right || !left) {
			failedLinks_Akimbo.emplace_back(new FailedAkimboLink(inst, leftID, rightID));
		}

		return inst;

	}

	void InstanceLoadManager::LoadInstanceDataAkimbo_Skip() {

		SkipNBytes(6);
		UInt32 length = ReadRecord32();
		SkipNBytes(length);
		LoadExtraDataList_Skip();
		LoadExtraDataList_Skip();

		return;

	}

	void InstanceLoadManager::LoadInstanceLinksAkimbo() {

		for (FailedAkimboLink* failedData : failedLinks_Akimbo) {

			StaticInstance_Akimbo* baseInstance = (StaticInstance_Akimbo*)failedData->weap->baseInstance;
			failedData->weap->right = (Instance_WEAP*)baseInstance->left->parent->LookupInstanceByID(failedData->leftID);
			failedData->weap->left = (Instance_WEAP*)baseInstance->right->parent->LookupInstanceByID(failedData->rightID);

			if (!failedData->weap->right || !failedData->weap->left) {
				delete failedData->weap; //Delete akimbo
			}
			delete failedData;
		}

	}

	void InstanceLoadManager::LoadInstanceLinksWEAP() {

		for (FailedWeaponLink* failedData : failedLinks_WEAP) {
			TESForm* attachment = deduceLinkToForm(&failedData->attachment);
			if (attachment) {
				failedData->weap->aAttachments[failedData->slot] = attachment->refID;
			}
			else {
				Console_Print("Attachment uninstalled %s", failedData->attachment.parent->GetEditorID());
			}
			delete failedData;
		}

	}

	TESForm* InstanceLoadManager::deduceLinkToForm(LoadDataLink* link) {

		if (link->parent && link->instID != InvalidInstID) { //Is an instance baseform

			return link->parent->LookupInstanceByID(link->instID)->clone;

		}
		else if (link->parent && !link->refID) { //Is a dynamic baseform

			return link->parent;

		}
		else if (link->parent && link->refID) { //Is a dynamic worldRef

			return LoadFromRefID(link->refID);

		}
		else if (link->refID) { //Vanilla

			UInt32* result;
			if (ResolveRefID(link->refID, result)) {
				return LookupFormByRefID(*result);
			};


		}

		return nullptr;

	}

	void InstanceLoadManager::LoadRefsList(TESForm* baseform) {

		UInt32 size = ReadRecord32();
		for (UInt32 i = 0; i < size; ++i) {
			LoadRef(baseform);
		}

	}

	void InstanceLoadManager::LoadRef(TESForm* baseform) {

		UInt8 type = ReadRecord8();
		if (type == 0) {						//Is link ref
			//linkRefs.emplace_back(LoadLink());	//Not used
		}
		else if (type == 1){	//Is entry
			LoadEntryRef(baseform);
		}
		else if (type == 2) {	//Is World ref
			LoadWorldRef(baseform);
		}

	}

	void InstanceLoadManager::LoadEntryRef(TESForm* baseform) {

		//logOperation("Save SaveDataEntry&");
		entries.emplace_back(new LoadDataEntry(baseform, LoadLink(), LoadExtraDataList()));

	}

	void InstanceLoadManager::LoadWorldRef(TESForm* baseform) {

		worldRefs.emplace_back(new LoadDataWorldREFR(baseform, LoadLink(), LoadExtraDataList(), ReadRecord32(),
			ReadRecordFloat(), ReadRecordFloat(), ReadRecordFloat(),
			ReadRecordFloat(), ReadRecordFloat(), ReadRecordFloat()));

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
		LoadLink_Skip(), LoadExtraDataList_Skip();
	}

	void InstanceLoadManager::LoadWorldRef_Skip() {
		LoadLink_Skip(), LoadExtraDataList_Skip();
		SkipNBytes(4 * 7);
	}

	void InstanceLoadManager::PlaceWorldRefs() {

		for (LoadDataWorldREFR* ref : worldRefs) {
			/*
			queueToSpawn.emplace_back(
				ref.baseform,
				deduceLinkToForm(&ref.location),
				ref.xData,
				ref.x, ref.y, ref.z,
				ref.xR, ref.yR, ref.zR
			);
			*/

			TESForm* location = deduceLinkToForm(&ref->location);
			if (!location || !ref->baseform) {
				return;
			}

			TESObjectREFR* refr = ThisCall<TESObjectREFR*>(0x55A2F0, Game_HeapAlloc<TESObjectREFR>());
			refr->baseForm = ref->baseform;
			refr->SetRefID(GetNextFreeFormID(), true);
			refr->MoveToCell(location, refr->posX, refr->posY, refr->posZ, refr->rotX, refr->rotY, refr->rotZ);
			ExtraDataList::CopyItemData(ref->xData, 1, &refr->extraDataList);
			refLocations[ref->worldRefID] = refr;

			delete ref;

		}

	}

	void InstanceLoadManager::PlaceEntryRefs() {

		for (LoadDataEntry* ref : entries) {

			TESForm* location = deduceLinkToForm(&ref->location);
			if (!location || !ref->baseform) {
				return;
			}

			if (ref->xData && ref->xData->HasType(kExtraData_Worn)) {
				ref->xData->RemoveByType(kExtraData_Worn);
				((Actor*)location)->AddItem(ref->baseform, ref->xData, 1);
				((Actor*)location)->SilentEquip(ref->baseform, ref->xData);
			}
			else {
				((TESObjectREFR*)location)->AddItem(ref->baseform, ref->xData, 1);
			}

			delete ref;

		}

	}

	void InstanceLoadManager::LinkRefs() {

		for (LoadDataREFR* ref : linkRefs) {
			//Not used
		}

	}

	void InstanceLoadManager::loadAll() {

		//SaveData::logOperation("Loading Forms:::::::::::::::::::::::::::::::::::::::::");
		UInt32 size = ReadRecord32();

		for (UInt32 i = 0; i < size; ++i) {
			LoadStaticLocations(i);
		}

		//SaveData::logOperation("Load instanceList::::::::::::::::::::::::::::::::::");
		size = ReadRecord32();

		for (UInt32 i = 0; i < size; ++i) {

			UInt32 extendedType = ReadRecord32();
			ExtendedBaseType* baseInstance = LoadBase(extendedType);

			if (baseInstance) {

				UInt32 instSize = ReadRecord32();

				for (UInt32 j = 0; j < instSize; ++j) {

					Instance* inst = LoadInstance(baseInstance);
					if (inst) {
						LoadRefsList(inst->clone);
					}
					else {
						LoadRefsList_Skip();
					}


				}

				//if worldRefs exist, that also happen to be dirived from dynamics.
				if (extendedType <= 120) {
					LoadRefsList(((StaticInstance*)baseInstance)->parent);
				}
				else {
					UInt32 size = SaveData::ReadRecord32();
				}

			}
			else {

				UInt32 instSize = ReadRecord32();

				for (UInt32 j = 0; j < instSize; ++j) {

					LoadInstance_Skip(extendedType);
					LoadRefsList_Skip();

				}

				if (extendedType <= 120) {
					LoadRefsList_Skip();
				}
				else {
					UInt32 size = SaveData::ReadRecord32();
				}

			}



		}//All Instances have been built

		LoadInstanceLinksWEAP();
		LoadInstanceLinksAkimbo();

		//logOperation("Loading dynamic refs::::::::::::::::::::::::::::::::::");
		size = ReadRecord32();

		for (UInt32 i = 0; i < size; ++i) {	//Iterate through left over dynamics

			UInt32 extendedType = ReadRecord32();
			ExtendedBaseType* baseInstance = LoadBase(extendedType);

			if (baseInstance) {
				if (extendedType <= 120) {
					LoadRefsList(((StaticInstance*)baseInstance)->parent); //Dynamic World Refs
				}
				else {
					UInt32 size = ReadRecord32();
				}
			}
			else {
				if (extendedType <= 120) {
					LoadRefsList_Skip(); //Skip Dynamic World Refs
				}
				else {
					UInt32 size = ReadRecord32();
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

	void InstanceLoadManager::LoadExtraDataList_Skip()
	{

		UInt8 type = SaveData::ReadRecord8();

		if (type == kXData_ExtraUnknown00) {
			return;
		}

		while (type != kXData_ExtraUnknown00) {

			switch (type) {
				case kXData_ExtraOwnership:
				case kXData_ExtraCount:
				case kXData_ExtraHealth:
				{
					SkipNBytes(4);
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