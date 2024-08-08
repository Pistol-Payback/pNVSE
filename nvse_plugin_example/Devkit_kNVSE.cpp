#include "DevkitCompiler.h"

namespace Kit {

    void OverrideFormAnimationForPerspective(bool firstPerson, TESForm* form, const std::filesystem::path& baseDirectory, Script* conditionScript, bool pollCondition) {

        std::unordered_set<UInt16> variantIds;
        for (const auto& iter : std::filesystem::recursive_directory_iterator(baseDirectory))
        {
            if (_stricmp(iter.path().extension().string().c_str(), ".kf") != 0)
                continue;
            const auto& path = iter.path().string();
            const auto& relPath = std::filesystem::path(path.substr(path.find("AnimGroupOverride\\")));
            std::string pathStr = relPath.string();
            PluginFunctions::OverrideFormAnimation(form, pathStr.c_str(), firstPerson, true, variantIds, conditionScript, pollCondition);
        }

    }

    void OverrideFormAnimationForDirectory(TESForm* form, const std::filesystem::path& baseDirectory, Script* conditionScript, bool pollCondition) {
        bool firstPerson = false;

        for (const auto& iter : std::filesystem::directory_iterator(baseDirectory)) {

            if (!iter.is_directory()) continue;

            const std::string dirName = StringUtils::toLowerCase(iter.path().filename().string());
            if (dirName == "_male") {
                firstPerson = false;
            }
            else if (dirName == "_1stperson") {
                firstPerson = true;
            }
            else {
                continue;
            }

            std::unordered_set<UInt16> variantIds;
            OverrideFormAnimationForPerspective(firstPerson, form, iter.path(), conditionScript, pollCondition);

        }
    }

    void OverrideFormAnimation(TESForm* form, const std::filesystem::path& path, Script* conditionScript, bool pollCondition) {
        std::filesystem::path newPath = std::filesystem::path(GetFalloutDirectory()) / "Data" / "Meshes" / "AnimGroupOverride" / path;
        if (std::filesystem::is_directory(newPath)) {
            OverrideFormAnimationForDirectory(form, newPath, conditionScript, pollCondition);
        }
        else if (std::filesystem::exists(newPath)) {
            std::unordered_set<UInt16> groupIdFillSet;
            std::string pathStr = newPath.string();
            PluginFunctions::OverrideFormAnimation(form, pathStr.c_str(), false, true, groupIdFillSet, conditionScript, pollCondition);
        }
    }

    void DevkitCompiler::kNVSECompileAnimSet(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string command;

        //commands
        std::string folder = "";
        Script* condition = nullptr;
        bool pollCondition = false;

        //Do first folder:
        if (!getQuotedString(argStream, folder)) {
            PrintKitError("Failed to parse folder.", argStream.str());
        }

        while (it != this->fileManager.currentFile->end()) {

            ++it;
            argStream.str(*it);
            argStream.clear();

            if (!(argStream >> command)) {
                PrintKitError("Error extracting function name.", argStream.str());
                continue;
            }

            char commandType = tolower(command[0]);
            switch (commandType) {
            case 'f':
                if (!folder.empty()) {
                    OverrideFormAnimation(form, folder, condition, pollCondition);
                    folder.clear();
                    condition = nullptr;
                    pollCondition = false;
                }
                if (!getQuotedString(argStream, folder)) {
                    PrintKitError("Failed to parse folder.", argStream.str());
                }
                break;
            case 'c':
                condition = BuildScriptCondition(argStream);
                break;
            case 'p':
                if (!(argStream >> pollCondition)) {
                    PrintKitError("Failed to parse poll condition.", argStream.str());
                }
                break;
            case 't':
            case 'e':
            case '}':
                if (!folder.empty()) {
                    OverrideFormAnimation(form, folder, condition, pollCondition);
                }
                --it;
                return;
            }

        }

    }

    TESForm* DevkitCompiler::lookupGroupEditorID(const std::string& argument) {
        auto formIter = AnimGroupLookup.find(argument);
        if (formIter != AnimGroupLookup.end()) {
            return formIter->second;
        }
        return nullptr;
    }

    void DevkitCompiler::AssignAnim(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            PrintKitError("Missing first argument.", argStream.str());
            return;
        }

        if (!isNested){ //link the set if it exists

            TESForm* set = lookupGroupEditorID(argument); //Lookup set
            if (set) {
                if (fileManager.type == 222) {
                    staticParent->parent = set;
                    AnimGroupsKeep.insert(staticParent);
                }
                else {
                    PluginFunctions::CopyAnimationsToForm(set, form);
                }
            }
            else {
                PrintKitError("animation set does not exist at the time of compilation for object.", argStream.str());
            }

        } else {
            this->fileManager.type = -1; //kNVSE function types, ready to copy into form
        }

    }

    void DevkitCompiler::BuildFormAnimSet(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            PrintKitError("EditorID missing first argument.", argStream.str());
            form = nullptr;
            this->skipForm(it);
            return;
        }

        form = lookupGroupEditorID(argument); //Lookup set
        if (!form) {

            if (templateForm && templateForm->typeID == 32) {
                form = TESForm::CreateNewForm(templateForm, true);
            }
            else {
                form = TESForm::CreateNewForm(32);
            }

            if (!form) {
                PrintKitError("Failed to create a new form from template or by type.", argStream.str());
                form = nullptr;
                this->skipForm(it);
            }

            AnimGroups.insert(form);
            AnimGroupLookup[argument] = form;

        }

    }

}