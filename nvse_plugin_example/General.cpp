#include "DevkitCompiler.h"

namespace Kit {

    void DevkitCompiler::skipFunction(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        return;
    }

    void DevkitCompiler::skipNestedFunction(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {
        if (isNested) {
            skipNested(it, argStream);
        }
        return;
    }

    void DevkitCompiler::skipNested(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string command;

        while (it != this->fileManager.currentFile->end()) {
            ++it;
            argStream.str(*it);
            argStream.clear(); // Clear any error flags

            if (!(argStream >> command)) {
                continue;
            }

            char commandType = tolower(command[0]);
            switch (commandType) {
            case '}':
                --it;
                return;
            }

        }

    }

    void DevkitCompiler::RunOperatorOverloads(const char* m_operator, std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        if (!this->fileManager.operatorOverload.empty()) {
            auto functIt = this->fileManager.operatorOverload.find(m_operator);
            if (functIt != this->fileManager.operatorOverload.end()) {
                auto funct = functIt->second;
                if (funct.type == 1) {
                    (this->*(funct.functIter))(it, argStream);
                }
                else {
                    (this->*(funct.functSelf))(funct, argStream);
                }
            }
        }

    }

    void DevkitCompiler::EnterNestedState() {

        this->nested.push_back(new NestedState{ form, templateForm, slot, staticParent, templateEXB, fileManager.type });
        //RunOperatorOverloads("{", it, argStream);

    }

    void DevkitCompiler::ExitNestedState() {

        NestedState* nest = this->nested.back();
        this->fileManager.type = nest->type;
        this->templateForm = nest->templateForm;
        this->templateEXB = nest->templateEXB;
        this->staticParent = nest->staticParent;
        this->form = nest->form;
        this->slot = nest->slot;
        this->nested.pop_back();
        delete nest;

       // RunOperatorOverloads("}", it, argStream);

    }

    void DevkitCompiler::ClearNestedState() {
        while (!this->nested.empty()) {
            NestedState* nest = this->nested.back();
            this->nested.pop_back();
            delete nest;
        }
    }

    void DevkitCompiler::EndOfDocument(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        this->templateForm = nullptr;
        this->templateEXB = nullptr;
        this->staticParent = nullptr;
        this->form = nullptr;
        this->slot = "null";
        this->fileManager.operatorOverload.clear();

        ClearNestedState();

    }

    void DevkitCompiler::SetType(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        if (!(argStream >> this->fileManager.type)) {
            this->fileManager.type = 0;
            return;
        }

        this->templateForm = nullptr;
        this->templateEXB = nullptr;
        this->staticParent = nullptr;
        this->form = nullptr;
        this->slot = "null";

    }

    void DevkitCompiler::SetTemplate(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

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
        ExtendedBaseType* staticObject = nullptr;

        if (form) {
            staticObject = form->LookupExtendedBase();
        }

        if (staticParent) {
            aux = staticParent->SetTrait(trait, staticObject, slot);
        }
        else if (staticObject){
            aux = staticObject->SetTrait(trait);
        }
        else {
            return;
        }

        int numArguments = function.argumentTypes.size();
        size_t i = 0;
        SInt32 iOffset = 0;

        while (i < numArguments && function.argumentTypes[i]) {

            bool allowDuplicate = (function.argumentTypes[i] & function.allowDuplicate) != 0;
            bool isOptional = (function.argumentTypes[i] & function.optional) != 0;
            UInt8 argTypeCode = function.argumentTypes[i] & ~(function.allowDuplicate | function.optional);

            switch (argTypeCode) {
            case 1: { // 'r' argument type
                std::string arg;
                if (argStream >> arg) {

                    TESForm* reference = LookupEditorID<TESForm*>(arg.c_str());

                    if (allowDuplicate) {

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
                else if (isOptional) {
                    aux->AddEmptyValue(iOffset);
                }
                else {
                    PrintKitError("Trait [" + function.name + "] failed to compile, could not extract ref argument " + std::to_string(i), argStream.str());
                    aux->AddEmptyValue(iOffset);
                }
                break;
            }
            case 2: // 'i' argument type
            case 3: { // 'f' argument type
                double arg;
                if (argStream >> arg) {

                    if (allowDuplicate) {
                        iOffset = aux->Find(arg);
                    }
                    aux->AddValue(iOffset, arg);

                }
                else if (isOptional) {
                    aux->AddEmptyValue(iOffset);
                }
                else {
                    PrintKitError("Trait [" + function.name + "] failed to compile, could not extract float argument " + std::to_string(i), argStream.str());
                    aux->AddEmptyValue(iOffset);
                }
                break;
            }
            case 4: { // 's' argument type
                std::string arg;
                if (getQuotedString(argStream, arg)) {

                    if (allowDuplicate) {
                        iOffset = aux->Find(arg.c_str());
                    }
                    aux->AddValue(iOffset, arg.c_str());

                }
                else if (isOptional) {
                    aux->AddEmptyValue(iOffset);
                }
                else {
                    PrintKitError("Trait [" + function.name + "] missing quotation marks, could not extract string argument " + std::to_string(i), argStream.str());
                    aux->AddValue(iOffset, "");
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

        if (!PluginFunctions::SetDescriptionScriptJIP) {
            PrintKitError("Error Function [Description:] Requires JIP", argStream.str());
            return;
        }
        std::string argument;
        if (!(getQuotedString(argStream, argument))) {
            PrintKitError("Error Function [Description:] missing first argument, or quotation marks", argStream.str());
            return;
        }

        TESDescription* description = DYNAMIC_CAST(form, TESForm, TESDescription);
        if (description || (IS_ID(form, BGSNote) && (description = ((BGSNote*)form)->noteText)))
        {
            //PluginFunctions::SetDescriptionJIP(description, argument.c_str());
            g_scriptInterface->CallFunction(PluginFunctions::SetDescriptionScriptJIP, nullptr, nullptr, nullptr, 2, form, argument.c_str());
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

    void DevkitCompiler::SetTextureSet(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        TESModelTextureSwap* modelSwap = DYNAMIC_CAST(form, TESForm, TESModelTextureSwap);
        if (!modelSwap) {
            return;
        }

        std::string path;
        if (!(argStream >> path)) {
            PrintKitError("TextureSet missing index or path.", argStream.str());
            return;
        }

        if (StringUtils::isNumber(path)) { // Path is an index

            int index = std::stoi(path);

            if (index == -1) {
                modelSwap->textureList.RemoveAll();
                return;
            }

            auto iter = modelSwap->find(index);
            if (argStream >> path) { // There is a path to replace the texture set

                BGSTextureSet* newTextureSet = LookupEditorID<BGSTextureSet*>(path.c_str());
                if (!newTextureSet) {
                    PrintKitError("TextureSet not valid for second argument.", argStream.str());
                    return;
                }
                if (iter) {
                    iter->data->textureID = newTextureSet;
                    iter->data->textureName[0] = '\0';
                }
                else {
                    modelSwap->insert(index, newTextureSet);
                }

            }
            else if (iter) { // No path provided, remove the existing texture set
                modelSwap->textureList.Remove(iter->data);
            }

        }
        else { // Path is a texture set form
            BGSTextureSet* toReplace = LookupEditorID<BGSTextureSet*>(path.c_str());
            if (!toReplace) {
                PrintKitError("First argument is not a valid form.", argStream.str());
                return;
            }
            auto iter = modelSwap->find(toReplace);
            if (iter && (argStream >> path)) { // Replace found texture set with new path
                BGSTextureSet* newTextureSet = LookupEditorID<BGSTextureSet*>(path.c_str());
                if (newTextureSet) {
                    iter->data->textureID = newTextureSet;
                }
                else {
                    PrintKitError("TextureSet not valid for second argument.", argStream.str());
                }
            }
            else if (iter) { // No replacement path provided, remove the texture set
                modelSwap->textureList.Remove(iter->data);
            }
            else {
                PrintKitError("Texture set not found in texture set list.", argStream.str());
            }
        }
    }
    

    void DevkitCompiler::skipForm(std::vector<std::string>::const_iterator& it) {

        std::string argument;

        while (it != this->fileManager.currentFile->end()) {

            ++it;
            std::istringstream iss(*it);
            if (!(iss >> argument)) {
                continue;
            }

            argument = StringUtils::toLowerCase(argument);
            if (argument == "editorid:" || argument == "}clear" || argument == "template:") {
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
                form = TESForm::CreateNewForm(templateForm, true, argument.c_str(), true);
            }
            else {
                form = TESForm::CreateNewForm(this->fileManager.type, argument.c_str(), true);
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

            staticParent = form->MarkAsStaticForm(this->fileManager.currentKitFile->data->index);

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
            rightID = leftID;
        }

        TESForm* left = LookupEditorID<TESForm*>(leftID.c_str());
        TESForm* right = LookupEditorID<TESForm*>(rightID.c_str());

        if (!left || !right || left->typeID != 40 || right->typeID != 40) {
            PrintKitError("Akimbo, both forms must be a weapon.", argStream.str());
            this->skipForm(it);
            return;
        }

        auto currentKitIndex = this->fileManager.currentKitFile->data->index;

        if (staticParent = StaticInstance_Akimbo::LookupAkimboSet(left, right)) { //Existing set

            if (templateEXB) {
                staticParent->parent = templateEXB->parent;
                staticParent->linkedTraits = templateEXB->linkedTraits;
                staticParent->traits = templateEXB->traits;
            }
            staticParent->MarkAsEdit(currentKitIndex);
            form = staticParent->parent;

        }
        else if (templateEXB) { //Template set

            staticParent = new StaticInstance_Akimbo(
                templateEXB->parent,
                currentKitIndex, 
                left, 
                right,
                templateEXB->traits,
                templateEXB->linkedTraits
            );

        }
        else { //New set

            staticParent = new StaticInstance_Akimbo(
                nullptr,
                currentKitIndex,
                left,
                right
            );

        }

    }

    void DevkitCompiler::SetAkimboTemplate(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;

        std::string leftID, rightID;
        if (!(argStream >> leftID)) {
            PrintKitError("Akimbo Template left EditorID missing for the first argument.", argStream.str());
            return;
        }

        if (!(argStream >> rightID) || (isNested && rightID == "{")) {
            rightID = leftID;
        }

        TESForm* left = LookupEditorID<TESForm*>(leftID.c_str());
        TESForm* right = LookupEditorID<TESForm*>(rightID.c_str());

        if (!left || !right || left->typeID != 40 || right->typeID != 40) {
            PrintKitError("Akimbo Template, both forms must be a weapon.", argStream.str());
            return;
        }

        templateEXB = StaticInstance_Akimbo::LookupAkimboSet(left, right);
        if (!templateEXB) {
            PrintKitError("Akimbo, template does not exist.", argStream.str());
        }

        return;

    }

    void DevkitCompiler::SetQuestItem(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

    }

}