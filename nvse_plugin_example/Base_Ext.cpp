#pragma once
#include "Base_Ext.h"

UInt32 GetFirstFormIDForModIndex(UInt32 modIndex) {

	if (modIndex > 0xFF) {
		return 0;
	}

	UInt32 firstFormID = (modIndex << 24); // Shift the mod index by 24 bits to the left, leaving the index at the begining
	return firstFormID;

}