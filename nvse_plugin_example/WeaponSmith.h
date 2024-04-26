#pragma once
#include "ppNVSE.h"
#include "EventHandlers.h"
#include <stdexcept>

extern EventHandler onInstanceDeconstructEvent;
extern EventHandler onInstanceReconstructEvent;

extern EventHandler onAttachWeapModEvent;
extern EventHandler onAttachWeapModReconstructEvent;

extern EventHandler onDetachWeapModEvent;
extern EventHandler onDetachWeapModDeconstructEvent;

struct InstanceInterface {
    static UInt32 cloneCount;
};

class StaticInstance_WEAP {
public:

    StaticInstance_WEAP(
        TESForm* parent = nullptr,
        const std::vector<Instance_WEAP*>& aInstances = {},
        const std::unordered_map<std::string, UInt32>& aBaseAttachments = {}
    ) : parent(parent), aInstances(aInstances), aBaseAttachments(aBaseAttachments) {
        this->Linker[this->parent->refID] = this;
    }

    ~StaticInstance_WEAP() {
    
    }

    TESForm* parent;
    static std::unordered_map<UInt32, StaticInstance_WEAP*> Linker;
    std::vector<Instance_WEAP*> aInstances;
    std::unordered_map<std::string, UInt32> aBaseAttachments;
};

class Instance_WEAP {
public:

    Instance_WEAP(
        StaticInstance_WEAP* baseInstance,
        TESForm* clone,
        UInt8 InstID,
        std::string key,
        const std::unordered_map<std::string, UInt32>& aAttachments = {}
    ) : baseInstance(baseInstance), clone(clone), InstID(InstID), key(key), aAttachments(aAttachments) {

        baseInstance->aInstances.push_back(this);
        this->Linker[this->clone->refID] = this;
        InstanceInterface::cloneCount++;

    }

    ~Instance_WEAP() {

        this->Linker.erase(this->clone->refID);
        --InstanceInterface::cloneCount;

    }

    void destroy();

    static Instance_WEAP* create(StaticInstance_WEAP* staticForm, std::string key);

    UInt8 InstID;
    TESForm* clone;
    std::string key;
    static std::unordered_map<UInt32, Instance_WEAP*> Linker;
    StaticInstance_WEAP* baseInstance;
    std::unordered_map<std::string, UInt32> aAttachments;
};