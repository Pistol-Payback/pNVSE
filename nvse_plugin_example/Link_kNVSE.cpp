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

    int (*AuxVarGetSize)(AuxVarInfo* varInfo);
    int (*AuxVarGetType)(AuxVarInfo* varInfo, SInt32 idx);

    double (*AuxVarGetFloat)(AuxVarInfo* varInfo, SInt32 idx);
    void (*AuxVarSetFloat)(double fltVal, AuxVarInfo* varInfo, SInt32 idx);

    UInt32(*AuxVarGetRef)(AuxVarInfo* varInfo, SInt32 idx);
    void (*AuxVarSetRef)(TESForm* refVal, AuxVarInfo* varInfo, SInt32 idx);

    char* (*AuxVarGetString)(AuxVarInfo* varInfo, SInt32 idx);
    void (*AuxVarSetString)(const char* buffer, AuxVarInfo* varInfo, SInt32 idx);

    AuxVarInfo(*CreateAuxVarInfo)(TESForm* form, TESObjectREFR* thisObj, char* pVarName);
    void (*AuxVarErase)(AuxVarInfo* varInfo, SInt32 idx);

    void (*SetDescriptionJIP)(TESDescription* description, const char* altText);

    HMODULE JIP = GetModuleHandle("jip_nvse.dll");

    void initJIP()
    {

        //Console_Print("Running initJIP");
        if (JIP != NULL) {


            AuxVarGetSize = (int (*)(AuxVarInfo * varInfo))GetProcAddress(JIP, MAKEINTRESOURCEA(12));
            AuxVarGetType = (int (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(13));
            AuxVarGetFloat = (double (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(14));
            AuxVarSetFloat = (void (*)(double fltVal, AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(15));
            AuxVarGetRef = (UInt32(*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(16));
            AuxVarSetRef = (void (*)(TESForm * refVal, AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(17));
            AuxVarGetString = (char* (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(18));
            AuxVarSetString = (void (*)(const char* buffer, AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(19));
            CreateAuxVarInfo = (AuxVarInfo(*)(TESForm * form, TESObjectREFR * thisObj, char* pVarName))GetProcAddress(JIP, MAKEINTRESOURCEA(20));
            AuxVarErase = (void (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(JIP, MAKEINTRESOURCEA(21));

            //SetDescriptionJIP = (void (*)(TESDescription * description, const char* altText))GetProcAddress(JIP, MAKEINTRESOURCEA(25));
            SetDescriptionJIP = (void (*)(TESDescription * description, const char* altText))GetProcAddress(JIP, "SetDescriptionAltText");

            if (AuxVarGetSize == NULL || AuxVarGetType == NULL || AuxVarGetFloat == NULL ||
                AuxVarSetFloat == NULL || AuxVarGetRef == NULL || AuxVarSetRef == NULL ||
                AuxVarGetString == NULL || AuxVarSetString == NULL || CreateAuxVarInfo == NULL || AuxVarErase == NULL || 
                SetDescriptionJIP == NULL) {
                gLog.Message("One or more function pointers are NULL");
            }
            else {
                gLog.Message("All Functions Valid");
            }

        }
        else {
            gLog.Message("JIP NULL");
        }

    }
 
}