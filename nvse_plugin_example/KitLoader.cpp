
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

}


