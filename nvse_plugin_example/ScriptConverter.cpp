#include "ScriptConverter.h"

std::string RemoveCurlyBraces(const std::string& input) {
    std::string result;
    for (char ch : input) {
        if (ch != '{' && ch != '}') {
            result += ch;
        }
    }
    return result;
}

std::string RemoveCurlyBracesSet(const std::string& segment) {
    std::string result = segment;
    size_t openPos = result.find('{');
    size_t closePos = result.find('}', openPos);

    if (openPos != std::string::npos && closePos != std::string::npos) {
        result.erase(openPos, 1); // Remove the '{'
        result.erase(closePos - 1, 1); // Remove the '}', note: adjust closePos due to previous erase
    }

    return result;
}

std::pair<std::string, std::string> SplitAtComment(const std::string& input) {

    std::pair<std::string, std::string> result;
    bool inQuotes = false;
    UInt32 validPos = 0;
    for (size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        if (ch == '"') {
            inQuotes = !inQuotes; // Toggle the inQuotes flag
        }
        if (ch == '/' && i + 1 < input.size() && input[i + 1] == '/' && !inQuotes) {

            result.first = input.substr(0, validPos + 1);
            result.second = input.substr(validPos + 1);
            return result;

        }
        else if (ch != ' ') {
            validPos = i;
        }

    }

    result.first = input;
    result.second = "";
    return result;

}

std::pair<UInt32, std::string> ScriptConverter::ProcessQuotes(const std::string& line, UInt32 startingpos) {

    std::pair<UInt32, std::string> result;

    UInt32 pos = startingpos;
    std::ostringstream wordStream;

    char ch;

    while (pos < line.length()) {

    loopTop:

        ++pos;

        for (const auto& op : stringOps) {
            if (line.substr(pos, op.first.length()) == op.first) {
                wordStream << op.second;
                goto loopTop;
            }
        }

        ch = line[pos];

        wordStream << ch;

        if (ch == '"') {
            ++pos;
            break;
        }

    }


    return std::make_pair(pos, wordStream.str());

}

std::pair<std::string, std::string> ScriptConverter::ProcessArguments(const std::string& line, UInt16 numArg) {

    std::pair<std::string, std::string> result;

    UInt32 pos = 0;
    SInt32 argCount = 0;

    std::unordered_set<std::string> comparisonOps = { "||", "&&", "!=", "==", "<=", ">=", "<", ">"};
    std::ostringstream wordStream;
    SInt32 braceDepth = 0;
    bool inQuotes = false;
    bool addComma = true;

    if (numArg > 0) {

        while (pos < line.length()) {

            ++pos;

            char ch = line[pos];

            if (ch == '"') {

                inQuotes = !inQuotes;
                wordStream << ch;

                auto pair = ProcessQuotes(line, pos);
                pos = pair.first;
                wordStream << pair.second;
                ch = line[pos];

            }

            if (ch == '(') {
                ++braceDepth;
                wordStream << ch;
                continue;
            }

            if (ch == ')') {
                --braceDepth;
            }

            if (braceDepth < 0) {
                break;
            }

            if (braceDepth != 0) {
                wordStream << ch;
                continue;
            }

            // Check for two-character operators
            if (pos + 1 < line.length()) {
                std::string token = { ch, line[pos + 1] };
                if (comparisonOps.count(token)) {
                    pos -= 2;
                    break;
                }
            }

            if (ch == ',') {
                addComma = false;
            }

            if (ch == ' ' || ch == ';' || ch == '\0' || ch == '\t') {

                if (!wordStream.str().empty()) {

                    if (ch == ' ' || ch == ';') {

                        if (addComma) {
                            wordStream << ',' << ch;

                        }
                        else {
                            wordStream << ch;
                        }

                    }

                    //Next argument
                    if (++argCount >= numArg) {
                        break;
                    }

                    addComma = true;
                    inQuotes = false;
                    continue;

                }

                wordStream.str(""); // Clear the stream
                wordStream.clear();

            }
            else {

                wordStream << ch;

            }
        }

    }

    std::string arguments = wordStream.str();

    // Trim ', '
    if (!arguments.empty() && (arguments.back() == ' ' || arguments.back() == ';')) {
        arguments.pop_back();
        if (!arguments.empty() && arguments.back() == ',') {
            arguments.pop_back();
        }
    }

    result.first = "(" + ProcessSegment(arguments);

    if (braceDepth <= 0) {  //Lambda multi line
        result.first += ")";
    }

    result.second = line.substr(pos);

    return result;
}

std::string ScriptConverter::ProcessFunctionArguments(const std::string& segment) {

    UInt32 pos = 0;
    std::ostringstream wordStream;

    while (pos < segment.size()) {

        auto arg = NextSegment(segment, pos);
        std::string varName = arg.second;
        pos = arg.first;

        if (!varName.empty()) {

            std::string varNameLower = StringUtils::toLowerCase(varName);
            auto it = variables.find(varNameLower);
            if (it != variables.end()) {

                if (it->second == 3) {

                    varName = "string " + varName;

                }
                else if (it->second == 2) {

                    varName = "float " + varName;

                }
                else if (it->second == 1) {

                    varName = "int " + varName;

                }
                else if (it->second == 4) {

                    varName = "array " + varName;

                }
                else if (it->second == 0) {

                    varName = "ref " + varName;

                }

                varNameLower = StringUtils::toLowerCase(varName);
                varNameLower += ";";

                for (auto& line : convertedFile) {
                    if (StringUtils::toLowerCase(line) == varNameLower) {
                        line.clear();
                    }
                }

                if (varName.back() == ',') {
                    wordStream << (varName + " ");
                }
                else {
                    wordStream << (varName + ", ");
                }
            }

        }

    }

    std::string arguments = wordStream.str();

    // Trim ', '
    if (!arguments.empty() && (arguments.back() == ' ' || arguments.back() == ';')) {
        arguments.pop_back();
        if (!arguments.empty() && arguments.back() == ',') {
            arguments.pop_back();
        }
    }

    return arguments;

}

std::string ScriptConverter::DeclareVar(std::string operation, std::string segment) {

    auto pair = NextSegment(segment);
    std::string varName = pair.second;

    varName = StringUtils::toLowerCase(varName);
    operation = StringUtils::toLowerCase(operation);

    if (operation == "array_var"){
        this->variables[varName] = 4;
        return ("array" + ProcessSegment(segment));
    }
    else if (operation == "string_var") {
        this->variables[varName] = 3;
        return ("string" + ProcessSegment(segment));
    }
    else if (operation == "float") {
        this->variables[varName] = 2;
        return (operation + ProcessSegment(segment));
    }
    else if (operation == "int") {
        this->variables[varName] = 1;
        return (operation + ProcessSegment(segment));
    }
    else if (operation == "ref") {
        this->variables[varName] = 0;
        return (operation + ProcessSegment(segment));
    }

}

std::string ScriptConverter::ReplaceScn(std::string operation, std::string segment) {

    return ("name" + ProcessSegment(segment));

}

std::string ScriptConverter::ReplaceComment(std::string operation, std::string segment) {

    return ("//" + segment);

}

std::string ScriptConverter::Remove(std::string operation, std::string segment) {

    size_t whitespace = segment.find_first_not_of(" ");

    if (whitespace == std::string::npos) {
        return "";
    }

    return ProcessSegment(segment.substr(whitespace));

}

std::string TypeToMutator(UInt16 type) {

    switch (type) {
    case 1:
    case 2:
        return "#";
    case 3:
        return "$";
    default:
        return "";
    }
}

std::string ScriptConverter::ProcessMutator(std::string operation, std::string segment) {

    auto pair = NextSegment(segment);
    std::string varName = pair.second;
    varName = StringUtils::toLowerCase(varName);

    auto it = this->variables.find(varName);
    if (it != this->variables.end()) {

        if (operation != TypeToMutator(it->second)) {

            segment = ProcessSegment(segment);
            return (operation + segment);

        }

    }

    return ProcessSegment(segment);

}

std::string ScriptConverter::ReplaceEq(std::string operation, std::string segment) {

    return ("=" + ProcessSegment(segment));

}

std::string ScriptConverter::ReplaceIf(std::string operation, std::string segment) {

    skipCap = true;

    segment = ProcessSegment(segment);
    auto splitPair = SplitAtComment(segment);

    return (operation + "(" + splitPair.first + ")" + "{" + splitPair.second);

}

std::string ScriptConverter::ReplaceEnd(std::string operation, std::string segment) {

    skipCap = true;
    return ("}" + ProcessSegment(segment));

}

std::string ScriptConverter::ReplaceElse(std::string operation, std::string segment) {

    skipCap = true;
    return ("} " + operation + "{" + ProcessSegment(segment));

}

std::string ScriptConverter::ReplaceBegin(std::string operation, std::string segment) {

    skipCap = true;

    auto pair = NextSegment(segment);
    operation = pair.second;
    operation = StringUtils::toLowerCase(operation);

    segment = (segment.substr(operation.length() + 1));
    auto splitPair = SplitAtComment(segment);

    if (operation == "function") {
        return("fn (" + ProcessFunctionArguments(RemoveCurlyBraces(splitPair.first)) + ")" + "{" + splitPair.second);
    }
    else {

        if (splitPair.first.empty()) {
            return (operation + "{" + splitPair.second);
        }
        return (operation + "::" + splitPair.first + "{" + splitPair.second);

    }

}

std::string ScriptConverter::ReplaceFunction(const CommandInfo& cmdInfo, std::string operation, std::string segment) {

    std::pair<std::string, std::string> argPair;
    if (StringUtils::toLowerCase(operation) == "call") {
        argPair = ProcessArguments(segment, 25);
    }
    else {
        argPair = ProcessArguments(segment, cmdInfo.numParams);
    }
    argPair.first += ProcessSegment(argPair.second);
    return (operation + argPair.first);

}

std::pair<size_t, std::string> ScriptConverter::NextSegment(const std::string& line, UInt32 startingpos) {

    std::stringstream ss(line.substr(startingpos));
    std::string word = "";
    bool inQuotes = false;

    UInt32 pos = startingpos;

    char ch;
    while (ss.get(ch)) {

        ++pos;

        if (ch == '"') {
            inQuotes = !inQuotes; // Toggle the inQuotes flag
            continue;
        }

        if (inQuotes) {
            continue;
        }

        if (whiteSpace.find(ch) == whiteSpace.end()) {

            word += ch;

        }
        else {

            if (!word.empty()) {
                return std::make_pair(--pos, word);
            }

            if (mutators.find(ch) != mutators.end()) {
                return std::make_pair(pos, std::string(1, ch));
            }

            word.clear();
        }

    }

    if (!word.empty()) {
        return std::make_pair(pos, word);
    }

    return std::make_pair(pos, "");
}

std::string ScriptConverter::ProcessSegment(const std::string& line) {

    UInt32 pos = 0;

    while (pos < line.size()) {

        auto segment = NextSegment(line, pos);
        std::string operation = segment.second;
        pos = segment.first;

        if (!operation.empty() ) {

            size_t dotPos = operation.find_last_of('.');
            if (dotPos != std::string::npos) {
                operation = operation.substr(dotPos + 1);   //Return the right of '.' for function processing
            }

            std::string operationLower = StringUtils::toLowerCase(operation);
            auto it = functs.find(operationLower);
            if (it != functs.end()) {

                TypeFunction<ScriptConverter>& funct = it->second;
                std::string skipped = line.substr(0, pos - (operation.length()));
                return skipped + (this->*(funct.functIterConvert))(operation, line.substr(pos));

            }
            else if (const CommandInfo* cmdInfo = GetCmdByName(operationLower.c_str())) {

                std::string skipped = line.substr(0, pos - (operation.length()));
                return skipped + ReplaceFunction(*cmdInfo, operation, line.substr(pos));

            }

        }

    }

    return line;
}

std::string ScriptConverter::ProcessFirstLine(const std::string& line, const std::filesystem::path& filePath) {

    UInt32 pos = 0;
    std::string result = "";

    auto segment = NextSegment(line);
    std::string operation = segment.second;
    pos = segment.first;

    if (!operation.empty()) {

        std::string operationLower = StringUtils::toLowerCase(operation);
        if (operationLower == "scn") {

            result = ReplaceScn(operation, line.substr(pos));
            CapLine(result);

        }
        else {

            operation = filePath.stem().string();
            result = ("name " + operation + ";");

        }

    }

    convertedFile.push_back(result);
    convertedFile.push_back("");
    convertedFile.push_back("//This script was converted using pNVSE script converter v1:");
    convertedFile.push_back("");

    return line;
}

std::string ScriptConverter::CapLine(std::string& line) {
    // Remove trailing whitespace
    line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

    if (line.empty()) {
        return line;
    }

    std::size_t pos = line.find("//");

    if (pos != std::string::npos) {

        std::string beforeComment = line.substr(0, pos);
        pos = beforeComment.find_last_not_of(" \t\n\r\f\v") + 1;

        if (pos != 0) {
            line.insert(pos, ";");
        }

    }
    else {
        if (pos != 0) {
            line += ";";
        }
    }

    return line;
}

void ScriptConverter::WriteToFile(const std::filesystem::path& outputDir, const std::string& fileName) {

    std::filesystem::path outputPath = outputDir / fileName;
    std::ofstream outFile(outputPath);
    if (outFile.is_open()) {
        for (const auto& line : convertedFile) {
            outFile << line << '\n';
        }
        outFile.close();
    }
    else {
        std::cerr << "Failed to open file: " << outputPath << std::endl;
    }
}

void ScriptConverter::Convert() {

    for (const auto& entry : std::filesystem::directory_iterator(sRoot)) {

        if (entry.is_regular_file()) {

            ReadFile(entry.path());
            std::filesystem::path outputFilePath = entry.path().filename(); // Keep the same file name
            WriteToFile(sOutput, outputFilePath.string());
            convertedFile.clear();

        }

    }

}

std::vector<std::string> ScriptConverter::ReadFile(const std::filesystem::path& filePath) {

    std::ifstream file(filePath);
 
    if (file.is_open()) {

        std::string line;

        std::getline(file, line);
        line = ProcessFirstLine(line, filePath);

        while (std::getline(file, line)) {

            line = ProcessSegment(line);
            if (!skipCap) {
                CapLine(line);
            }
            else {
                skipCap = false;
            }

            convertedFile.push_back(line); // Add the processed line to the vector

        }

        variables.clear();
        file.close();
    }

    return convertedFile;

}