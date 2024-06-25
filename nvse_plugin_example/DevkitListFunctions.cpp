#include "DevkitCompiler.h"

namespace Kit {

    void DevkitCompiler::FormListAdd(std::vector<std::string>::const_iterator& it, std::istringstream& argStream)
    {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }
        BGSListForm* listForm = (BGSListForm*)form;
        TESForm* toAdd = LookupEditorID<TESForm*>(argument.c_str());
        UInt32 bCheckForDupes = false;

        if (listForm && toAdd) {
            listForm->AddAt(toAdd, eListEnd, bCheckForDupes);
        }
    }

    void ParseOwner(ContainerExtraData* newExtra, std::istringstream& iss, const std::string& argument, DevkitCompiler& compiler) {

        TESForm* owner = LookupEditorID<TESForm*>(argument.c_str());
        if (!owner) {
            compiler.PrintKitError("Owner not found for Editor ID: " + argument, iss.str());
            return;
        }

        switch (owner->typeID) {
        case 8: {
            auto* factionOwner = dynamic_cast<TESFaction*>(owner);
            if (factionOwner) {
                newExtra->ownerFaction = factionOwner;
                UInt32 rank;
                if (!(iss >> rank)) {
                    compiler.PrintKitError("Failed to parse rank for faction owner.", iss.str());
                }
                newExtra->requiredRank = rank;
            }
            else {
                compiler.PrintKitError("Failed to cast owner to TESFaction.", iss.str());
            }
            break;
        }
        case 42: {
            auto* npcOwner = dynamic_cast<TESNPC*>(owner);
            if (npcOwner) {
                newExtra->ownerNPC = npcOwner;
                std::string globalVarID;
                if (!(iss >> globalVarID) || !(newExtra->globalVar = LookupEditorID<TESGlobal*>(globalVarID.c_str()))) {
                    compiler.PrintKitError("Failed to parse or find global variable ID: " + globalVarID, iss.str());
                }
            }
            else {
                compiler.PrintKitError("Failed to cast owner to TESNPC.", iss.str());
            }
            break;
        }
        default:
            compiler.PrintKitError("Unsupported owner type ID: " + std::to_string(owner->typeID), iss.str());
            break;
        }

    }


    void DevkitCompiler::ChanceOfNone(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            PrintKitError("Unable to extract first argument for leveled list chance.", argStream.str());
            return;
        }

        bool isNumeric = true;
        for (char c : argument) {
            if (!isdigit(c)) {
                isNumeric = false;
                break;
            }
        }

        TESLeveledList* lvlList = form->GetLvlList();
        if (!lvlList) {
            PrintKitError("Leveled list not found in form.", argStream.str());
            return;
        }

        if (isNumeric) {

            int numericChance = std::atoi(argument.c_str());
            if (numericChance < 0 || numericChance > 100) {
                PrintKitError("Chance value out of bounds (0-100): " + argument, argStream.str());
                return;
            }
            lvlList->chanceNone = static_cast<UInt8>(numericChance);
            lvlList->global = nullptr;

        }
        else {

            TESGlobal* chance = LookupEditorID<TESGlobal*>(argument.c_str());
            if (!chance) {
                PrintKitError("Global not found for Editor ID: " + argument, argStream.str());
                return;
            }
            lvlList->chanceNone = chance->data;
            lvlList->global = chance;
        }

    }

    void DevkitCompiler::GatherLeveledData(std::vector<std::string>::const_iterator& it, TESLeveledList::BaseData* newData) {

        std::string argument;
        while (it != this->fileManager.currentKitFile->file.end()) {

            std::istringstream iss(*(++it));
            if (!(iss >> argument)) {
                PrintKitError("Error extracting function name.", iss.str());
                continue;
            }

            char commandType = tolower(argument[0]);
            switch (commandType) {
            case 'l':
                if (!(iss >> newData->level))
                    PrintKitError("Failed to parse level.", iss.str());
                break;
            case 'c':
                if (!(iss >> newData->count))
                    PrintKitError("Failed to parse count.", iss.str());
                break;
            case 'o':
                if (!(iss >> argument)) {
                    PrintKitError("Failed to parse owner argument.", iss.str());
                }
                else {
                    ParseOwner(newData->extra, iss, argument, *this);
                }
                break;
            case 'h':
                if (!(iss >> newData->extra->health))
                    PrintKitError("Failed to parse health.", iss.str());
                break;
            case 'e':
            case '}':
                --it;
                return;
            default:
                --it;
                return;
            }
        }
    }

    void DevkitCompiler::LeveledListAdd(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            PrintKitError("Unable to extract first argument for leveled list add.", argStream.str());
            return;
        }

        TESForm* toAdd = LookupEditorID<TESForm*>(argument.c_str());
        if (!toAdd) {
            PrintKitError("Form not found for Editor ID: " + argument, argStream.str());
            return;
        }

        TESLeveledList* lvlList = form->GetLvlList();
        if (!lvlList) {
            PrintKitError("Leveled list not found in form.", argStream.str());
            return;
        }

        auto* newData = Game_HeapAlloc<TESLeveledList::BaseData>();
        newData->form = toAdd;
        newData->level = 1;
        newData->count = 1;

        auto* newExtra = Game_HeapAlloc<ContainerExtraData>();
        newExtra->ownerNPC = nullptr;
        newExtra->globalVar = nullptr;
        newExtra->health = 1;

        newData->extra = newExtra;

        GatherLeveledData(it, newData);

        // Calculate the insertion index
        SInt32 index = 0;
        for (auto iter = lvlList->datas.Head(); iter && iter->data && iter->data->level < newData->level; iter = iter->next) {
            index++;
        }

        lvlList->datas.Insert(newData, index);
    }

}