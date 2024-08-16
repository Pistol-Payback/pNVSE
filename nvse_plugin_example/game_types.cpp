#include "game_types.h"
#include "commands_animation.h"

#include "GameData.h"
#include "GameOSDepend.h"
#include "GameProcess.h"
#include "NiObjects.h"

#include <span>
/*
bool AnimSequenceBase::Contains(BSAnimGroupSequence* anim)
{
	if (this->IsSingle())
	{
		auto* single = static_cast<AnimSequenceSingle*>(this);
		return single->anim == anim;
	}
	const auto* multi = static_cast<AnimSequenceMultiple*>(this);
	return std::any_of(multi->anims->begin(), multi->anims->end(), [&](auto* mAnim) { return mAnim == anim; });
}
*/
AnimGroupID AnimData::GetNextAttackGroupID() const
{
	const auto type = ThisStdCall<char>(0x495E40, this, 0);
	switch (type)
	{
	case '3':
		return kAnimGroup_Attack3;
	case '4':
		return kAnimGroup_Attack4;
	case '5':
		return kAnimGroup_Attack5;
	case '6':
		return kAnimGroup_Attack6;
	case '7':
		return kAnimGroup_Attack7;
	case '8':
		return kAnimGroup_Attack8;
	case 'l':
		return kAnimGroup_AttackLeft;
	default:
		TESObjectWEAP* weap;
		if (this->actor->baseProcess->GetWeaponInfo() && (weap = this->actor->GetEquippedWeapon()))
		{
			if (weap->attackAnim != 0xFF)
				return static_cast<AnimGroupID>(weap->attackAnim);
		}
		return kAnimGroup_AttackRight;
	}
}

void Actor::FireWeapon()
{
	nextAttackAnimGroupId110 = static_cast<UInt32>(GetAnimData()->GetNextAttackGroupID());
	this->baseProcess->SetQueuedIdleFlag(kIdleFlag_FireWeapon);
	GameFuncs::HandleQueuedAnimFlags(this); //Actor::HandleQueuedIdleFlags
}

OSInputGlobals** g_inputGlobals = reinterpret_cast<OSInputGlobals**>(0x11F35CC);


std::span<NiControllerSequence::ControlledBlock> NiControllerSequence::GetControlledBlocks() const
{
	return { controlledBlocks, numControlledBlocks };
}

/*
std::vector<NiControllerSequence::ControlledBlock*> NiControllerSequence::GetControlledBlocks() const {
	std::vector<ControlledBlock*> blocks;
	for (unsigned int i = 0; i < numControlledBlocks; ++i) {
		blocks.push_back(controlledBlocks[i]);
	}
	return blocks;
}
*/
std::vector<NiAVObject*> NiControllerSequence::NiMultiTargetTransformController::GetAllTargets() const {
	std::vector<NiAVObject*> allTargets;
	for (UInt16 i = 0; i < numInterps; ++i) {
		allTargets.push_back(targets[i]);
	}
	return allTargets;
}

std::span<NiControllerSequence::IDTag> NiControllerSequence::GetIDTags() const
{
	return { IDTagArray, numControlledBlocks };
}

NiControllerSequence::ControlledBlock* NiControllerSequence::GetControlledBlock(const char* name) const
{
	std::span idTags(IDTagArray, numControlledBlocks);
	const auto it = std::ranges::find_if(idTags, [&](const IDTag& tag) { return strcmp(tag.m_kAVObjectName.CStr(), name) == 0; });
	if (it != idTags.end())
	{
		return controlledBlocks + std::distance(idTags.begin(), it);
	}
	return nullptr;
}

bool NiControllerSequence::RemoveControlledBlock(const char* name) {

	std::span idTags(IDTagArray, numControlledBlocks);
	const auto it = std::ranges::find_if(idTags, [&](const IDTag& tag) { return strcmp(tag.m_kAVObjectName.CStr(), name) == 0; });
	if (it != idTags.end())
	{
		NiControllerSequence::ControlledBlock* location = controlledBlocks + std::distance(idTags.begin(), it);
		location->interpolator = nullptr;
		location->blendIdx = 0;
		location->multiTargetCtrl = nullptr;
		location->blendInterpolator = nullptr;
		location->priority = 0;
	}

	return false;
}
/*
BSAnimGroupSequence* BSAnimGroupSequence::Get3rdPersonCounterpart() const
{
	auto* animData = PlayerCharacter::GetSingleton()->baseProcess->GetAnimData();
	return GetAnimByFullGroupID(animData, this->animGroup->groupID);
}
*/
/*
bool TESAnimGroup::IsAttackIS()
{
	const auto idMinor = static_cast<AnimGroupID>(groupID & 0xFF);
	switch (idMinor)
	{
	case kAnimGroup_AttackLeftIS: break;
	case kAnimGroup_AttackRightIS: break;
	case kAnimGroup_Attack3IS: break;
	case kAnimGroup_Attack4IS: break;
	case kAnimGroup_Attack5IS: break;
	case kAnimGroup_Attack6IS: break;
	case kAnimGroup_Attack7IS: break;
	case kAnimGroup_Attack8IS: break;
	case kAnimGroup_AttackLoopIS: break;
	case kAnimGroup_AttackSpinIS: break;
	case kAnimGroup_AttackSpin2IS: break;
	case kAnimGroup_PlaceMineIS: break;
	case kAnimGroup_PlaceMine2IS: break;
	case kAnimGroup_AttackThrowIS: break;
	case kAnimGroup_AttackThrow2IS: break;
	case kAnimGroup_AttackThrow3IS: break;
	case kAnimGroup_AttackThrow4IS: break;
	case kAnimGroup_AttackThrow5IS: break;
	case kAnimGroup_Attack9IS: break;
	case kAnimGroup_AttackThrow6IS: break;
	case kAnimGroup_AttackThrow7IS: break;
	case kAnimGroup_AttackThrow8IS: break;
	default: return false;
	}
	return true;
}*/
/*
bool TESAnimGroup::IsAttackNonIS()
{
	const auto idMinor = static_cast<AnimGroupID>(groupID & 0xFF);
	switch (idMinor)
	{
	case kAnimGroup_AttackLeft: break;
	case kAnimGroup_AttackRight: break;
	case kAnimGroup_Attack3: break;
	case kAnimGroup_Attack4: break;
	case kAnimGroup_Attack5: break;
	case kAnimGroup_Attack6: break;
	case kAnimGroup_Attack7: break;
	case kAnimGroup_Attack8: break;
	case kAnimGroup_AttackLoop: break;
	case kAnimGroup_AttackSpin: break;
	case kAnimGroup_AttackSpin2: break;
	case kAnimGroup_AttackPower: break;
	case kAnimGroup_AttackForwardPower: break;
	case kAnimGroup_AttackBackPower: break;
	case kAnimGroup_AttackLeftPower: break;
	case kAnimGroup_AttackRightPower: break;
	case kAnimGroup_AttackCustom1Power: break;
	case kAnimGroup_AttackCustom2Power: break;
	case kAnimGroup_AttackCustom3Power: break;
	case kAnimGroup_AttackCustom4Power: break;
	case kAnimGroup_AttackCustom5Power: break;
	case kAnimGroup_PlaceMine: break;
	case kAnimGroup_PlaceMine2: break;
	case kAnimGroup_AttackThrow: break;
	case kAnimGroup_AttackThrow2: break;
	case kAnimGroup_AttackThrow3: break;
	case kAnimGroup_AttackThrow4: break;
	case kAnimGroup_AttackThrow5: break;
	case kAnimGroup_Attack9: break;
	case kAnimGroup_AttackThrow6: break;
	case kAnimGroup_AttackThrow7: break;
	case kAnimGroup_AttackThrow8: break;

	default: return false;
	}
	return true;
}
*/
TESAnimGroup::AnimGroupInfo* GetGroupInfo(UInt8 groupId)
{
	return &g_animGroupInfos[groupId];
}

UInt32 GetSequenceType(UInt8 groupId)
{
	return GetGroupInfo(groupId)->sequenceType;
}

TESAnimGroup::AnimGroupInfo* TESAnimGroup::GetGroupInfo() const
{
	return ::GetGroupInfo(groupID);
}

AnimGroupID TESAnimGroup::GetBaseGroupID() const
{
	return static_cast<AnimGroupID>(groupID);
}

enum
{
	kAddr_AddExtraData = 0x40FF60,
	kAddr_RemoveExtraType = 0x410140,
	kAddr_LoadModel = 0x447080,
	kAddr_ApplyAmmoEffects = 0x59A030,
	kAddr_MoveToMarker = 0x5CCB20,
	kAddr_ApplyPerkModifiers = 0x5E58F0,
	kAddr_ReturnThis = 0x6815C0,
	kAddr_PurgeTerminalModel = 0x7FFE00,
	kAddr_EquipItem = 0x88C650,
	kAddr_UnequipItem = 0x88C790,
	kAddr_ReturnTrue = 0x8D0360,
	kAddr_TileGetFloat = 0xA011B0,
	kAddr_TileSetFloat = 0xA012D0,
	kAddr_TileSetString = 0xA01350,
	kAddr_InitFontInfo = 0xA12020,
};

#define CALL_EAX(addr) __asm mov eax, addr __asm call eax
#define JMP_EAX(addr)  __asm mov eax, addr __asm jmp eax
#define JMP_EDX(addr)  __asm mov edx, addr __asm jmp edx

__declspec(naked) NiAVObject* __fastcall NiNode::GetBlockByName(const char* nameStr)	//	str of NiString
{
	__asm
	{
		movzx	eax, word ptr[ecx + 0xA6]
		test	eax, eax
		jz		done
		push	esi
		push	edi
		mov		esi, [ecx + 0xA0]
		mov		edi, eax
		ALIGN 16
		iterHead:
		dec		edi
		js		iterEnd
		mov		eax, [esi]
		add		esi, 4
		test	eax, eax
		jz		iterHead
		cmp[eax + 8], edx
		jz		found
		mov		ecx, [eax]
		cmp		dword ptr[ecx + 0xC], kAddr_ReturnThis
		jnz		iterHead
		mov		ecx, eax
		call	NiNode::GetBlockByName
		test	eax, eax
		jz		iterHead
		found :
		pop		edi
		pop		esi
		retn
		ALIGN 16
		iterEnd :
		xor eax, eax
		pop		edi
		pop		esi
		done :
		retn
	}
}

__declspec(naked) NiAVObject* __fastcall NiNode::GetBlock(const char* blockName)
{
	__asm
	{
		cmp[edx], 0
		jz		retnNULL
		push	ecx
		push	edx
		CALL_EAX(0xA5B690)
		pop		ecx
		pop		ecx
		test	eax, eax
		jz		done
		lock dec dword ptr[eax - 8]
		jz		retnNULL
		cmp[ecx + 8], eax
		jz		found
		mov		edx, eax
		call	NiNode::GetBlockByName
		retn
		found :
		mov		eax, ecx
		retn
		retnNULL :
		xor eax, eax
		done :
		retn
	}
}

__declspec(naked) NiNode* __fastcall NiNode::GetNode(const char* nodeName)
{
	__asm
	{
		call	NiNode::GetBlock
		test	eax, eax
		jz		done
		xor edx, edx
		mov		ecx, [eax]
		cmp		dword ptr[ecx + 0xC], kAddr_ReturnThis
		cmovnz	eax, edx
		done :
		retn
	}
}

__declspec(naked) NiAVObject* __fastcall GetNifBlock(TESObjectREFR* thisObj, UInt32 pcNode, const char* blockName)
{
	__asm
	{
		test	dl, dl
		jz		notPlayer
		cmp		dword ptr[ecx + 0xC], 0x14
		jnz		notPlayer
		test	dl, 1
		jz		get1stP
		mov		eax, [ecx + 0x64]
		test	eax, eax
		jz		done
		mov		eax, [eax + 0x14]
		jmp		gotRoot
		get1stP :
		mov		eax, [ecx + 0x694]
		jmp		gotRoot
		notPlayer :
		mov		eax, [ecx]
		call	dword ptr[eax + 0x1D0]
		gotRoot :
		test	eax, eax
		jz		done
		mov		edx, [esp + 4]
		cmp[edx], 0
		jz		done
		mov		ecx, eax
		call	NiNode::GetBlock
		done :
		retn	4
	}
}

bool Actor::IsAnimActionReload() const
{
	const auto currentAnimAction = static_cast<AnimAction>(baseProcess->GetCurrentAnimAction());
	const static std::unordered_set s_reloads = { kAnimAction_Reload, kAnimAction_ReloadLoop, kAnimAction_ReloadLoopStart, kAnimAction_ReloadLoopEnd };
	return s_reloads.contains(currentAnimAction);
}

bool TESForm::IsTypeActor()
{
	return IS_ID(this, Character) || IS_ID(this, Creature);
}

void FormatScriptText(std::string& str)
{
	UInt32 pos = 0;

	while (((pos = str.find('%', pos)) != -1) && pos < str.length() - 1)
	{
		char toInsert = 0;
		switch (str[pos + 1])
		{
		case '%':
			pos += 2;
			continue;
		case 'r':
		case 'R':
			toInsert = '\n';
			break;
		case 'q':
		case 'Q':
			toInsert = '"';
			break;
		default:
			pos += 1;
			continue;
		}

		str.insert(pos, 1, toInsert); // insert char at current pos
		str.erase(pos + 1, 2);		  // erase format specifier
		pos += 1;
	}
}

UInt32* g_weaponTypeToAnim = reinterpret_cast<UInt32*>(0x118A838);

UInt16 GetActorRealAnimGroup(Actor* actor, UInt8 groupID)
{
	UInt8 animHandType = 0;
	if (auto* form = actor->GetEquippedWeapon(); form && actor->baseProcess->IsWeaponOut())
		animHandType = g_weaponTypeToAnim[form->eWeaponType];
	else if (actor->baseProcess->IsWeaponOut())
		animHandType = 1; // arms
	auto moveFlags = actor->actorMover->GetMovementFlags();
	UInt8 moveType = 0;
	if ((moveFlags & 0x800) != 0)
		moveType = kAnimMoveType_Swimming;
	else if ((moveFlags & 0x2000) != 0)
		moveType = kAnimMoveType_Flying;
	else if ((moveFlags & 0x400) != 0)
		moveType = kAnimMoveType_Sneaking;
	const auto isPowerArmor = ThisStdCall<bool>(0x8BA3E0, actor) || ThisStdCall<bool>(0x8BA410, actor);
	return (moveType << 12) + (isPowerArmor << 15) + (animHandType << 8) + groupID;
}

const char* MoveFlagToString(UInt32 flag)
{
	auto flag8 = static_cast<MovementFlags>(flag & 0xFF);
	switch (flag8)
	{
	case kMoveFlag_Forward: return "Forward";
	case kMoveFlag_Backward: return "Backward";
	case kMoveFlag_Left: return "Left";
	case kMoveFlag_Right: return "Right";
	case kMoveFlag_TurnLeft: return "TurnLeft";
	case kMoveFlag_TurnRight: return "TurnRight";
	case kMoveFlag_NonController: break;
	case kMoveFlag_Walking: break;
	case kMoveFlag_Running: break;
	case kMoveFlag_Sneaking: break;
	case kMoveFlag_Swimming: break;
	case kMoveFlag_Jump: break;
	case kMoveFlag_Flying: break;
	case kMoveFlag_Fall: break;
	case kMoveFlag_Slide: break;
	default:;
	}
	return "";
}

