//#pragma once
#include "ppNVSE.h"

namespace PluginFunctions {

	//GetSharedMapFunc getSharedMapFunc = NULL;
	//HINSTANCE hDLL = LoadLibrary(TEXT("kNVSE.dll"));

	//void initkNVSE()
	//{
		//if (hDLL != NULL) {

			//getSharedMapFunc = reinterpret_cast<GetSharedMapFunc>(GetProcAddress(hDLL, "GetSharedMap"));

			//FreeLibrary(hDLL);


		//}
	//}

	//auto JIP = GetModuleHandle("JIP LN NVSE.dll");
    //HMODULE JIP = GetModuleHandle("jip_nvse.dll");
    /*
    void initJIP()
    {
        int (*AuxVarGetSize)(AuxVarInfo * varInfo);
        int (*AuxVarGetType)(AuxVarInfo * varInfo, SInt32 idx);

        double (*AuxVarGetFloat)(AuxVarInfo * varInfo, SInt32 idx);
        void (*AuxVarSetFloat)(double fltVal, AuxVarInfo * varInfo, SInt32 idx);

        UInt32(*AuxVarGetRef)(AuxVarInfo * varInfo, SInt32 idx);
        void (*AuxVarSetRef)(TESForm * refVal, AuxVarInfo * varInfo, SInt32 idx);

        char* (*AuxVarGetString)(AuxVarInfo * varInfo, SInt32 idx);
        void (*AuxVarSetString)(const char* buffer, AuxVarInfo * varInfo, SInt32 idx);

        AuxVarInfo(*CreateAuxVarInfo)(TESForm * form, TESObjectREFR * thisObj, UInt32 modIndex, char* pVarName);
        void (*AuxVarErase)(AuxVarInfo * varInfo, SInt32 idx);

        if (JIP != NULL) {

            AuxVarGetSize = (int (*)(AuxVarInfo * varInfo))GetProcAddress(JIP, MAKEINTRESOURCEA(12));
            AuxVarGetType = (int (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(13));
            AuxVarGetFloat = (double (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(14));
            AuxVarSetFloat = (void (*)(double fltVal, AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(15));
            AuxVarGetRef = (UInt32(*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(16));
            AuxVarSetRef = (void (*)(TESForm * refVal, AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(17));
            AuxVarGetString = (char* (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(18));
            AuxVarSetString = (void (*)(const char* buffer, AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(19));

            CreateAuxVarInfo = (AuxVarInfo(*)(TESForm * form, TESObjectREFR * thisObj, UInt32 modIndex, char* pVarName))GetProcAddress(JIP, MAKEINTRESOURCEA(20));
            AuxVarErase = (void (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(21));
        }
    }
    */
}