#pragma once
#include "WeaponSmith.h"

void UnpackTraitArguments(PluginExpressionEvaluator& eval, const char*& trait, SInt32& index, UInt32& refID) {

	trait = eval.GetNthArg(0)->GetString();
	if (eval.NumArgs() > 1) {
		index = eval.GetNthArg(1)->GetInt();
		if (eval.NumArgs() > 2) {
			refID = eval.GetNthArg(2)->GetFormID();
		}
	}

}

DEFINE_COMMAND_ALT_PLUGIN_EXP(GetFormTraitType, TR_GetType, "Gets the var type", false, kNVSEParams_OneString_OneOptionalNumber_OneOptionalForm);
bool Cmd_GetFormTraitType_Execute(COMMAND_ARGS)
{

	*result = -2;
	SInt32 index = 0;
	UInt32 refID = 0;
	const char* trait = NULL;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		UnpackTraitArguments(eval, trait, index, refID);
		const AuxVector* aux = FormTraits::findAuxVector(refID, trait);
		if (!aux || index >= aux->size()) {
			*result = -2;
			return true;
		}

		*result = (*aux)[index].type;

	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN_EXP(EraseFormTrait, TR_Erase, "Checks to see if the trait exists", false, kNVSEParams_OneString_OneOptionalNumber_OneOptionalForm);
bool Cmd_EraseFormTrait_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32 refID = 0;
	SInt32 index = -1;
	const char* trait = NULL;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		UnpackTraitArguments(eval, trait, index, refID);
		trait = eval.GetNthArg(0)->GetString();
		FormTraits::eraseAuxFromVector(refID, trait, index, true);
	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN_EXP(NullFormTraitIndex, TR_Null, "Checks to see if the trait exists", false, kNVSEParams_OneString_OneOptionalNumber_OneOptionalForm);
bool Cmd_NullFormTraitIndex_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32 refID = 0;
	SInt32 index = -1;
	const char* trait = NULL;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		UnpackTraitArguments(eval, trait, index, refID);
		trait = eval.GetNthArg(0)->GetString();
		FormTraits::eraseAuxFromVector(refID, trait, index, false);
	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN_EXP(GetFormTrait, TR_Get, "Gets the var", false, kNVSEParams_OneString_OneOptionalNumber_OneOptionalForm);
bool Cmd_GetFormTrait_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32* refResult = (UInt32*)result;
	SInt32 index = 0;
	UInt32 refID = 0;
	const char* trait = NULL;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		UnpackTraitArguments(eval, trait, index, refID);
		const AuxVector* aux = FormTraits::findAuxVector(refID, trait);

		if ((*aux)[index].type != -1) {
			eval.SetExpectedReturnType((CommandReturnType)(*aux)[index].type);

			switch ((*aux)[index].type) {
			case kRetnType_Default:
				*result = (*aux)[index].num;
				break;
			case kRetnType_Form:
				*refResult = (*aux)[index].refID;
				break;
			case kRetnType_String:
				AssignString(PASS_COMMAND_ARGS, (*aux)[index].str);
				break;
			case kRetnType_Array:
				g_arrInterface->AssignCommandResult((*aux)[index].CopyToNVSEArray(scriptObj), result);
				break;
			}

		}

	}

	return true;

}

void UnpackTraitArguments(PluginExpressionEvaluator& eval, PluginScriptToken*& value, const char*& trait, SInt32& index, UInt32& refID) {

	value = eval.GetNthArg(0);
	trait = eval.GetNthArg(1)->GetString();
	if (eval.NumArgs() > 2) {
		index = eval.GetNthArg(2)->GetInt();
		if (eval.NumArgs() > 3) {
			refID = eval.GetNthArg(3)->GetFormID();
		}
	}

}

DEFINE_COMMAND_ALT_PLUGIN_EXP(SetFormTrait, TR_Set, "Set the var", false, kNVSEParams_OneBasicType_OneString_OneOptionalNumber_OneOptionalForm);
bool Cmd_SetFormTrait_Execute(COMMAND_ARGS) {

	*result = 0;

	//BasicType value;
	SInt32 index = 0;
	UInt32 refID = 0;
	const char* trait = NULL;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		PluginScriptToken* value;
		UnpackTraitArguments(eval, value, trait, index, refID);

		AuxVector* aux = FormTraits::setAuxVector(refID, trait);

		if (!aux) {
			Console_Print("Error: Unable to get trait");
			return true;
		}

		switch (value->GetTypeAlt()) {
		case kRetnType_Default:
		{
			double valueFlt = value->GetFloat(); // Store the float value
			aux->AddValue(index, valueFlt);
			break;
		}
		case kRetnType_Form:
		{
			TESForm* valueRef = value->GetTESForm();
			if (valueRef) {
				aux->AddValue(index, valueRef->refID);
			}
			break;
		}
		case kRetnType_String:
		{
			const char* valueStr = value->GetString();
			if (valueStr) {
				aux->AddValue(index, valueStr);
			}
			break;
		}
		case kRetnType_Array:
		{
			NVSEArrayVarInterface::Array* valueArr = value->GetArrayVar();
			if (valueArr) {
				aux->AddValue(index, valueArr);
			}
			break;
		}
		}

	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN_EXP(FindFormTrait, TR_Find, "finds a var", false, kNVSEParams_OneBasicType_OneString_OneOptionalNumber_OneOptionalForm);
bool Cmd_FindFormTrait_Execute(COMMAND_ARGS) {

	*result = -1;

	SInt32 deepSearch = 0;
	UInt32 refID = 0;
	const char* trait = NULL;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		PluginScriptToken* value;
		UnpackTraitArguments(eval, value, trait, deepSearch, refID);

		AuxVector* aux = FormTraits::setAuxVector(refID, trait);

		if (!aux) {
			Console_Print("Error: Unable to get trait");
			return true;
		}

		switch (value->GetTypeAlt()) {
		case kRetnType_Default:
		{
			double valueFlt = value->GetFloat(); // Store the float value
			*result = aux->Find(valueFlt);
			break;
		}
		case kRetnType_Form:
		{
			TESForm* valueRef = value->GetTESForm();
			if (valueRef) {
				*result = aux->Find(valueRef->refID);
			}
			break;
		}
		case kRetnType_String:
		{
			const char* valueStr = value->GetString();
			if (valueStr) {
				*result = aux->Find(valueStr);
			}
			break;
		}
		case kRetnType_Array:
		{
			//NVSEArrayVarInterface::Array* valueArr = value->GetArrayVar();
			//if (valueArr) {
				//AuxVector* valueVect = AuxValue::CopyFromNVSEArray(valueArr);
				//*result = aux->Find<AuxVector*>(valueVect, kRetnType_Array);
			//}
			break;
		}
		}

	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN_EXP(GetFormTraitSize, TR_Size, "Gets the var effect", false, kNVSEParams_OneString_OneOptionalForm);
bool Cmd_GetFormTraitSize_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32 refID = 0;
	const char* trait = NULL;


	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		trait = eval.GetNthArg(0)->GetString();
		refID = eval.GetNthArg(1)->GetFormID();
		const AuxVector* aux = FormTraits::findAuxVector(refID, trait);
		if (aux) {
			*result = aux->size();
		}

	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN_EXP(HasFormTrait, TR_Exists, "Checks to see if the trait exists", false, kNVSEParams_OneString_OneOptionalForm);
bool Cmd_HasFormTrait_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32 refID = 0;
	const char* trait = NULL;


	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		trait = eval.GetNthArg(0)->GetString();
		refID = eval.GetNthArg(1)->GetFormID();
		const AuxVector* aux = FormTraits::findAuxVector(refID, trait);
		if (aux) {
			aux->Dump();
		}

	}

	return true;

}

DEFINE_COMMAND_ALT_PLUGIN_EXP(DumpFormTrait, TR_Dump, "Checks to see if the trait exists", false, kNVSEParams_OneString_OneOptionalForm);
bool Cmd_DumpFormTrait_Execute(COMMAND_ARGS)
{

	*result = 0;
	UInt32 refID = 0;
	const char* trait = NULL;

	if (PluginExpressionEvaluator eval(PASS_COMMAND_ARGS); eval.ExtractArgs())
	{
		trait = eval.GetNthArg(0)->GetString();
		refID = eval.GetNthArg(1)->GetFormID();
		const AuxVector* aux = FormTraits::findAuxVector(refID, trait);
		if (aux) {
			*result = 1;
		}
	}

	return true;

}
