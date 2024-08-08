#pragma once
#include "WeaponSmith.h"

//Instance function hooks

bool Hook_GetBaseObject_Execute(COMMAND_ARGS);

bool Cmd_ListGetFormIndex_Eval(COMMAND_ARGS_EVAL);
bool Hook_IsInList_Execute(COMMAND_ARGS);
bool Hook_IsWeaponInList_Execute(COMMAND_ARGS);

//Kit files....................................................................................

//JIP hooks........................................................................

bool Hook_GetSelfModIndex_Execute(COMMAND_ARGS);
bool Hook_GetFormMods_Execute(COMMAND_ARGS);
bool Hook_GetModName_Execute(COMMAND_ARGS);

//NVSE hooks.........................................................................

bool Hook_GetNthModName_Execute(COMMAND_ARGS);
bool Hook_GetModIndex_Execute(COMMAND_ARGS);
bool Hook_GetSourceModIndex_Execute(COMMAND_ARGS);
bool Hook_IsModLoaded_Execute(COMMAND_ARGS);

bool Hook_GetNumLoadedMods_Execute(COMMAND_ARGS);

//....................................................................................

namespace Hooks
{
	/*
	void HookIsInList() {
		UInt32 vtableAddress = 0x01194334; // example address, adjust to actual
		UInt32 offsetForIsInList = 4; // Offset from the start of the vtable to the `IsInList` pointer
		UInt32 absoluteAddressForIsInList = vtableAddress + offsetForIsInList;

		// Address of your custom function
		UInt32 myFunctionAddress = (UInt32)&Hook_IsInList_Execute;

		// Replace the vtable entry
		SafeWrite32(absoluteAddressForIsInList, myFunctionAddress);
	}
	*/

	void DumpFunctionOpcode(const char* functionName) {
		const CommandInfo* cmdInfo = g_cmdTableInterface.GetByName(functionName);
		if (cmdInfo) {
			std::string output = std::string(functionName) + ": Opcode " + std::to_string(cmdInfo->opcode);
			gLog.Message(output.c_str());
		}
		else {
			std::string errorOutput = "Error, function not found: " + std::string(functionName);
			gLog.Message(errorOutput.c_str());
		}
	}

	void CMDPatchHooks()
	{

		CommandInfo* cmdInfo = g_cmdTableInterface.GetByOpcode(0x1403);
		cmdInfo->execute = Hook_GetBaseObject_Execute;

		//DumpFunctionOpcode("IsInList");
		cmdInfo = g_cmdTableInterface.GetByOpcode(4468);
		cmdInfo->execute = Hook_IsInList_Execute;

		//DumpFunctionOpcode("ListGetFormIndex");
		//cmdInfo = g_cmdTableInterface.GetByOpcode(5185);
		//cmdInfo->execute = Hook_ListGetFormIndex_Execute;
		//cmdInfo->eval = Hook_ListGetFormIndex_Eval;

		if (PluginFunctions::JIP) {

			//DumpFunctionOpcode("GetSelfModIndex");
			cmdInfo = g_cmdTableInterface.GetByOpcode(8733);
			cmdInfo->execute = Hook_GetSelfModIndex_Execute;

			//DumpFunctionOpcode("GetFormMods");
			cmdInfo = g_cmdTableInterface.GetByOpcode(8761);
			cmdInfo->execute = Hook_GetFormMods_Execute;

			//DumpFunctionOpcode("GetModName");
			cmdInfo = g_cmdTableInterface.GetByOpcode(8730);
			cmdInfo->execute = Hook_GetModName_Execute;

		}
		//DumpFunctionOpcode("IsWeaponInList");
		cmdInfo = g_cmdTableInterface.GetByOpcode(4495);
		cmdInfo->execute = Hook_IsWeaponInList_Execute;

		//DumpFunctionOpcode("GetNthModName");
		cmdInfo = g_cmdTableInterface.GetByOpcode(5510);
		cmdInfo->execute = Hook_GetNthModName_Execute;

		//DumpFunctionOpcode("GetModIndex");
		cmdInfo = g_cmdTableInterface.GetByOpcode(5295);
		cmdInfo->execute = Hook_GetModIndex_Execute;

		//DumpFunctionOpcode("GetSourceModIndex");
		cmdInfo = g_cmdTableInterface.GetByOpcode(5297);
		cmdInfo->execute = Hook_GetSourceModIndex_Execute;

		//DumpFunctionOpcode("IsModLoaded");
		cmdInfo = g_cmdTableInterface.GetByOpcode(5294);
		cmdInfo->execute = Hook_IsModLoaded_Execute;

		//DumpFunctionOpcode("GetNumLoadedMods");
		cmdInfo = g_cmdTableInterface.GetByOpcode(5296);
		cmdInfo->execute = Hook_GetNumLoadedMods_Execute;

	}

}