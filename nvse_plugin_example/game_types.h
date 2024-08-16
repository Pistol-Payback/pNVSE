#pragma once
#include "GameObjects.h"
#include "GameOSDepend.h"

enum AnimMoveTypes
{
    kAnimMoveType_Walking = 0x0,
    kAnimMoveType_Sneaking = 0x1,
    kAnimMoveType_Swimming = 0x2,
    kAnimMoveType_Flying = 0x3,
};

enum MovementFlags
{
    kMoveFlag_Forward = 0x1,
    kMoveFlag_Backward = 0x2,
    kMoveFlag_Left = 0x4,
    kMoveFlag_Right = 0x8,
    kMoveFlag_TurnLeft = 0x10,
    kMoveFlag_TurnRight = 0x20,
    kMoveFlag_NonController = 0x40,
    kMoveFlag_Walking = 0x100,
    kMoveFlag_Running = 0x200,
    kMoveFlag_Sneaking = 0x400,
    kMoveFlag_Swimming = 0x800,
    kMoveFlag_Jump = 0x1000,
    kMoveFlag_Flying = 0x2000,
    kMoveFlag_Fall = 0x4000,
    kMoveFlag_Slide = 0x8000,
};

/*
void extendTopicConditions(TESTopicInfo* topicInfo) {

    if (!topicInfo || topicInfo->typeID != kFormType_TESTopicInfo) {
        return;
    }

    if (topicInfo->prompt.CStr() != nullptr) {

        auto it = topicInfo->conditions.Begin(); // Iterate through conditions list to check for skill checks.

        while (it != topicInfo->conditions.end()) {

            Condition* cond = (*it.Get());

            if (cond->function) {

                std::string skill = ExtractSkillFromCondition(cond); // Not a real function.
                if (!skill.empty()) {
                    topicInfo->prompt.Append((" " + skill).c_str());
                }
            }

            ++it; // it.Next();
        }

        topicInfo->prompt.Append("]");
    }
}
*/