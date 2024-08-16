#pragma once
#include <SafeWrite.h>
#include "ppNVSE.h"

namespace Hooks
{

	TESForm* __cdecl SaveData(void* refID);
	int __fastcall StopAnimationType(AnimData* animData, void* edx, UInt16 GroupID, int a3, float a4, int a5);
	int __fastcall OnEquipAlt(TESObjectREFR* equipper, void* edx, TESForm* item, SInt32 count, ExtraDataList* xData, int noMessage, bool lockEquipment, bool playsound);

	int __fastcall SetPersistentHook(TESForm* form);

	namespace AnimHooks
	{

		extern UInt32 originalAddress;
		//BSAnimGroupSequence* __fastcall HandleAnimationChange(AnimData* animData, void*, BSAnimGroupSequence* destAnim, UInt16 animGroupId, eAnimSequence animSequence);
		void __fastcall OnAnimActivate(BSAnimGroupSequence* anim, void* edx, char a2);
		void hook() {
			//AppendToCallChain(0x494989, (UInt32)HandleAnimationChange, originalAddress);
			//AppendToCallChain(0x495E2A, (UInt32)HandleAnimationChange, originalAddress);
			//AppendToCallChain(0x4956FF, (UInt32)HandleAnimationChange, originalAddress);
			//AppendToCallChain(0x4973B4, (UInt32)HandleAnimationChange, originalAddress);
			AppendToCallChain(0x0A34F71, reinterpret_cast<UInt32>(OnAnimActivate), originalAddress);
		}

	}


	//int __fastcall OnActorEquipAlt(TESObjectREFR* a1);
	//int __cdecl OnActorEquipAlt2(TESForm* a1, TESForm* a2, TESForm* a3);
	//int __fastcall GetType(ExtraDataList* a1);

	//int __fastcall HookTest(AnimData* animData, void* edx, UInt16 GroupID, int a3, float a4, int a5);

	void initHooks()
	{
		AnimHooks::hook();
		
		WriteRelCall(0x04C3ED3, (UInt32)OnEquipAlt);
		WriteRelCall(0x05752D4, (UInt32)OnEquipAlt);
		WriteRelCall(0x0604AD2, (UInt32)OnEquipAlt);
		WriteRelCall(0x07ADBC1, (UInt32)OnEquipAlt);
		WriteRelCall(0x088C6E8, (UInt32)OnEquipAlt);
		WriteRelCall(0x088C74E, (UInt32)OnEquipAlt);
		WriteRelCall(0x08927AE, (UInt32)OnEquipAlt);
		WriteRelCall(0x08F023F, (UInt32)OnEquipAlt);
		WriteRelCall(0x08F0345, (UInt32)OnEquipAlt);
		WriteRelCall(0x091559D, (UInt32)OnEquipAlt);
		WriteRelCall(0x0915631, (UInt32)OnEquipAlt);
		WriteRelCall(0x0943E20, (UInt32)OnEquipAlt);
		WriteRelCall(0x097F13B, (UInt32)OnEquipAlt);

		WriteRelCall(0x0847A20, (UInt32)SaveData);

		WriteRelCall(0x056549F, (UInt32)SetPersistentHook);
		
		WriteRelCall(0x089738D, (UInt32)StopAnimationType); //Third Person
		WriteRelCall(0x09521C0, (UInt32)StopAnimationType); //First Person anim
		WriteRelCall(0x0923A79, (UInt32)StopAnimationType); //Play Animation Group Hook on first equip, idk.

	}
}