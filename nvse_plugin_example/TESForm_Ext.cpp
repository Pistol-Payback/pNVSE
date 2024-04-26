#pragma once
#include <ppNVSE.h>

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

bool TESForm::IsBaseForm()
{
	return !((*(UInt32**)this)[0x3C] == kAddr_ReturnTrue);
}

bool TESForm::IsReference()
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
