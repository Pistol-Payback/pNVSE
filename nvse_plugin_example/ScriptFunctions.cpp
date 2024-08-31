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

    void DevkitCompiler::BuildScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string scriptName;
        if (!(argStream >> scriptName)) {
            return;
        }

        Script* script = LookupEditorID<Script*>(scriptName.c_str());
        if (!script) {
            script = (Script*)TESForm::CreateNewForm(17, scriptName.c_str());
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

        while (it != this->fileManager.currentFile->end()) {

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
            ++it;
        }
        auto iter = toCompileLookup.find(script);

        if (iter == toCompileLookup.end()) { //set to be compiled

            scriptToCompile* toCompileScript = new scriptToCompile{ script, scriptStream.str(), false};
            toCompile.push_back(toCompileScript);
            toCompileLookup[script] = toCompileScript;

        }
        else {

            iter->second->text = scriptStream.str(); //Overwrite the text for the script.

        }

        form = script;

    }

    Script* DevkitCompiler::BuildScriptCondition(std::istringstream& iss, const std::string& arguments)
    {
        std::string condition;
        if (getQuotedString(iss, condition)) { //Inline

            Script* partial = (Script*)TESForm::CreateNewForm(17);
            if (!partial) {
                PrintKitError("Failed to parse condition.", iss.str());
            }
            else {
                toCompile.push_back(new scriptToCompile{ partial, condition, true, arguments});
                return partial;
            }
        }
        else if (!(iss >> condition)) {
            PrintKitError("missing argument for condition.", iss.str());
        }
        else { //Passed a form
            return (Script*)LookupEditorID<TESForm*>(condition.c_str());
        }

        return nullptr;

    }

    void DevkitCompiler::skipDocument(std::vector<std::string>::const_iterator& it) {

        while (it != fileManager.currentFile->end()) {

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