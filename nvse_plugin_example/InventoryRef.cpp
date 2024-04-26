#include "InventoryRef.h"

	ExtraDataList* __fastcall InventoryRef::SplitFromStack(SInt32 maxStack)
	{
		if (auto* xCount = (ExtraCount*)data.xData->GetByType(kExtraData_Count))
		{
			SInt32 delta = xCount->count - maxStack;
			if (delta <= 0)
				return data.xData;
			ExtraContainerChanges::EntryData* pEntry = containerRef->GetContainerChangesEntry(data.type);
			if (!pEntry) return data.xData;
			ExtraDataList* xDataOut = data.xData->CreateCopy();
			pEntry->extendData->Prepend(xDataOut);
			xDataOut->RemoveByType(kExtraData_Worn);
			xDataOut->RemoveByType(kExtraData_Hotkey);
			if (delta < 2)
			{
				data.xData->RemoveByType(kExtraData_Count);
				if (!data.xData->m_data)
				{
					pEntry->extendData->Remove(data.xData);
					Game_HeapFree(data.xData);
				}
			}
			else xCount->count = delta;
			data.xData = xDataOut;
			if (maxStack > 1)
			{
				xCount = (ExtraCount*)data.xData->GetByType(kExtraData_Count);
				xCount->count = maxStack;
			}
			else data.xData->RemoveByType(kExtraData_Count);
		}
		else if (maxStack > 1)
			data.xData->AddExtraCount(maxStack);
		return data.xData;
	}

	InventoryRef::InventoryRefCreateFunc InventoryRef::InventoryRefCreate = nullptr;
	InventoryRef::InventoryRefCreateFunc InventoryRef::InventoryRefCreateEntry = nullptr;
	InventoryRef::InventoryRefGetForIDFunc InventoryRef::InventoryRefGetForID = nullptr;

	void InventoryRef::InitInventoryRefFunct(const NVSEInterface* nvse)
	{
		NVSEDataInterface* nvseData = (NVSEDataInterface*)nvse->QueryInterface(kInterface_Data);

		InventoryRefCreate = (InventoryRefCreateFunc)nvseData->GetFunc(NVSEDataInterface::kNVSEData_InventoryReferenceCreate);
		InventoryRefCreateEntry = (InventoryRefCreateFunc)nvseData->GetFunc(NVSEDataInterface::kNVSEData_InventoryReferenceCreateEntry);
		InventoryRefGetForID = (InventoryRefGetForIDFunc)nvseData->GetFunc(NVSEDataInterface::kNVSEData_InventoryReferenceGetForRefID);


	}
