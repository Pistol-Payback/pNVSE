#pragma once
#include "KitLoader.h"

namespace Kit {

    class DevkitCompiler {

        void skipDocument(std::vector<std::string>::const_iterator& it);
        void GatherLeveledData(std::vector<std::string>::const_iterator& it, TESLeveledList::BaseData* newData);
        void skipForm(std::vector<std::string>::const_iterator& it);
        void skipLink(std::vector<std::string>::const_iterator& it);

        void SetTrait(TypeFunction<DevkitCompiler>& function, std::istringstream& argStream);

        void SetType(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void SetTemplate(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void BreakFromTemplate(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void QuestDelay(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void QuestFlags(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void QuestScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void BuildScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void RegFloatVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void RegIntVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void RegRefVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void RegStringVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void RegArrayVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetScriptType(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void CompileScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void BuildSlot(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void BuildWeaponModLink(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetWeaponAnimation(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void Set1stPersonWeaponModel(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void SetBaseMod(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void Set1stPersonAttachmentModel(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void SetDescription(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetName(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetWeight(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetValue(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void SetInvenotryImage(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetMessageIcon(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetModel(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetQuestItem(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void BuildForm(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void BuildAkimboForm(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        //FormsList
        void FormListAdd(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void LeveledListAdd(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void ChanceOfNone(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        //KitInfo...............................................................................................................

        void KitConflicts(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void KitVersion(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void KitUpdateWearning(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void KitIsSafeToRemove(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void KitUpdater(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void KitUninstaller(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        //BuilderKit...................................................................................................................

        void RegisterWeaponTypeFunctions();
        void RegisterWeaponModTypeFunctions();
        void RegisterLists();
        void RegisterStatic();
        void compileTraitFiles();

        //.........................................................................................................................

    public:

        TESForm* form = nullptr;

        TESForm* templateForm = nullptr;
        std::string slot = "null";
        ExtendedBaseType* staticParent = nullptr;

        KitFileManager fileManager;

        DevkitCompiler() : fileManager(GetFalloutDirectory() + "Data\\Devkit") {

            //fileManager.compileLoadOrder();
            fileManager.compileLoadOrderFromAll();
            fileManager.loadKitFolders();

            fileManager.typeFunctions[0]["type"] = TypeFunction{ &DevkitCompiler::SetType };
            fileManager.typeFunctions[0]["template"] = TypeFunction{ &DevkitCompiler::SetTemplate };
            fileManager.typeFunctions[0]["}clear"] = TypeFunction{ &DevkitCompiler::BreakFromTemplate };

            for (int i = 1; i <= 120; ++i) {

                switch (i) {
                case 71:
                    fileManager.typeFunctions[i]["editorid"] = TypeFunction(&DevkitCompiler::BuildForm);
                    fileManager.typeFunctions[i]["script"] = TypeFunction(&DevkitCompiler::QuestScript);
                    break;
                case 17:
                    fileManager.typeFunctions[i]["scn"] = TypeFunction(&DevkitCompiler::BuildScript);
                    fileManager.typeFunctions[i]["name"] = TypeFunction(&DevkitCompiler::BuildScript);
                    fileManager.typeFunctions[i]["begin"] = TypeFunction(&DevkitCompiler::SetScriptType);
                    fileManager.typeFunctions[i]["float"] = TypeFunction(&DevkitCompiler::RegFloatVar);
                    fileManager.typeFunctions[i]["int"] = TypeFunction(&DevkitCompiler::RegIntVar);
                    fileManager.typeFunctions[i]["ref"] = TypeFunction(&DevkitCompiler::RegRefVar);
                    fileManager.typeFunctions[i]["array_var"] = TypeFunction(&DevkitCompiler::RegArrayVar);
                    fileManager.typeFunctions[i]["string_var"] = TypeFunction(&DevkitCompiler::RegStringVar);
                    break;
                default:
                    fileManager.typeFunctions[i]["editorid"] = TypeFunction(&DevkitCompiler::BuildForm);
                    break;
                }
            }

            fileManager.compressKitFolders();
            fileManager.BuildForms(*this);

            fileManager.typeFunctions[999]["conflicts"] = TypeFunction{ &DevkitCompiler::KitConflicts };
            fileManager.typeFunctions[999]["version"] = TypeFunction{ &DevkitCompiler::KitVersion };
            fileManager.typeFunctions[999]["updatewarning"] = TypeFunction{ &DevkitCompiler::KitUpdateWearning };
            fileManager.typeFunctions[999]["safetoremove"] = TypeFunction{ &DevkitCompiler::KitIsSafeToRemove };
            fileManager.typeFunctions[999]["updater"] = TypeFunction{ &DevkitCompiler::KitUpdater };
            fileManager.typeFunctions[999]["uninstaller"] = TypeFunction{ &DevkitCompiler::KitUninstaller };

            fileManager.typeFunctions[17]["scn"] = TypeFunction(&DevkitCompiler::CompileScript);
            fileManager.typeFunctions[17]["name"] = TypeFunction(&DevkitCompiler::CompileScript);

            fileManager.typeFunctions[71]["name"] = TypeFunction(&DevkitCompiler::SetName);
            fileManager.typeFunctions[71]["questdelay"] = TypeFunction(&DevkitCompiler::QuestDelay);
            fileManager.typeFunctions[71]["flags"] = TypeFunction(&DevkitCompiler::QuestFlags);

            RegisterWeaponTypeFunctions();
            RegisterWeaponModTypeFunctions();
            RegisterLists();
            RegisterStatic();
            compileTraitFiles();

            fileManager.ReadKitFiles(*this);

        }

        void PrintKitError(const std::string& message, const std::string& lineDump) {

            std::ostringstream errorStream;
            std::string kitFileName = fileManager.currentKitFile ? fileManager.currentKitFile->data->name : "Undefined Kit";

            int borderLength = 70;
            std::string baseBorder(borderLength, '*');

            // Calculate the position to insert the kit file name
            std::string topBorder = baseBorder;
            std::string kitNameFormatted = '[' + kitFileName + ']';
            int startInsert = (borderLength - kitNameFormatted.length()) / 2; // Center the kit file name

            // Insert the kit file name into the top border
            topBorder.replace(startInsert, kitNameFormatted.length(), kitNameFormatted);

            errorStream << "\n" << topBorder << "\n";
            errorStream << message << '\n';

            if (form) {
                errorStream << "Form: " << form->GetEditorID() << '\n';
            }

            if (!slot.empty() && slot != "null") {
                errorStream << "Slot: " << slot << '\n';
            }

            if (fileManager.currentKitFile) {
                errorStream << "FolderType: " << fileManager.type << '\n';
            }
            else {
                errorStream << "FolderType: Undefined " << '\n';
            }

            errorStream << "Line: " << lineDump << '\n';
            errorStream << baseBorder << "\n"; // Bottom border is just stars

            Console_Print("%s", errorStream.str().c_str());
        }

    };

}