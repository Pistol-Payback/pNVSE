#include "netimmerse.h"

__declspec(naked) void __fastcall NiObjectNET::SetName(const char* newName)
{
	__asm
	{
		push	ecx
		push	edx
		call	GetNiFixedString
		pop		ecx
		pop		ecx
		mov		edx, [ecx + 8]
		cmp		eax, edx
		jz		decCount
		test	edx, edx
		jz		noCurrName
		lock dec dword ptr[edx - 8]
		noCurrName:
		mov[ecx + 8], eax
			retn
			decCount :
		test	eax, eax
			jz		done
			lock dec dword ptr[eax - 8]
			done :
			retn
	}
}

__declspec(naked) void NiAVObject::Update()
{
	__asm
	{
		push	ecx
		push	0
		push	offset kNiUpdateData
		mov		eax, [ecx]
		call	dword ptr[eax + 0xA4]
		pop		ecx
		mov		ecx, [ecx + 0x18]
		test	ecx, ecx
		jz		done
		mov		eax, [ecx]
		call	dword ptr[eax + 0xFC]
		done:
		retn
	}
}

__declspec(naked) UInt32 NiAVObject::GetIndex() const
{
	__asm
	{
		mov		eax, [ecx + 0x18]
		test	eax, eax
		jz		done
		mov		edx, [eax + 0xA0]
		movzx	eax, word ptr[eax + 0xA6]
		test	eax, eax
		jz		done
		sub		edx, 4
		ALIGN 16
		iterHead:
		cmp[edx + eax * 4], ecx
			jz		done
			dec		eax
			jnz		iterHead
			done :
		retn
	}
}

__declspec(naked) bool NiAVObject::ReplaceObject(NiAVObject* object)
{
	__asm
	{
		mov		edx, [ecx + 0x18]
		test	edx, edx
		jz		done
		mov		eax, [edx + 0xA0]
		movzx	edx, word ptr[edx + 0xA6]
		ALIGN 16
		iterHead:
		dec		edx
			js		done
			cmp[eax + edx * 4], ecx
			jnz		iterHead
			lea		eax, [eax + edx * 4]
			mov		edx, [esp + 4]
			push	edx
			push	eax
			mov		eax, [ecx + 0x18]
			and dword ptr[ecx + 0x18], 0
			mov[edx + 0x18], eax
			call	NiReplaceObject
			mov		al, 1
			retn	4
			ALIGN 16
			done:
		xor al, al
			retn	4
	}
}

__declspec(naked) void NiAVObject::AssignGeometryProps()
{
	__asm
	{
		push	0
		push	0
		push	0
		push	ecx
		CALL_EAX(0xA5A040)
		CALL_EAX(0xB57BD0)
		add		esp, 0x10
		retn
	}
}

__declspec(naked) void NiAVObject::ExportToFile(const char* filePath)
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		sub		esp, 0x5C8
		push	esi
		mov		esi, ecx
		lea		ecx, [esp + 4]
		CALL_EAX(0xA66150)
		push	esi
		mov		esi, eax
		mov		ecx, eax
		CALL_EAX(0xA66370)
		push	dword ptr[ebp + 8]
		mov		ecx, esi
		CALL_EAX(0xA640B0)
		mov		ecx, esi
		CALL_EAX(0xA65300)
		pop		esi
		leave
		retn	4
	}
}

__declspec(naked) NiAVObject* NiAVObject::CreateCopy()
{
	__asm
	{
		push	esi
		push	edi
		mov		esi, ecx
		call	GetNiObjectCopyInfo
		mov		edi, eax
		push	eax
		mov		ecx, esi
		mov		eax, [ecx]
		call	dword ptr[eax + 0x48]
		push	edi
		mov		ecx, esi
		mov		esi, eax
		mov		eax, [ecx]
		call	dword ptr[eax + 0x68]
		and byte ptr[esi + 0x30], 0xFE
		movss	xmm0, PS_V3_One
		movups[esi + 0x34], xmm0
		movups[esi + 0x44], xmm0
		movups[esi + 0x54], xmm0
		mov		ecx, [edi]
		call	NiTMap<UInt32, UInt32>::FreeBuckets
		mov		ecx, [edi + 4]
		call	NiTMap<UInt32, UInt32>::FreeBuckets
		mov		eax, esi
		pop		edi
		pop		esi
		retn
	}
}