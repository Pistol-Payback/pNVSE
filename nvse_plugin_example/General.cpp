#include "DevkitCompiler.h"

namespace Kit {

    void DevkitCompiler::BreakFromTemplate(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        this->templateForm = nullptr;
        this->staticParent = nullptr;
        this->form = nullptr;
        this->slot = "null";

    }

    void DevkitCompiler::SetType(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        if (!(argStream >> this->fileManager.type)) {
            this->fileManager.type = 0;
            return;
        }

        this->templateForm = nullptr;
        this->staticParent = nullptr;
        this->form = nullptr;
        this->slot = "null";

    }

    void DevkitCompiler::SetTemplate(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;

        if (!(argStream >> argument)) {
            this->templateForm = nullptr;
            return;
        }

        this->templateForm = LookupEditorID<TESForm*>(argument.c_str());
        if (this->templateForm) {

            if (this->templateForm->typeID != this->fileManager.type) {
                PrintKitError("template " + argument + " was not the same type as " + argument, argStream.str());
                this->templateForm = nullptr;
            }

        }
        else {
            PrintKitError("template " + argument + " was not a valid EditorID", argStream.str());
        }

        this->staticParent = nullptr;
        this->form = nullptr;
        this->slot = "null";

    }

    void DevkitCompiler::SetTrait(TypeFunction<DevkitCompiler>& function, std::istringstream& argStream) {

        // Process each argument based on its type
        const char* trait = function.name.c_str();
        AuxVector* aux;

        if (!form) {
            return;
        }

        StaticInstance* staticObject = form->LookupStaticInstance();

        if (staticParent) {
            aux = staticParent->SetTrait(trait, staticObject, slot);
        }
        else {
            aux = staticObject->SetTrait(trait);
        }

        int numArguments = function.argumentTypes.size();
        size_t i = 0;
        SInt32 iOffset = 0;

        while (i < numArguments && function.argumentTypes[i]) {

            switch (function.argumentTypes[i] & ~function.allowDuplicate) {
            case 1: { // 'r' argument type
                std::string arg;
                if (argStream >> arg) {
                    TESForm* reference = LookupEditorID<TESForm*>(arg.c_str());

                    if (function.argumentTypes[i] & function.allowDuplicate) {

                        if (!reference) {
                            PrintKitError("Trait [" + function.name + "] failed to compile, " + arg + " does not exist for argument " + std::to_string(i), argStream.str());
                            numArguments = 0;
                            break;
                        }
                        iOffset = aux->Find(reference->refID);
                    }
                    else if (!reference) {
                        PrintKitError("Trait [" + function.name + "] failed to compile, " + arg + " does not exist for argument " + std::to_string(i), argStream.str());
                        aux->AddEmptyValue(iOffset);
                        break;
                    }

                    aux->AddValue(iOffset, reference->refID);

                }
                else {
                    PrintKitError("Trait [" + function.name + "] failed to compile, could not extract ref argument " + std::to_string(i), argStream.str());
                    aux->AddEmptyValue(iOffset);
                }
                break;
            }
            case 2: { // 'i' argument type
                double arg;
                if (argStream >> arg) {
                    aux->AddValue(iOffset, arg);
                }
                else {
                    PrintKitError("Trait [" + function.name + "] failed to compile, could not extract float argument " + std::to_string(i), argStream.str());
                    aux->AddEmptyValue(iOffset);
                }
                break;
            }
            case 3: { // 'f' argument type
                double arg;
                if (argStream >> arg) {
                    aux->AddValue(iOffset, arg);
                }
                else {
                    PrintKitError("Trait [" + function.name + "] failed to compile, could not extract float argument " + std::to_string(i), argStream.str());
                    aux->AddEmptyValue(iOffset);
                }
                break;
            }
            case 4: { // 's' argument type
                std::string arg;
                if (!(getQuotedString(argStream, arg))) {
                    PrintKitError("Trait [" + function.name + "] missing quotation marks, could not extract string argument " + std::to_string(i), argStream.str());
                }
                if (!arg.empty()) {

                    if (function.argumentTypes[i] & function.allowDuplicate) {
                        iOffset = aux->Find(arg.c_str());
                    }

                    aux->AddValue(iOffset, arg.c_str());

                }
                else {
                    PrintKitError("Trait [" + function.name + "] failed to compile, could not extract string argument " + std::to_string(i), argStream.str());
                    aux->AddEmptyValue(iOffset);
                }
                break;
            }
            default:
                // Handle unsupported argument types or additional types if needed
                break;
            }

            ++i;
            if (iOffset != -1) {
                ++iOffset;
            }

        }

    }

    void DevkitCompiler::SetDescription(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(getQuotedString(argStream, argument))) {
            Console_Print("Error Function [Description:] failed to compile for object: %s", form->GetEditorID());
            return;
        }

        TESDescription* description = DYNAMIC_CAST(form, TESForm, TESDescription);
        if (description || (IS_ID(form, BGSNote) && (description = ((BGSNote*)form)->noteText)))
        {
            PluginFunctions::SetDescriptionJIP(description, argument.c_str());
        }

    }

    void DevkitCompiler::SetName(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(getQuotedString(argStream, argument))) {
            Console_Print("Error Function [Name:] failed to compile for object: %s", form->GetEditorID());
            return;
        }

        TESFullName* fullName = form->GetFullName();
        if (fullName) fullName->name.Set(argument.c_str());

    }

    void DevkitCompiler::SetWeight(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        float argument;

        if (!(argStream >> argument)) {
            return;
        }

        if (form) {
            TESWeightForm* pWeightForm = DYNAMIC_CAST(form, TESForm, TESWeightForm);
            if (pWeightForm) {
                pWeightForm->weight = argument;
            }
            else {
                TESAmmo* pAmmo = DYNAMIC_CAST(form, TESForm, TESAmmo);
                if (pAmmo) {
                    pAmmo->weight = argument;
                }
            }
        }

    }

    void DevkitCompiler::SetValue(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        UInt32 argument;
        if (!(argStream >> argument)) {
            return;
        }

        if (TESValueForm* valForm = DYNAMIC_CAST(form, TESForm, TESValueForm))
        {
            valForm->value = argument;
        }
        else if IS_ID(form, AlchemyItem)
            ((AlchemyItem*)form)->value = argument;

    }

    void DevkitCompiler::SetScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

    }

    void DevkitCompiler::SetInvenotryImage(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(getQuotedString(argStream, argument))) {
            return;
        }

        TESTexture* texture = DYNAMIC_CAST(form, TESForm, TESTexture);
        if (texture)
        {
            texture->ddsPath.Set(argument.c_str());
        }


    }

    void DevkitCompiler::SetMessageIcon(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(getQuotedString(argStream, argument))) {
            return;
        }

        bool isFemale = 0;
        argStream >> isFemale;

        TESBipedModelForm* bipedModel = DYNAMIC_CAST(form, TESForm, TESBipedModelForm);
        if (bipedModel)
        {
            bipedModel->messageIcon[isFemale].icon.ddsPath.Set(argument.c_str());
        }
        else {
            BGSMessageIcon* icon = DYNAMIC_CAST(form, TESForm, BGSMessageIcon);
            if (icon) {
                icon->icon.ddsPath.Set(argument.c_str());
            }
        }

    }

    void DevkitCompiler::SetModel(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!getQuotedString(argStream, argument)) {
            return;
        }

        TESModel* model = DYNAMIC_CAST(form, TESForm, TESModel);
        if (model)
            model->SetPath(argument.c_str());

    }

    void DevkitCompiler::skipForm(std::vector<std::string>::const_iterator& it) {

        std::string argument;

        while (it != this->fileManager.currentKitFile->file.end()) {

            std::istringstream iss(*(++it));
            if (!(iss >> argument)) {
                continue;
            }

            argument = StringUtils::toLowerCase(argument);
            if (argument == "editorid:" || argument == "}clear") {
                --it;
                return;
            }
        }

    }

    void DevkitCompiler::BuildForm(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            PrintKitError("EditorID missing first argument.", argStream.str());
            form = nullptr;
            this->skipForm(it);
            return;
        }

        form = LookupEditorID<TESForm*>(argument.c_str());
        if (!form) {

            if (templateForm && templateForm->typeID == this->fileManager.type) {
                form = TESForm::CreateNewForm(templateForm, argument.c_str(), false, 0, this->fileManager.currentKitFile->data->index);
            }
            else {
                form = TESForm::CreateNewForm(this->fileManager.type, argument.c_str(), false, 0, this->fileManager.currentKitFile->data->index);
            }


            if (!form) {
                PrintKitError("Failed to create a new form from template or by type.", argStream.str());
                form = nullptr;
                this->skipForm(it);
            }
            else if (form->typeID != this->fileManager.type) {
                PrintKitError("Form type mismatch: expected " + std::to_string(this->fileManager.type) + ", got " + std::to_string(form->typeID), argStream.str());
                form = nullptr;
                this->skipForm(it);
                return;
            }
        }
        else {

            // If form is created or found, ensure it has a static parent
            if (!(staticParent = form->LookupStaticInstance())) {
                staticParent = form->MarkAsStaticForm(this->fileManager.currentKitFile->data->index);
                if (!staticParent) {
                    PrintKitError("Failed to mark form as static or find static instance.", argStream.str());
                    form = nullptr;
                    this->skipForm(it);
                }
                return;
            }

            staticParent->MarkAsEdit(this->fileManager.currentKitFile->data->index);

        }

    }

    void DevkitCompiler::BuildAkimboForm(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string leftID, rightID;
        if (!(argStream >> leftID)) {
            PrintKitError("Akimbo left EditorID missing for the first argument.", argStream.str());
            this->skipForm(it);
            return;
        }

        if (!(argStream >> rightID)) {
            //PrintKitError("Akimbo right EditorID missing for the second argument.", argStream.str());
            rightID = leftID;
           // this->skipForm(it);
           // return;
        }

        TESForm* left = LookupEditorID<TESForm*>(leftID.c_str());
        TESForm* right = LookupEditorID<TESForm*>(rightID.c_str());

        if (!left || !right || left->typeID != 40 || right->typeID != 40) {
            PrintKitError("Akimbo, both forms must be a weapon.", argStream.str());
            this->skipForm(it);
            return;
        }

        auto currentKitIndex = this->fileManager.currentKitFile->data->index;

        if (staticParent = StaticInstance_Akimbo::LookupAkimboSet(left, right)) {

            staticParent->MarkAsEdit(currentKitIndex);

        }
        else {

            staticParent = new StaticInstance_Akimbo(
                currentKitIndex, 
                (StaticInstance_WEAP*)left->LookupStaticInstance(), 
                (StaticInstance_WEAP*)right->LookupStaticInstance()
            );

        }
    }

    void DevkitCompiler::SetQuestItem(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

    }

}