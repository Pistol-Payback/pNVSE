#include "WeaponSmith.h"

//Is this even used?
Instance* TESObjectREFR::GetWeaponBase() {

	if (InstanceLinker[this->baseForm->typeID].find(this->baseForm->refID) != InstanceLinker[this->baseForm->typeID].end()) {
		return InstanceLinker[this->baseForm->typeID][this->baseForm->refID];
	}
	return nullptr;
}

UInt32 TESObjectWEAP::GetModdedClipSize(UInt32 modFlags) const {
	UInt32 extendedMag = 0;
	if (modFlags != 0) {
		for (UInt32 iSlot = 0; iSlot < 3; ++iSlot) {
			if (modFlags & (1 << iSlot)) {
				if (effectMods[iSlot] == kWeaponModEffect_IncreaseClipCapacity) {
					extendedMag += value1Mod[iSlot];
				}
			}
		}
	}
	return clipRounds.clipRounds + extendedMag;
}

UInt8 TESObjectREFR::GetWeaponModFlags() {

	ExtraDataList* xData = &this->extraDataList;
	if (xData) {
		ExtraWeaponModFlags* xWeaponModFlags = static_cast<ExtraWeaponModFlags*>(xData->GetByType(kExtraData_WeaponModFlags));
		if (xWeaponModFlags) {
			return xWeaponModFlags->flags; 
		}
	}

	return 0;  // Return 0 if no mod flags are found
}


void TESObjectREFR::SetWeaponModFlags(UInt8 flags)
{

	InventoryRef* invRef = InventoryRef::InventoryRefGetForID(this->refID);
	ExtraDataList* xData = invRef ? invRef->data.xData : &this->extraDataList;
	if (xData) {

		ExtraWeaponModFlags* xWeaponModFlags = static_cast<ExtraWeaponModFlags*>(xData->GetByType(kExtraData_WeaponModFlags));

		// Modify existing flags
		if (xWeaponModFlags) {
			if (flags) {
				xWeaponModFlags->flags = (UInt8)flags;
			}
			else {
				xData->Remove(xWeaponModFlags, true);
			}

		} // Create new extra data
		else if (flags) {

			if (invRef) {
				if (invRef->GetCount() > 1) {
					xData = invRef->SplitFromStack(1);
				}
				xData->Add(ExtraWeaponModFlags::Create(flags));

			}
			else {
				xData->Add(ExtraWeaponModFlags::Create(flags));
			}

		}

	}

}

TESForm* TESObjectREFR::GetLocation(){

	TESObjectCELL* cell = this->parentCell;
	if (cell) {
		TESForm* location = cell->worldSpace;
		if (!location) {
			location = cell;
		}
		return location;
	}
	else if(ExtraPersistentCell* xData = (ExtraPersistentCell*)this->extraDataList.GetByType(kXData_ExtraPersistentCell)) {
		
		TESForm* location = xData->persistentCell->worldSpace;
		if (!location) {
			location = xData->persistentCell;
		}
		return location;

	}
	return nullptr;

}

std::pair<TESAmmo*, UInt32> TESObjectREFR::FindAmmoType(TESObjectWEAP* weapon) {

	if (!weapon) return { nullptr, 0 };

	TESForm* ammo = weapon->ammo.ammo;
	if (!ammo) return { nullptr, 0 };

	UInt32 count = this->GetItemCount(ammo); // Get the count here for efficiency
	if (ammo->typeID == kFormType_TESAmmo) {
		return (count > 0) ? std::make_pair((TESAmmo*)ammo, count) : std::make_pair(nullptr, 0);
	}
	BGSListForm* ammoList = static_cast<BGSListForm*>(ammo);
	if (ammoList) {
		for (auto iter = ammoList->list.Head(); iter; iter = iter->next) {
			TESForm* entry = iter->data;
			if (entry && entry->typeID == kFormType_TESAmmo) {
				count = this->GetItemCount(entry); // Reuse the variable to get each entry's count
				if (count > 0) {
					return { static_cast<TESAmmo*>(entry), count }; // Return the entry with its count
				}
			}
		}
	}

	return { nullptr, 0 }; // Return null with zero count if no valid ammo is found
}

TESAmmo* TESObjectREFR::CycleAmmoType(TESObjectWEAP* weapon, TESAmmo* currentAmmo) {

	TESForm* ammo = weapon->ammo.ammo;
	if (!ammo || ammo->typeID != kFormType_BGSListForm) {
		return nullptr; //No ammo to cycle
	}

	BGSListForm* ammoList = static_cast<BGSListForm*>(ammo);
	if (!ammoList) return nullptr;

	ListNode<TESForm>* iter = ammoList->list.Head();
	ListNode<TESForm>* iterPos = nullptr;
	bool foundCurrent = false;

	//Find current ammo
	while (iter) {
		if (iter->data == currentAmmo) {
			iterPos = iter;  // Mark the start position
			foundCurrent = true;
			break;
		}
		iter = iter->next;
	}
	//Find next ammo
	if (foundCurrent) {
		do {
			iter = iter->next ? iter->next : ammoList->list.Head(); // Move to next or wrap around
			if (iter->data->typeID == kFormType_TESAmmo && this->GetItemCount(iter->data) > 0) {
				return static_cast<TESAmmo*>(iter->data);
			}
		} while (iter != iterPos);
	}

	return nullptr;
}

TESForm* TESObjectREFR::GetBaseObject() const
{
	TESForm* form = this->baseForm;

	if (form->IsInstancedForm()) {

		form = form->GetStaticParent();
		if (form) {
			return form;
		}

	}
	return this->baseForm;
}