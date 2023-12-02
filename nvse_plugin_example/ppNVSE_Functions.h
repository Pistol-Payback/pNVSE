#pragma once
#include<iostream>
#include <fstream>
#include <filesystem>
#define PI 3.14159265

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

static ParamInfo kParams_pSaveNif[2] =
{
	{"string", kParamType_String, 0},
	{"int", kParamType_ObjectID, 0}
};

DEFINE_COMMAND_PLUGIN(pSaveNif, 0, 1, kParams_OneOptionalInt);
bool Cmd_pSaveNif_Execute(COMMAND_ARGS)
{

	NiObject* apNode = NULL;
	TESForm *tesForm = NULL;
	TESModel *tesModel = NULL;

	char filePath[MAX_PATH]; // relative to "Fallout New Vegas" folder.
	filePath[0] = 0;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &filePath, &tesForm) && filePath[0]) {

		if (!tesForm)
			if (thisObj)
				tesForm = thisObj->baseForm;
		//NiNode* node = ThisStdCall<NiNode*>(0x43FCD0, tesForm);
		NiObject* apNode = ThisStdCall<NiObject*>(0x43FCD0, tesForm);
		char Stream[sizeof(NiStream)];
		NiStream* pStream = (NiStream*)Stream;
		NiStream::Create(pStream);
		pStream->InsertObject(apNode);
		pStream->Save2(filePath);
		pStream->~NiStream();

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
