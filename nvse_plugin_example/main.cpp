#pragma once
#include "nvse/PluginAPI.h"
#include "nvse/CommandTable.h"
#include "nvse/GameAPI.h"
#include "nvse/ParamInfos.h"
#include "nvse/ParamInfosNVSE.h"
#include "nvse/GameObjects.h"
#include "nvse/Hooks_DirectInput8Create.h"
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

IDebugLog		gLog("pNVSE.log");
constexpr UInt32 g_PluginVersion = 1;

PluginHandle	g_pluginHandle = kPluginHandle_Invalid;

NVSEMessagingInterface* g_messagingInterface{};
NVSEInterface* g_nvseInterface{};
NVSECommandTableInterface g_cmdTableInterface;

NiCamera* g_mainCamera = nullptr;

//UI 11-15-2023
InterfaceManager* g_interfaceManager;
DIHookControl* g_keyInterface = nullptr;

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

bool getQuotedString(std::istringstream& argStream, std::string& argument) {

	argStream >> std::ws; //Skip white space

	char ch;
	if (argStream.peek() != '\"') {
		return false;
	}

	argStream.get(); // Consume the quotation mark
	argument.clear();
	while (argStream.get(ch)) {
		if (ch == '\"') {
			return true;
		}
		argument += ch;
	}

	return false;
}

Script* CompileScriptAlt(Script* script)
{
	const auto buffer = MakeUnique<ScriptBuffer, 0x5AE490, 0x5AE5C0>();

	buffer->scriptName.Set(script->GetEditorID());
	buffer->scriptText = script->text;
	*buffer->scriptData = 0x1D;
	buffer->dataOffset = 4;

	buffer->partialScript = (script->flags & 1) != 0;
	buffer->runtimeMode = ScriptBuffer::kEditor;
	buffer->currentScript = script;

	script->info.varCount = 0;
	script->info.numRefs = 0;
	script->varList.DeleteAll();
	script->refList.DeleteAll();

	//buffer->info.numRefs = script->info.numRefs;
	//buffer->info.varCount = script->info.varCount;

	const auto result = script->Compile(buffer.get());
	buffer->scriptText = nullptr;
	script->text = nullptr;
	if (!result)
		return nullptr;
	if (script->quest) {

		for (auto it = script->varList.begin(); it != script->varList.end(); ++it) {
			VariableInfo* var = *it;
			TESQuest::LocalVariableOrObjectivePtr* lvo = static_cast<TESQuest::LocalVariableOrObjectivePtr*>(FormHeap_Allocate(sizeof(TESQuest::LocalVariableOrObjectivePtr)));
			lvo->varInfoIndex = var;
			script->quest->lVarOrObjectives.Append(lvo);
		}

		ScriptEventList* eventList = script->CreateEventList();
		script->quest->scriptEventList = eventList;

	}

	return script;

}

void DumpInfoToLog(const std::string& info) {
	std::string sFile = GetFalloutDirectory() + "pNVSE.log1";

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
		Hooks::CMDPatchHooks();
		PluginFunctions::initJIP();
		SceneGraph* sing_SceneGraph = *(SceneGraph**)0x11DEB7C;
		g_mainCamera = sing_SceneGraph->camera;


		if (std::filesystem::exists(GetFalloutDirectory() + "Data\\ScriptConverter") && std::filesystem::is_directory(GetFalloutDirectory() + "Data\\ScriptConverter")) {
			ScriptConverter* converter = ScriptConverter::Create();
			converter->Convert();
			delete converter;
		}

		if (std::filesystem::exists(GetFalloutDirectory() + "Data\\Devkit") && std::filesystem::is_directory(GetFalloutDirectory() + "Data\\Devkit")) {

			#ifdef _DEBUG
				auto createStart = std::chrono::high_resolution_clock::now();
			#endif

			Kit::DevkitCompiler* LoadKitFiles = new Kit::DevkitCompiler();

			#ifdef _DEBUG
				auto createEnd = std::chrono::high_resolution_clock::now(); // End timing creation
			#endif

			delete LoadKitFiles;

			#ifdef _DEBUG
				auto deleteEnd = std::chrono::high_resolution_clock::now(); // End timing after delete

				std::chrono::duration<double, std::milli> creationTime = createEnd - createStart;
				double time = creationTime.count();

				std::chrono::duration<double, std::milli> totalTime = deleteEnd - createStart;
				time = totalTime.count();

			#endif

		}


	}
		break;
	case NVSEMessagingInterface::kMessage_ClearScriptDataCache: break;
	case NVSEMessagingInterface::kMessage_MainGameLoop:

		if (!IsGamePaused() && !BGSSaveLoadGame::GetSingleton()->IsLoading()) {

			//For skipping anim groups.
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

			//When references with Timer Policies are moved. Generally depreciated code.
			if (!newlyCreatedReferences.empty()) {

				for (auto it = newlyCreatedReferences.begin(); it != newlyCreatedReferences.end(); ++it) {

					TESObjectREFR* form = *it;
					if (Instance* inst = form->baseForm->pLookupInstance()) {

						if (inst->lifecycle.isPolicyEnabled(LifecycleManager::Timed)) {
							float time = inst->lifecycle.getLifeTime();
							LifecycleManager::addLifecycleTimer(form, form->baseForm, time, &form->extraDataList);
						}

					}
					else if (ExtendedBaseType* staticForm = form->baseForm->LookupStaticInstance()) {

						if (staticForm->baseLifecycle.isPolicyEnabled(LifecycleManager::Timed)) {
							float time = staticForm->baseLifecycle.getLifeTime();
							LifecycleManager::addLifecycleTimer(form, form->baseForm, time, &form->extraDataList);
						}

					}

				}

				newlyCreatedReferences.clear();

			}

			//Tick objects with lifecycle timers.
			if (!lifecycleTimer.empty()) {

				for (auto it = lifecycleTimer.begin(); it != lifecycleTimer.end();) {
					ExpirationTimer* expiration = *it;

					// Check for invalid references and baseform mismatches
					if (!expiration->ref || expiration->ref->baseForm != expiration->baseform) {
						it = lifecycleTimer.erase(it);
						continue;
					}

					if (expiration->type == kXData_ExtraContainerChanges) {

						ExtraContainerChanges* xData = static_cast<ExtraContainerChanges*>(expiration->xDataList->GetByType(expiration->type));
						ContChangesEntry* entry = expiration->ref->GetContainerChangesEntry(expiration->baseform);

						UInt32 count = 0;
						for (auto j = entry->extendData->begin(); j != entry->extendData->end();) {
							++count;
							ExtraDataList* xDataList = j.Get();
							ExtraTimeLeft* xDataTimer = static_cast<ExtraTimeLeft*>(xDataList->GetByType(kExtraData_TimeLeft));

							if (!xDataTimer) {
								--count;
								++j;
								continue;
							}

							if ((xDataTimer->time -= g_timeGlobal->secondsPassed) <= 1e-5) {
								--count;
								expiration->ref->RemoveItem(expiration->baseform, xDataList, xDataList->GetCount(), false, false, nullptr, 0, 0, true, false);
							}
							else {
								++j;
							}
						}

						if (count == 0) {
							it = lifecycleTimer.erase(it);
						}
						else {
							++it;
						}

					}
					else if (expiration->type == kExtraData_TimeLeft) {

						ExtraTimeLeft* xDataTimer = static_cast<ExtraTimeLeft*>(expiration->xDataList->GetByType(expiration->type));

						if (!xDataTimer) {
							it = lifecycleTimer.erase(it);
							continue;
						}

						if ((xDataTimer->time -= g_timeGlobal->secondsPassed) <= 1e-5) {

							if (expiration->ref) {
								expiration->ref->DeleteReference();
							}
							it = lifecycleTimer.erase(it);

						}
						else {
							++it;
						}

					}

				}

			}
			SaveSystem::ExecuteSpawnQueue();
			SaveSystem::ExecuteUpdate3dQueue();

			//AnimLockManager::UpdateAllAnims();

		}
	
		if (!obsCallLoopBoth.infos.empty()) {
			
			std::vector<CallLoopInfo> infosToErase;

			for (auto& info : obsCallLoopBoth.infos) {

				if (info.timer <= 0) {
					double result = info.CallLoopFunction();
					if (result == -1) { //If script returns -1, cancel.
						infosToErase.push_back(info);
					}
					else if (result >= 0) {
						info.delay = result;
					}
					info.timer = info.delay;
				}
				else {
					--info.timer;
				}
			}

			for (const auto& info : infosToErase) {
				obsCallLoopBoth.infos.erase(info); // Erase based on the exact info object
			}

			/* Vector based callback
			if (!obsCallLoopBoth.infos.empty()) {
				for (auto it = obsCallLoopBoth.infos.begin(); it != obsCallLoopBoth.infos.end(); ) {
					if (it->timer <= 0) {
						double result = it->CallLoopFunction();
						if (result == -1) {
							it = obsCallLoopBoth.infos.erase(it);
							continue;
						}
						else {
							it->delay = result;
							it->timer = result;
						}
					}
					else {
						--it->timer;
					}
					++it;
				}
			}
			*/

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
	info->name = "pNVSE";
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
		_MESSAGE("pNVSE Loaded successfully (Editor).\npNVSE Plug Version: %u\n", g_PluginVersion);
	}
	else
	{
		_MESSAGE("pNVSE Loaded successfully (In-Game).\npNVSE Plug Version: %u\n", g_PluginVersion);
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

		g_keyInterface = (DIHookControl*)nvse->QueryInterface(NVSEDataInterface::kNVSEData_DIHookControl);

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
;
		PluginHandle pluginHandle = nvse->GetPluginHandle();
		
		SaveSystem::SaveWeaponInst(nvse, pluginHandle);
		Hooks::initHooks();
		PluginFunctions::init_kNVSE();

		InventoryRef::InitInventoryRefFunct(nvse);
		DevKitDummyMarker = TESObjectREFR::Create(1);
		//g_1stPersonWeapModel = (TESObjectSTAT *)TESObjectREFR::Create(1);

	}

	//	See https://geckwiki.com/index.php?title=NVSE_Opcode_Base

	UInt32 const pNVSEPluginOpcodeBase = 0x3E1C;

	// register commands
	nvse->SetOpcodeBase(pNVSEPluginOpcodeBase);

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
	REG_CMD_ARR(GetAllBaseInstances)

	REG_CMD(ReplaceItemInInventory)

	REG_CMD(SetOnInstanceReconstruct)
	REG_CMD(SetOnAttachWeaponMod)
	REG_CMD(SetOnDetachWeaponMod)
		
	REG_CMD_STR(GetFormInstanceKey)

	REG_CMD_FORM(GetFormInstance)

	REG_CMD(SetOnInstanceDeconstruct)

	REG_CMD(SetBaseTrait)
	REG_CMD_AMB(GetBaseTrait)

	REG_CMD(GetBaseTraitListSize)

	REG_CMD_ARR(GetAllWeaponMods)

	REG_CMD(GetBaseTraitType)

	REG_CMD(HasExtendedWeaponMods)

	REG_CMD(CallLoop)

	REG_CMD(SetOnEquipAlt)

	REG_CMD_FORM(GetBaseInstance)

	REG_CMD(DeleteFormInstance)

	REG_CMD_FORM(CreateAkimboInstance)
	REG_CMD(IsAkimboForm)
	REG_CMD_FORM(GetAkimboWeapons)

	REG_CMD_FORM(GetWeaponAmmoType)
	REG_CMD(GetWeaponAmmoCount)

	REG_CMD(SetWeaponAmmoCount)
	REG_CMD(SetWeaponAmmoType)
	REG_CMD(LoadWeaponWithAmmo)

	REG_CMD_FORM(GetStaticAkimbo)
	REG_CMD(HasAkimboSet)

	REG_CMD_FORM(FindWeaponAmmo)
	REG_CMD_ARR(GetInventoryFromList)

	REG_CMD(CallLoopAlt)

	REG_CMD(GetWeaponClipRoundsAlt)

	REG_CMD_ARR(GetObjectInstances)

	REG_CMD(GetPlayerCameraRotation)

	REG_CMD(OverrideAnimatedNodePriority)
	REG_CMD(RemoveAnimatedNodePriority)
	REG_CMD(HasAnimatedNodeOverride)

	REG_CMD(onAnimationStart)
	REG_CMD(IsThirdPerson)

	REG_CMD(pRenameFile)

	REG_CMD(GetAkimboWeaponsAlt)
		
		

	//REG_CMD(SetSpeedMultAlt)
		
		
	return true;
}

namespace StringUtils {

	std::string extractFirstLine(const char* data, size_t length) {
		size_t firstLineEnd = std::find(data, data + length, '\n') - data;
		return std::string(data, firstLineEnd);
	}

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
	/*
	#include <locale>
	char* toLowerCase(const char* str) {
		if (!str) return nullptr;
		size_t length = std::strlen(str);
		char* lowerStr = new char[length + 1];  // Allocate space for the new string + null terminator
		for (size_t i = 0; i < length; ++i) {
			lowerStr[i] = std::tolower(static_cast<unsigned char>(str[i]), std::locale::classic());
		}
		lowerStr[length] = '\0';
		return lowerStr;
	}
	*/
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

	bool isNumber(const std::string& s) {
		if (s.empty()) return false;
		std::size_t start = (s[0] == '-' || s[0] == '+') ? 1 : 0;
		if (start == 1 && s.size() == 1) return false;
		return std::all_of(s.begin() + start, s.end(), [](unsigned char c) {
			return std::isdigit(c);
			});
	}
	/*
	bool isNumber(const std::string& s) {
		return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
	}*/

}

