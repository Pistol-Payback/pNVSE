#pragma once
#include <variant>

struct AuxVector;

struct AuxValue
{
    SInt8		type;
    union
    {
        double	num;
        UInt32	refID;
        char* str;
        AuxVector* NVSEArray;
    };

    AuxValue();
    AuxValue(double num);
    AuxValue(UInt32 refID);
    AuxValue(TESForm* form);
    AuxValue(const char* value);
    AuxValue(NVSEArrayVarInterface::Array* ToCopy);

    //Copy constructor:
    AuxValue(const AuxValue& other);
    AuxValue& operator=(const AuxValue& other);

    ~AuxValue()
    {
        CleanUpUnion();
    }

    AuxVector* CopyFromNVSEArray(NVSEArrayVarInterface::Array* ToCopy);
    NVSEArrayVarInterface::Array* CopyToNVSEArray(Script* resultArray) const;

    UInt8 GetType() const { return type; }
    double GetFlt() const { return (type == kRetnType_Default) ? num : 0; }
    UInt32 GetRef() const { return (type == kRetnType_Form) ? refID : 0; }
    const char* GetStr() const { return (type == kRetnType_String) ? str : nullptr; }
    AuxVector* GetArray() const { return (type == kRetnType_Array) ? NVSEArray : nullptr; }

    void SetValue();
    void SetValue(double value);
    void SetValue(UInt32 value);
    void SetValue(const char* value);
    void SetValue(NVSEArrayVarInterface::Array* value);

    double GetValue(double value);
    UInt32 GetValue(UInt32 value);
    const char* GetValue(const char* value);
    AuxVector* GetValue(NVSEArrayVarInterface::Array* value);

    ArrayElementL GetAsElement() const;

    void CleanUpUnion();

};


struct AuxVector : public std::vector<AuxValue> {

    using std::vector<AuxValue>::vector;

    template<class T>
    void AddValue(int index, T value) {
        if (index < 0) {
            push_back(AuxValue{ value });
        }
        else {
            if (index >= size()) {
                resize(index + 1);
            }
            (*this)[index].SetValue(value);
        }
    }

    template<class T>
    void SetValue(int index, T value) {
        if (index >= size()) {
            throw std::out_of_range("Index out of range");
        }
        (*this)[index].SetValue(value);
    }

    void AddEmptyValue(int index) {
        if (index < 0) {
            push_back(AuxValue{});
        }
        else {
            if (index >= size()) {
                resize(index + 1);
            }
            (*this)[index].SetValue();
        }
    }

    template<class T>
    SInt32 Find(T value) {

        for (size_t index = 0; index < this->size(); ++index) {
            if ((*this)[index].GetValue(value) == value) {
                return index;
            }
        }
        return -1; // Return a special value to indicate that the element was not found
    }

};

struct CallLoopInfo {

    double delay;
    double timer;
    TESObjectREFR* callingObj;
    std::vector<UInt32> arguments;

    double CallLoopFunction(Script* script, TESObjectREFR* callingObj, TESObjectREFR* container);

};