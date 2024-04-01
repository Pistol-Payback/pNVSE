#pragma once

#include "commands_animation.h"
#include <SafeWrite.h>

namespace Hooks
{

	//int __fastcall HookActorInventorySave(TESObjectREFR* rContainer, void* edx, int a2);
	//int __fastcall StopAnimationType(AnimData* animData, void*, BSAnimGroupSequence* destAnim, UInt16 animGroupId, eAnimSequence animSequence);


	void initHooks()
	{
		//WriteRelCall(0x09328BB, (UInt32)HookActorInventorySave);	//Save Inv Data
		//WriteRelCall(0x494989, (UInt32)StopAnimationType); //Play Animation Group Hook
	}
}
