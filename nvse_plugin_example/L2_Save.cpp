#pragma once
#include "SaveSystem.h"

/*
namespace std {

	template<>
	struct hash<SaveSystem::SaveDataLink> {
		std::size_t operator()(const SaveSystem::SaveDataLink& link) const {
			return std::hash<TESForm*>()(link.parent);
		}
	};

}
*/
// Force instantiation of the template specializations
//template struct std::hash<SaveSystem::SaveDataLink>;
//template struct std::hash<SaveSystem::SaveDataREFR>;

namespace SaveSystem {

	InstanceSaveManager SaveLoadManager::saveManager;

	void SaveGameCallback(void*) {
		SaveData::clearLog();
		SaveData::OpenRecord(1, 2);
		SaveLoadManager::saveManager.saveAll();
		SaveLoadManager::saveManager.reset();
	}

	void InstanceSaveManager::reset() {
		queue.searchLocation.clear();
		queue.locations.clear();
		queue.instanceRefs.clear();
		queue.dynamicsRefs.clear();
	}

	void SaveDataLink::forceQueueToSave(TESForm* form) {
		SaveLoadManager::saveManager.queue.queueToSave(form);
	}

	void InstanceSaveManager::Save(Kit::KitData* thisObj) {

		WriteRecordString(thisObj->name);
		WriteRecord32(thisObj->index);
		WriteRecord32(thisObj->version);
		WriteRecord8(thisObj->safeUninstall);

	}

	void InstanceSaveManager::saveAll() {

		WriteRecord32(Kit::loadedKitFiles.size());

		for (auto it : Kit::loadedKitFiles) {
			Save(&it.second);
		}

		queue.queueAllWithSaveBehavior();

		logOperation("Saveing Forms:::::::::::::::::::::::::::::::::::::::::" + std::to_string(queue.locations.size()));
		WriteRecord32(queue.locations.size());

		for (UInt32 ref : queue.locations) {
			Save(LookupFormByRefID(ref));
		}

		logOperation("Saveing instanceList::::::::::::::::::::::::::::::::::" + std::to_string(queue.instanceRefs.size()));
		WriteRecord32(queue.instanceRefs.size());

		for (auto& extendedTypes : queue.instanceRefs) {

			WriteRecord32(extendedTypes.first); //Type
			WriteRecord32(extendedTypes.second.size());

			for (auto& [baseInstance, instanceList] : extendedTypes.second) {

				std::string printEditor = ((StaticInstance*)baseInstance)->parent->GetEditorID();
				logOperation("Saveing static instance.........................." + printEditor);

				SaveBase(baseInstance);

				logOperation("Saveing instances.................count: " + std::to_string(instanceList.size()));
				WriteRecord32(instanceList.size());

				for (auto& [inst, refList] : instanceList) {

					printEditor = inst->clone->GetEditorID();
					logOperation("Saveing instance.................EditorID: " + printEditor);

					WriteRecord32(inst->modIndex);
					SaveInstance(inst);
					logOperation("Saveing instance refs...........count: " + std::to_string(refList.size()));
					SaveRefsList(refList);

				}

				if (extendedTypes.first <= 120) { //Only vanilla base types can be placed directly in the world without instancing.
					auto it = queue.dynamicsRefs.find(baseInstance);
					if (it != queue.dynamicsRefs.end()) { //if worldRefs exist, that also happen to be dirived from dynamics.

						logOperation("Saveing dynamics instance mix::::::::::::::::::::::::::::::::::" + std::to_string(queue.dynamicsRefs[baseInstance].size()));

						SaveRefsList(it->second);
						queue.dynamicsRefs.erase(it);	//Erase so they don't get saved again.

					}
					else {
						WriteRecord32(0);
					}
				}

			}

		}

		logOperation("Saveing dynamic refs::::::::::::::::::::::::::::::::::" + std::to_string(queue.dynamicsRefs.size()));
		WriteRecord32(queue.dynamicsRefs.size());

		for (auto& [baseInstance, refList] : queue.dynamicsRefs) {	//Iterate through left over dynamics

			WriteRecord32(baseInstance->extendedType);

			if (baseInstance->extendedType <= 120) { //Only vanilla statics can be placed directly in the world.

				SaveBase(baseInstance);

				logOperation("Saveing pure dynamic refs............" + std::to_string(refList.size()));
				SaveRefsList(refList); //Dynamic World Refs

			}

		}

	}

	void InstanceSaveManager::SaveRefsList(const std::unordered_set<SaveDataREFR*>& refList) {

		WriteRecord32(refList.size());
		for (const auto& ref : refList) {
			Save(*ref);
		}

	}

	void InstanceSaveManager::SaveBase(ExtendedBaseType* thisObj) {
		switch (thisObj->extendedType) {
		case 222:
			Save(static_cast<StaticInstance_Akimbo*>(thisObj));
			break;
		default:
			Save(static_cast<StaticInstance*>(thisObj));
			break;
		}
	}

	void InstanceSaveManager::SaveInstance(Instance* thisObj) {
		switch (thisObj->baseInstance->extendedType) {
		case 40:
			Save(static_cast<Instance_WEAP*>(thisObj));
			break;
		case 222:
			Save(static_cast<Instance_Akimbo*>(thisObj));
			break;
		default:
			Save(thisObj);
			break;
		}
	}

	void InstanceSaveManager::Save(StaticInstance* thisObj) {

		logOperation("Save StaticInstance*");
		SaveLink({ thisObj->parent, nullptr });

	}
	void InstanceSaveManager::Save(StaticInstance_Akimbo* thisObj) {

		logOperation("Saveing StaticInstance_Akimbo*");

		if (!thisObj->left || !thisObj->right || thisObj->aInstances.empty()) {
			return;
		}

		//We assume there aren't any instances of instances right now.
		SaveLink({ thisObj->right, nullptr });

		//We assume there aren't any instances of instances right now.
		SaveLink({ thisObj->left, nullptr });

	}

	void InstanceSaveManager::SaveLifecycle(LifecycleManager& thisObj) {

		logOperation("Saving Lifecycle");

		WriteRecord8(thisObj.getPolicies());
		WriteRecordFloat(thisObj.getLifeTime());

	}

	void InstanceSaveManager::Save(Instance* thisObj) {

		logOperation("Saving Instance* InstID below");

		WriteRecord16(thisObj->InstID);
		WriteRecordString(thisObj->key);

		SaveLifecycle(thisObj->lifecycle);

	}

	void InstanceSaveManager::Save(Instance_WEAP* thisObj) {

		logOperation("Saveing Instance_WEAP*");

		Save((Instance*)thisObj); //Save regulare instance data

		WriteRecord32(thisObj->aAttachments.size());

		for (const auto& pair : thisObj->aAttachments) {
			
			UInt32 refID = pair.second;
			if (refID) {

				const std::string& slot = pair.first;

				WriteRecordString(slot);

				TESForm* attachment = LookupFormByRefID(refID);

				if (attachment) {
					if (Instance* worldInstance = attachment->pLookupInstance()) {
						attachment = ((StaticInstance*)worldInstance->baseInstance)->parent;
						SaveLink({ attachment, worldInstance });
						continue;
					}
					else if (StaticInstance* staticWorldInstance = attachment->LookupStaticInstance()) {
						attachment = staticWorldInstance->parent;
					}
					SaveLink({ attachment, nullptr });

				}

			}
		}

	}
	void InstanceSaveManager::Save(Instance_Akimbo* thisObj) {

		SaveData::logOperation("Save Instance_Akimbo*");

		Save(static_cast<Instance*>(thisObj)); //Saves regular instance data

		TESObjectREFR* right = static_cast<TESObjectREFR*>(LookupFormByRefID(thisObj->right));
		TESObjectREFR* left = static_cast<TESObjectREFR*>(LookupFormByRefID(thisObj->left));

		SaveLink({ right->GetBaseObject(), right->baseForm->pLookupInstance()});
		SaveLink({ left->GetBaseObject(), left->baseForm->pLookupInstance() });

		SaveExtraData(&right->extraDataList);
		SaveExtraData(&left->extraDataList);

	}

	void InstanceSaveManager::Save(const SaveDataREFR& thisObj) {

		logOperation("Save SaveDataREFR&");
		WriteRecord8(thisObj.type);
		SaveLink(thisObj.location);
		if (thisObj.type == 0) {
			delete& thisObj;
			return;
		}
		Save(static_cast<const SaveDataEntry&>(thisObj));

	}

	void InstanceSaveManager::Save(const SaveDataEntry& thisObj) {

		logOperation("Save SaveDataEntry&");
		SaveExtraData(thisObj.xData);
		WriteRecord32(thisObj.countDelta);
		if (thisObj.type == 1) {
			if (thisObj.instance) {
				RestoreEntryRefs(thisObj);
			}
			delete& thisObj;
			return;
		}
		Save(static_cast<const SaveDataWorldREFR&>(thisObj));

	}

	void InstanceSaveManager::RestoreEntryRefs(const SaveDataEntry& thisObj) {

		TESForm* location = LookupFormByRefID(thisObj.location.parentID);
		if (!location || !thisObj.instance || !thisObj.instance->clone) {
			return;
		}

		if (location->IsReference()) {

			TESObjectREFR* locationRef = (TESObjectREFR*)location;
			if (thisObj.xData && thisObj.xData->HasType(kExtraData_Worn)) {
				thisObj.xData->RemoveByType(kExtraData_Worn);
				((Actor*)locationRef)->AddItem(thisObj.instance->clone, thisObj.xData, thisObj.countDelta);
				((Actor*)locationRef)->SilentEquip(thisObj.instance->clone, thisObj.xData);
			}
			else {
				(locationRef)->AddItem(thisObj.instance->clone, thisObj.xData, thisObj.countDelta);
			}

		}

	}

	void InstanceSaveManager::Save(const SaveDataWorldREFR& thisObj) {

		logOperation("Save SaveDataWorldREFR&");
		WriteRecord32(thisObj.worldRefID);
		WriteRecordFloat(thisObj.x);
		WriteRecordFloat(thisObj.y);
		WriteRecordFloat(thisObj.z);
		WriteRecordFloat(thisObj.xR);
		WriteRecordFloat(thisObj.yR);
		WriteRecordFloat(thisObj.zR);
		//RestoreWorldRefs(thisObj);
		delete& thisObj;
	}

	void InstanceSaveManager::RestoreWorldRefs(const SaveDataWorldREFR& thisObj) {

		if (!thisObj.location.parentID || !thisObj.instance || !thisObj.instance->clone) {
			return;
		}

		//TESObjectREFR* refr = ThisCall<TESObjectREFR*>(0x55A2F0, Game_HeapAlloc<TESObjectREFR>());
		//refr->baseForm = thisObj.instance->clone;
		TESObjectREFR* refr = thisObj.instance->clone->PlaceAtCellAlt(LookupFormByRefID(thisObj.location.parentID), thisObj.x, thisObj.y, thisObj.z, thisObj.xR, thisObj.yR, thisObj.zR, thisObj.xData);
		//ExtraDataList::CopyItemData(thisObj.xData, 1, &refr->extraDataList);

	}

	void InstanceSaveManager::Save(TESForm* thisObj) {

		const char* EditorID = thisObj->GetEditorID();
		WriteRecordString(EditorID);
		WriteRecord8(thisObj->typeID);

	}

	void InstanceSaveManager::SaveLinkType(UInt8 type, const UInt32* ID32, const UInt16* ID16) {

		if (ID32 || ID16) {
			WriteRecord8(type);

			if (ID32) {
				WriteRecord32(*ID32);
			}
			if (ID16) {
				WriteRecord16(*ID16);
			}

		}

	}

	void InstanceSaveManager::SaveLink(const SaveDataLink& thisObj) {
		TESForm* parent = LookupFormByRefID(thisObj.parentID);
		if (parent->IsReference()) { //Is reference

			TESObjectREFR* parentRef = static_cast<TESObjectREFR*>(parent);

			if (!parentRef->baseForm->pIsDynamicForm()) { //ref vanilla
				SaveLinkType(1, &parentRef->refID);
			}
			else {										//ref dynamic
				SaveLinkType(3, &parentRef->refID);
				WriteRecord32(*GetSaveIndex(parentRef->baseForm->refID));
			}

		}
		else if (!thisObj.instance) { //no instance

			if (!parent->pIsDynamicForm()) { //baseform vanilla
				SaveLinkType(2, &thisObj.parentID);
			}
			else {									//baseform dynamic
				SaveLinkType(4, GetSaveIndex(thisObj.parentID));
			}

		}
		else if (thisObj.instance){ //Instances

			if (!parent->pIsDynamicForm()) { //vanilla with instance
				SaveLinkType(5, &thisObj.parentID, &thisObj.instance->InstID);
			}
			else {									//dynamic with instance
				SaveLinkType(6, GetSaveIndex(thisObj.parentID), &thisObj.instance->InstID);
			}

		}

	}

	UInt32* InstanceSaveManager::GetSaveIndex(UInt32 thisObj) {

		auto it = queue.searchLocation.find(thisObj);
		if (it != queue.searchLocation.end()) {
			return &it->second;
		}

		const char* EditorID = LookupFormByRefID(thisObj)->GetEditorID();
		std::string editorIDStr = EditorID ? EditorID : "Unknown EditorID";
		SaveData::logOperation("SAVE Error, GetSaveIndex failed to find form: " + editorIDStr);

		return nullptr;
	}

	void InstanceSaveManager::SaveQueue::queueAllWithSaveBehavior() {

		for (auto& typeMap : StaticLinker) {
			for (auto& pair : typeMap.second) {
				StaticInstance* staticInst = pair.second;
				if (staticInst) {

					switch (typeMap.first) {	//TypeID
					case 40:  // Weapon
						queueToSave(staticInst);
						break;
					default:
						queueToSave(staticInst);
						break;
					}

				}
			}
		}

		for (auto& akimboMap : AkimboSets) {
			for (auto& akimboPair : akimboMap.second) {
				StaticInstance_Akimbo* akimbo = akimboPair.second;
				if (akimbo) {
					queueToSave(akimbo);
				}
			}
		}
	}

	void InstanceSaveManager::SaveQueue::queueToSave(ExtendedBaseType* thisObj) {

		for (auto it = thisObj->aInstances.begin(); it != thisObj->aInstances.end(); ++it) {
			Instance* inst = *it;
			if (inst && !inst->lifecycle.isPolicyEnabled(LifecycleManager::Recycle)) {
				queueToSave(inst, 0); //Only queue objects with saveBehavior
			}
		}

	}

	bool InstanceSaveManager::SaveQueue::queueToSave(Instance* instance, bool forceQueue) {

		if (!forceQueue && !instance->lifecycle.saveBehavior) {
			return false;
		}

		switch (instance->baseInstance->extendedType) {
		case 40:  // Weapons
			queueToSaveWEAP(static_cast<Instance_WEAP*>(instance));
			break;
		case 222:  // Akimbo
			queueToSaveAkimbo(static_cast<Instance_Akimbo*>(instance));
			break;
		default:
			instanceRefs[instance->baseInstance->extendedType][instance->baseInstance].insert({ instance, std::unordered_set<SaveDataREFR*>() });
			break;
		}

		return true;

	}

	void InstanceSaveManager::SaveQueue::queueToSaveWEAP(Instance_WEAP* thisObj) {

		for (auto slot = thisObj->aAttachments.begin(); slot != thisObj->aAttachments.end(); ++slot) {

			UInt32 refID = slot->second;
			if (refID) {
				TESForm* attachment = LookupFormByRefID(refID);
				if (attachment) {

					//Think more about this
					TESForm* containerForm = thisObj->baseInstance->parent;
					if (Instance* worldInstance = attachment->pLookupInstance()) {
						queueToSave(worldInstance, 0);
					}
					else if (StaticInstance* staticWorldInstance = attachment->LookupStaticInstance()){
						queueToSave(staticWorldInstance->parent);
					}

				}
			}
		}

		instanceRefs[thisObj->baseInstance->extendedType][thisObj->baseInstance].insert({ thisObj, std::unordered_set<SaveDataREFR*>() });

	}

	void InstanceSaveManager::SaveQueue::queueToSaveAkimbo(Instance_Akimbo* thisObj) {

		if (!thisObj->left || !thisObj->right) {
			return;
		}

		TESForm* left = LookupFormByRefID(thisObj->left);
		TESForm* right = LookupFormByRefID(thisObj->right);
		if (!left || !right) {
			return;
		}

		if (!left->IsReference() || !right->IsReference()) {
			return;
		}

		TESObjectREFR* leftRefr = static_cast<TESObjectREFR*>(left);
		TESObjectREFR* rightRefr = static_cast<TESObjectREFR*>(right);

		if (!queueToSaveRefDirect(leftRefr) || !queueToSaveRefDirect(rightRefr)) {
			return;
		}

		instanceRefs[thisObj->baseInstance->extendedType][thisObj->baseInstance].insert({ thisObj, std::unordered_set<SaveDataREFR*>()});

	}

	//Instance
	void InstanceSaveManager::SaveQueue::queueToSaveRef(Instance* instance, SaveDataREFR* worldRef) {
		queueToSave(instance, 0);
		if (isQueueToSave(instance)) {
			if (instance->baseInstance->parent) {
				queueToSave(instance->baseInstance->parent);
			}
			instanceRefs[instance->baseInstance->extendedType][instance->baseInstance][instance].insert(worldRef);
		}
	}

	//Only used for instance world refs
	bool InstanceSaveManager::SaveQueue::isQueueToSave(Instance* instance) {
		return instanceRefs[instance->baseInstance->extendedType][instance->baseInstance].find(instance) != instanceRefs[instance->baseInstance->extendedType][instance->baseInstance].end();
	}

	//Dynamics
	void InstanceSaveManager::SaveQueue::queueToSaveRef(ExtendedBaseType* base, SaveDataREFR* worldRef) {

		if (!base->baseLifecycle.saveBehavior) {
			return;
		}
		if(base->parent) {
			queueToSave(base->parent);
		}

		dynamicsRefs[base].insert(worldRef);
	}

	bool InstanceSaveManager::SaveQueue::queueToSave(TESForm* thisObj) {	//Saves static baseforms

		if (thisObj->IsInstancedForm()) {
			return queueToSave(thisObj->pLookupInstance(), 0);
		}
		else if (thisObj->pIsDynamicForm()) {

			auto it = searchLocation.find(thisObj->refID);
			if (it == searchLocation.end()) {
				//if (locations.empty) {
					UInt32 newIndex = locations.size();
					locations.push_back(thisObj->refID);
					searchLocation[thisObj->refID] = newIndex;
				//}
				return true;
			}

		}

		return false;

	}

	//Saves a reference that is stored directly. References that are not in the world, and not in a container.
	bool InstanceSaveManager::SaveQueue::queueToSaveRefDirect(TESObjectREFR* thisObj) {

		if (thisObj->baseForm->IsInstancedForm() || thisObj->baseForm->pIsDynamicForm()) {

			if (Instance* thisInstance = thisObj->baseForm->pLookupInstance()) {
				if (thisInstance->lifecycle.isPolicyEnabled(LifecycleManager::OnUnload)) {
					return false;
				}
				return queueToSave(thisInstance, 0);

			}
			else if (StaticInstance* thisStaticInstance = thisObj->baseForm->LookupStaticInstance()) {

				if (thisStaticInstance->baseLifecycle.isPolicyEnabled(LifecycleManager::OnUnload)) {
					return false;
				}

				return queueToSave(thisStaticInstance->parent);

			}

		}
		return true;
	}

	TESObjectREFR* InstanceSaveManager::SaveQueue::queueToSave(TESObjectREFR* thisObj) {

		TESForm* containerForm = thisObj;
		Instance* thisInstance = nullptr;
		TESObjectREFR* vanillaSaveRef = thisObj; //The form to return to the vanilla save system.

		if (thisObj->baseForm->IsInstancedForm() || thisObj->baseForm->pIsDynamicForm()) {

			if (thisInstance = thisObj->baseForm->pLookupInstance()) {
				if (thisInstance->lifecycle.isPolicyEnabled(LifecycleManager::OnUnload) && !thisObj->GetInSameCellOrWorld(*g_thePlayer)) {
					return nullptr;
				}

				ExtraDataList* xData = &thisObj->extraDataList;
				SInt16 count = 1;
				if (xData) {
					if (xData->HasType(kExtraData_Count)) {
						ExtraCount* xCount = (ExtraCount*)xData->GetByType(kExtraData_Count);
						count = xCount->count;
					}
				}

				containerForm = thisObj->baseForm;
				//We assume there will be no instances of cells/worldspaces.
				if (TESForm* location = thisObj->GetLocation()) {
					queueToSaveRef(thisInstance, new SaveDataWorldREFR{ thisObj->refID, location, nullptr, thisInstance, xData, count,
						thisObj->posX, thisObj->posY, thisObj->posZ, thisObj->rotX, thisObj->rotY, thisObj->rotZ });
				}
				else {
					Console_Print("Error, location null for saving ref: %s", thisObj->baseForm->GetEditorID());
				}
				//vanillaSaveRef->DeleteReference(); //Try clearing the cell buffer instead.
				vanillaSaveRef = nullptr;

			}
			else if (StaticInstance* thisStaticInstance = thisObj->baseForm->LookupStaticInstance()) {

				if (thisStaticInstance->baseLifecycle.isPolicyEnabled(LifecycleManager::OnUnload) && !thisObj->GetInSameCellOrWorld(*g_thePlayer)) {
					return nullptr;
				}

				ExtraDataList* xData = &thisObj->extraDataList;
				SInt16 count = 1;
				if (xData) {
					if (xData->HasType(kExtraData_Count)) {
						ExtraCount* xCount = (ExtraCount*)xData->GetByType(kExtraData_Count);
						count = xCount->count;
					}
				}

				//We assume there will be no instances of cells/worldspaces.
				if (TESForm* location = thisObj->GetLocation()) {
					queueToSaveRef(thisStaticInstance, new SaveDataWorldREFR{ thisObj->refID, location, nullptr, nullptr, xData, count,
						thisObj->posX, thisObj->posY, thisObj->posZ, thisObj->rotX, thisObj->rotY, thisObj->rotZ });
				}
				else {
					Console_Print("Error, location null for saving ref: %s", thisObj->baseForm->GetEditorID());
				}
				//vanillaSaveRef->DeleteReference(); //Try clearing the cell buffer instead.
				vanillaSaveRef = nullptr;

			}

		}

		ContChangesEntryList* entryList = thisObj->GetContainerChangesList();

		if (!entryList) return vanillaSaveRef;  // Not a container

		ListNode<ExtraContainerChanges::EntryData>* curr = entryList->Head(), * prev = NULL;
		do
		{

			ContChangesEntry* entry = curr->data;
			if (!entry) {
				curr = curr->next; // Move to next before continuing to skip null entries
				continue;
			}

			// Skip entries not marked as instanced or dynamic forms
			if (!(entry->type->IsInstancedForm() || entry->type->pIsDynamicForm())) {
				curr = curr->next;
				continue;
			}

			Instance* entryInstance = entry->type->pLookupInstance();
			StaticInstance* staticEntryInstance = entry->type->LookupStaticInstance();

			// Skip based on lifecycle policies and player proximity
			if ((entryInstance && entryInstance->lifecycle.isPolicyEnabled(LifecycleManager::OnUnload) && !thisObj->GetInSameCellOrWorld(*g_thePlayer)) ||
				(staticEntryInstance && staticEntryInstance->baseLifecycle.isPolicyEnabled(LifecycleManager::OnUnload) && !thisObj->GetInSameCellOrWorld(*g_thePlayer))) {
				curr = curr->next;
				continue;
			}

			if (entry->extendData) {

				for (auto xData : *entry->extendData) {

					ExtraDataList* xDataSave = xData ? xData->CreateCopy() : nullptr;
					SInt16 count = 1;
					if (xDataSave) {

						if (xData->HasType(kExtraData_Count)) {
							ExtraCount* xCount = (ExtraCount*)xData->GetByType(kExtraData_Count);
							count = xCount->count;
						}
						if (xData->HasType(kExtraData_Worn)) {
							((Actor*)thisObj)->UnequipItem(entry->type, count, xData, 1, 0, 0);  // Unequip item if worn
						}

					}
					if (entryInstance) {
						queueToSave(containerForm);
						queueToSaveRef(entryInstance, new SaveDataEntry{ containerForm, thisInstance, entryInstance, xDataSave, count });
					}
					else if (staticEntryInstance) {
						queueToSave(containerForm);
						queueToSaveRef(staticEntryInstance, new SaveDataEntry{ containerForm, thisInstance, nullptr, xDataSave, count });
					}

				}

				entry->extendData->RemoveAll(true);

			}
			else {
				// Handle static instances without extended data
				if (entryInstance) {
					queueToSave(containerForm);
					queueToSaveRef(entryInstance, new SaveDataEntry{ containerForm, thisInstance, entryInstance, nullptr, entry->countDelta });
				}
				else if (staticEntryInstance) {
					queueToSave(containerForm);
					queueToSaveRef(staticEntryInstance, new SaveDataEntry{ containerForm, thisInstance, nullptr, nullptr, entry->countDelta });
				}
			}

			curr = prev ? prev->RemoveNext() : curr->RemoveMe();

		} while (curr);

		return vanillaSaveRef;

	}

	void InstanceSaveManager::SaveExtraData(ExtraDataList* xDataList)
	{
		if (xDataList) {
			BSExtraData* extraData = xDataList->m_data;

			while (extraData) {

				switch (extraData->type) {
				case kXData_ExtraOwnership: {
					ExtraOwnership* xData = (ExtraOwnership*)extraData;
					if (xData)
					{
						WriteRecord8(extraData->type);
						WriteRecord32(xData->owner->refID);
					}
				}
				break;
				case kXData_ExtraCount: {

					ExtraCount* xData = (ExtraCount*)extraData;
					if (xData)
					{
						WriteRecord8(extraData->type);
						WriteRecord16(xData->count);
					}
				}
				break;
				case kExtraData_TimeLeft:
				{
					ExtraTimeLeft* xData = (ExtraTimeLeft*)extraData;
					if (xData)
					{
						WriteRecord8(extraData->type);
						WriteRecordFloat(xData->time);
					}
				}
				break;
				case kXData_ExtraHealth: {

					ExtraHealth* xData = (ExtraHealth*)extraData;
					if (xData)
					{
						WriteRecord8(extraData->type);
						WriteRecordFloat(xData->health);
					}
				}
				break;
				case kXData_ExtraHotkey: {

					ExtraHotkey* xData = (ExtraHotkey*)extraData;
					if (xData)
					{
						WriteRecord8(extraData->type);
						WriteRecord8(xData->index);
					}
				}
				break;
				case kXData_ExtraWorn: {

					ExtraWorn* xData = (ExtraWorn*)extraData;
					if (xData)
					{
						WriteRecord8(extraData->type);
					}
				}
				break;
				default:
					break;
				}

				extraData = extraData->next;

			}

		}

		WriteRecord8(kXData_ExtraUnknown00);

	}

}