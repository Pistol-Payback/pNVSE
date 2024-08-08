#pragma once
#include <array>
#include "ppNVSE.h"
#include "WeaponSmith.h"
#include <fstream>
#include <filesystem>
#include <variant>

class ScriptConverter;

template<typename IterConverter>
struct TypeFunction {
    static const UInt8 allowDuplicate = 0b10000000;
    static const UInt8 optional = 0b01000000;

    bool isNested;
    UInt8 type = 0;
    std::string name = "null";

    using FunctionScriptConverter = std::string(IterConverter::*)(std::string operation, std::string segment);
    using FunctionPtrIter = void (IterConverter::*)(std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
    using FunctionPtrIterNested = void (IterConverter::*)(bool nested, std::vector<std::string>::const_iterator& it, std::istringstream& argStream);
    using FunctionPtrSelf = void (IterConverter::*)(TypeFunction& function, std::istringstream& argStream);

    union {
        FunctionScriptConverter functIterConvert;
        FunctionPtrIter functIter;
        FunctionPtrIterNested functionPtr;
        FunctionPtrSelf functSelf;
    };

    TypeFunction() : type(0) {}

    explicit TypeFunction(FunctionPtrIterNested funcPtr) : functionPtr(funcPtr), type(2) {}
    explicit TypeFunction(FunctionPtrIter funcPtr) : functIter(funcPtr), type(1) {}
    explicit TypeFunction(FunctionScriptConverter funcPtr) : functIterConvert(funcPtr), type(1) {}

    std::array<UInt8, 8> argumentTypes{};

    TypeFunction(std::string& functionName, std::istringstream& arguments, FunctionPtrSelf funcPtr)
        : name(functionName), functSelf(funcPtr) {

        std::string argType;
        bool isOptional = false;
        int index = 0;

        while (arguments >> argType && index < this->argumentTypes.size()) {
            if (argType == "|") {
                isOptional = true;
            }
            else {
                int typeCode = this->convertArgumentType(argType);  // Assuming convertArgumentType is accessible
                if (typeCode) {
                    if (isOptional) {
                        typeCode |= TypeFunction<IterConverter>::optional;
                    }
                    this->argumentTypes[index++] = typeCode;
                }
                else {
                    throw std::runtime_error("Invalid argument type '" + argType + "' for function '" + functionName + "'");
                }
            }
        }

        if (index == this->argumentTypes.size() && (arguments >> argType)) {
            Console_Print("Error: Too many arguments for function: %s", functionName.c_str());
        }
    }

    int convertArgumentType(const std::string& argType) const {
        switch (argType[0]) {
        case '*': switch (argType[1]) {
        case 'r': return allowDuplicate | 1;
        case 'i': return allowDuplicate | 2;
        case 'f': return allowDuplicate | 3;
        case 's': return allowDuplicate | 4;
        default: return 0;
        }
        case 'r': return 1;
        case 'i': return 2;
        case 'f': return 3;
        case 's': return 4;
        default: return 0;
        }
    }

};
/*
template<typename IterConverter>
struct TraitFunction : TypeFunction<IterConverter> {

    std::array<UInt8, 8> argumentTypes{};

    TraitFunction(std::string& functionName, std::istringstream& arguments, typename TypeFunction<IterConverter>::FunctionPtrSelf funcPtr)
        : TypeFunction<IterConverter>(funcPtr) {  // Properly initialize base class
        this->name = functionName;  // Set the name
        std::string argType;
        bool isOptional = false;
        int index = 0;

        while (arguments >> argType && index < this->argumentTypes.size()) {
            if (argType == "|") {
                isOptional = true;
            }
            else {
                int typeCode = this->convertArgumentType(argType);  // Assuming convertArgumentType is accessible
                if (typeCode) {
                    if (isOptional) {
                        typeCode |= TypeFunction<IterConverter>::optional;
                    }
                    this->argumentTypes[index++] = typeCode;
                }
                else {
                    throw std::runtime_error("Invalid argument type '" + argType + "' for function '" + functionName + "'");
                }
            }
        }

        if (index == this->argumentTypes.size() && (arguments >> argType)) {
            Console_Print("Error: Too many arguments for function: %s", functionName.c_str());
        }
    }

    int convertArgumentType(const std::string& argType) const {
        switch (argType[0]) {
        case '*': switch (argType[1]) {
        case 'r': return allowDuplicate | 1;
        case 'i': return allowDuplicate | 2;
        case 'f': return allowDuplicate | 3;
        case 's': return allowDuplicate | 4;
        default: return 0;
        }
        case 'r': return 1;
        case 'i': return 2;
        case 'f': return 3;
        case 's': return 4;
        default: return 0;
        }
    }
};
*/
namespace Kit {

    constexpr std::array<std::pair<std::string_view, int>, 200> extensionToType{
        std::pair{"animset", -1},
        std::pair{"fileheader", 1},
        std::pair{"group", 2},
        std::pair{"gamesetting", 3},
        std::pair{"textureset", 4},
        std::pair{"menuicon", 5},
        std::pair{"global", 6},
        std::pair{"class", 7},
        std::pair{"faction", 8},
        std::pair{"headpart", 9},
        std::pair{"hair", 10},
        std::pair{"eyes", 11},
        std::pair{"race", 12},
        std::pair{"sound", 13},
        std::pair{"acousticspace", 14},
        std::pair{"skill", 15},
        std::pair{"baseeffect", 16},
        std::pair{"script", 17},
        std::pair{"landtexture", 18},
        std::pair{"objecteffect", 19},
        std::pair{"actoreffect", 20},
        std::pair{"activator", 21},
        std::pair{"talkingactivator", 22},
        std::pair{"terminal", 23},
        std::pair{"armor", 24},
        std::pair{"book", 25},
        std::pair{"clothing", 26},
        std::pair{"container", 27},
        std::pair{"door", 28},
        std::pair{"ingredient", 29},
        std::pair{"light", 30},
        std::pair{"miscellaneous", 31},
        std::pair{"static", 32},
        std::pair{"staticcollection", 33},
        std::pair{"moveablestatic", 34},
        std::pair{"placeablewater", 35},
        std::pair{"grass", 36},
        std::pair{"tree", 37},
        std::pair{"flora", 38},
        std::pair{"furniture", 39},
        std::pair{"weapon", 40},
        std::pair{"ammo", 41},
        std::pair{"npc", 42},
        std::pair{"creature", 43},
        std::pair{"leveledcreature", 44},
        std::pair{"leveledcharacter", 45},
        std::pair{"key", 46},
        std::pair{"ingestible", 47},
        std::pair{"idlemarker", 48},
        std::pair{"note", 49},
        std::pair{"constructibleobject", 50},
        std::pair{"projectile", 51},
        std::pair{"leveleditem", 52},
        std::pair{"weather", 53},
        std::pair{"climate", 54},
        std::pair{"region", 55},
        std::pair{"navmeshinfomap", 56},
        std::pair{"cell", 57},
        std::pair{"reference", 58},
        std::pair{"characterreference", 59},
        std::pair{"creaturereference", 60},
        std::pair{"missileprojectile", 61},
        std::pair{"grenadeprojectile", 62},
        std::pair{"beamprojectile", 63},
        std::pair{"flameprojectile", 64},
        std::pair{"worldspace", 65},
        std::pair{"land", 66},
        std::pair{"navmesh", 67},
        std::pair{"unknown", 68},
        std::pair{"dialogtopic", 69},
        std::pair{"dialogtopicinfo", 70},
        std::pair{"quest", 71},
        std::pair{"idle", 72},
        std::pair{"package", 73},
        std::pair{"combatstyle", 74},
        std::pair{"loadscreen", 75},
        std::pair{"leveledspell", 76},
        std::pair{"animatedobject", 77},
        std::pair{"water", 78},
        std::pair{"effectshader", 79},
        std::pair{"offsettable", 80},
        std::pair{"explosion", 81},
        std::pair{"debris", 82},
        std::pair{"imagespace", 83},
        std::pair{"imagespaceadapter", 84},
        std::pair{"formlist", 85},
        std::pair{"perk", 86},
        std::pair{"bodypartdata", 87},
        std::pair{"addonnode", 88},
        std::pair{"actorvalueinfo", 89},
        std::pair{"radiationstage", 90},
        std::pair{"camerashot", 91},
        std::pair{"camerapath", 92},
        std::pair{"voicetype", 93},
        std::pair{"impactdata", 94},
        std::pair{"impactdataset", 95},
        std::pair{"armoraddon", 96},
        std::pair{"encounterzone", 97},
        std::pair{"message", 98},
        std::pair{"ragdoll", 99},
        std::pair{"defaultobjectmanager", 100},
        std::pair{"lightingtemplate", 101},
        std::pair{"music", 102},
        std::pair{"itemmod", 103},
        std::pair{"reputation", 104},
        std::pair{"continuousbeamprojectile", 105},
        std::pair{"recipe", 106},
        std::pair{"recipecategory", 107},
        std::pair{"casinochip", 108},
        std::pair{"casino", 109},
        std::pair{"loadscreen", 110},
        std::pair{"mediaset", 111},
        std::pair{"medialocationcontroller", 112},
        std::pair{"challenge", 113},
        std::pair{"ammoeffect", 114},
        std::pair{"caravancard", 115},
        std::pair{"caravanmoney", 116},
        std::pair{"caravandeck", 117},
        std::pair{"dehydrationstage", 118},
        std::pair{"hungerstage", 119},
        std::pair{"sleepdeprivationstage", 120},
        std::pair{"akimbo", 222},
        std::pair{"kitInfo", 999}
    };

    //extern const std::unordered_map<int, std::vector<std::string>> typeToExtensions;

    constexpr int ConvertExtensionToType(std::string_view extension);
    constexpr std::string_view ConvertTypeToExtension(SInt32 type);

    class DevkitCompiler;
    class KitCompressedFile;
    class KitData;

    extern std::unordered_map<UInt32, KitData> loadedKitFiles;
    extern std::unordered_map<std::string, KitData*> reverseNameLookup;
    extern KitData* GetKitDataByName(std::string name);
    extern KitData* GetKitDataByIndex(UInt32 index);

    //Saves with loaded game.
    class KitData {

        public:
            std::string name = "null";
            UInt32 index = 0;
            UInt32 version = 0;
            bool safeUninstall = false;

            std::map<UInt32, Script*> updater; //First is the version, updates from your version to latest in order

            void runUpdater(UInt32 currentVersion) {
                for (const auto& update : updater) {
                    if (update.first > currentVersion) {
                        if (update.second) {
                            g_scriptInterface->CallFunction(update.second, nullptr, nullptr, nullptr, 1, currentVersion);
                        }
                    }
                }
            }

            static KitData* create(const std::string& name, UInt32 modIndex) {
                auto result = loadedKitFiles.emplace(modIndex, KitData(name, modIndex));
                if (result.second) {
                    reverseNameLookup[name] = &(result.first->second);
                }
                return &(result.first->second);
            }

            //Use create
            KitData(const std::string& name, UInt32 modIndex)
                : name(name), index(modIndex) {}

    };

    //Compiler................................................................................

    class KitFolder {
    public:
        explicit KitFolder(const std::filesystem::path& path);
        const std::filesystem::path filePath;
        std::map<SInt32, std::vector<std::filesystem::path>> filesByType;

    private:
        void CompileTypes();
        SInt32 EvaluateType(const std::filesystem::path& filePath);
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
        const std::vector<std::string>* currentFile;

        std::string currentFunction;
        SInt32 type = 0;
        std::unordered_map<SInt32, std::unordered_map<std::string, TypeFunction<DevkitCompiler>>> typeFunctions;
        std::unordered_map<std::string, TypeFunction<DevkitCompiler>> operatorOverload;
        std::string sRoot;

        bool linkerFaze;

        //For conflict detector
        std::unordered_map<UInt32, std::string> conflictList; //UInt32 is the index identifier. kitIndex + kitIndex = identifier.

        void compileLoadOrder();
        void compileLoadOrderFromAll();   //Use one or the other, but not both.

        bool validateLoadOrder();
        bool validateMaster(UInt32 index, std::string& master);
        bool validateAndReorderLoadOrder();
        void writeLoadOrderToFile(const std::string& filePath);

        void loadESPToKitData();
        void loadKitFolders();
        void compressKitFolders();

        //Builds all forms accross all compressed files
        void BuildForms(DevkitCompiler& functionsList);

        //Processes functions
        void ReadKitFiles(DevkitCompiler& functionsList);
        void CallTypeFunction(DevkitCompiler& functionsList, std::vector<std::string>::const_iterator& it, SInt32 functionType);
        bool IsNested(DevkitCompiler& functionsList, std::string& line, std::vector<std::string>::const_iterator& it);

    };

    class KitCompressedFile {

    public:

        std::vector<std::string> file;
        std::vector<std::string> formBuilder;

        KitFileManager& fileManager;
        KitData* data;

        KitCompressedFile(KitFileManager& fileManager, KitData* kitData) : fileManager(fileManager), data(kitData) {}

        KitCompressedFile(const KitFolder& kitFolder, KitFileManager& fileManager, KitData* kitData)
            : KitCompressedFile(fileManager, kitData) {
            file.reserve(kitFolder.filesByType.size());
            for (const auto& typeEntry : kitFolder.filesByType) {
                processTypeEntry(typeEntry);
            }
        }

        //For processing strings into kits
        KitCompressedFile(const std::string& kitInfo, KitFileManager& fileManager, KitData* kitData, SInt32 type)
            : KitCompressedFile(fileManager, kitData) {
            processKitInfo(kitInfo, type);
        }

        static std::optional<KitCompressedFile> KitCompressESP(const ModInfo* mod, KitFileManager& manager, KitData* kitData) {
            if (mod->description.m_data) {
                std::string kitInfo(mod->description.m_data);
                std::string firstLine = StringUtils::extractFirstLine(kitInfo.data(), kitInfo.length());
                StringUtils::toLowerCase(firstLine);
                if (firstLine == "devkit") {
                    return KitCompressedFile(kitInfo, manager, kitData, 999);
                }
            }
            return std::nullopt;
        }

    private:

        void processTypeEntry(const std::pair<int, std::vector<std::filesystem::path>>& typeEntry) {
            for (const auto& filePath : typeEntry.second) {
                std::string fileStringPath = filePath.string();
                file.push_back("type: " + std::to_string(typeEntry.first));
                formBuilder.push_back("type: " + std::to_string(typeEntry.first));
                fileManager.type = typeEntry.first;
                ProcessFile(fileStringPath);
            }
        }

        void processKitInfo(const std::string& kitInfo, SInt32 type) {
            file.push_back("type: " + std::to_string(type));
            fileManager.type = type;
            ProcessInputString(kitInfo);
        }

        void ProcessLine(std::string line, bool toBuilder) {

            trim(line);

            size_t commentPos = line.find("//");
            if (commentPos != std::string::npos) {
                line.erase(commentPos);
                trim(line);
            }

            if (!line.empty()) {
                if (toBuilder) {
                    CompressToBuilder(line);
                }
                file.push_back(line);
            }

        }

        void ProcessInputString(const std::string& input) {
            std::istringstream stream(input);
            std::string line;
            while (std::getline(stream, line)) {
                ProcessLine(line, false);
            }
            file.push_back("}clear");
        }

        void ProcessFile(const std::string& filePath) {
            std::ifstream inputFile(filePath);
            if (!inputFile.is_open()) {
                throw std::runtime_error("Failed to open file: " + filePath);
            }

            std::string line;
            while (std::getline(inputFile, line)) {
                ProcessLine(line, true);
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

            if (functionName == "}") {   //Allows nesting
                formBuilder.push_back(line);
                return;
            }
            else if (functionName == "{") {
                formBuilder.push_back(line);
                return;
            }

            //Process Build Function

            if (!functionName.empty() && functionName.back() == ':') {
                functionName.pop_back();
            }

            functionName = StringUtils::toLowerCase(functionName);

            auto itFunct = fileManager.typeFunctions[fileManager.type].find(functionName);
            if (itFunct == fileManager.typeFunctions[fileManager.type].end()) {
                if (fileManager.type != 0) {
                    itFunct = fileManager.typeFunctions[0].find(functionName);
                    if (itFunct == fileManager.typeFunctions[0].end()) {
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