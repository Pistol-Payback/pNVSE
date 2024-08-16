#pragma once

#include <chrono>
#include <chrono>
#include <filesystem>
#include <optional>
#include <set>
#include <span>
#include <unordered_set>

#include "CommandTable.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "GameProcess.h"
#include "game_types.h"
#include "ParamInfos.h"
#include "utility.h"

struct SavedAnims;
extern std::span<TESAnimGroup::AnimGroupInfo> g_animGroupInfos;
using FormID = UInt32;
using GroupID = UInt16;

enum QueuedIdleFlags
{
	kIdleFlag_FireWeapon = 0x1,
	kIdleFlag_Reload = 0x2,
	kIdleFlag_CrippledLimb = 0x10,
	kIdleFlag_Death = 0x20,
	kIdleFlag_ForcedIdle = 0x80,
	kIdleFlag_HandGrip = 0x100,
	kIdleFlag_Activate = 0x400,
	kIdleFlag_StandingLayingDownChange = 0x800,
	kIdleFlag_EquipOrUnequip = 0x4000,
	kIdleFlag_AimWeapon = 0x10000,
	kIdleFlag_AttackEjectEaseInFollowThrough = 0x20000,
	kIdleFlag_SomethingAnimatingReloadLoop = 0x40000,
};

struct AnimStacks
{
	std::vector<SavedAnims> anims;
	std::vector<SavedAnims> maleAnims;
	std::vector<SavedAnims> femaleAnims;
	std::vector<SavedAnims> mod1Anims;
	std::vector<SavedAnims> mod2Anims;
	std::vector<SavedAnims> mod3Anims;

	std::vector<SavedAnims> hurtAnims;
	std::vector<SavedAnims> humanAnims;

};

using FormID = UInt32;
using FullAnimGroupID = UInt32;
// Per ref ID there is a stack of animation variants per group ID
class AnimOverrideStruct
{
public:
	std::unordered_map<FullAnimGroupID, AnimStacks> stacks;
};

#define THISCALL(address, returnType, ...) reinterpret_cast<returnType(__thiscall*)(__VA_ARGS__)>(address)
#define _CDECL(address, returnType, ...) reinterpret_cast<returnType(__cdecl*)(__VA_ARGS__)>(address)

namespace GameFuncs
{
	inline auto* PlayIdle = reinterpret_cast<void(__thiscall*)(void*, TESIdleForm*, Actor*, int, int)>(0x497F20);
	inline auto* ConstructAnimIdle = reinterpret_cast<void* (__thiscall*)(AnimIdle*, TESIdleForm*, eAnimSequence, int, MobileObject*, bool,
		AnimData*)>(0x4965D0);
	inline auto* PlayAnimation = reinterpret_cast<void(__thiscall*)(AnimData*, UInt32, int flags, int loopRange, eAnimSequence)>(0x494740);
	inline auto* LoadKFModel = reinterpret_cast<KFModel * (__thiscall*)(ModelLoader*, const char*)>(0x4471C0);
	inline auto* BSAnimGroupSequence_Init = reinterpret_cast<void(__thiscall*)(BSAnimGroupSequence*, TESAnimGroup*, BSAnimGroupSequence*)>(0x4EE9F0);
	inline auto* KFModel_Init = reinterpret_cast<void(__thiscall*)(KFModel * alloc, const char* filePath, char* bsStream)>(0x43B640);
	inline auto* GetFilePtr = reinterpret_cast<BSFile * (__cdecl*)(const char* path, int const_0, int const_negative_1, int const_1)>(0xAFDF20); // add Meshes in front!
	inline auto* BSStream_SetFileAndName = reinterpret_cast<bool(__thiscall*)(char* bsStreamBuf, const char* fileName, BSFile*)>(0xC3A8A0);
	inline auto* BSStream_Init = reinterpret_cast<char* (__thiscall*)(char* bsStream)>(0x43CFD0);
	inline auto* BSStream_Clear = THISCALL(0x43D090, void, void*);
	inline auto* GetAnims = reinterpret_cast<tList<char>*(__thiscall*)(TESObjectREFR*, int)>(0x566970);
	inline auto* LoadAnimation = reinterpret_cast<bool(__thiscall*)(AnimData*, KFModel*, bool)>(0x490500);
	inline auto* MorphToSequence = reinterpret_cast<BSAnimGroupSequence * (__thiscall*)(AnimData*, BSAnimGroupSequence*, int, int)>(0x4949A0);
	inline auto* PlayAnimGroup = reinterpret_cast<BSAnimGroupSequence * (__thiscall*)(AnimData*, int, int, int, int)>(0x494740);
	inline auto* NiTPointerMap_Lookup = reinterpret_cast<bool(__thiscall*)(void*, int, AnimSequenceBase**)>(0x49C390);
	inline auto* NiTPointerMap_RemoveKey = reinterpret_cast<bool(__thiscall*)(void*, UInt16)>(0x49C250);

	// Multiple "Hit" per anim
	inline auto* AnimData_GetSequenceOffsetPlusTimePassed = reinterpret_cast<float(__thiscall*)(AnimData*, BSAnimGroupSequence*)>(0x493800);
	inline auto* TESAnimGroup_GetTimeForAction = reinterpret_cast<double(__thiscall*)(TESAnimGroup*, UInt32)>(0x5F3780);
	inline auto* AnimData_GetAnimSequenceElement = reinterpret_cast<BSAnimGroupSequence * (__thiscall*)(AnimData*, eAnimSequence a2)>(0x491040);

	inline auto IsDoingAttackAnimation = THISCALL(0x894900, bool, Actor * actor);
	inline auto HandleQueuedAnimFlags = THISCALL(0x8BA600, void, Actor * actor);

	inline auto CrossFade = THISCALL(0xA2E280, bool, NiControllerManager * manager, NiControllerSequence * source, NiControllerSequence * destination, float blend, int priority, bool startOver, float morphWeight, NiControllerSequence * pkTimeSyncSeq);
	inline auto ActivateSequence = THISCALL(0x47AAB0, bool, NiControllerManager * manager, NiControllerSequence * source, int priority, bool bStartOver, float fWeight, float fEaseInTime, NiControllerSequence * pkTimeSyncSeq);

	inline auto MorphSequence = THISCALL(0xA351D0, bool, NiControllerSequence * source, NiControllerSequence * pkDestSequence, float fDuration, int iPriority, float fSourceWeight, float fDestWeight);
	inline auto BlendFromPose = THISCALL(0xA2F800, bool, NiControllerManager * mgr, NiControllerSequence * pkSequence, float fDestFrame, float fDuration, int iPriority, NiControllerSequence * pkSequenceToSynchronize);
	inline auto StartBlend = THISCALL(0xA350D0, bool, NiControllerSequence * pkSourceSequence, NiControllerSequence * pkDestSequence, float fDuration, float fDestFrame, int iPriority, float fSourceWeight,
		float fDestWeight, NiControllerSequence * pkSequenceToSynchronize);
	inline auto DeactivateSequence = THISCALL(0x47B220, int, NiControllerManager * mgr, NiControllerSequence * pkSequence, float fEaseOut);
	inline auto GetActorAnimGroupId = THISCALL(0x897910, UInt16, Actor * actor, UInt32 groupId, BaseProcess::WeaponInfo * weapInfo, bool aFalse, AnimData * animData);
	inline auto NiControllerManager_RemoveSequence = THISCALL(0xA2EC50, NiControllerSequence*, NiControllerManager * mgr, NiControllerSequence * anim);
	inline auto GetNearestGroupID = THISCALL(0x495740, UInt16, AnimData * animData, UInt16 groupID, bool noRecurse);
	inline auto NiControllerManager_LookupSequence = THISCALL(0x47A520, NiControllerSequence*, NiControllerManager * mgr, const char** animGroupName);
	inline auto Actor_Attack = THISCALL(0x8935F0, bool, Actor * actor, UInt32 animGroupId);
	inline auto AnimData_ResetSequenceState = THISCALL(0x496080, void, AnimData * animData, eAnimSequence sequenceId, float blendAmound);

	inline auto InitAnimGroup = _CDECL(0x5F3A20, TESAnimGroup*, BSAnimGroupSequence * anim, const char* path);
	inline auto BSFixedString_CreateFromPool = _CDECL(0xA5B690, const char*, const char* str);
	inline auto NiTextKeyExtraData_Destroy = THISCALL(0xA46D50, UInt32, NiTextKeyExtraData * textKeys);
	inline auto TESAnimGroup_Destroy = THISCALL(0x5F22A0, void, TESAnimGroup * animGroup, bool free);
	inline auto NiRefObject_Replace = THISCALL(0x66B0D0, void, void* target, void* src);
	inline auto NiRefObject_IncRefCount = THISCALL(0x40F6E0, void, void* target);
	inline auto NiRefObject_DecRefCount_FreeIfZero = THISCALL(0x401970, void, void* target);
}