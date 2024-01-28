#include "nvse/PluginAPI.h"
#include "nvse/CommandTable.h"
#include "nvse/GameAPI.h"
#include "nvse/ParamInfos.h"
#include "nvse/GameObjects.h"
#include <string>
#include "ppNVSE.h"
#include "GameUI.h"

#include "ppNVSE_functions.h"
#include "WeapInstFunct.h"
//NoGore is unsupported in xNVSE

IDebugLog		gLog("ppNVSE.log");
constexpr UInt32 g_PluginVersion = 1;

PluginHandle	g_pluginHandle = kPluginHandle_Invalid;

NVSEMessagingInterface* g_messagingInterface{};
NVSEInterface* g_nvseInterface{};
NVSECommandTableInterface* g_cmdTableInterface{};

//UI 11-15-2023
UInt32 s_UpdateCursor = 0;
InterfaceManager* g_interfaceManager;
NiNode* s_pc1stPersonNode = nullptr, * g_cursorNode;

// RUNTIME = Is not being compiled as a GECK plugin.

NVSEScriptInterface* g_script{};
NVSEStringVarInterface* g_stringInterface{};

NVSEDataInterface* g_dataInterface{};
NVSESerializationInterface* g_serializationInterface{};
NVSEConsoleInterface* g_consoleInterface{};
NVSEEventManagerInterface* g_eventInterface{};

#define WantInventoryRefFunctions 1 // set to 1 if you want these PluginAPI functions
#if WantInventoryRefFunctions
_InventoryReferenceCreate InventoryReferenceCreate{};
_InventoryReferenceGetForRefID InventoryReferenceGetForRefID{};
_InventoryReferenceGetRefBySelf InventoryReferenceGetRefBySelf{};
_InventoryReferenceCreateEntry InventoryReferenceCreateEntry{};
#endif

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

NVSEStringVarInterface* g_strInterface = nullptr;
bool (*AssignString)(COMMAND_ARGS, const char* newValue);
const char* (*GetStringVar)(UInt32 stringID);
NVSEMessagingInterface* g_msg = nullptr;
NVSEScriptInterface* g_scriptInterface = nullptr;
NVSECommandTableInterface* g_commandInterface = nullptr;
const CommandInfo* (*GetCmdByName)(const char* name);
bool (*FunctionCallScript)(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container, NVSEArrayElement* result, UInt8 numArgs, ...);
bool (*FunctionCallScriptAlt)(Script* funcScript, TESObjectREFR* callingObj, UInt8 numArgs, ...);
TESObjectREFR* (__stdcall* InventoryRefCreateEntry)(TESObjectREFR* container, TESForm* itemForm, SInt32 countDelta, ExtraDataList* xData);
ExpressionEvaluatorUtils g_expEvalUtils;

// Singletons
TileMenu** g_tileMenuArray = nullptr;
UInt32 g_screenWidth = 0;
UInt32 g_screenHeight = 0;
ActorValueOwner* g_playerAVOwner = nullptr;
DataHandler* g_dataHandler = nullptr;
TESObjectWEAP* g_fistsWeapon = nullptr;

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
	case NVSEMessagingInterface::kMessage_DeferredInit: break;
	case NVSEMessagingInterface::kMessage_ClearScriptDataCache: break;
	case NVSEMessagingInterface::kMessage_MainGameLoop: break;
		/*
		if (s_UpdateCursor) {
			if (g_interfaceManager->hasMouseMoved)
			{
				POINT p;
				GetCursorPos(&p);
				g_interfaceManager->cursor->node->Update();
				Console_Print("Running Update");
				g_interfaceManager->cursorX = p.x;
				g_interfaceManager->cursorY = p.y;
				float converter = *(float*)0x11D8A48;
				g_cursorNode->LocalTranslate().x = ((p.x) * converter) - g_screenWidth;
				g_cursorNode->LocalTranslate().z = g_screenHeight - ((p.y) * converter);
			}
		}
		*/
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
		PluginHandle const nvsePluginHandle = nvse->GetPluginHandle();  //from JiPLN

		auto const nvseData = (NVSEDataInterface*)nvse->QueryInterface(kInterface_Data);

		// From JiPLN (jip_nvse.cpp) 
		auto const serialization = (NVSESerializationInterface*)nvse->QueryInterface(kInterface_Serialization);

		g_scriptInterface = (NVSEScriptInterface*)nvse->QueryInterface(kInterface_Script);
		ExtractArgsEx = g_scriptInterface->ExtractArgsEx;
		ExtractFormatStringArgs = g_scriptInterface->ExtractFormatStringArgs;
		FunctionCallScript = g_scriptInterface->CallFunction;
		FunctionCallScriptAlt = g_scriptInterface->CallFunctionAlt;

		g_commandInterface = (NVSECommandTableInterface*)nvse->QueryInterface(kInterface_CommandTable);
		GetCmdByName = g_commandInterface->GetByName;

		g_strInterface = (NVSEStringVarInterface*)nvse->QueryInterface(kInterface_StringVar);
		GetStringVar = g_strInterface->GetString;
		AssignString = g_strInterface->Assign;

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

		nvse->InitExpressionEvaluatorUtils(&g_expEvalUtils);

		g_eventInterface = (NVSEEventManagerInterface*)nvse->QueryInterface(kInterface_EventManager);

		#if WantInventoryRefFunctions
				InventoryReferenceCreate = (_InventoryReferenceCreate)nvseData->GetFunc(NVSEDataInterface::kNVSEData_InventoryReferenceCreate);
				InventoryReferenceGetForRefID = (_InventoryReferenceGetForRefID)nvseData->GetFunc(NVSEDataInterface::kNVSEData_InventoryReferenceGetForRefID);
				InventoryReferenceGetRefBySelf = (_InventoryReferenceGetRefBySelf)nvseData->GetFunc(NVSEDataInterface::kNVSEData_InventoryReferenceGetRefBySelf);
				InventoryReferenceCreateEntry = (_InventoryReferenceCreateEntry)nvseData->GetFunc(NVSEDataInterface::kNVSEData_InventoryReferenceCreateEntry);
		#endif

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
	REG_CMD(NewWeapInst)
	REG_CMD(IsWeapInst)
	REG_CMD_FORM(GetWeaponBase)
	REG_CMD(iModAlt)
	REG_CMD(GetWeaponModFlags)
	REG_CMD(SetWeaponModFlags)

	REG_CMD(MoveFileTo)
	REG_CMD_FORM(GetTopicSpeaker)

	return true;
}
