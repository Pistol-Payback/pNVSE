#include "DevkitCompiler.h"

namespace Kit {

    void DevkitCompiler::RegisterArmorTypeFunctions() {
        fileManager.typeFunctions[24]["texture"] = TypeFunction(&DevkitCompiler::SetTextureSetArmor);
    }

    void DevkitCompiler::RegisterWeaponModTypeFunctions() {

        fileManager.typeFunctions[103]["name"] = TypeFunction(&DevkitCompiler::SetName);
        fileManager.typeFunctions[103]["description"] = TypeFunction(&DevkitCompiler::SetDescription);

        fileManager.typeFunctions[103]["weight"] = TypeFunction(&DevkitCompiler::SetWeight);
        fileManager.typeFunctions[103]["value"] = TypeFunction(&DevkitCompiler::SetValue);
        fileManager.typeFunctions[103]["invenotryimage"] = TypeFunction(&DevkitCompiler::SetInvenotryImage);
        fileManager.typeFunctions[103]["messageicon"] = TypeFunction(&DevkitCompiler::SetMessageIcon);

        /*
        fileManager.typeFunctions[103]["Script"] = TypeFunction(&DevkitCompiler::SetScript);
        fileManager.typeFunctions[103]["QuestItem"] = TypeFunction(&DevkitCompiler::SetQuestItem);
        */

        fileManager.typeFunctions[103]["link"] = TypeFunction(&DevkitCompiler::BuildWeaponModLink);
        fileManager.typeFunctions[103]["isbasemod"] = TypeFunction(&DevkitCompiler::SetBaseMod);
        fileManager.typeFunctions[103]["worldmodel"] = TypeFunction(&DevkitCompiler::SetModel);
        fileManager.typeFunctions[103]["attachedmodel"] = TypeFunction(&DevkitCompiler::Set1stPersonAttachmentModel);
        fileManager.typeFunctions[103]["ondetachweaponmod"] = TypeFunction(&DevkitCompiler::SetOnDetachWeaponMod);
        fileManager.typeFunctions[103]["onattachweaponmod"] = TypeFunction(&DevkitCompiler::SetOnAttachWeaponMod);

        //fileManager.typeFunctions[103]["1stpersonmodel"] = TypeFunction(&DevkitCompiler::SetModel);

    }

    void DevkitCompiler::RegisterLists() {

        fileManager.typeFunctions[85]["add"] = TypeFunction(&DevkitCompiler::FormListAdd);
        //fileManager.typeFunctions[85]["remove"] = TypeFunction(&DevkitCompiler::);

        //Leveled Item
        fileManager.typeFunctions[52]["chance"] = TypeFunction(&DevkitCompiler::ChanceOfNone);   //Use global var, or value
        fileManager.typeFunctions[52]["add"] = TypeFunction(&DevkitCompiler::LeveledListAdd);  
        //fileManager.typeFunctions[52]["remove"] = TypeFunction(&DevkitCompiler::);
        //fileManager.typeFunctions[52]["flag"] = TypeFunction(&DevkitCompiler::); //Calculate from all levels <= PCs level
                                                                                        //Calculate for each item in count
        //Leveled Character
        fileManager.typeFunctions[45]["add"] = TypeFunction(&DevkitCompiler::LeveledListAdd);
        //fileManager.typeFunctions[45]["remove"] = TypeFunction(&DevkitCompiler::);
        //fileManager.typeFunctions[45]["flag"] = TypeFunction(&DevkitCompiler::); //Calculate from all levels <= PCs level
                                                                                        //Calculate for each item in count
        //Leveled Creature
        fileManager.typeFunctions[44]["add"] = TypeFunction(&DevkitCompiler::LeveledListAdd);
        //fileManager.typeFunctions[44]["remove"] = TypeFunction(&DevkitCompiler::);
        //fileManager.typeFunctions[44]["flag"] = TypeFunction(&DevkitCompiler::); //Calculate from all levels <= PCs level
                                                                                        //Calculate for each item in count

    }

    void DevkitCompiler::RegisterStatic() {

        fileManager.typeFunctions[32]["worldmodel"] = TypeFunction(&DevkitCompiler::SetModel);
        //fileManager.typeFunctions[32]["sound"] = TypeFunction(&DevkitCompiler::);
        //fileManager.typeFunctions[32]["soundPassThrough"] = TypeFunction(&DevkitCompiler::);
        //fileManager.typeFunctions[32]["flags"] = TypeFunction(&DevkitCompiler::);

        fileManager.typeFunctions[33]["worldmodel"] = TypeFunction(&DevkitCompiler::SetModel);
        //fileManager.typeFunctions[33]["flags"] = TypeFunction(&DevkitCompiler::); //Collection

        fileManager.typeFunctions[34]["name"] = TypeFunction(&DevkitCompiler::SetName);
        fileManager.typeFunctions[34]["worldmodel"] = TypeFunction(&DevkitCompiler::SetModel); //moveable
        //fileManager.typeFunctions[34]["sound"] = TypeFunction(&DevkitCompiler::);
        //fileManager.typeFunctions[34]["destruction Data"] = TypeFunction(&DevkitCompiler::);
        //fileManager.typeFunctions[34]["flags"] = TypeFunction(&DevkitCompiler::);

    }

    void DevkitCompiler::RegisterWeaponTypeFunctions() {

        fileManager.typeFunctions[40]["dmg"] = TypeFunction(&DevkitCompiler::setDamage);
        fileManager.typeFunctions[40]["strreq"] = TypeFunction(&DevkitCompiler::setStrReq);
        fileManager.typeFunctions[40]["skillreq"] = TypeFunction(&DevkitCompiler::setSkillReq);
        fileManager.typeFunctions[40]["weaponskill"] = TypeFunction(&DevkitCompiler::setSkill);
        fileManager.typeFunctions[40]["firerate"] = TypeFunction(&DevkitCompiler::setFireRate);
        fileManager.typeFunctions[40]["spread"] = TypeFunction(&DevkitCompiler::setSpread);
        fileManager.typeFunctions[40]["minspread"] = TypeFunction(&DevkitCompiler::setMinSpread);
        fileManager.typeFunctions[40]["critdmg"] = TypeFunction(&DevkitCompiler::setCritDmg);
        fileManager.typeFunctions[40]["critchance"] = TypeFunction(&DevkitCompiler::setCritChance);
        fileManager.typeFunctions[40]["reach"] = TypeFunction(&DevkitCompiler::setReach);
        fileManager.typeFunctions[40]["projectilesfired"] = TypeFunction(&DevkitCompiler::setNumProjectiles);
        fileManager.typeFunctions[40]["ammoused"] = TypeFunction(&DevkitCompiler::setAmmoUse);
        fileManager.typeFunctions[40]["limbdmgmult"] = TypeFunction(&DevkitCompiler::setLimbDmgMult);
        fileManager.typeFunctions[40]["vatshitchance"] = TypeFunction(&DevkitCompiler::setVatsHitChance);
        fileManager.typeFunctions[40]["clipsize"] = TypeFunction(&DevkitCompiler::setClipSize);
        fileManager.typeFunctions[40]["degmult"] = TypeFunction(&DevkitCompiler::setDegradationMult);

        /*
            fileManager.typeFunctions[40]["objecteffect"] = TypeFunction(&DevkitCompiler::SetScript);
            fileManager.typeFunctions[40]["ammunition"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["cliprounds"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["skill"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["regenrate"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["damage"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["firerate"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["attackmult"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["repairitemlist"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["equiptype"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["spread"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["minspread"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["sightfov"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["sightusage"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["critdamage"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["ammouse"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["projectiles"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["limbdmgmult"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["critmult"] = TypeFunction(&DevkitCompiler::SetQuestItem);

            fileManager.typeFunctions[40]["burstshot"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["longbursts"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["aimarc"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["killimpulse"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["impulsedist"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["firedelaymin"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["firedelaymax"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["criteffect"] = TypeFunction(&DevkitCompiler::SetQuestItem);
            fileManager.typeFunctions[40]["resist"] = TypeFunction(&DevkitCompiler::SetQuestItem);

            fileManager.typeFunctions[40]["flags"] = TypeFunction(&DevkitCompiler::SetQuestItem);
        */
        /*
        fileManager.typeFunctions[40]["Script"] = TypeFunction(&DevkitCompiler::SetScript);
        fileManager.typeFunctions[40]["QuestItem"] = TypeFunction(&DevkitCompiler::SetQuestItem);
        */

        //akimbos and weapons
        std::vector<std::pair<std::string, TypeFunction<DevkitCompiler>>> commonFunctions = {
            {"name", TypeFunction(&DevkitCompiler::SetName)},
            {"description", TypeFunction(&DevkitCompiler::SetDescription)},
            {"invenotryimage", TypeFunction(&DevkitCompiler::SetInvenotryImage)},
            {"messageicon", TypeFunction(&DevkitCompiler::SetMessageIcon)},
        };

        for (const auto& func : commonFunctions) {
            fileManager.typeFunctions[40][func.first] = func.second;
            fileManager.typeFunctions[222][func.first] = func.second;
        }

        //Only on weapons
        fileManager.typeFunctions[40]["slot"] = TypeFunction(&DevkitCompiler::BuildSlot);
        fileManager.typeFunctions[40]["1stpersonmodel"] = TypeFunction(&DevkitCompiler::Set1stPersonWeaponModel);
        fileManager.typeFunctions[40]["worldmodel"] = TypeFunction(&DevkitCompiler::SetWeaponWorldModel);
        fileManager.typeFunctions[40]["value"] = TypeFunction(&DevkitCompiler::SetValue);
        fileManager.typeFunctions[40]["weight"] = TypeFunction(&DevkitCompiler::SetWeight);

    }

    void DevkitCompiler::compileTraitFiles() {

        std::vector<std::string> traitFiles;
        UInt32 type = 0;

        for (const auto& entry : std::filesystem::directory_iterator(fileManager.sRoot)) {
            if (entry.path().extension() == ".trait") {

                std::ifstream file(entry.path());
                if (!file.is_open()) {
                    return;
                }

                std::string line;
                while (std::getline(file, line)) {

                    std::istringstream iss(line);
                    std::string functionName;
                    if (!(iss >> functionName)) {
                        //Console_Print( "Trait Error: Unable to extract function name from line: ", line);
                        continue;
                    }

                    if (functionName == "Type:") {
                        iss >> type;
                    }
                    else {

                        if (!functionName.empty() && functionName.back() == ':') {
                            functionName.pop_back();
                        }

                        fileManager.typeFunctions[type][StringUtils::toLowerCase(functionName)] = TypeFunction{ functionName, iss , &DevkitCompiler::SetTrait };

                    }

                }

            }

        }

    }

    void DevkitCompiler::compile() {

        std::partition(toCompile.begin(), toCompile.end(), [](const scriptToCompile* script) {
            return script->script->info.type == Script::eType_Quest;
            });

        for (scriptToCompile* iter : this->toCompile) {
            if (!iter->isPartial && !CompileScriptAlt(iter->script, iter->text)) {
                Console_Print("failed to compile script: %s", iter->script->GetEditorID());
            }
            else if (!CompilePartial(iter->script, iter->text, iter->arguments)) {
                Console_Print("failed to compile script: %s", iter->text.c_str());
            }
        }

    }

    void DevkitCompiler::clearAnimGroups() {

        for (StaticInstance* staticInst : AnimGroupsKeep) {
            AnimGroups.erase(staticInst->parent);
            staticInst->setParent(staticInst->parent);
        }

        for (TESForm* set : AnimGroups) {
            PluginFunctions::RemoveFormAnimations(set);
            set->Destroy(true);
        }
        AnimGroups.clear();
        AnimGroupsKeep.clear();
        AnimGroupLookup.clear();
    }

}
