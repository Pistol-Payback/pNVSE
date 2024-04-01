#include "game_types.h"
#include "commands_animation.h"

#include "GameData.h"
#include "GameOSDepend.h"
#include "GameProcess.h"
#include "NiObjects.h"

AnimGroupID AnimData::GetNextAttackGroupID(TESObjectWEAP* weap) const
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
		if (weap) {
			if (weap->attackAnim != 0xFF)
				return static_cast<AnimGroupID>(weap->attackAnim);
		}
		else if (this->actor->baseProcess->GetWeaponInfo() && (weap = (TESObjectWEAP * )this->actor->baseProcess->GetWeaponInfo()->type))
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
	//this->baseProcess->SetQueuedIdleFlag(kIdleFlag_FireWeapon);
	//GameFuncs::HandleQueuedAnimFlags(this); //Actor::HandleQueuedIdleFlags
}
