#pragma once
#include "ppNVSE.h"

#include <list>
#include <variant>

    void AuxValue::SetValue() {
        this->CleanUpUnion();
        type = -1;
        num = 0;
    }

    void AuxValue::SetValue(double value) {
        this->CleanUpUnion();
        type = kRetnType_Default;
        num = value;
    }
    void AuxValue::SetValue(UInt32 value) {
        this->CleanUpUnion();
        type = kRetnType_Form;
        refID = value;
    }
    void AuxValue::SetValue(const char* value) {
        this->CleanUpUnion();
        type = kRetnType_String;
        if (value) {
            str = strdup(value);
        }
        else {
            str = nullptr;
        }
    }

    void AuxValue::SetValue(NVSEArrayVarInterface::Array* value) {
        this->CleanUpUnion();
        type = kRetnType_Array;
        NVSEArray = CopyFromNVSEArray(value);
    }

    double AuxValue::GetValue(double value) {
        return GetFlt();
    }

    UInt32 AuxValue::GetValue(UInt32 value) {
        return GetRef();
    }

    const char* AuxValue::GetValue(const char* value) {
        return GetStr();
    }

    AuxVector* AuxValue::GetValue(NVSEArrayVarInterface::Array* value) {
        return GetArray();
    }

    ArrayElementL AuxValue::GetAsElement() const
    {
        if (type == kRetnType_Form) return ArrayElementL(LookupFormByRefID(refID));
        if (type == kRetnType_String) return ArrayElementL(GetStr());
        return ArrayElementL(num);
    }

    AuxVector* AuxValue::CopyFromNVSEArray(NVSEArrayVarInterface::Array* ToCopy) {
        if (!ToCopy)
            return nullptr;

        const auto& arrData = ArrayData(ToCopy, true);
        int iSize = g_arrInterface->GetArraySize(ToCopy);
        if (iSize <= 0)
            return nullptr;

        AuxVector* valuesList = new AuxVector();
        valuesList->reserve(iSize);

        for (UInt32 idx = 0; idx < arrData.size; idx++) {
            switch (arrData.vals[idx].type) {
            case NVSEArrayVarInterface::kType_Numeric:
                valuesList->emplace_back(arrData.vals[idx].Number());
                break;
            case NVSEArrayVarInterface::kType_Form:
                if (auto temp = arrData.vals[idx].Form()) {
                    valuesList->emplace_back(temp->refID);
                }
                break;
            case NVSEArrayVarInterface::kType_String:
                if (auto temp = arrData.vals[idx].String()) {
                    valuesList->emplace_back(temp);
                }
                break;
            case NVSEArrayVarInterface::kType_Array:
                if (auto temp = arrData.vals[idx].Array()) {
                    valuesList->emplace_back(temp);
                }
                break;
            }
        }

        return valuesList;
    }

    NVSEArrayVarInterface::Array* AuxValue::CopyToNVSEArray(Script* script) const {

        AuxVector* valuesList = NVSEArray;
        NVSEArrayVarInterface::Array* resultArray = g_arrInterface->CreateArray(nullptr, 0, script);

        for (AuxValue& value : *valuesList) {
            switch (value.type) {
            case kRetnType_Default:
                g_arrInterface->AppendElement(resultArray, (ArrayElementL)value.num);
                break;
            case kRetnType_Form:
                g_arrInterface->AppendElement(resultArray, (ArrayElementL)LookupFormByRefID(value.refID));
                break;
            case kRetnType_String:
                g_arrInterface->AppendElement(resultArray, (ArrayElementL)value.str);
                break;
            case kRetnType_Array:
                g_arrInterface->AppendElement(resultArray, (ArrayElementL)value.CopyToNVSEArray(script));
                break;
            }
        }

        return resultArray;
    }

    void AuxValue::CleanUpUnion()
    {
        switch (type)
        {
        case kRetnType_String:
            if (str)
            {
                free(str);
                str = nullptr;
            }
            break;
        case kRetnType_Array:
            if (NVSEArray)
            {
                delete NVSEArray;
                NVSEArray = nullptr;
            }
            break;
        }
    }

    AuxValue::AuxValue()
        :
        type(-1),
        num(0) {}

    AuxValue::AuxValue(
        double num)
        :
        type(kRetnType_Default),
        num(num) {}

    AuxValue::AuxValue(
        UInt32 refID)
        :
        type(kRetnType_Form),
        refID(refID) {}

    AuxValue::AuxValue(
        TESForm* form)
        :
        type(kRetnType_Form),
        refID(form->refID) {}

    AuxValue::AuxValue(
        const char* value)
        : type(kRetnType_String),
        str(nullptr) {

        if (value) {
            str = strdup(value);
        }

    }

    AuxValue::AuxValue(
        NVSEArrayVarInterface::Array* ToCopy)
        :
        type(kRetnType_Array),
        NVSEArray(nullptr) {
        NVSEArray = CopyFromNVSEArray(ToCopy);
    };

    AuxValue::AuxValue(const AuxValue& other) : type(other.type) {  //Copy constructor
        switch (type) {
        case kRetnType_Default:
            num = other.num;
            break;
        case kRetnType_Form:
            refID = other.refID;
            break;
        case kRetnType_String:
            if (other.str) {
                str = strdup(other.str);
            }
            else {
                str = nullptr;
            }
            break;
        case kRetnType_Array:
            if (other.NVSEArray) {
                NVSEArray = new AuxVector(*other.NVSEArray);
            }
            else {
                NVSEArray = nullptr;
            }
            break;
        }
    }

    AuxValue& AuxValue::operator=(const AuxValue& other) {
        if (this != &other) {
            CleanUpUnion(); // Clean up existing resources
            type = other.type;
            switch (type) {
            case kRetnType_Default:
                num = other.num;
                break;
            case kRetnType_Form:
                refID = other.refID;
                break;
            case kRetnType_String:
                if (other.str) {
                    str = strdup(other.str);
                }
                else {
                    str = nullptr;
                }
                break;
            case kRetnType_Array:
                if (other.NVSEArray) {
                    NVSEArray = new AuxVector(*other.NVSEArray);
                }
                else {
                    NVSEArray = nullptr;
                }
                break;
            }
        }
        return *this;
    }

    double CallLoopInfo::CallLoopFunction(Script* script, TESObjectREFR* callingObj, TESObjectREFR* container) {

        UInt8 numArgs = arguments.size();

        ArrayElementL scriptReturn;

        auto it = arguments.begin();

        switch (numArgs) {
        case 0:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs);
            break;
        case 1:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0]);
            break;
        case 2:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1]);
            break;
        case 3:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2]);
            break;
        case 4:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3]);
            break;
        case 5:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
            break;
        case 6:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
            break;
        case 7:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
            break;
        case 8:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
            break;
        case 9:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
            break;
        case 10:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
            break;
        case 11:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);
            break;
        case 12:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);
            break;
        case 13:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
            break;
        case 14:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);
            break;
        case 15:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
            break;
        case 16:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);
            break;
        case 17:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);
            break;
        case 18:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);
            break;
        case 19:
            g_scriptInterface->CallFunction(script, callingObj, container, &scriptReturn, numArgs, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);
            break;
        default:
            break;
        }

        return scriptReturn.Number();

    }