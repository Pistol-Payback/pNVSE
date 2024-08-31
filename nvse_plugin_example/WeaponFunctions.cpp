#include "DevkitCompiler.h"

namespace Kit {

    void DevkitCompiler::Set1stPersonWeaponModel(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        std::string modelPath;
        TESObjectSTAT* FirstPersonModel = nullptr;
        TESObjectWEAP* weapForm = static_cast<TESObjectWEAP*>(form);

        if (!weapForm) return;

        if (getQuotedString(argStream, modelPath)) {

            FirstPersonModel = weapForm->firstPersonStatic;
            if (!FirstPersonModel) {
                //std::string newEditorID = "FirstPerson" + std::string(EditorID); //Do these need editorIDs?
                FirstPersonModel = (TESObjectSTAT*)TESForm::CreateNewForm(32, nullptr, true, 0); //Make a to delete vector
            }
            FirstPersonModel->model.SetPath(modelPath.c_str());

        }
        else if (!(argStream >> argument)) {
            PrintKitError("missing argument for first person.", argStream.str());
            return;
        }
        else {

            FirstPersonModel = LookupEditorID<TESObjectSTAT*>(argument.c_str());
            if (getQuotedString(argStream, modelPath)) {
                FirstPersonModel->model.SetPath(modelPath.c_str());
            }

        }

        int modelIndex = -1;

        if (!FirstPersonModel) return;

        if (!(argStream >> modelIndex) || modelIndex == -1) { //Second argument index for weapons
            std::fill_n(weapForm->modStatics, 7, FirstPersonModel);
            weapForm->firstPersonStatic = FirstPersonModel;
        }
        else {
            if (modelIndex == 0) {
                weapForm->firstPersonStatic = FirstPersonModel;
            }
            else if (modelIndex >= 1 && modelIndex <= 7) {
                weapForm->modStatics[modelIndex - 1] = FirstPersonModel;
            }
            else {
                PrintKitError("Model index out of range.", argStream.str());
            }
        }

        if (isNested) {
            form = FirstPersonModel;
        }
        else {
            argStream >> std::ws;
            if (argStream.peek() != EOF) {
                TESForm* temp = form;
                form = FirstPersonModel;
                SetTextureSet(it, argStream);
                form = temp;
            }
        }

    }

    void DevkitCompiler::SetWeaponWorldModel(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string modelPath;
        TESObjectWEAP* weapForm = static_cast<TESObjectWEAP*>(form);

        if (!weapForm) return;

        if (!getQuotedString(argStream, modelPath)) {
            return;
        }

        std::vector<char> transferablePath(modelPath.begin(), modelPath.end());
        transferablePath.push_back('\0');

        int modelIndex = -1;
        if (!(argStream >> modelIndex) || modelIndex == -1) {
            weapForm->textureSwap.SetModelPath(transferablePath.data());
            for (int i = 0; i < 7; ++i) {
                weapForm->modModels[i].SetModelPath(transferablePath.data());
            }
        }
        else {// Apply the model path to a specific index
            if (modelIndex == 0) {
                weapForm->textureSwap.SetModelPath(transferablePath.data());
            }
            else if (modelIndex >= 1 && modelIndex <= 7) {
                weapForm->modModels[modelIndex - 1].SetModelPath(transferablePath.data());
            }
            else {
                PrintKitError("Model index out of range.", argStream.str());
            }
        }
    }

    void DevkitCompiler::BuildSlot(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(getQuotedString(argStream, argument))) {
            slot = "null";
            return;
        }

        slot = argument;
        std::string functionName;

        while (it != this->fileManager.currentFile->end()) {

            ++it;
            std::istringstream iss(*it);
            if (!(iss >> functionName)) {
                PrintKitError("In slot, unable to extract function name " + functionName, iss.str());
            }
            else if (functionName = StringUtils::toLowerCase(functionName); functionName[0] == '}' || functionName == "editorid:" || functionName == "slot:") {
                slot = "null";
                --it;
                break;
            }
            else {
                this->fileManager.CallTypeFunction(*this, it, 103);
            }

        }

    }

    void DevkitCompiler::skipLink(std::vector<std::string>::const_iterator& it) {

        std::string argument;

        while (it != this->fileManager.currentFile->end()) {

            ++it;
            std::istringstream iss(*it);
            if (!(iss >> argument)) {
                continue;
            }

            argument = StringUtils::toLowerCase(argument);
            if (argument == "editorid:" || argument[0] == '}' || argument == "link:" || argument == "slot:") {
                --it;
                return;
            }
        }

    }

    void DevkitCompiler::BuildWeaponModLink(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            form = nullptr;
            this->skipLink(it);
            return;
        }

        form = LookupEditorID<TESForm*>(argument.c_str());

        if (form && staticParent) {

            form->MarkAsStaticForm(this->fileManager.currentKitFile->data->index);

            StaticInstance_WEAP* staticWeap = (StaticInstance_WEAP*)staticParent;

            if (std::find(staticWeap->aAllAttachments[slot].begin(), staticWeap->aAllAttachments[slot].end(), form->refID) == staticWeap->aAllAttachments[slot].end()) {
                staticWeap->aAllAttachments[slot].push_back(form->refID);
            }

            StaticInstance* staticForm = form->LookupStaticInstance();
            if (staticForm) {
                AuxVector* aux = staticForm->GetBaseTrait("IsBaseMod");
                if (aux) {
                    staticWeap->aBaseAttachments[slot] = form->refID;
                    staticForm->EraseBaseTrait("IsBaseMod");
                }
            }

        }
        else {
            form = nullptr;
            this->skipLink(it);
        }

    }

    void DevkitCompiler::SetWeaponAnimation(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

    }

    template<typename T, typename U>
    void setProperty(TESObjectWEAP* weapon, std::istringstream& argStream, U T::* property) {
        if (!weapon) return;

        U argument;
        if (!(argStream >> argument)) return;
        weapon->*property = argument;
    }

    // Specialization for nested member within TESObjectWEAP
    template<typename T, typename U>
    void setSubProperty(TESObjectWEAP* weapon, std::istringstream& argStream, T TESObjectWEAP::* subObject, U T::* member) {
        if (!weapon) return;

        T* subObjectPtr = &(weapon->*subObject);
        U argument;
        if (!(argStream >> argument)) return;
        subObjectPtr->*member = argument;
    }

    void DevkitCompiler::setStrReq(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::strRequired);
    }
    void DevkitCompiler::setSkillReq(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::skillRequirement);
    }
    void DevkitCompiler::setSkill(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::weaponSkill);
    }
    void DevkitCompiler::setFireRate(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::fireRate);
    }
    void DevkitCompiler::setSpread(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::spread);
    }
    void DevkitCompiler::setMinSpread(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::minSpread);
    }
    void DevkitCompiler::setCritDmg(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::criticalDamage);
    }
    void DevkitCompiler::setCritChance(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::criticalPercent);
    }
    void DevkitCompiler::setReach(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::reach);
    }


    void DevkitCompiler::setProjectile(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
    }



    void DevkitCompiler::setNumProjectiles(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::numProjectiles);
    }
    void DevkitCompiler::setAmmoUse(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::ammoUse);
    }
    void DevkitCompiler::setLimbDmgMult(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::limbDamageMult);
    }
    void DevkitCompiler::setDamage(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setSubProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::attackDmg, &TESAttackDamageForm::damage);
    }




    void DevkitCompiler::setActionPointsUsed(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
    }
    void DevkitCompiler::setCritEffect(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
    }





    void DevkitCompiler::setVatsHitChance(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::baseVATSChance);
    }

    void DevkitCompiler::setDegradationMult(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::damageToWeaponMult);
    }

    void DevkitCompiler::setClipSize(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        setSubProperty(static_cast<TESObjectWEAP*>(form), argStream, &TESObjectWEAP::clipRounds, &BGSClipRoundsForm::clipRounds);
    }

}
