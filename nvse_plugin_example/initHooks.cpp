#pragma once
#include <SafeWrite.h>
#include "ppNVSE.h"

namespace Hooks
{

	TESForm* __cdecl SaveData(void* refID);
	int __fastcall StopAnimationType(AnimData* animData, void* edx, UInt16 GroupID, int a3, float a4, int a5);
	int __fastcall OnEquipAlt(TESObjectREFR* equipper, void* edx, TESForm* item, SInt32 count, ExtraDataList* xData, int noMessage, bool lockEquipment, bool playsound);

	//int __fastcall OnActorEquipAlt(TESObjectREFR* a1);
	//int __cdecl OnActorEquipAlt2(TESForm* a1, TESForm* a2, TESForm* a3);
	//int __fastcall GetType(ExtraDataList* a1);

	//int __fastcall HookTest(AnimData* animData, void* edx, UInt16 GroupID, int a3, float a4, int a5);

	void initHooks()
	{

		//WriteRelCall(0x0562240, (UInt32)HookActorInventorySave);	//Save Inv Data

		//WriteRelCall(0x08AAF50, (UInt32)SaveData);					//Save Inv Data Actors.........

		//WriteRelCall(0x0959B02, (UInt32)SaveData);
		//////WriteRelCall(0x04D410D, (UInt32)SaveData);		//Save container entry data
		//WriteRelCall(0x09C525B, (UInt32)SaveData);

		//WriteRelCall(0x08AAF50, (UInt32)HookActorInventorySave);		//Save container entry data
		//WriteRelCall(0x0562240, (UInt32)SaveData);		//Save container entry data

		//WriteRelCall(0x0426A68, (UInt32)SaveData);		//Save container entry data

		//WriteRelCall(0x08AAF50, (UInt32)SaveData);		//Save container entry data


		//WriteRelCall(0x050ADBE, (UInt32)SaveData2);		//Save Leveled Lists
		
		//WriteRelCall(0x05806F0, (UInt32)SaveData2);		//Nothing
		//WriteRelCall(0x061666E, (UInt32)SaveData2);		//Nothing

		//WriteRelCall(0x04BE806, (UInt32)SaveData2);		//Save container entry data
		
		//WriteRelCall(0x09C5000, (UInt32)SaveData3);		//Save container entry data

		//WriteRelCall(0x084EE72, (UInt32)SaveData2);		//
		
		//WriteRelCall(0x088C8A5, (UInt32)OnActorEquipAlt2);	//Nothing

		//WriteRelCall(0x88C8BC, (UInt32)OnActorEquipAlt2);	//Nothing
		//WriteRelCall(0x88CF32, (UInt32)OnActorEquipAlt2);	//Nothing
		//WriteRelCall(0x88D25B, (UInt32)OnActorEquipAlt2);	//OnEquip Player

		//WriteRelCall(0x088C87A, (UInt32)GetType);
		
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

		WriteRelCall(0x0847A20, (UInt32)SaveData);		//
		
		WriteRelCall(0x089738D, (UInt32)StopAnimationType); //Third Person
		WriteRelCall(0x09521C0, (UInt32)StopAnimationType); //First Person anim
		WriteRelCall(0x0923A79, (UInt32)StopAnimationType); //Play Animation Group Hook on first equip, idk.

		//WriteRelCall(0x0FA2F1A, (UInt32)HookTest);	//CellBorderBug
		//WriteRelCall(0x0FA24AA, (UInt32)HookTest);	//CellBorderBug
		//WriteRelCall(0x0FA248A, (UInt32)HookTest);	//CellBorderBug
		//WriteRelCall(0x0FA246A, (UInt32)HookTest);	//CellBorderBug
		//WriteRelCall(0x0FA244A, (UInt32)HookTest);	//CellBorderBug
		//WriteRelCall(0x0FA242A, (UInt32)HookTest);	//CellBorderBug
		// 
		//WriteRelCall(0x494989, (UInt32)StopAnimationType); //Play Animation Group Hook

		//WriteRelCall(0x095DD37, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x095DB63, (UInt32)StopAnimationType); //Play Animation Group Hook	
		//WriteRelCall(0x095D805, (UInt32)StopAnimationType); //Play Animation Group Hook	
		//WriteRelCall(0x0952208, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x09521C0, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x0948D1B, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x092368C, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x09235B9, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x08BB429, (UInt32)StopAnimationType); //Play Animation Group Hook		
		//WriteRelCall(0x08BB3B1, (UInt32)StopAnimationType); //Play Animation Group Hook	
		//WriteRelCall(0x08BAD2C, (UInt32)StopAnimationType); //Play Animation Group Hook

		//WriteRelCall(0x08BAC5E, (UInt32)StopAnimationType); //Play Animation Group Hook	
		//WriteRelCall(0x08BABFB, (UInt32)StopAnimationType); //Play Animation Group Hook	
		//WriteRelCall(0x08BA6EA, (UInt32)StopAnimationType); //Play Animation Group Hook	

		//WriteRelCall(0x08B2AAF, (UInt32)HookTest); //Lose control of inputs	

		//WriteRelCall(0x08A8788, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x08A863B, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x08A7D37, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x08A7C2B, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x0897660, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x08974D3, (UInt32)StopAnimationType); //Play Animation Group Hook

		//WriteRelCall(0x0494846, (UInt32)StopAnimationType); // Try this
		
		//WriteRelCall(0x089735A, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x0895756, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x089504A, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x0894F17, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x0894E48, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x088D76B, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x0826FF1, (UInt32)StopAnimationType); //Play Animation Group Hook
		//WriteRelCall(0x07FD85B, (UInt32)StopAnimationType); //Play Animation Group Hook

	}
}