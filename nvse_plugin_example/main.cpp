#pragma once
#include "nvse/PluginAPI.h"
#include "nvse/CommandTable.h"
#include "nvse/GameAPI.h"
#include "nvse/ParamInfos.h"
#include "nvse/ParamInfosNVSE.h"
#include "nvse/GameObjects.h"
#include <string>
#include "ppNVSE.h"
#include "EventHandlers.h"

#include "GameUI.h"

#include "ppNVSE_functions.h"
#include "WS_Funct.h"

#include "Initializers.h"
#include "Actor_Ext.h"

#include "SaveSystem.h"

#include "SafeWrite.h"
#include "DevkitCompiler.h"
#include "ScriptConverter.h"

//NoGore is unsupported in xNVSE

IDebugLog		gLog("ppNVSE.log");
constexpr UInt32 g_PluginVersion = 1;

PluginHandle	g_pluginHandle = kPluginHandle_Invalid;

NVSEMessagingInterface* g_messagingInterface{};
NVSEInterface* g_nvseInterface{};
NVSECommandTableInterface g_cmdTableInterface;

//UI 11-15-2023
InterfaceManager* g_interfaceManager;

// RUNTIME = Is not being compiled as a GECK plugin.

NVSEScriptInterface* g_script{};
NVSEStringVarInterface* g_stringInterface{};

NVSEDataInterface* g_dataInterface{};
NVSESerializationInterface* g_serializationInterface{};
NVSEConsoleInterface* g_consoleInterface{};
NVSEEventManagerInterface* g_eventInterface{};

#define WantLambdaFunctions 0 // set to 1 if you want these PluginAPI functions
#if WantLambdaFunctions
_LambdaDeleteAllForScript LambdaDeleteAllForScript{};
_LambdaSaveVariableList LambdaSaveVariableList{};
_LambdaUnsaveVariableList LambdaUnsaveVariableList{};
_IsScriptLambda IsScriptLambda{};
#endif

#define WantScriptFunctions 0 // set to 1 if you want these PluginAPI functions
#if WantScriptFunctions
_HasScriptCommand HasScriptCommand{};
_DecompileScript DecompileScript{};
#endif

NVSEArrayVarInterface* g_arrayInterface{};
NVSEArrayVarInterface g_arrayVar;

bool (*ExtractArgsEx)(COMMAND_ARGS_EX, ...);
bool (*ExtractFormatStringArgs)(UInt32 fmtStringPos, char* buffer, COMMAND_ARGS_EX, UInt32 maxParams, ...);  // From JIP_NVSE.H
NVSEArrayVarInterface* g_arrInterface = nullptr;
NVSEArrayVar* (*CreateArray)(const NVSEArrayElement* data, UInt32 size, Script* callingScript);
NVSEArrayVar* (*CreateStringMap)(const char** keys, const NVSEArrayElement* values, UInt32 size, Script* callingScript);
NVSEArrayVar* (*CreateMap)(const double* keys, const NVSEArrayElement* values, UInt32 size, Script* callingScript);
bool (*AssignArrayResult)(NVSEArrayVar* arr, double* dest);
void (*SetElement)(NVSEArrayVar* arr, const NVSEArrayElement& key, const NVSEArrayElement& value);
void (*AppendElement)(NVSEArrayVar* arr, const NVSEArrayElement& value);
UInt32(*GetArraySize)(NVSEArrayVar* arr);
NVSEArrayVar* (*LookupArrayByID)(UInt32 id);
bool (*GetElement)(NVSEArrayVar* arr, const NVSEArrayElement& key, NVSEArrayElement& outElement);
bool (*GetArrayElements)(NVSEArrayVar* arr, NVSEArrayElement* elements, NVSEArrayElement* keys);

const CommandInfo* (*GetCmdByName)(const char* name);

NVSEStringVarInterface g_strInterface;
bool (*AssignString)(COMMAND_ARGS, const char* newValue);
const char* (*GetStringVar)(UInt32 stringID);
NVSEMessagingInterface* g_msg = nullptr;
NVSEScriptInterface* g_scriptInterface = nullptr;

bool (*FunctionCallScript)(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container, NVSEArrayElement* result, UInt8 numArgs, ...);
bool (*FunctionCallScriptAlt)(Script* funcScript, TESObjectREFR* callingObj, UInt8 numArgs, ...);
Script* (*pCompileScript)(const char* scriptText);

ExpressionEvaluatorUtils s_expEvalUtils;

std::istringstream& getQuotedString(std::istringstream& argStream, std::string& argument) {

	char ch;
	if (!(argStream >> ch) || ch != '"') {
		return argStream;
	}

	argument.clear();
	while (argStream.get(ch)) {
		if (ch == '\\') { // Check for escape character
			argument += ch;
			if (!argStream.get(ch)) {
				break; // Break if escape character is the last character
			}
		}
		else if (ch == '"') {
			break; // Exit loop when closing quotation mark is found
		}
		argument += ch;
	}

	return argStream;
}

void DumpInfoToLog(const std::string& info) {
	std::string sFile = GetFalloutDirectory() + "ppNVSE.log1";

	std::ifstream checkFile(sFile);
	if (!checkFile.good()) {
		std::ofstream createFile(sFile);
		if (!createFile.is_open()) {
			Console_Print("Error: Unable to create log file.");
			return;
		}
	}

	std::ofstream logFile(sFile, std::ios_base::app);

	if (logFile.is_open()) {
		logFile << info << std::endl;
		logFile.close();
	}
	else {
		Console_Print("Error: Unable to open log file.");
	}
}


bool IsGamePaused()
{
	bool isMainOrPauseMenuOpen = *(Menu**)0x11DAAC0; // g_startMenu, credits to lStewieAl
	auto* console = ConsoleManager::GetSingleton();

	return isMainOrPauseMenuOpen || console->IsConsoleOpen();
}

// Singletons
TileMenu** g_tileMenuArray = nullptr;
UInt32 g_screenWidth = 0;
UInt32 g_screenHeight = 0;
ActorValueOwner* g_playerAVOwner = nullptr;
DataHandler* g_dataHandler = nullptr;
TESObjectWEAP* g_fistsWeapon = nullptr;
TESObjectREFR* DevKitDummyMarker = nullptr;
TESObjectSTAT* g_1stPersonWeapModel = nullptr;

/****************
 * Here we include the code + definitions for our script functions,
 * which are packed in header files to avoid lengthening this file.
 * Notice that these files don't require #include statements for globals/macros like ExtractArgsEx.
 * This is because the "fn_.h" files are only used here,
 * and they are included after such globals/macros have been defined.
 ***************/

 // Shortcut macro to register a script command (assigning it an Opcode)............................................................................
#define RegisterScriptCommand(name) nvse->RegisterCommand(&kCommandInfo_ ##name); //Default return type (return a number)
#define REG_CMD(name) nvse->RegisterCommand(&kCommandInfo_##name);  //Short version of RegisterScriptCommand, from JIP.
#define REG_TYPED_CMD(name, type) nvse->RegisterTypedCommand(&kCommandInfo_##name,kRetnType_##type);  //from JG
#define REG_CMD_STR(name) nvse->RegisterTypedCommand(&kCommandInfo_##name, kRetnType_String); //From JIPLN
#define REG_CMD_ARR(name) nvse->RegisterTypedCommand(&kCommandInfo_##name, kRetnType_Array); //From JIPLN
#define REG_CMD_FORM(name) nvse->RegisterTypedCommand(&kCommandInfo_##name, kRetnType_Form);
#define REG_CMD_AMB(name) nvse->RegisterTypedCommand(&kCommandInfo_##name, kRetnType_Ambiguous);
//...................................................................................................................................................

// This is a message handler for nvse events
// With this, plugins can listen to messages such as whenever the game loads
void MessageHandler(NVSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case NVSEMessagingInterface::kMessage_PostLoad: break;
	case NVSEMessagingInterface::kMessage_ExitGame: break;
	case NVSEMessagingInterface::kMessage_ExitToMainMenu: break;
	case NVSEMessagingInterface::kMessage_LoadGame: break;
	case NVSEMessagingInterface::kMessage_SaveGame: break;
#if EDITOR
	case NVSEMessagingInterface::kMessage_ScriptEditorPrecompile: break;
#endif
	case NVSEMessagingInterface::kMessage_PreLoadGame: break;
	case NVSEMessagingInterface::kMessage_ExitGame_Console: break;
	case NVSEMessagingInterface::kMessage_PostLoadGame: break;
	case NVSEMessagingInterface::kMessage_PostPostLoad: break;
	case NVSEMessagingInterface::kMessage_RuntimeScriptError: break;
	case NVSEMessagingInterface::kMessage_DeleteGame: break;
	case NVSEMessagingInterface::kMessage_RenameGame: break;
	case NVSEMessagingInterface::kMessage_RenameNewGame: break;
	case NVSEMessagingInterface::kMessage_NewGame: break;
	case NVSEMessagingInterface::kMessage_DeleteGameName: break;
	case NVSEMessagingInterface::kMessage_RenameGameName: break;
	case NVSEMessagingInterface::kMessage_RenameNewGameName: break;
	case NVSEMessagingInterface::kMessage_DeferredInit: {

		g_interfaceManager = InterfaceManager::GetSingleton();
		g_1stPersonWeapModel = (TESObjectSTAT*)TESObjectSTAT::CreateNewForm(32, "pNVSE1stPersonDummy", 0, 0, 0);
		Hooks::CMDPatchHooks();

		ScriptConverter* converter = new ScriptConverter();
		converter->Convert();
		delete converter;

		Kit::DevkitCompiler* LoadKitFiles = new Kit::DevkitCompiler();
		delete LoadKitFiles;


	}
		break;
	case NVSEMessagingInterface::kMessage_ClearScriptDataCache: break;
	case NVSEMessagingInterface::kMessage_MainGameLoop:

		if (!IsGamePaused() && !BGSSaveLoadGame::GetSingleton()->IsLoading()) {

			if (!queueToSkipGroup.empty()) {

				for (auto it = queueToSkipGroup.begin(); it != queueToSkipGroup.end(); ) {

					if (it->wait == 0 && (it->minFrames -= 1) <= 0 && (it->timer -= g_timeGlobal->secondsPassed) <= 1e-5) {
						//Console_Print("Timer died %f on group: %d", g_timeGlobal->secondsPassed, it->groupId);
						it = queueToSkipGroup.erase(it);
					}
					else {
						++it;
					}
				}

			}

			if (!SaveSystem::queueToSpawn.empty()) {

				auto& queueToSpawn = SaveSystem::queueToSpawn;

				for (auto it = queueToSpawn.begin(); it != queueToSpawn.end(); ) {
					SaveSystem::SpawnQueue& spawn = *it;

					TESObjectREFR* placedRef = spawn.baseForm->PlaceAtCell(spawn.location, spawn.x, spawn.y, spawn.z, spawn.xR, spawn.yR, spawn.zR);
					if (placedRef) {
						if (spawn.xData) {
							placedRef->extraDataList.CopyFrom(spawn.xData, 1);
						}
						it = queueToSpawn.erase(it);
					}
					else {
						++it;
					}
				}

			}

		}

		if (obsCallLoopBoth.size()) {

			std::vector<Script*> keysToErase;

			for (auto& it : obsCallLoopBoth) {

				Script* script = it.first;
				CallLoopInfo& auxVector = it.second;

				if (auxVector.timer <= 0) {

					double result = auxVector.CallLoopFunction(it.first, auxVector.callingObj, auxVector.callingObj);

					if (result == -1) {
						keysToErase.push_back(script);
					}

					auxVector.timer = auxVector.delay;

				}
				else {

					--auxVector.timer;

				}



			}

			for (Script* key : keysToErase) {
				obsCallLoopBoth.erase(key);
			}

		}
		
		break;

	case NVSEMessagingInterface::kMessage_ScriptCompile: break;
	default: break;
	}
}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	_MESSAGE("query");

	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "ppNVSE";
	info->version = g_PluginVersion;

	return true;
}

bool NVSEPlugin_Load(NVSEInterface* nvse)
{

	_MESSAGE("load");

	g_pluginHandle = nvse->GetPluginHandle();

	// save the NVSE interface in case we need it later
	g_nvseInterface = nvse;

	// register to receive messages from NVSE
	g_messagingInterface = static_cast<NVSEMessagingInterface*>(nvse->QueryInterface(kInterface_Messaging));
	g_messagingInterface->RegisterListener(g_pluginHandle, "NVSE", MessageHandler);

	if (nvse->isEditor)
	{
		_MESSAGE("ppNVSE Loaded successfully (Editor).\nppNVSE Plug Version: %u\n", g_PluginVersion);
	}
	else
	{
		_MESSAGE("ppNVSE Loaded successfully (In-Game).\nppNVSE Plug Version: %u\n", g_PluginVersion);
	}

	// register to receive messages from NVSE
	((NVSEMessagingInterface*)nvse->QueryInterface(kInterface_Messaging))->RegisterListener(nvse->GetPluginHandle(), "NVSE", MessageHandler);

	if (!nvse->isEditor)
	{
		//PluginHandle const nvsePluginHandle = nvse->GetPluginHandle();  //from JiPLN

		g_scriptInterface = (NVSEScriptInterface*)nvse->QueryInterface(kInterface_Script);
		ExtractArgsEx = g_scriptInterface->ExtractArgsEx;
		ExtractFormatStringArgs = g_scriptInterface->ExtractFormatStringArgs;
		FunctionCallScript = g_scriptInterface->CallFunction;
		FunctionCallScriptAlt = g_scriptInterface->CallFunctionAlt;
		pCompileScript = g_scriptInterface->CompileScript;

		g_cmdTableInterface = *(NVSECommandTableInterface*)nvse->QueryInterface(kInterface_CommandTable);
		GetCmdByName = g_cmdTableInterface.GetByName;

		g_strInterface = *(NVSEStringVarInterface*)nvse->QueryInterface(kInterface_StringVar);
		GetStringVar = g_strInterface.GetString;
		AssignString = g_strInterface.Assign;

		g_arrInterface = (NVSEArrayVarInterface*)nvse->QueryInterface(kInterface_ArrayVar);
		CreateArray = g_arrInterface->CreateArray;
		CreateMap = g_arrInterface->CreateMap;
		CreateStringMap = g_arrInterface->CreateStringMap;
		AssignArrayResult = g_arrInterface->AssignCommandResult;
		SetElement = g_arrInterface->SetElement;
		AppendElement = g_arrInterface->AppendElement;
		GetArraySize = g_arrInterface->GetArraySize;
		LookupArrayByID = g_arrInterface->LookupArrayByID;
		GetElement = g_arrInterface->GetElement;
		GetArrayElements = g_arrInterface->GetElements;

		nvse->InitExpressionEvaluatorUtils(&s_expEvalUtils);

		g_eventInterface = (NVSEEventManagerInterface*)nvse->QueryInterface(kInterface_EventManager);

		#if WantLambdaFunctions
				LambdaDeleteAllForScript = (_LambdaDeleteAllForScript)nvseData->GetFunc(NVSEDataInterface::kNVSEData_LambdaDeleteAllForScript);
				LambdaSaveVariableList = (_LambdaSaveVariableList)nvseData->GetFunc(NVSEDataInterface::kNVSEData_LambdaSaveVariableList);
				LambdaUnsaveVariableList = (_LambdaUnsaveVariableList)nvseData->GetFunc(NVSEDataInterface::kNVSEData_LambdaUnsaveVariableList);
				IsScriptLambda = (_IsScriptLambda)nvseData->GetFunc(NVSEDataInterface::kNVSEData_IsScriptLambda);
		#endif

		#if WantScriptFunctions
				HasScriptCommand = (_HasScriptCommand)nvseData->GetFunc(NVSEDataInterface::kNVSEData_HasScriptCommand);
				DecompileScript = (_DecompileScript)nvseData->GetFunc(NVSEDataInterface::kNVSEData_DecompileScript);
		#endif

		PluginHandle pluginHandle = nvse->GetPluginHandle();
		
		SaveSystem::SaveWeaponInst(nvse, pluginHandle);
		Hooks::initHooks();
		//kNVSE::initkNVSE();
		PluginFunctions::initJIP();

		InventoryRef::InitInventoryRefFunct(nvse);
		DevKitDummyMarker = TESObjectREFR::Create(1);
		//g_1stPersonWeapModel = (TESObjectSTAT *)TESObjectREFR::Create(1);

	}

	//	See https://geckwiki.com/index.php?title=NVSE_Opcode_Base

	UInt32 const ppNVSEPluginOpcodeBase = 0x3E1C;

	// register commands
	nvse->SetOpcodeBase(ppNVSEPluginOpcodeBase);

	REG_CMD_ARR(pRotateAroundObject)
	REG_CMD_ARR(pRotateAroundObjectLocally)
	REG_CMD_ARR(pReadFile)
	REG_CMD(pWriteFile)
	REG_CMD(pSaveNif)

	//REG_CMD(IsWeapInst)
	//REG_CMD_FORM(GetWeaponBase)
	REG_CMD(iModAlt)
	REG_CMD(GetWeaponModFlags)
	REG_CMD(SetWeaponModFlags)

	REG_CMD(MoveFileTo)
	REG_CMD_FORM(GetTopicSpeaker)
	REG_CMD_STR(GetTopicPrompt)

	//Weapon Modifiers

	REG_CMD(MarkAsStaticForm)
	REG_CMD_FORM(CreateFormInstance)

	REG_CMD(SetWeaponMod)
	REG_CMD_ARR(GetAllAttachedWeaponMods)
	REG_CMD_FORM(GetWeaponMod)

	REG_CMD(IsFormInstance)
	REG_CMD_FORM(GetStaticForm)

	REG_CMD(GetFormInstanceID)
	REG_CMD(IsStaticForm)
	REG_CMD_ARR(GetAllFormInstances)

	REG_CMD(ReplaceItemInInventory)

	REG_CMD(SetOnInstanceReconstruct)
	REG_CMD(SetOnAttachWeaponMod)
	REG_CMD(SetOnDetachWeaponMod)
		
	REG_CMD_STR(GetFormInstanceKey)

	REG_CMD_FORM(GetFormInstance)

	REG_CMD(SetOnInstanceDeconstruct)

	REG_CMD(SetFormTrait)
	REG_CMD_AMB(GetFormTrait)

	REG_CMD(GetFormTraitListSize)

	REG_CMD_ARR(GetAllWeaponMods)

	REG_CMD(GetFormTraitType)

	REG_CMD(HasExtendedWeaponMods)

	REG_CMD(CallLoop)

	REG_CMD(SetOnEquipAlt)

	REG_CMD_FORM(GetBaseInstance)

	REG_CMD(DeleteFormInstance)
		
	return true;
}

namespace StringUtils {

	std::string toLowerCase(const std::string& str) {
		std::string lowerStr;
		lowerStr.reserve(str.size());  // Reserve memory upfront to avoid reallocations
		std::transform(str.begin(), str.end(), std::back_inserter(lowerStr), [](unsigned char c) { return std::tolower(c); });
		return lowerStr;
	}

	char* toLowerCase(const char* str) {
		if (!str) return nullptr;
		size_t length = std::strlen(str);
		char* lowerStr = new char[length + 1];
		std::transform(str, str + length, lowerStr, [](unsigned char c) { return std::tolower(c); });
		lowerStr[length] = '\0';
		return lowerStr;
	}

	constexpr UInt32 ToUInt32(const char* str) {
		if (!str) throw std::invalid_argument("Null string");

		UInt32 result = 0;
		bool is_valid = true;
		const char* p = str;

		while (*p) {
			if (*p < '0' || *p > '9') {
				is_valid = false;
				break;
			}
			result = result * 10 + (*p - '0');
			if (result < 0) {
				throw std::overflow_error("Integer overflow");
			}
			++p;
		}

		if (!is_valid) {
			throw std::invalid_argument("Invalid integer string");
		}

		return result;
	}

}

