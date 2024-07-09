#pragma once
#include "KitLoader.h"
#include "DevkitCompiler.h"

class ScriptConverter {

    //Converter.........................................................................................................................

    std::string ReplaceScn(std::string operation, std::string segment);

    std::string ReplaceIf(std::string operation, std::string segment);
    std::string ReplaceEnd(std::string operation, std::string segment);
    std::string ReplaceElse(std::string operation, std::string segment);
    std::string ReplaceBegin(std::string operation, std::string segment);
    std::string ReplaceEq(std::string operation, std::string segment);
    std::string DeclareVar(std::string operation, std::string segment);
    std::string ProcessMutator(std::string operation, std::string segment);

    std::string ReplaceFunction(const CommandInfo& cmdInfo, std::string operation, std::string segment);
    
    std::string ReplaceComment(std::string operation, std::string segment);
    std::string Remove(std::string operation, std::string segment);

    std::string ProcessSegment(const std::string& line);
    std::string ProcessFirstLine(const std::string& line, const std::filesystem::path& filePath);
    std::string ProcessFunctionArguments(const std::string& line);
    std::pair<std::string, std::string> ProcessArguments(const std::string& line, UInt16 numArg);
    std::pair<UInt32, std::string> ProcessQuotes(const std::string& line, UInt32 startingpos = 0);
    std::pair<size_t, std::string> NextSegment(const std::string& line, UInt32 startingpos = 0);

    std::string CapLine(std::string& line);

    std::vector<std::string> ReadFile(const std::filesystem::path& filePath);
    void WriteToFile(const std::filesystem::path& outputDir, const std::string& fileName);

    bool skipCap = false;

public:


    void Convert();

    std::string sRoot = GetFalloutDirectory() + "Data\\ScriptConverter";
    std::string sOutput = GetFalloutDirectory() + "Data\\ScriptConverter\\Converted";

    std::unordered_map<std::string, TypeFunction<ScriptConverter>> functs;

    std::unordered_map<std::string, UInt16> variables;

    std::unordered_set<char> whiteSpace;
    std::unordered_set<char> mutators;

    std::unordered_map<std::string, std::string> stringOps;

    std::vector<std::string> convertedFile;

    bool Initialize() {
        // Validate the root directory
        if (!std::filesystem::exists(sRoot) || !std::filesystem::is_directory(sRoot)) {
            std::cerr << "Error: Root directory '" << sRoot << "' does not exist or is not a directory." << std::endl;
            return false; // Initialization failed
        }
        return true; // Initialization successful
    }

    static ScriptConverter* Create() {
        std::unique_ptr<ScriptConverter> converter(new ScriptConverter());
        if (!converter->Initialize()) {
            return nullptr; // Return nullptr if initialization failed
        }
        return converter.release(); // Return the raw pointer if successful
    }

    ScriptConverter() {

        stringOps[R"(\)"] = R"(\\)";

        mutators.insert('$');
        mutators.insert('#');
        mutators.insert(';');

        whiteSpace.insert('#');
        whiteSpace.insert('}');
        whiteSpace.insert('{');
        whiteSpace.insert(')');
        whiteSpace.insert('(');
        whiteSpace.insert(' ');
        whiteSpace.insert(';');
        whiteSpace.insert('$');
        whiteSpace.insert('\t');
        whiteSpace.insert('\0');

        functs["scn"] = TypeFunction( &ScriptConverter::ReplaceScn);
        functs["begin"] = TypeFunction( &ScriptConverter::ReplaceBegin);
        functs["end"] = TypeFunction(&ScriptConverter::ReplaceEnd);
        functs["endif"] = TypeFunction(&ScriptConverter::ReplaceEnd);
        functs["loop"] = TypeFunction(&ScriptConverter::ReplaceEnd);

        functs["if"] = TypeFunction(&ScriptConverter::ReplaceIf);
        functs["elseif"] = TypeFunction(&ScriptConverter::ReplaceIf);
        functs["while"] = TypeFunction(&ScriptConverter::ReplaceIf);

        functs["else"] = TypeFunction(&ScriptConverter::ReplaceElse);

        functs[";"] = TypeFunction(&ScriptConverter::ReplaceComment);

        functs["$"] = TypeFunction(&ScriptConverter::ProcessMutator);
        functs["#"] = TypeFunction(&ScriptConverter::ProcessMutator);

        functs[":="] = TypeFunction(&ScriptConverter::ReplaceEq);
        functs["to"] = TypeFunction(&ScriptConverter::ReplaceEq);

        functs["let"] = TypeFunction(&ScriptConverter::Remove);
        functs["set"] = TypeFunction(&ScriptConverter::Remove);
        functs["eval"] = TypeFunction(&ScriptConverter::Remove);

        functs["array_var"] = TypeFunction(&ScriptConverter::DeclareVar);
        functs["string_var"] = TypeFunction(&ScriptConverter::DeclareVar);
        functs["ref"] = TypeFunction(&ScriptConverter::DeclareVar);
        functs["int"] = TypeFunction(&ScriptConverter::DeclareVar);
        functs["float"] = TypeFunction(&ScriptConverter::DeclareVar);

    }

};