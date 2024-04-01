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