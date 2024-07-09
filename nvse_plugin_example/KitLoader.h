#pragma once
#include <array>
#include "ppNVSE.h"
#include "WeaponSmith.h"
#include <fstream>
#include <filesystem>

class ScriptConverter;

template<typename IterConverter>
struct TypeFunction {

    static const UInt8 allowDuplicate = 0b10000000;

    UInt8 type = 0;
    std::string name = "null";
    std::array<UInt8, 8> argumentTypes = std::array<UInt8, 8>{};

    using FunctionScriptConverter = std::string(IterConverter::*)(std::string operation, std::string segment);
    using FunctionPtrIter = void (IterConverter::*)(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
    using FunctionPtrSelf = void(IterConverter::*)(TypeFunction& function, std::istringstream& argStream);

    FunctionScriptConverter functIterConvert = nullptr;
    FunctionPtrIter functIter = nullptr;
    FunctionPtrSelf functSelf = nullptr;

    TypeFunction() : type(0) {}
    TypeFunction(FunctionPtrIter funcPtr) :functIter(funcPtr) {
        this->type = 1;
    }
    TypeFunction(FunctionScriptConverter funcPtr) :functIterConvert(funcPtr) {
        this->type = 1;
    }

    TypeFunction(std::string functionName, std::istringstream& arguments, FunctionPtrSelf funcPtr) :functSelf(funcPtr) {

        std::array<UInt8, 8> argumentTypes{};
        std::string argType;
        int index = 0;

        while (arguments >> argType && index < argumentTypes.size()) {
            int typeCode = convertArgumentType(argType);
            if (typeCode) {
                argumentTypes[index++] = typeCode;
            }
            else {
                throw std::runtime_error("Invalid argument type '" + argType + "' for function '" + functionName + "'");
                break;
            }
        }

        if (index == argumentTypes.size() && (arguments >> argType)) {
            Console_Print("Error: Too many arguments for function:  %s", functionName.c_str());
        }

        this->name = functionName;
        this->argumentTypes = argumentTypes;

    }

    int convertArgumentType(const std::string& argType) {
        switch (argType[0]) {
        case '*': {
            switch (argType[1]) {
            case 'r': return allowDuplicate | 1;
            case 'i': return allowDuplicate | 2;
            case 'f': return allowDuplicate | 3;
            case 's': return allowDuplicate | 4;
            default: return 0;
            }
        }
        case 'r': return 1;
        case 'i': return 2;
        case 'f': return 3;
        case 's': return 4;
        default: return 0;
        }
    }

};

namespace Kit {

    class DevkitCompiler;
    class KitCompressedFile;

    //Saves with loaded game.
    struct KitData {
        std::string name = "null";
        UInt32 version = 0;
        UInt32 index = 0;
        bool safeUninstall = false;
        std::map<UInt32, Script*> updater;

        KitData() = default;

        explicit KitData(const std::string& kitName) : name(kitName) {}

    };

    extern std::unordered_map<UInt32, KitData> loadedKitFiles;

    class KitFolder {
    public:
        explicit KitFolder(const std::filesystem::path& path);
        const std::filesystem::path filePath;
        std::map<UInt32, std::vector<std::filesystem::path>> filesByType;

    private:
        static const std::unordered_map<std::string, int> extensionToType;
        void CompileTypes();
        int EvaluateType(const std::filesystem::path& filePath);
        int ConvertExtensionToType(const std::string& extension);
    };

    struct KitInfo {

        std::string name;
        std::vector<std::string> masters;
        std::vector<std::string> kitEndData;

        KitInfo(const std::string& kitName, const std::string& kitPath) : name(kitName) {

            std::string kitInfoPath = kitPath + "\\kitInfo.ini";
            std::ifstream file(kitInfoPath);
            if (!file) {
                return;
            }

            std::string line;
            while (getline(file, line)) {

                std::istringstream iss(line);
                std::string segment;

                if (iss >> segment && StringUtils::toLowerCase(segment) == "masters:") {

                    while (getQuotedString(iss, segment)) {
                        masters.push_back(segment);
                    }

                }
                else {
                    kitEndData.push_back(std::move(line));  // Use move to avoid copying string
                }
            }
            file.close();
        }
    };

    class KitFileManager {
    private:

        std::vector<KitInfo> loadOrder;
        std::vector<KitFolder> kitFolders;
        std::vector<KitCompressedFile> kitFiles;

    public:

        KitFileManager(const std::string& root) : sRoot(root) {}

        const KitCompressedFile* currentKitFile;
        std::string currentFunction;
        UInt32 type = 0;
        std::unordered_map<UInt32, std::unordered_map<std::string, TypeFunction<DevkitCompiler>>> typeFunctions;
        std::string sRoot;

        //For conflict detector
        std::unordered_map<std::string, KitData*> reverseNameLookup;
        std::unordered_map<UInt32, std::string> conflictList; //UInt32 is the index identifier. kitIndex + kitIndex = identifier.

        void compileLoadOrder();
        void compileLoadOrderFromAll();   //Use one or the other, but not both.

        bool validateLoadOrder();
        bool validateMaster(UInt32 index, std::string& master);
        bool validateAndReorderLoadOrder();
        void writeLoadOrderToFile(const std::string& filePath);

        void loadKitFolders();
        void compressKitFolders();

        //Builds all forms accross all compressed files
        void BuildForms(DevkitCompiler& functionsList);

        //Processes functions
        void ReadKitFiles(DevkitCompiler& functionsList);
        void CallTypeFunction(DevkitCompiler& functionsList, std::vector<std::string>::const_iterator& it, UInt8 functionType);

    };

    class KitCompressedFile {

    public:

        std::vector<std::string> file;
        std::vector<std::string> formBuilder;
        KitFileManager* fileManager;

        KitData* data;

        KitCompressedFile(const KitFolder& kit, KitFileManager& mainFileManager, KitInfo& info, UInt32 kitIndex) {
            fileManager = &mainFileManager;

            for (const auto& typeEntry : kit.filesByType) {
                for (const auto& filePath : typeEntry.second) { // typeEntry.second is a vector of paths
                    std::string fileStringPath = filePath.string();
                    file.push_back("type: " + std::to_string(typeEntry.first));
                    formBuilder.push_back("type: " + std::to_string(typeEntry.first));
                    fileManager->type = typeEntry.first;

                    ProcessFile(fileStringPath);  // Process each file
                }
            }

            // Create Kit Data:
            auto& insertedData = loadedKitFiles[kitIndex] = KitData{ info.name };
            fileManager->reverseNameLookup[info.name] = &insertedData;
            data = &insertedData;
        }

    private:

        void ProcessFile(const std::string& filePath) {
            std::ifstream inputFile(filePath);
            if (!inputFile.is_open()) {
                throw std::runtime_error("Failed to open file: " + filePath);
            }

            std::string line;
            while (std::getline(inputFile, line)) {
                trim(line);

                size_t commentPos = line.find("//");
                if (commentPos != std::string::npos) {
                    line.erase(commentPos);
                    trim(line);
                }

                if (!line.empty()) {
                    CompressToBuilder(line);
                    file.push_back(line);
                }
            }

            file.push_back("}clear");
            formBuilder.push_back("}clear");

            inputFile.close();
        }

        void CompressToBuilder(std::string& line) {

            std::istringstream iss(line);
            std::string functionName;
            if (!(iss >> functionName)) {
                return;
            }

            if (!functionName.empty() && functionName.back() == ':') {
                functionName.pop_back();
            }

            functionName = StringUtils::toLowerCase(functionName);

            auto itFunct = fileManager->typeFunctions[fileManager->type].find(functionName);
            if (itFunct == fileManager->typeFunctions[fileManager->type].end()) {
                if (fileManager->type != 0) {
                    itFunct = fileManager->typeFunctions[0].find(functionName);
                    if (itFunct == fileManager->typeFunctions[0].end()) {
                        return;
                    }
                }
                else {
                    return;
                }
            }

            formBuilder.push_back(line);

        }

        void trim(std::string& str) {
            str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
                return !std::isspace(ch);
                }));
            str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
                }).base(), str.end());
        }
    };
}