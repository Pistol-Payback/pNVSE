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

    //Extern Functions in ppNVSE.h
   /*
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
    */
    void (*SetDescriptionJIP)(TESDescription* description, const char* altText);

    HMODULE m_JIP = GetModuleHandle("jip_nvse.dll");
    bool JIP = false;

    void initJIP()
    {

        if (m_JIP != NULL) {
            /*
            AuxVarGetSize = (int (*)(AuxVarInfo * varInfo))GetProcAddress(m_JIP, MAKEINTRESOURCEA(12));
            AuxVarGetType = (int (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(m_JIP, MAKEINTRESOURCEA(13));
            AuxVarGetFloat = (double (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(m_JIP, MAKEINTRESOURCEA(14));
            AuxVarSetFloat = (void (*)(double fltVal, AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(m_JIP, MAKEINTRESOURCEA(15));
            AuxVarGetRef = (UInt32(*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(m_JIP, MAKEINTRESOURCEA(16));
            AuxVarSetRef = (void (*)(TESForm * refVal, AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(m_JIP, MAKEINTRESOURCEA(17));
            AuxVarGetString = (char* (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(m_JIP, MAKEINTRESOURCEA(18));
            AuxVarSetString = (void (*)(const char* buffer, AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(m_JIP, MAKEINTRESOURCEA(19));
            CreateAuxVarInfo = (AuxVarInfo(*)(TESForm * form, TESObjectREFR * thisObj, char* pVarName))GetProcAddress(m_JIP, MAKEINTRESOURCEA(20));
            AuxVarErase = (void (*)(AuxVarInfo * varInfo, SInt32 idx))GetProcAddress(m_JIP, MAKEINTRESOURCEA(21));
            */
            SetDescriptionJIP = (void (*)(TESDescription * description, const char* altText))GetProcAddress(m_JIP, MAKEINTRESOURCEA(12));

            if (/*AuxVarGetSize == NULL || AuxVarGetType == NULL || AuxVarGetFloat == NULL ||
                AuxVarSetFloat == NULL || AuxVarGetRef == NULL || AuxVarSetRef == NULL ||
                AuxVarGetString == NULL || AuxVarSetString == NULL || CreateAuxVarInfo == NULL || AuxVarErase == NULL ||*/ 
                SetDescriptionJIP == NULL) {
                gLog.Message("One or more function pointers from JIP are invalid");
            }
            else {
                gLog.Message("All JIP Functions Valid");
                JIP = true;
            }

        }
        else {
            gLog.Message("JIP not installed");
        }

    }

    //Extern Functions in ppNVSE.h
    void (*OverrideFormAnimation)(const TESForm* form, const char* path, bool firstPerson, bool enable, Script* conditionScript, bool pollCondition);
    bool (*CopyAnimationsToForm)(TESForm* fromForm, TESForm* toForm);
    bool (*RemoveFormAnimations)(TESForm* form);

    //Wrapper function with build in validation
    void c_RemoveFormAnimations(TESForm* form) {
        if (PluginFunctions::kNVSE) {
            PluginFunctions::RemoveFormAnimations(form);
        }
    }

    HMODULE m_kNVSE = GetModuleHandle("kNVSE.dll");
    bool kNVSE = false;

    void init_kNVSE()
    {

        if (m_kNVSE != NULL) {

            OverrideFormAnimation = (void (*)(const TESForm * form, const char* path, bool firstPerson, bool enable, Script *conditionScript, bool pollCondition))GetProcAddress(m_kNVSE, MAKEINTRESOURCEA(3));

            CopyAnimationsToForm = (bool (*)(TESForm * fromForm, TESForm * toForm))GetProcAddress(m_kNVSE, MAKEINTRESOURCEA(4));
            RemoveFormAnimations = (bool (*)(TESForm * form))GetProcAddress(m_kNVSE, MAKEINTRESOURCEA(5));

            if (OverrideFormAnimation == NULL || CopyAnimationsToForm == NULL || RemoveFormAnimations == NULL) {
                gLog.Message("One or more function pointers from kNVSE are invalid");
            }
            else {
                gLog.Message("All kNVSE Functions Valid");
                kNVSE = true;
            }

        }
        else {
            gLog.Message("kNVSE not installed");
        }

    }

}