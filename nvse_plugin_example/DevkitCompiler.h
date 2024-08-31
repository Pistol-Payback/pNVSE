#pragma once
#include "KitLoader.h"

namespace Kit {

    class DevkitCompiler {

        void skipDocument(std::vector<std::string>::const_iterator& it);
        void GatherLeveledData(std::vector<std::string>::const_iterator& it, TESLeveledList::BaseData* newData);

        void skipForm(std::vector<std::string>::const_iterator& it);
        void skipLink(std::vector<std::string>::const_iterator& it);

        void skipNestedFunction(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void skipFunction(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void skipNested(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void SetTrait(TypeFunction<DevkitCompiler>& function, std::istringstream& argStream);

        void SetType(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void SetTemplate(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetAkimboTemplate(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
 
        void EndOfDocument(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void QuestDelay(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void QuestFlags(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void QuestScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        //void CompileScriptLink(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void BuildScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        Script* BuildScriptCondition(std::istringstream& iss, const std::string& arguments = "");
        void RegFloatVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void RegIntVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void RegRefVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void RegStringVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void RegArrayVar(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetScriptType(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        //void CompileScript(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void BuildSlot(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void BuildWeaponModLink(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetWeaponAnimation(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void Set1stPersonWeaponModel(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetWeaponWorldModel(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        void SetOnAttachWeaponMod(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetOnDetachWeaponMod(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
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
        void SetTextureSet(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetTextureSet(std::vector<std::string>::const_iterator& it, std::istringstream& argStream, TESModelTextureSwap* modelSwap);
        void SetTextureSetArmor(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void SetQuestItem(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        //Akimbo
        void BuildForm(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void BuildAkimboForm(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        //kNVSE
            void AssignAnim(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
            //-1
            void kNVSECompileAnimSet(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
            void BuildFormAnimSet(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
            TESForm* lookupGroupEditorID(const std::string& argument);

        //FormsList
        void FormListAdd(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void LeveledListAdd(bool isNested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void ChanceOfNone(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        //KitInfo...............................................................................................................

        void KitConflicts(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void KitVersion(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void KitUpdateWearning(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void KitIsSafeToRemove(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void KitUpdater(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void KitUninstaller(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        //BuilderKit...................................................................................................................

        void RegisterArmorTypeFunctions();
        void RegisterWeaponTypeFunctions();
        void RegisterWeaponModTypeFunctions();
        void RegisterLists();
        void RegisterStatic();
        void compileTraitFiles();
        void compile();
        void clearAnimGroups();

        //Weapons....................................................................................................................

        void setStrReq(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setSkillReq(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setSkill(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setFireRate(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setSpread(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setMinSpread(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setCritDmg(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setCritChance(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setReach(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setProjectile(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setNumProjectiles(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setAmmoUse(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setLimbDmgMult(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setDamage(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setActionPointsUsed(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setVatsHitChance(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setCritEffect(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setClipSize(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
        void setDegradationMult(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        //.........................................................................................................................

    public:

        TESForm* form = nullptr;

        TESForm* templateForm = nullptr;
        std::string slot = "null";
        StaticInstance* staticParent = nullptr;
        StaticInstance* templateEXB = nullptr;

        KitFileManager fileManager;

        struct NestedState {

            NestedState(TESForm* form, TESForm* templateForm, std::string slot, 
                StaticInstance* staticParent, StaticInstance* templateEXB, SInt32 type) :
                form(form), templateForm(templateForm), slot(slot), 
                staticParent(staticParent), templateEXB(templateEXB), type(type) {}

            SInt32 type;
            TESForm* form;
            TESForm* templateForm;
            std::string slot;
            StaticInstance* staticParent;
            StaticInstance* templateEXB;

        };

        std::vector<NestedState*> nested;
        void EnterNestedState();
        void ExitNestedState();
        void ClearNestedState();
        void RunOperatorOverloads(const char* m_operator, std::vector<std::string>::const_iterator& it, std::istringstream& argStream);

        struct scriptToCompile {

            scriptToCompile(Script* scriptPtr, const std::string& scriptText, bool isPartial, const std::string& arguments = "")
                : script(scriptPtr), text(scriptText), isPartial(isPartial), arguments(arguments) {
            }

            Script* script;
            std::string text;
            std::string arguments;
            bool isPartial;
        };

        std::vector<scriptToCompile*> toCompile;
        std::unordered_map<Script* , scriptToCompile*> toCompileLookup;

        std::unordered_set<TESForm*> AnimGroups; //For AnimGroups -1. All these get destoryed at the end.
        std::unordered_map<std::string, TESForm*> AnimGroupLookup;

        //Used for akimbos that need to keep their anim groups as parents
        std::unordered_set<StaticInstance*> AnimGroupsKeep;

        DevkitCompiler() : fileManager(GetFalloutDirectory() + "Data\\Devkit") {

        //Build faze

            bool staticLoadOrder = false;
            if (staticLoadOrder) {
                fileManager.compileLoadOrder();
            }
            else {
                fileManager.compileLoadOrderFromAll();
            }

            fileManager.loadKitFolders();

            fileManager.typeFunctions[0]["type"] = TypeFunction{ &DevkitCompiler::SetType };
            fileManager.typeFunctions[0]["template"] = TypeFunction{ &DevkitCompiler::SetTemplate };
            fileManager.typeFunctions[0]["}clear"] = TypeFunction{ &DevkitCompiler::EndOfDocument };

            if (PluginFunctions::kNVSE) { //kNVSE

              //AnimSets
              fileManager.typeFunctions[-1]["editorid"] = TypeFunction(&DevkitCompiler::BuildFormAnimSet);
              fileManager.typeFunctions[-1]["folder"] = TypeFunction(&DevkitCompiler::kNVSECompileAnimSet);

              //akimbos
              fileManager.typeFunctions[222]["template"] = TypeFunction{ &DevkitCompiler::SetAkimboTemplate };
              fileManager.typeFunctions[222]["editorid"] = TypeFunction(&DevkitCompiler::BuildAkimboForm);
              
            }
            else { //Skips

                //akimbos
                fileManager.typeFunctions[222]["template"] = TypeFunction{ &DevkitCompiler::skipNestedFunction };
                fileManager.typeFunctions[222]["editorid"] = TypeFunction(&DevkitCompiler::skipNestedFunction);

            }

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

            fileManager.loadESPToKitData();
            fileManager.compressKitFolders();
            fileManager.BuildForms(*this);

         //Linker faze below:

            if (PluginFunctions::kNVSE) { //kNVSE
                fileManager.typeFunctions[0]["animset"] = TypeFunction{ &DevkitCompiler::AssignAnim };
            }
            else { //Skips
                fileManager.typeFunctions[0]["animset"] = TypeFunction{ &DevkitCompiler::skipNestedFunction };
            }
            fileManager.typeFunctions[0]["texture"] = TypeFunction{ &DevkitCompiler::SetTextureSet };
            
            //fileManager.typeFunctions[17]["scn"] = TypeFunction(&DevkitCompiler::CompileScript);
            //fileManager.typeFunctions[17]["name"] = TypeFunction(&DevkitCompiler::CompileScript);

            fileManager.typeFunctions[71]["name"] = TypeFunction(&DevkitCompiler::SetName);
            fileManager.typeFunctions[71]["questdelay"] = TypeFunction(&DevkitCompiler::QuestDelay);
            fileManager.typeFunctions[71]["flags"] = TypeFunction(&DevkitCompiler::QuestFlags);
            //fileManager.typeFunctions[71]["script"] = TypeFunction(&DevkitCompiler::CompileScriptLink);

            //Kit info Functions
            fileManager.typeFunctions[999]["conflicts"] = TypeFunction{ &DevkitCompiler::KitConflicts };
            fileManager.typeFunctions[999]["version"] = TypeFunction{ &DevkitCompiler::KitVersion };
            fileManager.typeFunctions[999]["updatewarning"] = TypeFunction{ &DevkitCompiler::KitUpdateWearning };
            fileManager.typeFunctions[999]["safetoremove"] = TypeFunction{ &DevkitCompiler::KitIsSafeToRemove };
            fileManager.typeFunctions[999]["updater"] = TypeFunction{ &DevkitCompiler::KitUpdater };
            fileManager.typeFunctions[999]["uninstaller"] = TypeFunction{ &DevkitCompiler::KitUninstaller };

            RegisterArmorTypeFunctions();
            RegisterWeaponTypeFunctions();
            RegisterWeaponModTypeFunctions();
            RegisterLists();
            RegisterStatic();
            compileTraitFiles();

            fileManager.ReadKitFiles(*this);

            //Compile partials after core scripts have been compiled
            compile();
            clearAnimGroups();

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
                errorStream << "FolderType: " << ConvertTypeToExtension(fileManager.type) << '\n';
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