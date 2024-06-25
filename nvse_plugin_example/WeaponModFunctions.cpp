#include "DevkitCompiler.h"

namespace Kit {

    void DevkitCompiler::Set1stPersonAttachmentModel(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string modelPath;
        bool quoted = argStream.peek() == '"';

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

        if (staticParent->parent->typeID == 40) {

            StaticInstance_WEAP* staticWeap = (StaticInstance_WEAP*)staticParent;
            staticWeap->aBaseAttachments[slot] = form->refID;

        }
        else {

            StaticInstance* staticForm = form->LookupStaticInstance();
            if (staticForm) {
                staticForm->SetBaseTrait("IsBaseMod");
            }
        }

    }

}