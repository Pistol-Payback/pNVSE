#include "DevkitCompiler.h"

//std::vector<TESQuest*> testVector;

namespace Kit {

    void DevkitCompiler::QuestDelay(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        TESQuest* questForm = (TESQuest*)this->form;

        try {
            float delayTime = std::stof(argument);
            questForm->questDelayTime = delayTime;
        }
        catch (const std::invalid_argument& e) {
            Console_Print("Error: Invalid argument for QuestDelay: %s", argument.c_str());
        }
        catch (const std::out_of_range& e) {
            Console_Print("Error: Argument out of range for QuestDelay: %s", argument.c_str());
        }

    }

    void DevkitCompiler::QuestFlags(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        TESQuest* questForm = (TESQuest*)this->form;

        try {
            UInt8 flags = std::stof(argument);
            questForm->flags = flags;
        }
        catch (const std::invalid_argument& e) {
            Console_Print("Error: Invalid argument for QuestFlags: %s", argument.c_str());
        }
        catch (const std::out_of_range& e) {
            Console_Print("Error: Argument out of range for QuestFlags: %s", argument.c_str());
        }

    }

    void DevkitCompiler::QuestScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        Script* script = LookupEditorID<Script*>(argument.c_str());

        if (script) {

            TESQuest* questForm = static_cast<TESQuest*>(this->form);
            questForm->scriptable.script = script;
            script->quest = questForm;
            script->info.type = Script::eType_Quest;

        }

    }

    Script* CompileScriptAlt(Script* script)
    {
        const auto buffer = MakeUnique<ScriptBuffer, 0x5AE490, 0x5AE5C0>();

        buffer->scriptName.Set(script->GetEditorID());
        buffer->scriptText = script->text;
        *buffer->scriptData = 0x1D;
        buffer->dataOffset = 4;

        buffer->partialScript = (script->flags & 1) != 0;
        buffer->runtimeMode = ScriptBuffer::kEditor;
        buffer->currentScript = script;

        script->info.varCount = 0;
        script->info.numRefs = 0;
        script->varList.DeleteAll();
        script->refList.DeleteAll();

        //buffer->info.numRefs = script->info.numRefs;
        //buffer->info.varCount = script->info.varCount;

        const auto result = script->Compile(buffer.get());
        buffer->scriptText = nullptr;
        script->text = nullptr;
        if (!result)
            return nullptr;
        if (script->quest) {

            for (auto it = script->varList.begin(); it != script->varList.end(); ++it) {
                VariableInfo* var = *it;
                TESQuest::LocalVariableOrObjectivePtr* lvo = static_cast<TESQuest::LocalVariableOrObjectivePtr*>(FormHeap_Allocate(sizeof(TESQuest::LocalVariableOrObjectivePtr)));
                lvo->varInfoIndex = var;
                script->quest->lVarOrObjectives.Append(lvo);
            }

            ScriptEventList* eventList = script->CreateEventList();
            script->quest->scriptEventList = eventList;

        }

        return script;

    }

    void DevkitCompiler::CompileScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string scriptName;

        if (!(argStream >> scriptName)) {
            this->skipDocument(it);
            return;
        }

        Script* script = LookupEditorID<Script*>(scriptName.c_str());

        if (!script) {
            this->skipDocument(it);
            return;
        }

        std::stringstream scriptStream;
        std::string scriptLine;

        if ((script->flags & 1) == 0) {
            scriptStream << (this->fileManager.currentFunction + " " + scriptName) << '\n';
        }

        if (this->fileManager.currentFunction == "name" && scriptName.back() == ';') {
            scriptName.pop_back();
        }

        ++it;

        while (it != this->fileManager.currentKitFile->file.end()) {

            std::istringstream iss(*it);

            if (std::getline(iss, scriptLine)) {

                if (scriptLine == "}clear") {
                    --it;
                    break;
                }
                else {
                    scriptStream << scriptLine << '\n';
                }

            }
            else {
                Console_Print("Error: Unable to extract script line: %s", iss.str().c_str());
            }
            ++it;
        }
        std::string scriptText = scriptStream.str();
        script->text = const_cast<char*>(scriptText.c_str());
        if (!CompileScriptAlt(script)) {
            Console_Print("Failed to compile %s, in kit file %s", scriptName.c_str(), this->fileManager.currentKitFile->data->name.c_str());
        }

    }

    void DevkitCompiler::BuildScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        DataHandler::Get()->DisableAssignFormIDs(true);
        auto script = MakeUnique<Script, 0x5AA0F0, 0x5AA1A0>();
        DataHandler::Get()->DisableAssignFormIDs(false);

        script->SetRefID(GetNextFreeFormID(GetFirstFormIDForModIndex(0)), true);
        script->SetEditorID(argument.c_str());

        form = script.release();

    }

    void DevkitCompiler::skipDocument(std::vector<std::string>::const_iterator& it) {

        while (it != fileManager.currentKitFile->file.end()) {

            std::istringstream iss(*it);
            std::string scriptLine;

            if (std::getline(iss, scriptLine)) {

                if (scriptLine == "}clear") {
                    break;
                }

            }
            ++it;
        }
    }

    void DevkitCompiler::SetScriptType(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        Script* script = (Script*)form;

        if (!script) {  //Skip Script
            this->skipDocument(it);
            return;
        }

        argument = StringUtils::toLowerCase(argument);

        if (argument == "function") {
            script->flags |= 1; //IsPartial
        }

    }

    void DevkitCompiler::RegFloatVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        Script* script = (Script*)form;
        script->AddVariable(argument.c_str(), Script::eVarType_Float);

    }

    void DevkitCompiler::RegIntVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        Script* script = (Script*)form;
        script->AddVariable(argument.c_str(), Script::eVarType_Integer);

    }

    void DevkitCompiler::RegRefVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        Script* script = (Script*)form;
        script->AddVariable(nullptr, argument.c_str());
        script->AddVariable(argument.c_str(), Script::eVarType_Ref);

    }

    void DevkitCompiler::RegArrayVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        Script* script = (Script*)form;
        script->AddVariable(argument.c_str(), Script::eVarType_Array);

    }

    void DevkitCompiler::RegStringVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        Script* script = (Script*)form;
        script->AddVariable(argument.c_str(), Script::eVarType_String);

    }

}