#include "DevkitCompiler.h"

namespace Kit {

    void DevkitCompiler::KitConflicts(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;

        while (getQuotedString(argStream, argument)) {

            auto dataIter = this->fileManager.reverseNameLookup.find(argument);
            if (dataIter == this->fileManager.reverseNameLookup.end()) {
                continue;
            }

            KitData* conflict = dataIter->second;
            KitData* data = this->fileManager.currentKitFile->data;
            UInt32 conflictIdentifier = data->index + conflict->index;
            this->fileManager.conflictList[conflictIdentifier] = data->name + " conflicts with " + argument;

        }

    }

    void DevkitCompiler::KitVersion(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        UInt32 argument;
        if (!(argStream >> argument)) {
            PrintKitError("no a valid version number", argStream.str());
            return;
        }
        this->fileManager.currentKitFile->data->version = argument;
    }

    void DevkitCompiler::KitUpdateWearning(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(getQuotedString(argStream, argument))) {
            return;
        }

    }

    void DevkitCompiler::KitIsSafeToRemove(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        bool argument;
        if (!(argStream >> argument)) {
            PrintKitError("missing first argument", argStream.str());
            return;
        }
        this->fileManager.currentKitFile->data->safeUninstall = argument;

    }

    void DevkitCompiler::KitUpdater(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

        Script* script = LookupEditorID<Script*>(argument.c_str());

        if (!script) {
            PrintKitError("invalid script for updater", argStream.str());
            return;
        }

        UInt32 toVersion;
        if (!(argStream >> toVersion)) {
            this->fileManager.currentKitFile->data->updater[this->fileManager.currentKitFile->data->version] = script;
            return;
        }

        this->fileManager.currentKitFile->data->updater[toVersion] = script;

    }

    void DevkitCompiler::KitUninstaller(std::vector<std::string>::const_iterator& it, std::istringstream& argStream) {

        std::string argument;
        if (!(argStream >> argument)) {
            return;
        }

    }

}