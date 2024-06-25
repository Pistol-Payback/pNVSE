#pragma once
#include "GameObjects.h"
#include "GameOSDepend.h"
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