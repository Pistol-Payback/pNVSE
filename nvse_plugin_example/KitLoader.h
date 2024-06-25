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

        const std::filesystem::path filePath;
        std::map<UInt32, std::pair<std::string, UInt32>> folders;

        KitFolder(const std::filesystem::path& path) : filePath(path){
            CompileTypes();
        }

    private:

        //This dictates the order types are built, but it's not really necessary anymore 
        // now that all forms are built at once, then assigned values after.

        const std::unordered_map<unsigned int, unsigned int> order = {
            {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9, 9}, {10, 10},
            {11, 11}, {12, 12}, {13, 13}, {14, 14}, {15, 15}, {16, 16}, {17, 17}, {18, 18}, {19, 19}, {20, 20},
            {21, 21}, {22, 22}, {23, 23}, {24, 24}, {25, 25}, {26, 26}, {27, 27}, {28, 28}, {29, 29}, {30, 30},
            {31, 31}, {32, 32}, {33, 33}, {34, 34}, {35, 35}, {36, 36}, {37, 37}, {38, 38}, {39, 39}, {40, 40},
            {41, 41}, {42, 42}, {43, 43}, {44, 44}, {45, 45}, {46, 46}, {47, 47}, {48, 48}, {49, 49}, {50, 50},
            {51, 51}, {52, 52}, {53, 53}, {54, 54}, {55, 55}, {56, 56}, {57, 57}, {58, 58}, {59, 59}, {60, 60},
            {61, 61}, {62, 62}, {63, 63}, {64, 64}, {65, 65}, {66, 66}, {67, 67}, {68, 68}, {69, 69}, {70, 70},
            {71, 71}, {72, 72}, {73, 73}, {74, 74}, {75, 75}, {76, 76}, {77, 77}, {78, 78}, {79, 79}, {80, 80},
            {81, 81}, {82, 82}, {83, 83}, {84, 84}, {85, 85}, {86, 86}, {87, 87}, {88, 88}, {89, 89}, {90, 90},
            {91, 91}, {92, 92}, {93, 93}, {94, 94}, {95, 95}, {96, 96}, {97, 97}, {98, 98}, {99, 99}, {100, 100},
            {101, 101}, {102, 102}, {103, 103}, {104, 104}, {105, 105}, {106, 106}, {107, 107}, {108, 108}, {109, 109}, {110, 110},
            {111, 111}, {112, 112}, {113, 113}, {114, 114}, {115, 115}, {116, 116}, {117, 117}, {118, 118}, {119, 119}, {120, 120}
        };

        void CompileTypes() {

            for (const auto& typeEntry : std::filesystem::directory_iterator(filePath)) {

                std::string folderName = typeEntry.path().filename().string();
                int type = EvaluateType(folderName);
                auto it = order.find(type);
                if (it != order.end()) {
                    folders.emplace(it->second, std::make_pair(std::move(folderName), type));
                }
            }
        }

        int EvaluateType(const std::string& folderName) {
            try {
                size_t pos;
                int type = std::stoi(folderName, &pos);
                if (pos != folderName.size()) {
                    return type;
                }
                return -1;
            }
            catch (const std::exception& e) {
                return -1;
            }
        }
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

            for (const auto& folderEntry : kit.folders) {

                const auto& pair = folderEntry.second;
                const std::string& folderName = pair.first;
                std::string folderPath = (kit.filePath / folderName).string();

                file.push_back("type: " + std::to_string(pair.second));
                formBuilder.push_back("type: " + std::to_string(pair.second));
                fileManager->type = pair.second;

                ProcessFolder(folderPath);

            }

            //Create Kit Data:
            auto& insertedData = loadedKitFiles[kitIndex] = KitData{ info.name };
            fileManager->reverseNameLookup[info.name] = &insertedData;
            data = &insertedData;

            file.push_back("type: 201");
            file.insert(file.end(), std::make_move_iterator(info.kitEndData.begin()), std::make_move_iterator(info.kitEndData.end()));

        }

    private:

        void ProcessFile(const std::string& filePath) {

            std::ifstream inputFile(filePath);
            if (!inputFile.is_open()) {
                throw std::runtime_error("Failed to open file: " + filePath);
            }

            std::string line;
            while (std::getline(inputFile, line)) {
                line = trim(line);
                size_t commentPos = line.find("//");
                if (commentPos != std::string::npos) {
                    line = line.substr(0, commentPos);
                    line = trim(line);
                }
                if (!line.empty()) {

                    //if (!skip) {  //Maybe implement this later.
                        CompressToBuilder(line);
                    //}
                    file.push_back(std::move(line));
                }
            }

            //skip = false;
            file.push_back("}clear");   // Clear Template after every file.
            formBuilder.push_back("}clear");   // Clear Template after every file.

        }

        void ProcessFolder(const std::string& folderPath) {
            for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
                if (entry.is_regular_file()) {
                    const std::string& filePath = entry.path().string();
                    ProcessFile(filePath);
                }
                else if (entry.is_directory()) {
                    ProcessFolder(entry.path().string());
                }
            }
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

        std::string trim(const std::string& str) {
            size_t start = str.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) {
                return "";
            }
            size_t end = str.find_last_not_of(" \t\n\r");
            return str.substr(start, end - start + 1);
        }
    };
}