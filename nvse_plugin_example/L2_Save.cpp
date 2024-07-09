#pragma once
#include "SaveSystem.h"


namespace std {

	template<>
	struct hash<SaveSystem::SaveDataLink> {
		std::size_t operator()(const SaveSystem::SaveDataLink& link) const {
			return std::hash<TESForm*>()(link.parent);
		}
	};

}

// Force instantiation of the template specializations
//template struct std::hash<SaveSystem::SaveDataLink>;
//template struct std::hash<SaveSystem::SaveDataREFR>;

namespace SaveSystem {

	InstanceSaveManager SaveLoadManager::saveManager;

	void SaveGameCallback(void*) {
		SaveData::clearLog();
		SaveData::OpenRecord(1, 2);
		SaveLoadManager::saveManager.saveAll();
		SaveLoadManager::reset();
	}

	void SaveLoadManager::reset(){
		SaveLoadManager::saveManager.reset();
	}

	void InstanceSaveManager::reset() {
		searchLocation.clear();
		locations.clear();
		instanceRefs.clear();
		dynamicsRefs.clear();
	}

	void SaveDataLink::forceQueueToSave(TESForm* form) {
		SaveLoadManager::saveManager.queueToSave(form);
	}

	void InstanceSaveManager::saveAll() {

		queueAllWithSaveBehavior();

		logOperation("Saveing Forms:::::::::::::::::::::::::::::::::::::::::" + std::to_string(locations.size()));
		WriteRecord32(locations.size());

		for (TESForm* form : locations) {

			Save(form);

		}

		logOperation("Saveing instanceList::::::::::::::::::::::::::::::::::" + std::to_string(instanceRefs.size()));
		WriteRecord32(instanceRefs.size());

		for (auto& [baseInstance, instanceList] : instanceRefs) {

			std::string printEditor = ((StaticInstance*)baseInstance)->parent->GetEditorID();
			logOperation("Saveing static instance.........................." + printEditor);

			WriteRecord32(baseInstance->extendedType);
			SaveBase(baseInstance);


			logOperation("Saveing instances.................count: " + std::to_string(instanceList.size()));
			WriteRecord32(instanceList.size());

			for (auto& [inst, refList] : instanceList) {

				printEditor = inst->clone->GetEditorID();
				logOperation("Saveing instance.................EditorID: " + printEditor);

				SaveInstance(inst);
				logOperation("Saveing instance refs...........count: " + std::to_string(refList.size()));
				SaveRefsList(refList);

			}

			auto it = dynamicsRefs.find(baseInstance);
			if (it != dynamicsRefs.end()) { //if worldRefs exist, that also happen to be dirived from dynamics.

				logOperation("Saveing dynamics instance mix::::::::::::::::::::::::::::::::::" + std::to_string(dynamicsRefs[baseInstance].size()));
				
				SaveRefsList(it->second);
				dynamicsRefs.erase(it);	//Erase so they don't get saved again.

			}
			else {
				WriteRecord32(0); //SaveRefsList size 0
			}

		}

		logOperation("Saveing dynamic refs::::::::::::::::::::::::::::::::::" + std::to_string(dynamicsRefs.size()));
		WriteRecord32(dynamicsRefs.size());

		for (auto& [baseInstance, refList] : dynamicsRefs) {	//Iterate through left over dynamics

			WriteRecord32(baseInstance->extendedType);
			SaveBase(baseInstance);

			logOperation("Saveing pure dynamic refs............" + std::to_string(refList.size()));
			SaveRefsList(refList); //Dynamic World Refs

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
		case 40:
			Save(static_cast<StaticInstance_WEAP*>(thisObj));
			break;
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

		//if (thisObj->aInstances.empty()) {
			//return;
		//}

		//SaveSystem::SaveData::WriteRecord32(thisObj->parent->typeID);

		//We assume there aren't any instances of instances right now.
		SaveLink({ thisObj->parent, nullptr });

	}
	void InstanceSaveManager::Save(StaticInstance_Akimbo* thisObj) {

		logOperation("Saveing StaticInstance_Akimbo*");

		if (!thisObj->left || !thisObj->right || thisObj->aInstances.empty()) {
			return;
		}

		//We assume there aren't any instances of instances right now.
		SaveLink({ thisObj->right->parent, nullptr });

		//We assume there aren't any instances of instances right now.
		SaveLink({ thisObj->left->parent, nullptr });

	}

	void InstanceSaveManager::Save(Instance* thisObj) {

		logOperation("Saving Instance* InstID below");

		WriteRecord16(thisObj->InstID);

		UInt32 length = thisObj->key.length();
		WriteRecord32(length);
		WriteRecordData(thisObj->key.c_str(), length);

	}

	void InstanceSaveManager::Save(Instance_WEAP* thisObj) {

		logOperation("Saveing Instance_WEAP*");

		Save((Instance*)thisObj);

		UInt32 length;
		WriteRecord32(thisObj->aAttachments.size());

		for (const auto& pair : thisObj->aAttachments) {
			
			UInt32 refID = pair.second;
			if (refID) {

				const std::string& slot = pair.first;

				length = slot.length();

				WriteRecord32(length);
				WriteRecordData(slot.c_str(), length);

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

		WriteRecord16(thisObj->left->InstID);	//Fetch static instance using these
		WriteRecord16(thisObj->right->InstID);

		Save((Instance*)thisObj);

		SaveExtraData(thisObj->xDataLeft);
		SaveExtraData(thisObj->xDataRight);

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
		if (thisObj.type == 1) {
			if (thisObj.instance) {
				RestorEntryRefs(thisObj);
			}
			delete& thisObj;
			return;
		}
		Save(static_cast<const SaveDataWorldREFR&>(thisObj));

	}

	void InstanceSaveManager::RestorEntryRefs(const SaveDataEntry& thisObj) {

		if (thisObj.location.parent->IsReference()) {

			TESObjectREFR* location = (TESObjectREFR*)thisObj.location.parent;
			if (thisObj.xData && thisObj.xData->HasType(kExtraData_Worn)) {
				thisObj.xData->RemoveByType(kExtraData_Worn);
				((Actor*)location)->AddItem(thisObj.instance->clone, thisObj.xData, 1);
				((Actor*)location)->SilentEquip(thisObj.instance->clone, thisObj.xData);
			}
			else {
				(location)->AddItem(thisObj.instance->clone, thisObj.xData, 1);
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
		delete& thisObj;
	}

	void InstanceSaveManager::Save(TESForm* thisObj) {

		const char* EditorID = thisObj->GetEditorID();
		UInt32 length = strlen(EditorID);
		WriteRecord32(length);
		WriteRecordData(EditorID, length);
		WriteRecord8(thisObj->typeID);

	}

	void InstanceSaveManager::SaveLink(const SaveDataLink& thisObj) {

		if (thisObj.parent->IsReference()) {

			TESObjectREFR* parent = (TESObjectREFR*)thisObj.parent;

			if (!parent->baseForm->pIsDynamicForm()) { //Vanilla
				logOperation("Saveing Link Vanilla:");
				WriteRecord8(0);
				WriteRecord32(thisObj.parent->refID);
			}
			else { //ref dynamic

				const char* EditorID = parent->baseForm->GetEditorID();
				std::string editorIDStr = EditorID ? EditorID : "Unknown EditorID";
				logOperation("Saving Link Dynamic: " + editorIDStr);

				WriteRecord8(1);
				WriteRecord32(GetSaveIndex(parent->baseForm));
				WriteRecord32(parent->refID);

			}


		}
		else if (!thisObj.instance) { //baseform Dynamic

			const char* EditorID = thisObj.parent->GetEditorID();
			std::string editorIDStr = EditorID ? EditorID : "Unknown EditorID";
			logOperation("Saving Link Dynamic: " + editorIDStr);

			WriteRecord8(2);
			WriteRecord32(GetSaveIndex(thisObj.parent));

		}
		else if (thisObj.instance){ //Instances

			const char* EditorID = thisObj.parent->GetEditorID();
			std::string editorIDStr = EditorID ? EditorID : "Unknown EditorID";
			logOperation("Saving Link Instance: " + editorIDStr);

			WriteRecord8(3);
			WriteRecord32(GetSaveIndex(thisObj.parent));

			logOperation("Saveing InstID:");
			WriteRecord16(thisObj.instance->InstID);

		}

	}

	UInt32 InstanceSaveManager::GetSaveIndex(TESForm* thisObj) {

		auto it = searchLocation.find(thisObj);
		if (it != searchLocation.end()) {
			return it->second;
		}

		const char* EditorID = thisObj->GetEditorID();
		std::string editorIDStr = EditorID ? EditorID : "Unknown EditorID";
		SaveData::logOperation("Error, GetSaveIndex failed to fine form: " + editorIDStr);

		return 0;
	}

	void InstanceSaveManager::queueAllWithSaveBehavior() {

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

	void InstanceSaveManager::queueToSave(ExtendedBaseType* thisObj) {

		for (auto it = thisObj->aInstances.begin(); it != thisObj->aInstances.end(); ++it) {
			Instance* inst = *it;
			if (inst) {
				queueToSave(inst, 0); //Only queue objects with saveBehavior
			}
		}

	}

	void InstanceSaveManager::queueToSave(Instance* instance, bool forceQueue) {

		if (!forceQueue && !instance->lifecycle.saveBehavior) {
			return;
		}

		switch (instance->baseInstance->extendedType) {
		case 40:  // Weapons
			queueToSave(static_cast<Instance_WEAP*>(instance));
		case 222:  // Akimbo
			queueToSave(static_cast<Instance_Akimbo*>(instance));
			break;
		default:
			instanceRefs[instance->baseInstance].insert({ instance, std::unordered_set<SaveDataREFR*>() });
			break;
		}


	}

	void InstanceSaveManager::queueToSave(Instance_WEAP* thisObj) {

		for (auto slot = thisObj->aAttachments.begin(); slot != thisObj->aAttachments.end(); ++slot) {

			UInt32 refID = slot->second;
			if (refID) {
				TESForm* attachment = LookupFormByRefID(refID);
				if (attachment) {

					TESForm* containerForm = ((StaticInstance*)thisObj->baseInstance)->parent;
					if (Instance* worldInstance = attachment->pLookupInstance()) {
						//queueToSaveRef(worldInstance, new SaveDataREFR(containerForm, thisObj));
						queueToSave(worldInstance, 0);
					}
					//else if (StaticInstance* staticWorldInstance = attachment->LookupStaticInstance()){
						//queueToSaveRef(staticWorldInstance, new SaveDataREFR(containerForm, thisObj));
					//}

				}
			}
		}

		instanceRefs[thisObj->baseInstance].insert({ thisObj, std::unordered_set<SaveDataREFR*>() });

	}

	void InstanceSaveManager::queueToSave(Instance_Akimbo* thisObj) {

		if (!thisObj->left || !thisObj->right) {
			return;
		}

		if (!thisObj->left->lifecycle.saveBehavior) {
			queueToSave(thisObj->left, 0);
			//queueToSaveRef(thisObj->left, new SaveDataREFR(thisObj->clone, thisObj));
		}

		if (!thisObj->right->lifecycle.saveBehavior) {
			queueToSave(thisObj->right, 0);
			//queueToSaveRef(thisObj->right, new SaveDataREFR(thisObj->clone, thisObj));
		}

		instanceRefs[thisObj->baseInstance].insert({ thisObj, std::unordered_set<SaveDataREFR*>() });

	}

	//Instance
	void InstanceSaveManager::queueToSaveRef(Instance* instance, SaveDataREFR* worldRef) {
		queueToSave(instance, 0);
		if (isQueueToSave(instance)) {
			instanceRefs[instance->baseInstance][instance].insert(std::move(worldRef));
		}
	}

	//Only used for instance world refs
	bool InstanceSaveManager::isQueueToSave(Instance* instance) {
		return instanceRefs[instance->baseInstance].find(instance) != instanceRefs[instance->baseInstance].end();
	}

	//Dynamics
	void InstanceSaveManager::queueToSaveRef(ExtendedBaseType* base, SaveDataREFR* worldRef) {

		if (!base->baseLifecycle.saveBehavior) {
			return;
		}
		dynamicsRefs[base].insert(std::move(worldRef));
	}

	void InstanceSaveManager::queueToSave(TESForm* thisObj) {	//Saves static baseforms

		if (thisObj->IsInstancedForm()) {
			queueToSave(thisObj->pLookupInstance(), 0);
			return;
		}

		auto it = searchLocation.find(thisObj);
		if (it == searchLocation.end()) {
			UInt32 newIndex = locations.size();
			locations.push_back(thisObj);
			searchLocation[thisObj] = newIndex;
		}

	}

	TESObjectREFR* InstanceSaveManager::queueToSave(TESObjectREFR* thisObj) {

		TESForm* containerForm = thisObj;
		Instance* thisInstance = nullptr;
		TESObjectREFR* vanillaSaveRef = thisObj; //The form to return to the vanilla save system.

		if (thisObj->baseForm->IsInstancedForm() || thisObj->baseForm->pIsDynamicForm()) {

			if (thisInstance = thisObj->baseForm->pLookupInstance()) {
				if (thisInstance->lifecycle.isPolicyEnabled(LifecycleManager::OnUnload) && !thisObj->GetInSameCellOrWorld(*g_thePlayer)) {
					return nullptr;
				}
				containerForm = thisObj->baseForm;
				//We assume there will be no instances of cells/worldspaces.
				InstanceSaveManager::queueToSaveRef(thisInstance, new SaveDataWorldREFR{ thisObj->refID, thisObj->GetLocation(), nullptr, thisInstance, thisObj->extraDataList.CreateCopy(),
					thisObj->posX, thisObj->posY, thisObj->posZ, thisObj->rotX, thisObj->rotY, thisObj->rotZ });

				vanillaSaveRef->DeleteReference(); //Try clearing the cell buffer instead.
				vanillaSaveRef = nullptr;

			}
			else if (StaticInstance* thisStaticInstance = thisObj->baseForm->LookupStaticInstance()) {

				if (thisStaticInstance->baseLifecycle.isPolicyEnabled(LifecycleManager::OnUnload) && !thisObj->GetInSameCellOrWorld(*g_thePlayer)) {
					return nullptr;
				}

				//We assume there will be no instances of cells/worldspaces.
				InstanceSaveManager::queueToSaveRef(thisStaticInstance, new SaveDataWorldREFR{ thisObj->refID, thisObj->GetLocation(), nullptr, nullptr, thisObj->extraDataList.CreateCopy(),
					thisObj->posX, thisObj->posY, thisObj->posZ, thisObj->rotX, thisObj->rotY, thisObj->rotZ });

				vanillaSaveRef->DeleteReference(); //Try clearing the cell buffer instead.
				vanillaSaveRef = nullptr;

			}

		}

		ContChangesEntryList* entryList = thisObj->GetContainerChangesList();

		if (entryList) {

			ContChangesEntry* entry;

			for (auto iter = entryList->Begin(); !iter.End(); ++iter) {

				entry = iter.Get();

				if (entry && (entry->type->IsInstancedForm() || entry->type->pIsDynamicForm())) {

					UInt32 objectsToRemove = entry->countDelta;
					while (objectsToRemove > 0) {

						ExtraDataList* xData = nullptr;
						ExtraDataList* xDataSave = nullptr;
						if (entry->extendData) {
							xData = entry->extendData->GetFirstItem();
							if (xData) {
								xDataSave = ExtraDataList::CopyItemData(xData, 0);
								if (xData->HasType(kExtraData_Worn)) {
									((Actor*)thisObj)->UnequipItem(entry->type, 1, xData, 1, 0, 0);
								}
							}
						}

						entry->Remove(xData, 1);
						objectsToRemove--;

						if (Instance* entryInstance = entry->type->pLookupInstance()) {
							if (entryInstance->lifecycle.isPolicyEnabled(LifecycleManager::OnUnload) && !thisObj->GetInSameCellOrWorld(*g_thePlayer)) {
								continue;
							}
							queueToSave(containerForm);
							InstanceSaveManager::queueToSaveRef(entryInstance, new SaveDataEntry{ containerForm, thisInstance, entryInstance, xDataSave });
						}
						else if (StaticInstance* staticEntryInstance = entry->type->LookupStaticInstance()) {
							if (staticEntryInstance->baseLifecycle.isPolicyEnabled(LifecycleManager::OnUnload) && !thisObj->GetInSameCellOrWorld(*g_thePlayer)) {
								continue;
							}
							queueToSave(containerForm);
							InstanceSaveManager::queueToSaveRef(staticEntryInstance, new SaveDataEntry{ containerForm, thisInstance, nullptr, xDataSave });
						}

					}

				}


			}

		}

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
						WriteRecord32(xData->count);
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