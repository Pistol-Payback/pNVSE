#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <InventoryRef.h>
#include "StringVar.h"

#define PI 3.14159265

static EquipData FindEquipped(TESObjectREFR* thisObj, FormMatcher& matcher) {
	ExtraContainerChanges* pContainerChanges = static_cast<ExtraContainerChanges*>(thisObj->extraDataList.GetByType(kExtraData_ContainerChanges));
	return (pContainerChanges) ? pContainerChanges->FindEquipped(matcher) : EquipData();
}

class MatchBySlot : public FormMatcher
{
	UInt32 m_slotMask;
public:
	MatchBySlot(UInt32 slot) : m_slotMask(TESBipedModelForm::MaskForSlot(slot)) {}
	bool Matches(TESForm* pForm) const {
		UInt32 formMask = 0;
		if (pForm) {
			if (pForm->IsWeapon()) {
				formMask = TESBipedModelForm::eSlot_Weapon;
			}
			else {
				TESBipedModelForm* pBip = DYNAMIC_CAST(pForm, TESForm, TESBipedModelForm);
				if (pBip) {
					formMask = pBip->partMask;
				}
			}
		}
		return (formMask & m_slotMask) != 0;
	}
};

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
	UInt32 Speaker = topicInfo->speaker;

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

DEFINE_COMMAND_PLUGIN(GetTopicSpeaker, "Converts a wav file to a voice file path", false, kParams_OneForm);
bool Cmd_GetTopicSpeaker_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESTopicInfo* topicInfo = NULL;
	UInt32 Speaker = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &topicInfo)) {

		Speaker = topicInfo->speaker;
		*result = Speaker;

	}
	return true;
}

DEFINE_COMMAND_PLUGIN(GetTopicPrompt, "Grabs the prompt of a topic info", false, kParams_OneForm);
bool Cmd_GetTopicPrompt_Execute(COMMAND_ARGS)
{

	*result = 0;
	TESTopicInfo* topicInfo = NULL;
	String sPrompt;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &topicInfo)) {

		sPrompt = topicInfo->prompt;
		Console_Print("%s", sPrompt);
		const char* sReturn = sPrompt.CStr();
		Console_Print(sReturn);
		//*result = AssignToStringVar(PASS_COMMAND_ARGS, sReturn);
		g_strInterface->Assign(PASS_COMMAND_ARGS, sReturn);

	}
	return true;
}

static ParamInfo kParamsGetWeaponModFlags[1] =
{
	{"Object Ref", kParamType_ObjectRef, 0}
};

static ParamInfo kParamsSetWeaponModFlags[2] =
{
	{"Object Ref", kParamType_ObjectRef, 0},
	{"int", kParamType_Integer, 0}
};

DEFINE_COMMAND_PLUGIN(GetWeaponModFlags, "Allows you to get both world and inv ref mod flags", false, kParamsGetWeaponModFlags);
bool Cmd_GetWeaponModFlags_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESObjectREFR* Weapon;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &Weapon)) {

		ExtraWeaponModFlags* xWeaponModFlags = static_cast<ExtraWeaponModFlags*>(Weapon->extraDataList.GetByType(kExtraData_WeaponModFlags));
		if (xWeaponModFlags)
			*result = xWeaponModFlags->flags;

	}

	return true;

}

DEFINE_COMMAND_PLUGIN(SetWeaponModFlags, "Allows you to set both world and inv ref mod flags", false, kParamsSetWeaponModFlags);
bool Cmd_SetWeaponModFlags_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESObjectREFR* Weapon;
	UInt32 flags = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &Weapon, &flags)) {

		ExtraWeaponModFlags* xWeaponModFlags = static_cast<ExtraWeaponModFlags*>(Weapon->extraDataList.GetByType(kExtraData_WeaponModFlags));

		// Modify existing flags
		if (xWeaponModFlags) {

			if (flags) {
				xWeaponModFlags->flags = (UInt8)flags;
			}
			else{

				Weapon->extraDataList.Remove(xWeaponModFlags, true);
			}

		} // Create new extra data
		else if (flags) {
			Console_Print("Create");
			xWeaponModFlags = ExtraWeaponModFlags::Create();
			if (xWeaponModFlags) {

				InventoryRef* invRef; //= InventoryReferenceGetForRefID(Weapon->refID);
				if (invRef) {

					Console_Print("Is Inv Ref");
					//xWeaponModFlags = invRef->CreateExtraData();
					xWeaponModFlags->flags = (UInt8)flags;
					if (invRef->GetCount() > 1){
						//invRef = invRef->SplitFromStack();
					}

					Weapon->extraDataList.Add(xWeaponModFlags);

				}
				else {
					Console_Print("Is Not Inv Ref");
					xWeaponModFlags->flags = (UInt8)flags;
					Weapon->extraDataList.Add(xWeaponModFlags);
				}

			}

		}

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
			NiStream::Create(pStream);
			pStream->InsertObject(apNode);
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
