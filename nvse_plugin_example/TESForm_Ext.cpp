#pragma once
#include "ppNVSE.h"
#include "SaveSystem.h"

enum  //Memory Addresses
{
	kAddr_AddExtraData = 0x40FF60,
	kAddr_RemoveExtraType = 0x410140,
	kAddr_LoadModel = 0x447080,
	kAddr_ApplyAmmoEffects = 0x59A030,
	kAddr_MoveToMarker = 0x5CCB20,
	kAddr_ApplyPerkModifiers = 0x5E58F0,
	kAddr_ReturnThis = 0x6815C0,
	kAddr_PurgeTerminalModel = 0x7FFE00,
	kAddr_EquipItem = 0x88C650,
	kAddr_UnequipItem = 0x88C790,
	kAddr_ReturnTrue = 0x8D0360,
	kAddr_TileGetFloat = 0xA011B0,
	kAddr_TileSetFloat = 0xA012D0,
	kAddr_TileSetString = 0xA01350,
	kAddr_InitFontInfo = 0xA12020,
};

UInt32 TESForm::GetModIndexAlt() const {
    if (ExtendedBaseType* staticInst = this->LookupExtendedBase()) {
        return staticInst->edits[0];
    }
    else {
        return (refID >> 24);
    }
}

std::vector<UInt32> TESForm::GetFormEdits(bool getParentEdits) const {
    std::vector<UInt32> edits;
    if (this->IsInstancedForm()) {

        Instance* inst = this->pLookupInstance();
        edits.push_back(inst->modIndex);

        if (getParentEdits && inst->baseInstance) {
            edits.insert(edits.end(), inst->baseInstance->edits.begin(), inst->baseInstance->edits.end());
        }

    }
    else if (this->IsStaticForm()) {
        StaticInstance* statInst = this->LookupStaticInstance();
        if (statInst) {
            edits = statInst->edits;  // This will use the copy assignment operator
        }
    }
    return edits; // Return by value (uses move semantics if possible)
}

TESForm* TESForm::GetBaseObject()
{
    TESForm* form = this->IsReference() ? ((TESObjectREFR*)this)->baseForm : this;

    if (form->IsInstancedForm()) {
        TESForm* pform = form->GetStaticParent();
        if (pform) {
            return pform;
        }
    }
    return form;
}

bool TESForm::IsBaseForm() const
{
	return !((*(UInt32**)this)[0x3C] == kAddr_ReturnTrue);
}

bool TESForm::IsReference() const
{
	return ((*(UInt32**)this)[0x3C] == kAddr_ReturnTrue);

}

TESObjectREFR* TESForm::PlaceAtCell(TESForm* worldOrCell, float x, float y, float z, float xR, float yR, float zR)
{
    TESObjectCELL* targetCell = nullptr;

    if (worldOrCell->typeID == kFormType_TESObjectCELL)
    {
        targetCell = static_cast<TESObjectCELL*>(worldOrCell);
    }
    else
    {
        targetCell = static_cast<TESWorldSpace*>(worldOrCell)->cell;
    }

    DevKitDummyMarker->parentCell = targetCell;
    DevKitDummyMarker->posX = x;
    DevKitDummyMarker->posY = y;
    DevKitDummyMarker->posZ = z;
    DevKitDummyMarker->rotX = xR;
    DevKitDummyMarker->rotY = yR;
    DevKitDummyMarker->rotZ = zR;

    if (targetCell)
    {
        return DevKitDummyMarker->PlaceAtMe(this, 1, 0, 0, 1);
    }

    return nullptr;
}

TESObjectREFR* TESForm::PlaceAtCellAlt(TESForm* worldOrCell, float x = 0, float y = 0, float z = 0, float xR = 0, float yR = 0, float zR = 0, ExtraDataList* xData = nullptr)
{
    TESObjectCELL* targetCell = nullptr;
    TESWorldSpace* targetWorld = nullptr;
    if (worldOrCell->typeID == kFormType_TESObjectCELL)
    {
        targetCell = static_cast<TESObjectCELL*>(worldOrCell);
    }
    else if (worldOrCell->typeID == kFormType_TESWorldSpace)
    {
        targetWorld = static_cast<TESWorldSpace*>(worldOrCell);
        targetCell = targetWorld->cell;
    }

    if (targetCell == nullptr) {
        Console_Print("Error in PlaceAtCellAlt, Target Cell was null for object %s", this->GetEditorID());
        return nullptr;
    }
    TESObjectREFR* ref = DataHandler::Get()->PlaceObject(this, new NiPoint3(x, y, z), new NiPoint3(xR, yR, zR), targetCell, targetWorld, nullptr, nullptr, xData);
    if (xData) {
        ref->extraDataList.Copy(xData);
        xData->RemoveAll(true);
        FormHeap_Free(xData);
    }
    ref->SetRefPersists(1);

    return ref;

}

TESForm* TESForm::CreateNewForm(UInt8 typeID, const char* editorID, bool bPersist, UInt32 offset)
{
    TESForm* result = CreateFormInstance(typeID);

    if (!result)
    {
        return nullptr;
    }

    result->DoAddForm(result, 0);
    result->SetRefID(GetNextFreeFormID(GetFirstFormIDForModIndex(offset)), true);
    if (editorID) {
        result->SetEditorID(editorID);
    }

    if (bPersist) {
        DynamicallyCreatedForms.insert(result->refID);
    }

    return result;
}

TESForm* TESForm::CreateNewForm(TESForm* copyFrom, bool copyAnims, const char* editorID, bool bPersist, UInt32 offset)
{

    TESForm* result = nullptr;
    if (copyFrom)
    {
        result = copyFrom->CloneForm(0);
    }

    if (!result)
    {
        return nullptr;
    }

    result->SetRefID(GetNextFreeFormID(GetFirstFormIDForModIndex(offset)), true);
    if (editorID) {
        result->SetEditorID(editorID);
    }

    if (bPersist) {
        DynamicallyCreatedForms.insert(result->refID);
    }

    if (copyAnims && PluginFunctions::kNVSE) {
        PluginFunctions::CopyAnimationsToForm(copyFrom, result);
    }

    return result;
}