#pragma once

/*
namespace MyHooks
{

	//constexpr char eventName[] = "OnPreAttack";

	void __fastcall RedirectBaseformHook(TESForm* form)
	{
		GetWeaponModifier(form, 0);
	}

	UInt32 OldAddy = 0;

	//int __fastcall HookDetour(ContChangesEntry* rContainer)

	TESObjectWEAP* __fastcall HandleEvent(Projectile* proj, Actor* rActor, TESObjectWEAP* weapon)
	{

		Console_Print("Hook ran HookDetour");

		TESObjectWEAP* rWeapon = ((Actor*)rActor)->GetEquippedWeapon();

		HighProcess* hiProc = (HighProcess*)((Actor*)rActor)->baseProcess;
		ExtraContainerChanges::EntryData* weapInfo;
		if (hiProc != nullptr) {

			//NiNode* altProjNode = rActor->GetNode("nodeName");
			//hiProc->projectileNode = altProjNode;

			TESObjectREFR* invRef = nullptr;

			weapInfo = hiProc->GetWeaponInfo();
			if (weapInfo && weapInfo->extendData)
			{
				//rWeapon = (TESObjectWEAP*)weapInfo->type;

				//ArrayElementL iCallReturn;
				//invRef = InventoryRefCreateEntry(rActor, rWeapon, weapInfo->countDelta, weapInfo->extendData->GetFirstItem());

				//if (invRef && rWeapon && IsRegisterWeapon(rWeapon)) {
					//Script* PistolGetModifierID = ((Script * (__cdecl*)(const char*))(0x483A00))("PistolGetModifierID");
					//g_scriptInterface->CallFunction(PistolGetModifierID, invRef, nullptr, &iCallReturn, 0);
					//UInt32 InstID = static_cast<UInt32>(iCallReturn.Number());
					//rWeapon = (TESObjectWEAP*)GetWeaponModifier(invRef->baseForm, InstID);
				//}

			}
			//weapInfo->type = rWeapon;
			weapInfo->type = ((TESForm * (__cdecl*)(const char*))(0x483A00))("weapNV9mmPistol");

		}

		return rWeapon;

	}

	bool __fastcall HookDetour(TESObjectREFR* rActor, void* edx, int iSomething, int iSomeWeaponID)
	{
		Console_Print("Hook ran HookDetour");
		int weapon;

		TESObjectWEAP* rWeapon = ((Actor*)rActor)->GetEquippedWeapon();

		HighProcess* hiProc = (HighProcess*)((Actor*)rActor)->baseProcess;
		ExtraContainerChanges::EntryData* weapInfo;
		if (hiProc != nullptr) {

			TESObjectREFR* invRef = nullptr;

			weapInfo = hiProc->GetWeaponInfo();
			weapInfo->type = ((TESObjectWEAP * (__cdecl*)(const char*))(0x483A00))("weapNV9mmPistol");

			//weapInfo->type = rWeapon;

		}

		ThisStdCall<bool>(0x08A73E0, rActor, iSomething, iSomeWeaponID);	//Fork for melee weapons.

		return 0;

	}

	int __fastcall HookDetour2(ContChangesEntry* rContainer)
	{
		Console_Print("Hook ran HookDetour Inventory Change");
		return ThisStdCall<int>(0x044DDC0, rContainer);
	}

#define CALL_EAX(addr) __asm mov eax, addr __asm call eax

	__declspec(naked) void __fastcall DoFireWeaponEx(TESObjectREFR* refr, int, TESObjectWEAP* weapon)
	{
		__asm
		{
			mov		edx, [ecx + 0x68]
			push	dword ptr[edx + 0x118]
			and dword ptr[edx + 0x118], 0
			push	dword ptr[edx + 0x114]
			and dword ptr[edx + 0x114], 0
			push	edx
			push	ecx
			mov		ecx, [esp + 0x14]
			CALL_EAX(0x523150)
			pop		eax
			pop		dword ptr[eax + 0x114]
			pop		dword ptr[eax + 0x118]
			mov		ecx, [eax + 0x3D4]
			test	ecx, ecx
			jz		done
			mov		byte ptr[ecx + 3], 1
			done:
			retn	4
		}
	}

	struct AnimGroupClassify
	{
		UInt8	category;	// 00
		UInt8	subType;	// 01
		UInt8	flags;		// 02
		UInt8	byte03;		// 03
	};
	extern const AnimGroupClassify kAnimGroupClassify[];

	const AnimGroupClassify kAnimGroupClassify[] =
	{
		{1, 1, 0, 0}, {1, 1, 0, 0}, {1, 1, 0, 0}, {2, 1, 0, 0}, {2, 2, 0, 0}, {2, 3, 0, 0}, {2, 4, 0, 0}, {2, 1, 1, 0}, {2, 2, 1, 0},
		{2, 3, 1, 0}, {2, 4, 1, 0}, {2, 1, 2, 0}, {2, 2, 2, 0}, {2, 3, 2, 0}, {2, 4, 2, 0}, {2, 3, 4, 0}, {2, 4, 4, 0}, {3, 0, 0, 0},
		{3, 0, 1, 0}, {3, 0, 2, 0}, {3, 0, 4, 0}, {3, 0, 5, 0}, {3, 0, 6, 0}, {1, 2, 0, 0}, {1, 2, 0, 0}, {1, 2, 0, 0}, {3, 1, 0, 0},
		{3, 1, 1, 0}, {3, 1, 2, 0}, {3, 1, 4, 0}, {3, 1, 5, 0}, {3, 1, 6, 0}, {3, 2, 0, 0}, {3, 2, 1, 0}, {3, 2, 2, 0}, {3, 2, 4, 0},
		{3, 2, 5, 0}, {3, 2, 6, 0}, {3, 3, 0, 0}, {3, 3, 1, 0}, {3, 3, 2, 0}, {3, 3, 4, 0}, {3, 3, 5, 0}, {3, 3, 6, 0}, {3, 4, 0, 0},
		{3, 4, 1, 0}, {3, 4, 2, 0}, {3, 4, 4, 0}, {3, 4, 5, 0}, {3, 4, 6, 0}, {3, 5, 0, 0}, {3, 5, 1, 0}, {3, 5, 2, 0}, {3, 5, 4, 0},
		{3, 5, 5, 0}, {3, 5, 6, 0}, {3, 6, 0, 0}, {3, 6, 1, 0}, {3, 6, 2, 0}, {3, 6, 4, 0}, {3, 6, 5, 0}, {3, 6, 6, 0}, {3, 7, 0, 0},
		{3, 7, 1, 0}, {3, 7, 2, 0}, {3, 7, 4, 0}, {3, 7, 5, 0}, {3, 7, 6, 0}, {3, 8, 0, 0}, {3, 8, 1, 0}, {3, 8, 2, 0}, {3, 8, 4, 0},
		{3, 8, 5, 0}, {3, 8, 6, 0}, {3, 10, 0, 0}, {3, 10, 1, 0}, {3, 10, 2, 0}, {3, 10, 4, 0}, {3, 10, 5, 0}, {3, 10, 6, 0}, {3, 11, 0, 0},
		{3, 11, 1, 0}, {3, 11, 2, 0}, {3, 11, 4, 0}, {3, 11, 5, 0}, {3, 11, 6, 0}, {3, 12, 0, 0}, {3, 12, 1, 0}, {3, 12, 2, 0}, {3, 12, 4, 0},
		{3, 12, 5, 0}, {3, 12, 6, 0}, {3, 23, 0, 0}, {3, 23, 0, 0}, {3, 23, 0, 0}, {3, 23, 0, 0}, {3, 23, 0, 0}, {3, 23, 0, 0}, {3, 23, 0, 0},
		{3, 23, 0, 0}, {3, 23, 0, 0}, {3, 23, 0, 0}, {3, 21, 0, 0}, {3, 21, 1, 0}, {3, 21, 2, 0}, {3, 21, 4, 0}, {3, 21, 5, 0}, {3, 21, 6, 0},
		{3, 22, 0, 0}, {3, 22, 1, 0}, {3, 22, 2, 0}, {3, 22, 4, 0}, {3, 22, 5, 0}, {3, 22, 6, 0}, {3, 13, 0, 0}, {3, 13, 1, 0}, {3, 13, 2, 0},
		{3, 13, 4, 0}, {3, 13, 5, 0}, {3, 13, 6, 0}, {3, 14, 0, 0}, {3, 14, 1, 0}, {3, 14, 2, 0}, {3, 14, 4, 0}, {3, 14, 5, 0}, {3, 14, 6, 0},
		{3, 15, 0, 0}, {3, 15, 1, 0}, {3, 15, 2, 0}, {3, 15, 4, 0}, {3, 15, 5, 0}, {3, 15, 6, 0}, {3, 16, 0, 0}, {3, 16, 1, 0}, {3, 16, 2, 0},
		{3, 16, 4, 0}, {3, 16, 5, 0}, {3, 16, 6, 0}, {3, 17, 0, 0}, {3, 17, 1, 0}, {3, 17, 2, 0}, {3, 17, 4, 0}, {3, 17, 5, 0}, {3, 17, 6, 0},
		{3, 9, 0, 0}, {3, 9, 1, 0}, {3, 9, 2, 0}, {3, 9, 4, 0}, {3, 9, 5, 0}, {3, 9, 6, 0}, {3, 18, 0, 0}, {3, 18, 1, 0}, {3, 18, 2, 0},
		{3, 18, 4, 0}, {3, 18, 5, 0}, {3, 18, 6, 0}, {3, 19, 0, 0}, {3, 19, 1, 0}, {3, 19, 2, 0}, {3, 19, 4, 0}, {3, 19, 5, 0}, {3, 19, 6, 0},
		{3, 20, 0, 0}, {3, 20, 1, 0}, {3, 20, 2, 0}, {3, 20, 4, 0}, {3, 20, 5, 0}, {3, 20, 6, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 3, 0, 0},
		{1, 3, 0, 0}, {1, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0},
		{4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0},
		{4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {4, 0, 0, 0}, {5, 0, 0, 0},
		{5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0},
		{5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0}, {5, 0, 0, 0},
		{5, 0, 0, 0}, {5, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {2, 5, 0, 0}, {2, 5, 0, 0}, {2, 5, 0, 0}, {1, 4, 0, 0},
		{1, 4, 0, 0}, {1, 4, 0, 0}, {1, 4, 0, 0}, {1, 4, 0, 0}, {1, 4, 0, 0}, {2, 5, 0, 0}, {2, 5, 0, 0}, {2, 5, 0, 0}, {2, 5, 0, 0}, {1, 0, 0, 0},
		{2, 5, 0, 0}, {2, 5, 0, 0}, {2, 5, 0, 0}, {2, 5, 0, 0}
	};

	bool IsAnimPlaying(TESObjectREFR* rActor, UInt32 category, UInt32 subType)
	{

		AnimData* animData = nullptr;
		if (rActor->IsPlayer() && !((PlayerCharacter*)rActor)->bThirdPerson)
		{
			Console_Print("Is in first person");
			animData = (AnimData*)((PlayerCharacter*)rActor)->extraAnimation;
		}
		else animData = rActor->GetAnimData();

		if (animData)
		{
			for (UInt16 groupID : animData->groupIDs)
			{
				UInt32 animID = groupID & 0xFF;
				if (animID >= 245) continue;
				const AnimGroupClassify* classify = &kAnimGroupClassify[animID];

				Console_Print("Cad %d", classify->category);
				Console_Print("SubCad %d", classify->subType);
				if ((classify->category == category) && ((category >= 4) || ((!subType || (classify->subType == subType)))))
				{
					return true;
					break;
				}
			}
		}
		return false;
	}

	void __fastcall HookDetour3(TESObjectWEAP* rWeapon, void* edx, TESObjectREFR* rActor)
	{

		//Console_Print("Hook ran HookDetour %s", rActor->GetTheName());
		HighProcess* hiProc = (HighProcess*)((Actor*)rActor)->baseProcess;
		if (hiProc != nullptr) {

			TESObjectREFR* invRef = nullptr;

			auto weapInfo = hiProc->GetWeaponInfo();
			if (weapInfo && weapInfo->extendData)
			{

				ArrayElementL iCallReturn;
				invRef = InventoryRefCreateEntry(rActor, rWeapon, weapInfo->countDelta, weapInfo->extendData->GetFirstItem());

				if (invRef && rWeapon && IsRegisterWeapon(rWeapon)) {
					UInt32 InstID = GetWeaponInstID(invRef);
					rWeapon = (TESObjectWEAP*)GetWeaponModifier(invRef->baseForm, InstID);
				}

				if (rWeapon) {

					//DoFireWeaponEx(rActor, 0, rWeapon);
					if (auto ammoInfo = hiProc->GetAmmoInfo())
						ammoInfo->count += 2;

					ThisStdCall<void>(0x523150, rWeapon, rActor);

				}

			}

		}
		//DoFireWeaponEx(rActor, 0, rWeapon);


	}

	std::unordered_map<UInt32, WeapInst*> aActors;
	std::vector<ExtraContainerChanges::ExtendDataList*> aExtendData;
	std::vector<TESForm*> aForms;
	int iCounter = 10;
	int __fastcall HookActorInventorySave(TESObjectREFR* rContainer, void* edx, int a2)
	{

		if (rContainer) {
			ContChangesEntryList* entryList = rContainer->GetContainerChangesList();

			if (entryList) {
				ContChangesEntry* entry;
				for (auto iter = entryList->Begin(); !iter.End(); ++iter) {

					entry = iter.Get();
					if (entry && entry->type->typeID == 40 && IsModifierClone(entry->type)) {

						UInt32 objectsToRemove = entry->countDelta;
						while (objectsToRemove > 0) {

							TESObjectWEAP* weap = (TESObjectWEAP*)(GetModifierParent(entry->type));
							ContChangesEntry* newEntry = entry->Create(weap->refID, 1, entry->extendData);
							newEntry->Cleanup();
							//entryList->Replace(entry, newEntry);

							ExtraDataList* xData = entry->extendData->GetFirstItem();
							bool IsEquipped = xData->HasType(kExtraData_Worn);

							SaveData* saveData = new SaveData(GetModifierParent(entry->type)->refID, 0, rContainer->refID, IsEquipped, xData->CreateCopy());
							aSaveData.push_back(saveData);

							if (IsEquipped) {

								SilentUnequip(((Actor*)rContainer), entry->type, xData);

							}

							entry->Remove(entry->extendData->GetFirstItem(), 1);

							objectsToRemove--;
						}

					}

				}
			}

		}

		//rContainer->Cleanup();

	//}
		ThisStdCall<int>(0x0562230, rContainer, a2);

		//if (rContainer) {

			//TESForm* rObject = ((TESForm * (__cdecl*)(const char*))(0x483A00))("WeapNVRiotShotgun");

			//if (rObject) {
				//rContainer->AddItem(rObject, nullptr, 1); //Can't do this.
			//}
		//}

		return 0;

	}

	//int __fastcall HookDetourTemp(TESForm* entry, void* edx)
	//{
		//return 0;

	//}

	int __fastcall HookDetourTemp(AnimData* animData, void*, BSAnimGroupSequence* destAnim, UInt16 animGroupId, eAnimSequence animSequence)
	{
		const auto baseAnimGroup = static_cast<AnimGroupID>(animGroupId);

		if (!animGroupToActorsMap.empty()) {
			//Console_Print("Ran HookDetourTemp, Not Empty");
			Actor* actor = animData->actor;
			auto it = animGroupToActorsMap.find(baseAnimGroup);

			if (it != animGroupToActorsMap.end()) {
				//Console_Print("Ran HookDetourTemp, Found");
				auto& actorsVector = it->second;
				for (auto it = actorsVector.begin(); it != actorsVector.end(); ++it) {

					if (*it == actor) {

						//Console_Print("Ran HookDetourTemp, Matching");
						QueueToSkipAnimation(actor);
						actorsVector.erase(it);
						if (actorsVector.empty()) {
							animGroupToActorsMap.erase(baseAnimGroup);
						}
						actor->RefreshAnimData();
						break;
					}

				}

			}

		}

		return ThisStdCall<int>(0x04949A0, animData, destAnim, animGroupId, animSequence);

	}

	void InitHook()
	{
		//OldAddy = GetRelJumpAddr(0x0893AAF);
		//WriteRelJump(0x0893AC6, (UInt32)HookDetour);
		// 
		//WriteRelCall(0x0893AAF, (UInt32)HookDetour); Start Attack Ammo Check

		/////WriteRelCall(0x089364F, (UInt32)HookDetour); //StartAttack Ref1

		//WriteRelCall(0x08BA77B, (UInt32)HookDetour); //StartAttack Ref2

		/////WriteRelCall(0x0948398, (UInt32)HookDetour); //StartAttack Ref3

		/////WriteRelCall(0x04C3968, (UInt32)HookDetour2);	//OnInventoryChange

		//WriteRelCall(0x089390E, (UInt32)HookDetour3);	//Hook Queue Attack
		//WriteRelCall(0x08BA7BF, (UInt32)HookDetour3);	//Hook Queue Attack

		//WriteRelCall(0x0949CF1, (UInt32)HookDetour3);	//Hook Queue Attack This works for player. Can replace animation system, etc etc. Old way.

		//WriteRelCall(0x0894351, (UInt32)HookDetour3);	//RemoveLine Still Stopped Anims

		//WriteRelCall(0x0894630, (UInt32)HookDetour3);	//Spawns projectile, and reduces ammo acount

		//WriteRelCall(0x09420FC, (UInt32)HookDetour);	//temp
		//WriteRelCall(0x0947117, (UInt32)HookDetour);	//temp


		//WriteRelCall(0x0894630, (UInt32)HookDetour);	//Spawns projectile, and reduces ammo acount
		//WriteRelCall(0x0524208, (UInt32)HookDetourTemp);	//temp

		//WriteRelCall(0x0978269, (UInt32)HookDetour);	//temp
		//WriteRelCall(0x0947A25, (UInt32)HookDetour);	//temp
		// 
		//.................................................................Last known fire weapon hook: WriteRelCall(0x08BADE9, (UInt32)HookDetour3);	//FireWeapon

		//WriteRelCall(0x05245BD, (UInt32)HookDetour);	//Spawn Projectile... 3-12-2024


		//WriteRelCall(0x0562300, (UInt32)HookDetourTemp);	//Saving

		WriteRelCall(0x09328BB, (UInt32)HookActorInventorySave);	//Save Inv Data

		//WriteRelCall(0x05C3DEC, (UInt32)HookDetourTemp);	//Equip

		//WriteRelCall(0x494989, (UInt32)HookDetourTemp);

		//WriteRelCall(0x0491858, (UInt32)HookDetourTemp);
		//WriteRelCall(0x0491DAF, (UInt32)HookDetourTemp);
		//WriteRelCall(0x0491EEE, (UInt32)HookDetourTemp);
		//WriteRelCall(0x0492094, (UInt32)HookDetourTemp);
		//WriteRelCall(0x04921CF, (UInt32)HookDetourTemp);
		//WriteRelCall(0x0492599, (UInt32)HookDetourTemp);
		//WriteRelCall(0x0492714, (UInt32)HookDetourTemp);
		//WriteRelCall(0x0492A29, (UInt32)HookDetourTemp);
		//WriteRelCall(0x0492C7B, (UInt32)HookDetourTemp);

		//WriteRelCall(0x0494846, (UInt32)HookDetourTemp);
		WriteRelCall(0x494989, (UInt32)HookDetourTemp); //Play Animation Group Hook

		//WriteRelCall(0x05C3DDF, (UInt32)HookDetourTemp); //Equip
		//WriteRelCall(0x05C3F81, (UInt32)HookDetourTemp); //Equip

			//WriteRelCall(0x0915E3F, (UInt32)HookDetourTemp); //Equip		

		//WriteRelCall(0x05C3D26, (UInt32)HookDetourTemp); //Equip
		//WriteRelCall(0x088CB3A, (UInt32)HookDetourTemp); //Equip
		//WriteRelCall(0x088CB49, (UInt32)HookDetourTemp); //Equip
		//WriteRelCall(0x088CB61, (UInt32)HookDetourTemp); //Equip
		//WriteRelCall(0x088CB69, (UInt32)HookDetourTemp); //Equip
		//WriteRelCall(0x088CB71, (UInt32)HookDetourTemp); //Equip
		//WriteRelCall(0x088CB79, (UInt32)HookDetourTemp); //Equip

		//WriteRelCall(0x495E2A, (UInt32)HookDetourTemp);
		//WriteRelCall(0x4956FF, (UInt32)HookDetourTemp);
		//WriteRelCall(0x4973B4, (UInt32)HookDetourTemp);


		//WriteRelCall(0x04D410D, (UInt32)HookDetourTemp);	//Save Inv Data
		//WriteRelCall(0x0959B02, (UInt32)HookDetourTemp);	//Save Inv Data
		//WriteRelCall(0x09C525B, (UInt32)HookDetourTemp);	//Save Inv Data

		//WriteRelCall(0x04D410D, (UInt32)HookDetourTemp);	//Saving
		//WriteRelCall(0x04BEDC7, (UInt32)HookDetourTemp);	//Saving

		//WriteRelCall(0x05622C6, (UInt32)HookDetourTemp);	//Saving
		//WriteRelCall(0x0894667, (UInt32)HookDetourTemp);	//Crash
		//WriteRelCall(0x0894643, (UInt32)HookDetourTemp);	//Stops animation from playing
		//WriteRelCall(0x089464A, (UInt32)HookDetourTemp);	//Stops animation from playing

		//WriteRelCall(0x0894630, (UInt32)HookDetour);	//Changes type

		//WriteRelCall(0x08A7565, (UInt32)HookDetourTemp);
		//WriteRelCall(0x08A73EC, (UInt32)HookDetourTemp);
		//WriteRelCall(0x08A73F7, (UInt32)HookDetourTemp);	//


		//WriteRelCall(0x089461D, (UInt32)HookDetourTemp);	//Nothing
		//WriteRelCall(0x08945E9, (UInt32)HookDetourTemp);	//Crashed
		//WriteRelCall(0x089464A, (UInt32)HookDetourTemp);	//Stopped only first person fire animation. Gun still fired.
		//WriteRelCall(0x08945D1, (UInt32)HookDetourTemp);	//idk
		//WriteRelCall(0x08B2AAF, (UInt32)HookDetourTemp);	//Stopped all firng/aiming animations in first person
		//WriteRelCall(0x08945C7, (UInt32)HookDetourTemp);	//Main Attack
		//WriteRelCall(0x089458D, (UInt32)HookDetourTemp);	//Doesn't stop fire.
		//WriteRelCall(0x089451F, (UInt32)HookDetourTemp);	//Doesn't stop fire.
		//WriteRelCall(0x089447D, (UInt32)HookDetourTemp);	//Doesn't stop fire.
		//WriteRelCall(0x0894490, (UInt32)HookDetourTemp);	//RemoveLine Crashed the game on second fire.
		//WriteRelCall(0x0894375, (UInt32)HookDetourTemp);	//RemoveLine Didn't Stop Anims
		//WriteRelCall(0x0894351, (UInt32)HookDetourTemp);	//RemoveLine Still Stopped Anims
		//WriteRelCall(0x08942A7, (UInt32)HookDetourTemp);	//RemoveLine Still Stopped Anims
		//WriteRelCall(0x0897AFB, (UInt32)HookDetourTemp);	//Freezes animations
		//WriteRelCall(0x0897B11, (UInt32)HookDetourTemp);	//Weapon Animations

		//WriteRelCall(0x0894299, (UInt32)HookDetourTemp);	//RemoveLine Stopped Anims
		//WriteRelCall(0x08945AA, (UInt32)HookDetourTemp);	//RemoveLine
		//WriteRelCall(0x0893CDD, (UInt32)HookDetourTemp);	//RemoveLine
		//WriteRelCall(0x0893CDD, (UInt32)HookDetourTemp);	//RemoveLine
		//WriteRelCall(0x0893D6C, (UInt32)HookDetourTemp);	//RemoveLine
		//WriteRelCall(0x0893B20, (UInt32)HookDetourTemp);	//RemoveLine
		//WriteRelCall(0x0893C43, (UInt32)HookDetourTemp);	//RemoveLine
		//WriteRelCall(0x0893AAF, (UInt32)HookDetourTemp);	//RemoveLine Call Ammo count
		//WriteRelCall(0x0893B20, (UInt32)HookDetourTemp);	//RemoveLine
		//WriteRelJump(0x08947A5, (UInt32)HookDetourTemp);	//RemoveLine



	}
}
*/