#pragma once
#include "ppNVSE.h"
#include "EventHandlers.h"

extern std::unordered_map<UInt32, std::unordered_map<UInt32, StaticInstance*>> StaticLinker;
extern std::unordered_map<UInt32, std::unordered_map<UInt32, Instance*>> InstanceLinker;

using TraitMap = std::unordered_map<std::string, AuxVector>;

using LinkedTraitMap = std::unordered_map<StaticInstance*, TraitMap>;

using SlotMap = std::unordered_map<std::string, LinkedTraitMap>;

struct InstanceInterface {
    static UInt32 cloneCount;
};

struct Instance;
struct StaticInstance;

class InstanceVector : public std::vector<Instance*> {

public:
    using std::vector<Instance*>::vector;  // Inherit constructors

    ~InstanceVector();

    UInt32 add(Instance* newInstance);
    void remove(UInt8 instID);
    void markForDelete(UInt8 instID);

    //Not used atm
    void cleanUp();

};

struct StaticInstance {

    StaticInstance(
        TESForm* parent = nullptr,
        UInt32 modIndex = 0,
        const std::unordered_map<std::string, AuxVector>& traits = {},
        const InstanceVector& aInstances = {}
    ) : parent(parent), edits({ modIndex }), traits(traits), aInstances(aInstances) {

        StaticLinker[this->parent->typeID][this->parent->refID] = this;

    }

    ~StaticInstance() {
        StaticLinker[this->parent->typeID].erase(this->parent->refID);
        //Shouldn't be destroying these
    }

    TESForm* parent;

    InstanceVector aInstances;

    TraitMap traits;
    SlotMap linkedTraits;

    std::vector<UInt32> edits;   //All the mod indexes that touch this form

    AuxVector* SetBaseTrait(const std::string& sTrait); //Returns base traits, and creates them if they don't exist.
    AuxVector* GetBaseTrait(const std::string& sTrait); //Returns base traits.
    void EraseBaseTrait(const std::string& sTrait); //Returns base traits.

    void MarkAsEdit(UInt32 modIndex);

    virtual AuxVector* GetTrait(const std::string& trait, StaticInstance* linkedObj = nullptr, const std::string& sSlot = "null", UInt8 priorityFlag = 0);
    virtual AuxVector* SetTrait(const std::string& trait, StaticInstance* linkedObj = nullptr, const std::string& sSlot = "null", UInt8 priorityFlag = 0);
    virtual AuxVector* GetLinkedTrait(const std::string& trait, StaticInstance* linkedObj, const std::string& sSlot = "null");
    virtual AuxVector* SetLinkedTrait(const std::string& trait, StaticInstance* linkedObj, const std::string& sSlot = "null");

    virtual Instance* create(std::string key);

};

struct StaticInstance_WEAP : StaticInstance {

    StaticInstance_WEAP(
        TESForm* parent = nullptr,
        UInt32 modIndex = 0,
        const std::unordered_map<std::string, UInt32>& aBaseAttachments = {},
        const std::unordered_map<std::string, AuxVector>& traits = {},
        const InstanceVector& aInstances = {}
    ) : StaticInstance(parent, modIndex, traits, aInstances), aBaseAttachments(aBaseAttachments) {}

    std::unordered_map<std::string, UInt32> aBaseAttachments;
    std::unordered_map<std::string, std::vector<UInt32>> aAllAttachments;

    std::string FirstPersonModelPath;

    Instance* create(std::string key) override;

};

struct Instance {

    Instance(
        StaticInstance* baseInstance,
        TESForm* clone,
        std::string key
    ) : baseInstance(baseInstance), clone(clone), key(key) {

        InstID = baseInstance->aInstances.add(this);
        InstanceLinker[this->clone->typeID][this->clone->refID] = this;
        ++InstanceInterface::cloneCount;

    }

    ///baseInstance->aInstances.remove(InstID); needs to be removed manually, or clear()
    ~Instance() {

        InstanceLinker[this->clone->typeID].erase(this->clone->refID);
        clone->Destroy(0);

    }

    void destroy();

    UInt8 InstID;
    TESForm* clone;
    std::string key;
    StaticInstance* baseInstance;

};

struct Instance_WEAP : Instance {

    Instance_WEAP(
        StaticInstance* baseInstance,
        TESForm* clone,
        std::string key,
        const std::unordered_map<std::string, UInt32>& aAttachments = {}
    ) : Instance(baseInstance, clone, key), aAttachments(aAttachments) {}

    std::unordered_map<std::string, UInt32> aAttachments;

};
