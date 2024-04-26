/*
void SetEquippedWeapon(Actor* rActor, TESForm* rWeap, ExtraDataList* xData) {

	Character* character = (Character*)rActor;
	//TESObjectWEAP* weapon = character->validBip01Names->slotData[5].weapon;
	TESObjectWEAP* weapon = (TESObjectWEAP*)rWeap;
	HighProcess* hiProc = (HighProcess*)character->baseProcess;
	if (ContChangesEntry* weapInfo = hiProc->GetWeaponInfo(); weapInfo && (weapInfo->type == weapon)) {
		Console_Print("Do Equip");
		hiProc->QueueEquipItem(character, 1, rWeap, 1, xData, 1, 0, 1, 0, 0);
		hiProc->HandleQueuedEquipItems(character);
	}

	character->RefreshAnimData();

}
*/
/*
int __fastcall HookActorInventorySave(TESObjectREFR* rContainer, void* edx, int a2)
{
	//Console_Print("Hook ran");

	if (rContainer) {

		//if (rContainer->IsReference() && rContainer->baseForm->typeID == 40 && rContainer->baseForm->IsInstancedForm()) {

			//Instance_WEAP* inst = rContainer->GetWeaponBase();
			//rContainer->baseForm = inst->Parent;
			//if (inst->Parent) {

				//ExtraDataList* xData = &rContainer->extraDataList;

				//std::string sString = "Cell: ";
				//sString += rContainer->parentCell->GetTheName();
				//sString += "\nItem: ";
				//sString += rContainer->GetTheName();
				//DumpInfoToLog(sString);

				//SaveSystem::SaveDataWorldObj* saveData = new SaveSystem::SaveDataWorldObj(inst->Parent->refID, inst->refID, rContainer->parentCell->refID, xData->CreateCopy(), rContainer->posX, rContainer->posY, rContainer->posZ, rContainer->rotZ, rContainer->rotY, rContainer->rotZ);
				//SaveSystem::aSaveData.push_back(saveData);

				//Console_Print("Delete ref: %s", rContainer->GetTheName());
				//rContainer->baseForm->flags |= TESObjectREFR::kFlags_Temporary;
				//rContainer->baseForm->flags |= TESObjectREFR::kFlags_Deleted;
				//rContainer->baseForm->flags |= TESObjectREFR::kFormFlags_DontSaveForm;
				//rContainer->parentCell->objectList.Remove(rContainer);

				//Console_Print("Tried to Remove: %s", rContainer->GetTheName());
				//if (rContainer->parentCell->objectList.IsInList(rContainer)) {
					//Console_Print("Remove: %s", rContainer->GetTheName());
					//rContainer->parentCell->objectList.Remove(rContainer);
				//}

				//ExtraObjectHealth* xhealth = (ExtraObjectHealth*)rContainer->extraDataList.GetByType(kExtraData_Health);
				//xhealth->health = 0.0;

				//rContainer->parentCell->objectList.RemoveAll();
				//ExtraDataList* list = &rContainer->parentCell->extraDataList;

				//rContainer->parentCell = nullptr;
				//list->DebugDump();
				//TESContainer* dropped = rContainer->GetContainer();
				//Console_Print("Hello2: %s", dropped);
				//rContainer->DeleteReference();

			//}

		//}

	}

	return ThisStdCall<int>(0x0484D60, rContainer, a2);

}

	int __fastcall SaveData3(TESForm* rContainer, void* edx, int a2)
	{

		//DumpInfoToLog("Global3");

		return ThisStdCall<int>(0x0932880, rContainer, a2);

	}
*/