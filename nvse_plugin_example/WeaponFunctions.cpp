#include "DevkitCompiler.h"

namespace Kit {

    void DevkitCompiler::Set1stPersonWeaponModel(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        std::string modelPath;
        TESObjectSTAT* FirstPersonModel = nullptr;
        bool quoted = false;

        if (getQuotedString(argStream, modelPath)) {

            quoted = true;

            if (!staticParent) {
                return;
            }
            ((StaticInstance_WEAP*)staticParent)->FirstPersonModelPath = modelPath;
            FirstPersonModel = g_1stPersonWeapModel;

        }
        else {

            FirstPersonModel = LookupEditorID<TESObjectSTAT*>(argument.c_str());

        }

        TESObjectWEAP* weapForm = reinterpret_cast<TESObjectWEAP*>(form);

        if (!FirstPersonModel || !weapForm || weapForm->worldStatic == FirstPersonModel) return;

        if (quoted) {

            std::fill_n(weapForm->modStatics, 7, FirstPersonModel);
            weapForm->worldStatic = FirstPersonModel;

        }
        else {

            int modelIndex = -1;
            if (!(argStream >> modelIndex) || modelIndex < 1 || modelIndex > 7) {
                std::fill_n(weapForm->modStatics, 7, FirstPersonModel);
                weapForm->worldStatic = FirstPersonModel;
            }
            else {
                weapForm->modStatics[modelIndex - 1] = FirstPersonModel;
            }
        }
    }

    void DevkitCompiler::BuildSlot(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(getQuotedString(argStream, argument))) {
            slot = "null";
            return;
        }

        slot = argument;
        std::string functionName;

        while (it != this->fileManager.currentKitFile->file.end()) {

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

        while (it != this->fileManager.currentKitFile->file.end()) {

            std::istringstream iss(*(++it));
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

    void DevkitCompiler::BuildWeaponModLink(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

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

}
