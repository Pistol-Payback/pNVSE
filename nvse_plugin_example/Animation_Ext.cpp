#pragma once
#include "Animation_Ext.h"
#include "EventHandlers.h"

namespace Hooks {
	namespace AnimHooks {

		UInt32 originalAddress;

		//kNVSE
		BSAnimGroupSequence* __fastcall HandleAnimationChange(AnimData* animData, void*, BSAnimGroupSequence* destAnim, UInt16 animGroupId, eAnimSequence animSequence) {

			destAnim = ThisStdCall<BSAnimGroupSequence*>(originalAddress, animData, destAnim, animGroupId, animSequence);

			if (destAnim) {
				if (destAnim->node060) {
					TESForm* form = destAnim->node060->findRefFrom3D();
					BSFadeNode* node = destAnim->GetFadeNode();
					if (form) {

						onAnimationStart.DispatchEvent(form, destAnim->sequenceName);

						if (const AnimDataLock* animLock = AnimLockManager::findAnimLockData(form->refID, destAnim->sequenceName, true)) {
							animLock->UpdateAllBlocks(destAnim);
						}

					}

				}

			}
			return destAnim;

		}

		void __fastcall OnAnimActivate(BSAnimGroupSequence* anim, void* edx, char a2)
		{

			ThisStdCall<int>(originalAddress, anim, a2);

			if (anim) {
				if (anim->node060) {
					TESForm* form = anim->node060->findRefFrom3D();
					if (form) {

						onAnimationStart.DispatchEvent(form, anim->sequenceName);

						if (const AnimDataLock* animLock = AnimLockManager::findAnimLockData(form->refID, anim->sequenceName, true)) {
							animLock->UpdateAllBlocks(anim);
						}

					}

				}

			}

		}

	}

}

std::unordered_set<AnimDataLock, AnimLockManager::Hash> AnimLockManager::allData;

size_t AnimLockManager::Hash::operator()(const AnimDataLock& adl) const {
	return std::hash<UInt32>()(adl.refID) ^ (hash_cstr(adl.animPath) << 1);
}

size_t AnimLockManager::Hash::hash_cstr(const char* str) const {
	if (str == nullptr) {
		return 0;
	}
	size_t hash = 5381;
	char c;
	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + static_cast<unsigned char>(c); // hash * 33 + c
	}
	return hash;
}

const AnimDataLock* AnimLockManager::getAnimLockData(UInt32 refID, const char* animName) {
	AnimDataLock temp(refID, animName);
	auto [it, inserted] = allData.insert(temp);
	return &(*it);
}

const AnimDataLock* AnimLockManager::findAnimLockData(UInt32 refID, const char* animName, bool deepSearch) {

	AnimDataLock temp(refID, animName);
	auto it = allData.find(temp);
	if (it != allData.end()) {
		return &(*it);
	}

	if (deepSearch && animName != nullptr) {
		AnimDataLock tempNoAnim(refID, nullptr);  // Find for anim omitted
		it = allData.find(tempNoAnim);
		if (it != allData.end()) {
			return &(*it);
		}
	}

	return nullptr;  // Not found
}

bool AnimLockManager::hasAnimLockData(UInt32 refID, const char* animName, bool deepSearch) {

	AnimDataLock temp(refID, animName);
	auto it = allData.find(temp);
	if (it != allData.end()) {
		return true;
	}

	if (deepSearch && animName != nullptr) {
		AnimDataLock tempNoAnim(refID, nullptr);  // Find for anim omitted
		it = allData.find(tempNoAnim);
		if (it != allData.end()) {
			return true;
		}
	}

	return false;
}
/*
void AnimLockManager::UpdateAllAnims() {
	Actor* actor;
	if (!allData.empty()) {
		for (auto iter : allData) {
			BSAnimGroupSequence* seq = PluginFunctions::FindActiveAnimationForActor((Actor*)LookupFormByRefID(iter.refID), iter.animPath);
			iter.UpdateAllBlocks(seq);
		}
	}
}
*/
//BlockOverwrite..........................................................................................................................

void BlockOverwrite::addOrUpdateOverride(const Overrides& newOverride) {
	if (hasType(newOverride.type)) {
		auto it = std::find_if(overrides.begin(), overrides.end(),
			[newOverride](const Overrides& o) { return o.type == newOverride.type; });
		if (it != overrides.end()) {
			*it = newOverride;
		}
	}
	else {
		overrides.push_back(newOverride);
		types |= static_cast<UInt8>(newOverride.type);
	}
}

bool BlockOverwrite::hasType(OverrideType oType) const {
	return (types & static_cast<UInt8>(oType)) != 0;
}

void BlockOverwrite::removeOverride(OverrideType oType) {
	overrides.erase(std::remove_if(overrides.begin(), overrides.end(),
		[oType, this](const Overrides& o) {
			if (o.type == oType) {
				types &= ~static_cast<UInt8>(oType); // Clear bit when type is removed
				return true;
			}
			return false;
		}),
	overrides.end());
}

void BlockOverwrite::setPriority(int priority) {
	Overrides override(OverrideType::Priority);
	override.priority = priority;
	addOrUpdateOverride(override);
}

void BlockOverwrite::setRotation(const NiQuaternion& rotation) {
	Overrides override(OverrideType::Rotation);
	override.rotations = rotation;
	addOrUpdateOverride(override);
}

void BlockOverwrite::setTranslation(const NiPoint3& translation) {
	Overrides override(OverrideType::Translation);
	override.translation = translation;
	addOrUpdateOverride(override);
}

void BlockOverwrite::setScale(float scale) {
	Overrides override(OverrideType::Scale);
	override.scale = scale;
	addOrUpdateOverride(override);
}
/* use once other override types are in working a condition.
void BlockOverwrite::UpdateBlockOverride(NiControllerSequence::ControlledBlock* controlledBlock) {
	if (controlledBlock) {
		for (const auto& bIter : overrides) {
			switch (bIter.type) {
			case OverrideType::Rotation:
				controlledBlock->UpdateRotation(bIter.rotations);
				break;
			case OverrideType::Priority:
				controlledBlock->UpdatePriority(bIter.priority);
				break;
			case OverrideType::Scale:
				controlledBlock->UpdateScale(bIter.scale);
				break;
			case OverrideType::Translation:
				controlledBlock->UpdateTranslation(bIter.translation);
				break;
			default:
				// Optionally handle unknown type
				break;
			}
		}
	}
}
*/
//Other override types don't work atm.
void BlockOverwrite::UpdateBlockOverride(NiControllerSequence::ControlledBlock* controlledBlock) {
	if (controlledBlock) {
		for (const auto& bIter : overrides) {
			if (bIter.type == OverrideType::Priority) {
				controlledBlock->UpdatePriority(bIter.priority);
			}
		}
	}
}

//AnimLock..........................................................................................................................

void AnimDataLock::SetPriority(const char* controlledBlock, UInt8 priority) const {
	this->blockOverwrites[controlledBlock].setPriority(priority);
};

/*
void AnimDataLock::SetRotation(const char* controlledBlock, float x, float y, float z) const {
	this->blockOverwrites[controlledBlock].setRotation(NiQuaternion::createFromEuler(x, y, z));
};


void AnimDataLock::SetScale(const char* controlledBlock, float scale) const {
	this->blockOverwrites[controlledBlock].setScale(scale);
};

void AnimDataLock::SetTranslation(const char* controlledBlock, float x, float y, float z) const {
	this->blockOverwrites[controlledBlock].setTranslation(NiPoint3(0, 0, 0));
};
*/
void AnimDataLock::UpdateAllBlocks(NiControllerSequence* destAnim) const {
	for (auto iter : blockOverwrites) {
		NiControllerSequence::ControlledBlock* controlBlock = destAnim->GetControlledBlock(iter.first.c_str());
		iter.second.UpdateBlockOverride(controlBlock);
	}
}

void AnimDataLock::removeOverride(const char* block) const {
	blockOverwrites.erase(block);
}

void AnimDataLock::removeOverride(const char* block, OverrideType type) const {

	auto it = blockOverwrites.find(block);
	if (it != blockOverwrites.end() && it->second.hasType(type)) {
		it->second.removeOverride(type);
		if (it->second.overrides.empty()) {
			blockOverwrites.erase(it);
			destroy();
		}
	}

}

bool AnimDataLock::hasOverride(const char* block, OverrideType type) {

	auto it = blockOverwrites.find(block);
	if (it != blockOverwrites.end()) {
		return it->second.hasType(type);
	}
	return false;
}

bool AnimDataLock::hasOverride(const char* block) {

	auto it = blockOverwrites.find(block);
	if (it != blockOverwrites.end()) {
		return true;
	}
	return false;
}

bool AnimDataLock::IsAnimSequencePlaying(NiControllerSequence* destAnim) const
{
	if (destAnim->state != NiControllerSequence::kAnimState_Inactive && destAnim->state != NiControllerSequence::kAnimState_EaseOut) {
		return true;
	}
	return false;
};

void AnimDataLock::AddAllCurrentAnimsToCache() const
{
		Actor* actor = (Actor*)LookupFormByRefID(refID);
		if (actor) {
			AnimData* anim = actor->GetAnimData();
			for (auto iter : anim->animSequence) 
			{
				if (iter) {
					this->animCache.insert(iter);
					this->UpdateAllBlocks(iter);
				}
			}

		}

};