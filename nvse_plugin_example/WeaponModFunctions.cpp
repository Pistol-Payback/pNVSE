#include "DevkitCompiler.h"

namespace Kit {

    void DevkitCompiler::Set1stPersonAttachmentModel(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string modelPath;

        if (!getQuotedString(argStream, modelPath)) {
            return;
        }

        StaticInstance* attachment = form->LookupStaticInstance();
        AuxVector* aux;
        const char* trait = "AttachedModel";
        if (staticParent) {
            aux = staticParent->SetTrait(trait, attachment, slot);
        }
        else {
            aux = attachment->SetTrait(trait);
        }

        UInt8 IsFirstPerson = 0;

        if (!(argStream >> IsFirstPerson)) {
            aux->AddValue(0, modelPath.c_str());
            aux->AddValue(1, modelPath.c_str());
        }
        else if (IsFirstPerson) {
            aux->AddValue(0, modelPath.c_str());
        }
        else {
            aux->AddValue(1, modelPath.c_str());
        }

    }

    void DevkitCompiler::SetBaseMod(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        if (!form) {
            return;
        }

        if (staticParent && staticParent->extendedType == 40 && ((StaticInstance*)staticParent)->parent->typeID == 40) {

            StaticInstance_WEAP* staticWeap = (StaticInstance_WEAP*)staticParent;
            staticWeap->aBaseAttachments[slot] = form->refID;

        }
        else if (staticParent && staticParent->extendedType == 103 && ((StaticInstance*)staticParent)->parent->typeID == 103) { //Is item mod

            StaticInstance* staticForm = form->LookupStaticInstance();
            if (staticForm) {
                staticForm->SetBaseTrait("IsBaseMod");
            }
        }

    }

    void DevkitCompiler::SetOnAttachWeaponMod(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        if (!form) {
            return;
        }

        bool runOnReconstruct;
        if (!(argStream >> runOnReconstruct)) {
            return;
        }

        Script* script = BuildScriptCondition(argStream, "ref rModifier, ref rWeapon");

        if (!script) {
            return;
        }

        AuxVector filter(2);
        filter.SetValue(0, form->refID);

        if (staticParent && staticParent->extendedType == 40 && ((StaticInstance*)staticParent)->parent->typeID == 40) { //If linked

            filter.SetValue(1, staticParent->parent->refID); //Only apply the event for this attachment on this weapon

        }

        Event eEvent(0, script, filter);
        onAttachWeapModEvent.AddEvent(eEvent);
        if (runOnReconstruct) {
            onAttachWeapModReconstructEvent.AddEvent(eEvent);
        }

    }

    void DevkitCompiler::SetOnDetachWeaponMod(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        if (!form) {
            return;
        }

        bool runOnDeconstruct;
        if (!(argStream >> runOnDeconstruct)) {
            return;
        }

        Script* script = BuildScriptCondition(argStream, "ref rModifier, ref rWeapon");

        if (!script) {
            return;
        }

        AuxVector filter(2);
        filter.SetValue(0, form->refID);

        if (staticParent && staticParent->extendedType == 40 && ((StaticInstance*)staticParent)->parent->typeID == 40) { //If linked

            filter.SetValue(1, staticParent->parent->refID); //Only apply the event for this attachment on this weapon

        }

        Event eEvent(0, script, filter);
        onDetachWeapModEvent.AddEvent(eEvent);
        if (runOnDeconstruct) {
            onDetachWeapModDeconstructEvent.AddEvent(eEvent);
        }

    }

}