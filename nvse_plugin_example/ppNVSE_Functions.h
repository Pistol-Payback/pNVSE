#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "StringVar.h"

#define PI 3.14159265

static EquipData FindEquipped(TESObjectREFR* thisObj, FormMatcher& matcher) {
	ExtraContainerChanges* pContainerChanges = static_cast<ExtraContainerChanges*>(thisObj->extraDataList.GetByType(kExtraData_ContainerChanges));
	return (pContainerChanges) ? pContainerChanges->FindEquipped(matcher) : EquipData();
}

bool Cmd_GetPlayerCameraRotation_Execute(COMMAND_ARGS)
{
	double outX;
	double outY;
	double outZ;

	PlayerCharacter* player = PlayerCharacter::GetSingleton();
	if (ExtractArgsEx(EXTRACT_ARGS_EX, &outX, &outY, &outZ) && player->parentCell){

		//NiMatrix33* angles = &g_mainCamera->WorldRotate();
		//angles->data;
		//outX = angles->data[0];
		//outY = angles->data[1];
		//outZ = angles->data[2];

	}

	return true;
}

bool Cmd_DisableKey_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32	keycode = 0;
	UInt32	mask = 0;

	if (ExtractArgs(EXTRACT_ARGS, &keycode, &mask))
		g_keyInterface->SetKeyDisableState(keycode, true, mask);

	return true;
}

bool Cmd_EnableKey_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32	keycode = 0;
	UInt32	mask = 0;

	if (ExtractArgs(EXTRACT_ARGS, &keycode, &mask))
		g_keyInterface->SetKeyDisableState(keycode, false, mask);

	return true;
}

//std::unordered_map<Script*, AuxVector> obsCallLoopMenu;
//std::unordered_map<Script*, AuxVector> obsCallLoopGame;
extern CallLoopHandler obsCallLoopBoth;
CallLoopHandler obsCallLoopBoth;

DEFINE_COMMAND_PLUGIN_EXP(CallLoop, "Like CallbackLoop", true, kNVSEParams_CallLoop);
bool Cmd_CallLoop_Execute(COMMAND_ARGS)
{
	*result = 0;
	Script* script = NULL;
	double delay = 0;
	TESForm* valueRef = NULL;
	NVSEArrayVarInterface::Array* valueArr = NULL;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs()) {

		delay = eval.GetNthArg(0)->GetFloat();
		script = static_cast<Script*>(eval.GetNthArg(1)->GetTESForm());

		if (!script) {
			return true;
		}

		auto const numArgs = eval.NumArgs();

		AuxVector arguments;
		arguments.reserve(numArgs - 2);

		for (UInt32 i = 2; i < numArgs; i++) {
			PluginScriptToken* arg = eval.GetNthArg(i);
			switch (arg->GetType()) {
			case kTokenType_Number:
			case kTokenType_NumericVar:
				arguments.AddValue<Float32>(-1, arg->GetFloat()); //Convert to Float32 to later call bitcast.
				break;
			case kTokenType_Form:
			case kTokenType_RefVar:
				valueRef = arg->GetTESForm();  // Just assign here
				if (valueRef) {
					arguments.AddValue<UInt32>(-1, valueRef->refID);
				}
				break;
			case kTokenType_String:
			case kTokenType_StringVar:
				arguments.AddValue<const char*>(-1, arg->GetString());
				break;
			case kTokenType_Array:
			case kTokenType_ArrayVar:
				valueArr = arg->GetArrayVar();
				if (valueArr) {
					arguments.AddValue<NVSEArrayVarInterface::Array*>(-1, valueArr);
				}
				break;
			}
		}

		if (delay >= 0) {
			obsCallLoopBoth.infos.emplace(CallLoopInfo{ delay, script, thisObj->refID, std::move(arguments) });
		}else{
			obsCallLoopBoth.removeInfo(CallLoopInfo{ delay, script, thisObj->refID, std::move(arguments) }); //Find matching info and remove it
		}

	}
	return true;
}

DEFINE_COMMAND_PLUGIN_EXP(CallLoopAlt, "Like CallbackLoop", true, kNVSEParams_CallLoop);
bool Cmd_CallLoopAlt_Execute(COMMAND_ARGS)
{

	*result = 0;
	Script* script = NULL;
	double delay = 0;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{

		delay = eval.GetNthArg(0)->GetFloat();
		script = (Script*)eval.GetNthArg(1)->GetTESForm();

		if (!script) {
			return true;
		}

		auto const numArgs = eval.NumArgs();

		std::vector<UInt32> arguments;
		arguments.reserve(numArgs - 2);

		if (numArgs >= 2)
		{

			for (UInt32 i = 2; i < numArgs; i++)
			{
				PluginScriptToken* arg = eval.GetNthArg(i);
				switch (arg->GetType()) {
				case kTokenType_Number:
				case kTokenType_NumericVar:
				{
					Float32 valueFlt = arg->GetFloat();
					arguments.push_back(*reinterpret_cast<UInt32*>(&valueFlt));
					break;
				}
				case kTokenType_Form:
				case kTokenType_RefVar:
				{
					TESObjectREFR* valueRef = (TESObjectREFR*)arg->GetTESForm();
					if (valueRef) {
						arguments.push_back(*reinterpret_cast<UInt32*>(&valueRef));
					}
					break;
				}
				case kTokenType_String:
				case kTokenType_StringVar:
				{
					const char* valueStr = arg->GetString();
					if (valueStr) {
						arguments.push_back(*reinterpret_cast<UInt32*>(&valueStr));
					}
					break;
				}
				case kTokenType_Array:
				case kTokenType_ArrayVar:
				{
					NVSEArrayVarInterface::Array* valueArr = arg->GetArrayVar();
					if (valueArr) {
						arguments.push_back(*reinterpret_cast<UInt32*>(&valueArr));
					}
					break;
				}
				}
			}

		}

		if (delay >= 0) {
			obsCallLoopBoth.infos.emplace(CallLoopInfo{ delay, script, thisObj->refID, std::move(arguments) });
		}
		else {
			obsCallLoopBoth.removeInfo(CallLoopInfo{ delay, script, thisObj->refID, std::move(arguments) }); //Find matching info and remove it
		}

	}

	return true;

}

DEFINE_COMMAND_PLUGIN(SetSpeedMultAlt, "SetsSpeedMult", true, kParams_OneInt);
bool Cmd_SetSpeedMultAlt_Execute(COMMAND_ARGS)
{
	UInt32 speedMult = 0;
	if (IS_ACTOR(thisObj) && ExtractArgsEx(EXTRACT_ARGS_EX, &speedMult))
	{
		if (speedMult)
			((Actor*)thisObj)->SetActorValueInt(0x15, speedMult);
		if (BaseProcess* baseProc = ((Actor*)thisObj)->baseProcess; baseProc) {
			if (AnimData* animData = baseProc->GetAnimData()) {
				animData->movementSpeedMult = speedMult;
			}
		}

	}
	return true;
}

DEFINE_COMMAND_ALT_PLUGIN(GetWeaponClipRoundsAlt, GetClipSizeAlt, "Gets weapon clip rounds, but also accounts for mods.", true, kParams_OneOptionalForm_OneOptionalInt);
bool Cmd_GetWeaponClipRoundsAlt_Execute(COMMAND_ARGS) {
	*result = 0;
	TESForm* form = nullptr;
	UInt8 assumEffect = 0;

	if (!ExtractArgs(EXTRACT_ARGS, &form, &assumEffect) || (form && form->typeID != kFormType_TESObjectWEAP)) {
		return true;
	}

	TESObjectWEAP* weapon = nullptr;
	UInt32 modFlags = 0;

	if (form) {
		weapon = static_cast<TESObjectWEAP*>(form->IsReference() ? static_cast<TESObjectREFR*>(form)->baseForm : form);
	}
	else if (thisObj && thisObj->baseForm && thisObj->baseForm->typeID == kFormType_TESObjectWEAP) {
		weapon = static_cast<TESObjectWEAP*>(thisObj->baseForm);
		modFlags = thisObj->GetWeaponModFlags();
	}

	if (weapon) {
		*result = weapon->GetModdedClipSize(assumEffect ? UINT_MAX : modFlags);
	}

	return true;
}

/*

DEFINE_COMMAND_PLUGIN(ToggleFreeCamCursor, 0, 1, kParams_OneOptionalInt);
bool Cmd_ToggleFreeCamCursor_Execute(COMMAND_ARGS)
{
	UInt32 toggle;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &toggle))
	{

		if (toggle)
		{
			g_interfaceManager->cursor->node->Show();
			float converter = *(float*)0x11D8A48;
		}
		else
		{
			g_interfaceManager->cursor->node->Hide();
		}

		s_UpdateCursor = toggle;
		g_interfaceManager->activeTile = nullptr;
		g_interfaceManager->activeMenu = nullptr;


	}

	return true;

}*/

//Functions....................................................................................................................................................

static ParamInfo kParams_ConvertWavToVoice[3] =
{
	{"form", kParamType_AnyForm, 0},
	{"form", kParamType_AnyForm, 0},
	{"string", kParamType_String, 0}
};

DEFINE_COMMAND_PLUGIN(ConvertWavToVoice, "Converts a wav file to a voice file path", false, kParams_ConvertWavToVoice);
bool Cmd_ConvertWavToVoice_Execute(COMMAND_ARGS)
{
	*result = 0;

	const std::string sourcePath;
	const std::string GeckPath;
	const std::string voicetype;


	//UInt32 ResponseNum = topicResponse->data.responseNumber;
	TESTopicInfo* topicInfo = NULL;
	tList<Condition*> conditions = topicInfo->conditions;
	//UInt32 Speaker = topicInfo->speaker;

	TESTopicInfoResponse* topicResponse = ThisStdCall<TESTopicInfoResponse*>(0x61E780, topicInfo, nullptr);
	UInt32 ResponseNum = topicResponse->data.responseNumber;
	tList<TESTopic*> ParentTopic = topicInfo->relatedTopics->linkFrom;

	TESActorBase* actorBase = nullptr;

	if (!actorBase)
	{
		//actorBase = ((Actor*)thisObj)->baseForm;
	}
	if (actorBase->baseData.voiceType)
		UInt32 VoiceType = actorBase->baseData.voiceType->refID;
		BGSVoiceType* VoiceTypeBGS = actorBase->baseData.voiceType;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &topicInfo, &topicResponse, &voicetype)) {

		auto Quest = topicInfo->quest->refID;

		//Plugin/VoiceType/Quest_ParentTopic_TopicInfoID_ResponseNum

		std::ifstream sourceFile(sourcePath, std::ios::binary);
		std::ofstream destinationFile(GeckPath, std::ios::binary);

		if (sourceFile && destinationFile) {
			// Read and write in chunks to efficiently copy the file
			const int bufferSize = 4096;
			char buffer[bufferSize];

			while (!sourceFile.eof()) {
				sourceFile.read(buffer, bufferSize);
				destinationFile.write(buffer, sourceFile.gcount());
			}

			std::cout << "File copied successfully." << std::endl;
		}
		else {
			std::cerr << "Error opening files." << std::endl;
		}

	}
	return true;
}

static ParamInfo kParams_MoveFileTo[2] =
{
	{"string", kParamType_String, 0},
	{"string", kParamType_String, 0}
};

DEFINE_COMMAND_PLUGIN(MoveFileTo, "Copy a file to another place on the drive", false, kParams_MoveFileTo);
bool Cmd_MoveFileTo_Execute(COMMAND_ARGS)
{
	*result = 0;

	char destinationPath[MAX_PATH]; // relative to "Fallout New Vegas" folder.
	destinationPath[0] = 0;

	char sourcePath[MAX_PATH]; // relative to "Fallout New Vegas" folder.
	sourcePath[0] = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sourcePath, &destinationPath) && sourcePath[0] && destinationPath[0]) {

		std::string sSourceFile = (GetFalloutDirectory() + sourcePath);
		std::ifstream sourceFile(sSourceFile, std::ios::binary);

		std::string sDestinationFile = (GetFalloutDirectory() + destinationPath);
		std::ofstream destinationFile(sDestinationFile, std::ios::binary | std::ios::trunc);

		if (sourceFile && destinationFile) {
			const int bufferSize = 4096;
			char buffer[bufferSize];

			while (sourceFile.read(buffer, bufferSize) && sourceFile.gcount() > 0) {
				destinationFile.write(buffer, sourceFile.gcount());
			}

			Console_Print("File copied successfully.");

		}
		else {

			Console_Print("Error opening files.");

		}

	}
	return true;

}

DEFINE_COMMAND_PLUGIN(GetTopicSpeaker, "Returns the line speaker", false, kParams_OneForm);
bool Cmd_GetTopicSpeaker_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESTopicInfo* topicInfo = NULL;
	TESObject* Speaker;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &topicInfo)) {

		Speaker = topicInfo->speaker;
		//Console_Print((const char*)Speaker);
		*result = Speaker ? Speaker->refID : 0;

	}
	return true;
}

static ParamInfo kParams_OneForm_OptInt[2] =
{
	{	"form",		kParamType_AnyForm,	0	},
	{	"int",		kParamType_Integer, 1	},
};
DEFINE_COMMAND_PLUGIN(GetTopicPrompt, "Grabs the prompt of a topic info", false, kParams_OneForm_OptInt);
bool Cmd_GetTopicPrompt_Execute(COMMAND_ARGS)
{

	*result = 0;
	const TESForm* Form = NULL;
	UInt32 promptType = 0;
	
	if (ExtractArgsEx(EXTRACT_ARGS_EX, &Form, &promptType)) {

		const char* sReturn = "NULL";
		TESTopic* topicRef = NULL;
		TESTopicInfo* topicInfo = NULL;

		if (Form->typeID == kFormType_TESTopicInfo) {

			topicInfo = static_cast<TESTopicInfo*>(const_cast<TESForm*>(Form));

			topicRef = (topicInfo->parentTopic);

		}else if (Form->typeID == kFormType_TESTopic) {

			topicRef = static_cast<TESTopic*>(const_cast<TESForm*>(Form));

		}
		else {
			Console_Print("Error GetTopicPrompt Form is not a Topic");
			return true;
		}

		if (topicInfo && promptType == 1 && (topicInfo->prompt).CStr() != nullptr) {
			sReturn = (topicInfo->prompt).CStr();
		}
		else if (promptType == 2 && topicRef->unk34.Includes("")) {
			sReturn = (topicRef->unk34).CStr();
		}
		else {
			sReturn = ((&topicRef->fullName)->name).CStr();
		}

		g_strInterface.Assign(PASS_COMMAND_ARGS, sReturn);

	}
	return true;
}

DEFINE_COMMAND_PLUGIN(TopicHasPromptType, "Checks if a topic or topic info has a specific prompt type", false, kParams_OneForm_OptInt);
bool Cmd_TopicHasPromptType_Execute(COMMAND_ARGS)
{

	*result = 0;
	const TESForm* Form = NULL;
	UInt32 promptType = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &Form, &promptType)) {

		TESTopic* topicRef = NULL;
		TESTopicInfo* topicInfo = NULL;

		if (Form->typeID == kFormType_TESTopicInfo) {

			topicInfo = static_cast<TESTopicInfo*>(const_cast<TESForm*>(Form));

			topicRef = (topicInfo->parentTopic);

		}
		else if (Form->typeID == kFormType_TESTopic) {

			topicRef = static_cast<TESTopic*>(const_cast<TESForm*>(Form));

		}
		else {
			Console_Print("Error TopicHasPromptType Form is not a Topic");
			return true;
		}

		if (topicInfo && promptType == 1 && (topicInfo->prompt).CStr() != nullptr) {
			*result = 1;
		}
		else if (promptType == 2 && topicRef->unk34.Includes("")) {
			*result = 1;
		}

	}
	return true;
}

static ParamInfo kParamsSetWeaponModFlags[1] =
{
	{"int", kParamType_Integer, 0}
};

DEFINE_COMMAND_PLUGIN(GetWeaponModFlags, "Allows you to get both world and inv ref mod flags", true, 0);
bool Cmd_GetWeaponModFlags_Execute(COMMAND_ARGS)
{
	*result = thisObj->GetWeaponModFlags();
	return true;
}

DEFINE_COMMAND_PLUGIN(SetWeaponModFlags, "Allows you to set both world and inv ref mod flags", true, kParamsSetWeaponModFlags);
bool Cmd_SetWeaponModFlags_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 flags = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &flags)) {
		thisObj->SetWeaponModFlags(flags);
	}

	return true;

}

//...........................................................................................................................................

static ParamInfo kParamsiModAlt[3] =
{
	{"IMOD", kParamType_ImageSpaceModifier, 0},
	{"Float", kParamType_Float, 1},
	{"Object Ref", kParamType_ObjectRef, 1}
};

DEFINE_COMMAND_ALT_PLUGIN(iModAlt, ApplyImageSpaceModifierAlt, "Alt verion of iMod that allows for a target", false, kParamsiModAlt);
bool Cmd_iModAlt_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESImageSpaceModifier *apMod;
	float afStrength = 1.0;
	TESObjectREFR *apTarget = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &apMod, &afStrength, &apTarget)) {

		CdeclCall(0x5299A0, apMod, afStrength, apTarget->GetNiNode());

	}

	return true;
}

//............................................................................................................................................

static ParamInfo kParams_pSaveNif[2] =
{
	{"string", kParamType_String, 0},
	{"form", kParamType_AnyForm, 0}
};

NiStream* NiStreamCreate(NiStream* apThis) {
	return ThisStdCall<NiStream*>(0xA66150, apThis);
}

// 0xA66370
void NiStreamInsertObject(NiStream* niStream, NiNode* apObject) {
	ThisStdCall(0xA66370, niStream, apObject);
}
DEFINE_COMMAND_PLUGIN(pSaveNif, 0, 1, kParams_pSaveNif);
bool Cmd_pSaveNif_Execute(COMMAND_ARGS)
{

	NiObject* apNode = NULL;
	TESForm *tesForm = NULL;
	TESModel *tesModel = NULL;

	char filePath[MAX_PATH]; // relative to "Fallout New Vegas" folder.
	filePath[0] = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &filePath, &tesForm) && filePath[0]) {

		if (tesForm) {
			//if (thisObj) {
				//tesForm = thisObj->baseForm;
			//}

			Console_Print("Saving Nif...", filePath);
			//NiNode* node = ThisStdCall<NiNode*>(0x43FCD0, tesForm);

			NiNode* apNode = ThisStdCall<NiNode*>(0x43FCD0, tesForm);

			//NiAVObject* apNode = ThisStdCall<NiAVObject*>(0x43FCD0, tesForm);
			//NiObject* apNode = ThisStdCall<NiObject*>(0x43FCD0, tesForm);

			char Stream[sizeof(NiStream)];
			NiStream* pStream = (NiStream*)Stream;
			NiStreamCreate(pStream);
			NiStreamInsertObject(pStream, apNode);
			pStream->Save2(filePath);
			pStream->~NiStream();

		}

	}

	return true;

}

static ParamInfo kParams_pWriteFile[3] =
{
	{"string", kParamType_String, 0},
	{"int", kParamType_Integer, 0},
	{"int", kParamType_Integer, 1}
};

DEFINE_COMMAND_PLUGIN(pWriteFile, "Writes an array of strings to a file.", false, kParams_pWriteFile);
bool Cmd_pWriteFile_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 arrID;
	UInt32 bAppend;

	char filePath[MAX_PATH]; // relative to "Fallout New Vegas" folder.
	filePath[0] = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &filePath, &arrID, &bAppend) && filePath[0]) {


		std::string sFile = (GetFalloutDirectory() + filePath);

		std::ofstream outputFile;

		if (bAppend == 1 && std::filesystem::exists(sFile)) {
			outputFile.open(sFile, std::ios::app); // Open file in append mode
		}
		else {
			outputFile.open(sFile, std::ios::out | std::ios::trunc);  // Open file in truncation mode
		}

		if (outputFile.is_open()) {

			NVSEArrayVarInterface::Array* aArray = LookupArrayByID(arrID);
			NVSEArrayVarInterface::ElementR val;
			NVSEArrayVarInterface::ElementR key;
			int size = GetArraySize(aArray);
			int lineNumber = 1;

			for (int lineNumber = 0; lineNumber < size; ++lineNumber) {

				key = lineNumber;

				GetElement(aArray, key, val);
				if (val.String() != NULL) {
					const char* sLine = val.String();
					outputFile << sLine << std::endl;
				}

			}

			outputFile.close();
			*result = 1;

		}
		else {
			Console_Print("Error: Unable to open/create the file.");
		}

	}

	return true;

}

static ParamInfo kParams_pReadFile[3] =
{
	{"string", kParamType_String, 0},
	{"int", kParamType_Integer, 1},
	{"int", kParamType_Integer, 1}
};

DEFINE_COMMAND_PLUGIN(pReadFile, , false, kParams_pReadFile);
bool Cmd_pReadFile_Execute(COMMAND_ARGS) {

	*result = 0;
	UInt32 iStartLine = 0;
	UInt32 iEndLine = 0;
	char filePath[MAX_PATH]; // relative to "Fallout New Vegas" folder.
	filePath[0] = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &filePath, &iStartLine, &iEndLine) && filePath[0])
	{

		std::string sFile = (GetFalloutDirectory() + filePath);
		auto aLines = g_arrInterface->CreateArray(nullptr, 0, scriptObj);	//Create an empty array
		ArrayElementL strElem;
		std::ifstream file(sFile);
		if (file.is_open()) {	//Open the file

			std::string sLine;

			UInt32 lineNumber = 1;
			while (std::getline(file, sLine)) {

				if ((iStartLine == 0 && iEndLine == 0) ||
					(iEndLine == 0 && lineNumber >= iStartLine) ||
					(lineNumber >= iStartLine && lineNumber <= iEndLine)) {

					const char* sobsLine = sLine.c_str();
					strElem = sobsLine;
					g_arrInterface->AppendElement(aLines, strElem);		//Add to array

				}

				lineNumber++;

				if (iEndLine != 0 && lineNumber > iEndLine) {
					break;
				}
			}

			file.close();
		}
		else {
			Console_Print("Error, path does not exist: >> %f", sFile);
		}

		g_arrInterface->AssignCommandResult(aLines, result);	//Return array
		//AssignArrayResult(aLines, result);

	}

	return true;

}

static ParamInfo kParams_pRotateAround[5] =
{
	{"float", kParamType_Double, 0},
	{"int", kParamType_Integer, 0},
	{"ref", kParamType_ObjectRef, 0},
	{"int", kParamType_Integer, 1},
	{"int", kParamType_Integer, 1}
};

DEFINE_COMMAND_PLUGIN(pRotateAroundObject, , true, kParams_pRotateAround);
bool Cmd_pRotateAroundObject_Execute(COMMAND_ARGS) {

	UInt32 iAxis;
	UInt32 bCheck;
	UInt32 bNoAngle;
	double iIncrements;
	TESObjectREFR* rObject;
	*result = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &iIncrements, &iAxis, &rObject, &bNoAngle, &bCheck))
	{

		double iIncrementsPi = iIncrements * (PI / 180.0);
		rObject->posX, rObject->posY, rObject->posZ;

		double XOrigin = rObject->posX;
		double YOrigin = rObject->posY;
		double ZOrigin = rObject->posZ;

		double fItemPosX = thisObj->posX;       //is the X position of the calling object
		double fItemPosY = thisObj->posY;      //is the Y position of the calling object
		double fItemPosZ = thisObj->posZ;      //is the Z position of the calling object

		double OutputX = thisObj->posX;
		double OutputY = thisObj->posY;
		double OutputZ = thisObj->posZ;

		//Console_Print("iAxis: >> %f", iAxis);
		//Console_Print("ItemPosX: >> %f", fItemPosX);
		//Console_Print("XOrigin: >> %f", XOrigin);

		switch (iAxis) {

		case 1:

			OutputY = cos(iIncrementsPi) * (fItemPosY - YOrigin) + (fItemPosZ - ZOrigin) * sin(iIncrementsPi) + YOrigin;
			OutputZ = -sin(iIncrementsPi) * (fItemPosY - YOrigin) + cos(iIncrementsPi) * (fItemPosZ - ZOrigin) + ZOrigin;

			if (bCheck != 1)
			{
				thisObj->posY = OutputY;
				thisObj->posZ = OutputZ;
			}
			//Console_Print("Y Output: >> %f", OutputY);
			//Console_Print("Z Output: >> %f", OutputZ);
			break;

		case 2:

			OutputX = cos(iIncrementsPi * -1) * (fItemPosX - XOrigin) + (fItemPosZ - ZOrigin) * sin(iIncrementsPi * -1) + XOrigin;
			OutputZ = -sin(iIncrementsPi * -1) * (fItemPosX - XOrigin) + cos(iIncrementsPi * -1) * (fItemPosZ - ZOrigin) + ZOrigin;
			if (bCheck != 1)
			{
				thisObj->posX = OutputX;
				thisObj->posZ = OutputZ;
			}
			break;

		case 3:

			OutputX = cos(iIncrementsPi) * (fItemPosX - XOrigin) + (fItemPosY - YOrigin) * sin(iIncrementsPi) + XOrigin;
			OutputY = -sin(iIncrementsPi) * (fItemPosX - XOrigin) + cos(iIncrementsPi) * (fItemPosY - YOrigin) + YOrigin;

			if (bCheck != 1)
			{
				thisObj->posX = OutputX;
				thisObj->posY = OutputY;

			}
			//Console_Print("X Output: >> %f", OutputX);
			//Console_Print("Y Output: >> %f", OutputY);
			break;

		}

		NVSEArrayVarInterface::ElementL outXY[3] = { OutputX, OutputY, OutputZ };
		AssignArrayResult(CreateArray(outXY, 3, scriptObj), result);

	}

	return true;
}

static ParamInfo kParams_pRotateAroundLocally[5] =
{
	{"float", kParamType_Double, 0},
	{"int", kParamType_Integer, 0},
	{"ref", kParamType_ObjectRef, 0},
	{"int", kParamType_Integer, 1},
	{"int", kParamType_Integer, 1}
};

DEFINE_COMMAND_PLUGIN(pRotateAroundObjectLocally, , true, kParams_pRotateAroundLocally);
bool Cmd_pRotateAroundObjectLocally_Execute(COMMAND_ARGS) {

	UInt32 iAxis;
	UInt32 bCheck;
	UInt32 bNoAngle;
	double iIncrements;
	TESObjectREFR* rObject;
	*result = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &iIncrements, &iAxis, &rObject, &bNoAngle, &bCheck))
	{

		double iIncrementsPi = iIncrements * (PI / 180.0);
		rObject->posX, rObject->posY, rObject->posZ;

		double XOrigin = rObject->posX;
		double YOrigin = rObject->posY;
		double ZOrigin = rObject->posZ;

		double fItemPosX = thisObj->posX;       //is the X position of the calling object
		double fItemPosY = thisObj->posY;      //is the Y position of the calling object
		double fItemPosZ = thisObj->posZ;      //is the Z position of the calling object

		double OutputX = thisObj->posX;
		double OutputY = thisObj->posY;
		double OutputZ = thisObj->posZ;

		//Console_Print("iAxis: >> %f", iAxis);
		//Console_Print("ItemPosX: >> %f", fItemPosX);
		//Console_Print("XOrigin: >> %f", XOrigin);

		switch (iAxis) {

		case 1:

			OutputY = cos(iIncrementsPi) * (fItemPosY - YOrigin) + (fItemPosZ - ZOrigin) * sin(iIncrementsPi) + YOrigin;
			OutputZ = -sin(iIncrementsPi) * (fItemPosY - YOrigin) + cos(iIncrementsPi) * (fItemPosZ - ZOrigin) + ZOrigin;

			if (bCheck != 1)
			{
				thisObj->posY = OutputY;
				thisObj->posZ = OutputZ;
			}
			//Console_Print("Y Output: >> %f", OutputY);
			//Console_Print("Z Output: >> %f", OutputZ);
			break;

		case 2:

			OutputX = cos(iIncrementsPi * -1) * (fItemPosX - XOrigin) + (fItemPosZ - ZOrigin) * sin(iIncrementsPi * -1) + XOrigin;
			OutputZ = -sin(iIncrementsPi * -1) * (fItemPosX - XOrigin) + cos(iIncrementsPi * -1) * (fItemPosZ - ZOrigin) + ZOrigin;
			if (bCheck != 1)
			{
				thisObj->posX = OutputX;
				thisObj->posZ = OutputZ;
			}
			break;

		case 3:

			OutputX = cos(iIncrementsPi * -1) * (fItemPosX - XOrigin) + (fItemPosY - YOrigin) * sin(iIncrementsPi * -1) + XOrigin;
			OutputY = -sin(iIncrementsPi * -1) * (fItemPosX - XOrigin) + cos(iIncrementsPi * -1) * (fItemPosY - YOrigin) + YOrigin;

			if (bCheck != 1)
			{
				thisObj->posX = OutputX;
				thisObj->posY = OutputY;

			}
			//Console_Print("X Output: >> %f", OutputX);
			//Console_Print("Y Output: >> %f", OutputY);
			break;

		}

		NVSEArrayVarInterface::ElementL outXY[3] = { OutputX, OutputY, OutputZ };
		AssignArrayResult(CreateArray(outXY, 3, scriptObj), result);

	}

	return true;
}
