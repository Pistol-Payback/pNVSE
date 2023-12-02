#pragma once

typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned long UInt32;
typedef unsigned long long UInt64;
typedef signed char SInt8;
typedef signed short SInt16;
typedef signed long SInt32;
typedef signed long long SInt64;

#define CALL_EAX(addr) __asm mov eax, addr __asm call eax
#define JMP_EAX(addr)  __asm mov eax, addr __asm jmp eax
#define JMP_EDX(addr)  __asm mov edx, addr __asm jmp edx

#define DUP_2(a) a a
#define DUP_3(a) a a a
#define DUP_4(a) a a a a

// These are used for 10h aligning segments in ASM code (massive performance gain, particularly with loops).
#define EMIT(bt) __asm _emit 0x ## bt

#define NOP_0x1 EMIT(90)
//	"\x90"
#define NOP_0x2 EMIT(66) NOP_0x1
//	"\x66\x90"
#define NOP_0x3 EMIT(0F) EMIT(1F) EMIT(00)
//	"\x0F\x1F\x00"
#define NOP_0x4 EMIT(0F) EMIT(1F) EMIT(40) EMIT(00)
//	"\x0F\x1F\x40\x00"
#define NOP_0x5 EMIT(0F) EMIT(1F) EMIT(44) EMIT(00) EMIT(00)
//	"\x0F\x1F\x44\x00\x00"
#define NOP_0x6 EMIT(66) NOP_0x5
//	"\x66\x0F\x1F\x44\x00\x00"
#define NOP_0x7 EMIT(0F) EMIT(1F) EMIT(80) EMIT(00) EMIT(00) EMIT(00) EMIT(00)
//	"\x0F\x1F\x80\x00\x00\x00\x00"
#define NOP_0x8 EMIT(0F) EMIT(1F) EMIT(84) EMIT(00) EMIT(00) EMIT(00) EMIT(00) EMIT(00)
//	"\x0F\x1F\x84\x00\x00\x00\x00\x00"
#define NOP_0x9 EMIT(66) NOP_0x8
//	"\x66\x0F\x1F\x84\x00\x00\x00\x00\x00"
#define NOP_0xA EMIT(66) NOP_0x9
//	"\x66\x66\x0F\x1F\x84\x00\x00\x00\x00\x00"
#define NOP_0xB EMIT(66) NOP_0xA
//	"\x66\x66\x66\x0F\x1F\x84\x00\x00\x00\x00\x00"
#define NOP_0xC NOP_0x8 NOP_0x4
#define NOP_0xD NOP_0x8 NOP_0x5
#define NOP_0xE NOP_0x7 NOP_0x7
#define NOP_0xF NOP_0x8 NOP_0x7

#define PS_DUP_1(a)	a, 0UL, 0UL, 0UL
#define PS_DUP_2(a)	a, a, 0UL, 0UL
#define PS_DUP_3(a)	a, a, a, 0UL
#define PS_DUP_4(a)	a, a, a, a

#define UBYT(a) *((UInt8*)&a)
#define USHT(a) *((UInt16*)&a)
#define ULNG(a) *((UInt32*)&a)

#define EMIT_DW(b0, b1, b2, b3) EMIT(b3) EMIT(b2) EMIT(b1) EMIT(b0)
#define EMIT_DW_3(b0, b1, b2) EMIT_DW(00, b0, b1, b2)
#define EMIT_DW_2(b0, b1) EMIT_DW(00, 00, b0, b1)
#define EMIT_DW_1(b0) EMIT_DW(00, 00, 00, b0)
#define EMIT_PS_1(b0, b1, b2, b3) EMIT_DW(b0, b1, b2, b3) DUP_3(EMIT_DW_1(00))
#define EMIT_PS_2(b0, b1, b2, b3) DUP_2(EMIT_DW(b0, b1, b2, b3)) DUP_2(EMIT_DW_1(00))
#define EMIT_PS_3(b0, b1, b2, b3) DUP_3(EMIT_DW(b0, b1, b2, b3)) EMIT_DW_1(00)
#define EMIT_PS_4(b0, b1, b2, b3) DUP_4(EMIT_DW(b0, b1, b2, b3))
#define EMIT_8(b0, b1, b2, b3, b4, b5, b6, b7) EMIT(b0) EMIT(b1) EMIT(b2) EMIT(b3) EMIT(b4) EMIT(b5) EMIT(b6) EMIT(b7)
#define EMIT_4W(b0, b1, b2, b3, b4, b5, b6, b7) EMIT(b1) EMIT(b0) EMIT(b3) EMIT(b2) EMIT(b5) EMIT(b4) EMIT(b7) EMIT(b6)

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret ThisCall(UInt32 _addr, void* _this, Args ...args)
{
	return ((T_Ret(__thiscall*)(void*, Args...))_addr)(_this, std::forward<Args>(args)...);
}