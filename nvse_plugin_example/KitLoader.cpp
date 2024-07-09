
#include "KitLoader.h"
#include "DevkitCompiler.h"

namespace Kit {

    std::unordered_map<UInt32, KitData> loadedKitFiles;

    void KitFileManager::compileLoadOrderFromAll() {

        std::filesystem::path dirPath(sRoot);

        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            std::cerr << "Provided path is not a valid directory: " << sRoot << std::endl;
            return;
        }

        // Iterate through each entry in the directory
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {

            if (entry.is_directory() && entry.path().filename().string() != "Overwrites") {

                loadOrder.push_back(KitInfo{ entry.path().filename().string(), entry.path().string() });

            }

        }

        if (!validateAndReorderLoadOrder()) {
            std::cerr << "Load order corrected and updated." << std::endl;
        }

        std::string loadOrderFile = sRoot + "\\LoadOrder.ini";
        writeLoadOrderToFile(loadOrderFile);

        // Append a special "Overwrites" entry to handle any override configurations
        std::string path = sRoot + "\\Overwrites";
        if (std::filesystem::is_directory(path)) {
            loadOrder.emplace_back("Overwrites", path);
        }


    }

    void KitFileManager::writeLoadOrderToFile(const std::string& filePath) {
        std::ofstream outFile(filePath);
        if (!outFile) {
            std::cerr << "Error: Unable to open " << filePath << " for writing." << std::endl;
            return;
        }

        for (const auto& kit : loadOrder) {
            outFile << kit.name << '\n';
        }

        outFile.close();
    }

    void KitFileManager::compileLoadOrder() {

        std::string loadOrderFile = sRoot + "\\LoadOrder.ini";
        std::ifstream file(loadOrderFile);
        std::string path;

        if (!file) {
            std::cerr << "Error: Load order file not found at " << loadOrderFile << std::endl;
            return;
        }

        std::string line;
        std::vector<std::string> kitNames;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                kitNames.push_back(line);
            }
        }
        file.close();

        loadOrder.reserve(kitNames.size());

        for (const auto& kitName : kitNames) {
            path = sRoot + "\\" + kitName;
            if (std::filesystem::is_directory(path)) {
                loadOrder.emplace_back(kitName, path);
            }

        }

        if (!validateAndReorderLoadOrder()) {
            std::cerr << "Load order corrected and updated." << std::endl;
            writeLoadOrderToFile(loadOrderFile);
        }

        // Append a special "Overwrites" entry to handle any override configurations
        path = sRoot + "\\Overwrites";
        if (std::filesystem::is_directory(path)) {
            loadOrder.emplace_back("Overwrites", path);
        }
    }

    bool KitFileManager::validateAndReorderLoadOrder() {

        std::unordered_map<std::string, size_t> masterPositions;
        for (size_t i = 0; i < loadOrder.size(); ++i) {
            masterPositions[loadOrder[i].name] = i;
        }

        bool isOrderValid = true;
        for (size_t i = 0; i < loadOrder.size();) {
            auto& kit = loadOrder[i];
            bool allMastersFound = true;
            for (const auto& master : kit.masters) {

                if (masterPositions.find(master) == masterPositions.end()) {
                    Console_Print("Master kit not found: %s for child %s", master.c_str(), kit.name.c_str());
                    loadOrder.erase(loadOrder.begin() + i);  // Remove the child as its master is not found
                    isOrderValid = false;
                    continue;
                }
                size_t masterPos = masterPositions[master];
                if (masterPos > i) {
                    std::swap(loadOrder[i], loadOrder[masterPos]);
                    std::swap(masterPositions[loadOrder[i].name], masterPositions[loadOrder[masterPos].name]);
                    i = min(i, masterPos) - 1;  // Revalidate from the earliest affected index
                    isOrderValid = false;
                    break;
                }

            }
            if (allMastersFound) {
                ++i;
            }
        }
        return isOrderValid;

    }

    void KitFileManager::loadKitFolders() {

        std::string path;

        for (const auto& entry : loadOrder) {

            path = sRoot + "\\" + entry.name;
            if (std::filesystem::is_directory(path)) {

                kitFolders.emplace_back(KitFolder{ path });

            }

        }

    }

    void KitFileManager::compressKitFolders() {    //Compresses every kit folder

        UInt32 count = DataHandler::Get()->GetActiveModCount();

        for (size_t i = 0; i < kitFolders.size(); ++i) {
            kitFiles.emplace_back(KitCompressedFile{kitFolders[i], *this, loadOrder[i], count});
            ++count;
        }

    }

    void KitFileManager::CallTypeFunction(DevkitCompiler& functionsList, std::vector<std::string>::const_iterator& it, UInt8 functionType) {

        std::istringstream iss(*it);
        if (!(iss >> currentFunction)) {
            functionsList.PrintKitError("Unable to extract function name " + currentFunction, iss.str());
            return;
        }

        if (!currentFunction.empty() && currentFunction.back() == ':') {
            currentFunction.pop_back();
        }

        currentFunction = StringUtils::toLowerCase(currentFunction);

        auto itFunct = typeFunctions[functionType].find(currentFunction);
        if (itFunct == typeFunctions[functionType].end()) {
            if (type != 0) {
                itFunct = typeFunctions[0].find(currentFunction);
                if (itFunct == typeFunctions[0].end()) {
                    functionsList.PrintKitError("Function not found " + currentFunction, iss.str());
                    return;
                }
            }
            else {
                functionsList.PrintKitError("Function not found " + currentFunction, iss.str());
                return;
            }
        }

       // if (iss.peek() != EOF) {
            TypeFunction<DevkitCompiler>& funct = itFunct->second;
            if (funct.type == 1) {
                (functionsList.*(funct.functIter))(it, iss);
            }
            else {
                (functionsList.*(funct.functSelf))(funct, iss);
            }
       // }
       // else {
            //Console_Print("Error: No argument found for function:  %s", functionName.c_str());
      //  }

    }

    void KitFileManager::BuildForms(DevkitCompiler& functionsList) {

        for (const auto& kitFile : kitFiles) {

            currentKitFile = &kitFile;

            for (auto it = kitFile.formBuilder.begin(); it != kitFile.formBuilder.end(); ++it) {
                CallTypeFunction(functionsList, it, type);
            }

        }

    };

    void KitFileManager::ReadKitFiles(DevkitCompiler& functionsList) {

        for (const auto& kitFile : kitFiles) {

            currentKitFile = &kitFile;

            for (auto it = kitFile.file.begin(); it != kitFile.file.end(); ++it) {
                CallTypeFunction(functionsList, it, type);
            }

        }

    };

    const std::unordered_map<std::string, int> KitFolder::extensionToType = {
        {"fileheader", 1},
        {"group", 2},
        {"gamesetting", 3},
        {"textureset", 4},
        {"menuicon", 5},
        {"global", 6},
        {"class", 7},
        {"faction", 8},
        {"headpart", 9},
        {"hair", 10},
        {"eyes", 11},
        {"race", 12},
        {"sound", 13},
        {"acousticspace", 14},
        {"skill", 15},
        {"baseeffect", 16},
        {"script", 17},
        {"landtexture", 18},
        {"objecteffect", 19},
        {"actoreffect", 20},
        {"activator", 21},
        {"talkingactivator", 22},
        {"terminal", 23},
        {"armor", 24},
        {"book", 25},
        {"clothing", 26},
        {"container", 27},
        {"door", 28},
        {"ingredient", 29},
        {"light", 30},
        {"miscellaneous", 31},
        {"static", 32},
        {"staticcollection", 33},
        {"moveablestatic", 34},
        {"placeablewater", 35},
        {"grass", 36},
        {"tree", 37},
        {"flora", 38},
        {"furniture", 39},
        {"weapon", 40},
        {"ammo", 41},
        {"npc", 42},
        {"creature", 43},
        {"leveledcreature", 44},
        {"leveledcharacter", 45},
        {"key", 46},
        {"ingestible", 47},
        {"idlemarker", 48},
        {"note", 49},
        {"constructibleobject", 50},
        {"projectile", 51},
        {"leveleditem", 52},
        {"weather", 53},
        {"climate", 54},
        {"region", 55},
        {"navmeshinfomap", 56},
        {"cell", 57},
        {"reference", 58},
        {"characterreference", 59},
        {"creaturereference", 60},
        {"missileprojectile", 61},
        {"grenadeprojectile", 62},
        {"beamprojectile", 63},
        {"flameprojectile", 64},
        {"worldspace", 65},
        {"land", 66},
        {"navmesh", 67},
        {"unknown", 68},
        {"dialogtopic", 69},
        {"dialogtopicinfo", 70},
        {"quest", 71},
        {"idle", 72},
        {"package", 73},
        {"combatstyle", 74},
        {"loadscreen", 75},
        {"leveledspell", 76},
        {"animatedobject", 77},
        {"water", 78},
        {"effectshader", 79},
        {"offsettable", 80},
        {"explosion", 81},
        {"debris", 82},
        {"imagespace", 83},
        {"imagespaceadapter", 84},
        {"formlist", 85},
        {"perk", 86},
        {"bodypartdata", 87},
        {"addonnode", 88},
        {"actorvalueinfo", 89},
        {"radiationstage", 90},
        {"camerashot", 91},
        {"camerapath", 92},
        {"voicetype", 93},
        {"impactdata", 94},
        {"impactdataset", 95},
        {"armoraddon", 96},
        {"encounterzone", 97},
        {"message", 98},
        {"ragdoll", 99},
        {"defaultobjectmanager", 100},
        {"lightingtemplate", 101},
        {"music", 102},
        {"itemmod", 103},
        {"reputation", 104},
        {"continuousbeamprojectile", 105},
        {"recipe", 106},
        {"recipecategory", 107},
        {"casinochip", 108},
        {"casino", 109},
        {"loadscreen", 110},
        {"mediaset", 111},
        {"medialocationcontroller", 112},
        {"challenge", 113},
        {"ammoeffect", 114},
        {"caravancard", 115},
        {"caravanmoney", 116},
        {"caravandeck", 117},
        {"dehydrationstage", 118},
        {"hungerstage", 119},
        {"sleepdeprivationstage", 120},
        {"akimbo", 222},
        {"kitInfo", 999}
    };

    KitFolder::KitFolder(const std::filesystem::path& path) : filePath(path) {
        CompileTypes();
    }

    void KitFolder::CompileTypes() {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(filePath)) {
            if (entry.is_regular_file()) {
                int type = EvaluateType(entry.path());
                if (type != -1) {
                    filesByType[type].push_back(entry.path());
                }
            }
        }
    }

    int KitFolder::EvaluateType(const std::filesystem::path& filePath) {
        std::string filename = filePath.filename().string();
        size_t startBracket = filename.find('[');
        size_t endBracket = filename.find(']');

        if (startBracket != std::string::npos && endBracket != std::string::npos && startBracket < endBracket) {
            std::string typeTag = filename.substr(startBracket + 1, endBracket - startBracket - 1);
            return ConvertExtensionToType(typeTag);
        }
        return -1;
    }

    int KitFolder::ConvertExtensionToType(const std::string& extension) {
        std::string lowerExtension = StringUtils::toLowerCase(extension);
        auto it = extensionToType.find(lowerExtension);
        if (it != extensionToType.end()) {
            return it->second;
        }
        return -1;
    }

}
