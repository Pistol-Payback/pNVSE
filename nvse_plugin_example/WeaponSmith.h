#pragma once
#include "ppNVSE.h"
#include "EventHandlers.h"

struct ExtendedBaseType;
struct StaticInstance;
struct Instance;
struct StaticInstance_Akimbo;
struct Instance_Akimbo;
struct TESRefr;

extern std::unordered_map<UInt32, std::unordered_map<UInt32, StaticInstance*>> StaticLinker;
extern std::unordered_map<UInt32, std::unordered_map<UInt32, Instance*>> InstanceLinker;
extern std::unordered_map<UInt32, std::unordered_map<UInt32, TESRefr*>> TESRefLinker;

extern std::unordered_map<UInt32, std::unordered_map<UInt32, StaticInstance_Akimbo*>> AkimboSets;

using TraitMap = std::unordered_map<std::string, AuxVector>;
using LinkedTraitMap = std::unordered_map<ExtendedBaseType*, TraitMap>;
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

class LifecycleManager {
public:
    enum DestructionPolicy {
        Timed = 1,
        OnUnload = 2,
        Immediate = 4,
        None = 0
    };

    LifecycleManager(int policies = None, double lifetime = 0.0)
        : policies(policies), lifetime(lifetime), timeElapsed(0.0) {}

    void enablePolicy(DestructionPolicy policy) {
        policies |= policy;
    }

    void disablePolicy(DestructionPolicy policy) {
        policies &= ~policy;
    }

    bool isPolicyEnabled(DestructionPolicy policy) const {
        return (policies & policy) != 0;
    }

    void update(double deltaTime) {
        if (policies & Timed) {
            timeElapsed += deltaTime;
            if (timeElapsed >= lifetime) {
                requestDestruction();
            }
        }
    }

    void unload() {
        if (policies & OnUnload) {
            requestDestruction();
        }
    }

    void requestImmediateDestruction() {
        if (policies & Immediate) {
            requestDestruction();
        }
    }

    void setSaveBehavior(bool save) {
        saveBehavior = save;
    }

    void requestDestruction() {
        if (destroyCallback) {
            destroyCallback();
        }
    }

    void onDestruction(std::function<void()> callback) {
        destroyCallback = callback;
    }

    bool saveBehavior = true;

private:
    int policies;
    double lifetime;
    double timeElapsed;
    std::function<void()> destroyCallback;
};

struct ExtendedBaseType {

protected:

    ExtendedBaseType(
        UInt32 modIndex = 0,
        UInt32 extendedType = 0,
        const TraitMap& traits = {},
        const SlotMap& linkedTraits = {},
        const InstanceVector& aInstances = {}
    ) : edits({ modIndex }), extendedType(extendedType), traits(traits), linkedTraits(linkedTraits), aInstances(aInstances) {}

public:

    ~ExtendedBaseType() {
        clearInstances();
    }

    LifecycleManager baseLifecycle; //All instances will inherit this

    InstanceVector aInstances;

    TraitMap traits;
    SlotMap linkedTraits;

    UInt32 extendedType;
    std::vector<UInt32> edits;   //All the mod indexes that touch this form

    AuxVector* SetBaseTrait(const std::string& sTrait); //Returns base traits, and creates them if they don't exist.
    AuxVector* GetBaseTrait(const std::string& sTrait); //Returns base traits.
    void EraseBaseTrait(const std::string& sTrait); //Deletes base traits.

    AuxVector* GetTrait(const std::string& trait, ExtendedBaseType* linkedObj = nullptr, const std::string& sSlot = "null", UInt8 priorityFlag = 0);
    AuxVector* SetTrait(const std::string& trait, ExtendedBaseType* linkedObj = nullptr, const std::string& sSlot = "null", UInt8 priorityFlag = 0);
    AuxVector* GetLinkedTrait(const std::string& trait, ExtendedBaseType* linkedObj, const std::string& sSlot = "null");
    AuxVector* SetLinkedTrait(const std::string& trait, ExtendedBaseType* linkedObj, const std::string& sSlot = "null");

    void MarkAsEdit(UInt32 modIndex);

    //Destroy all instances in aInstances
    void clearInstances();

};

struct StaticInstance : ExtendedBaseType {

    StaticInstance(
        TESForm* parent = nullptr,
        UInt32 modIndex = 0,
        UInt32 extendedType = 0,
        const TraitMap& traits = {},
        const SlotMap& linkedTraits = {},
        const InstanceVector& aInstances = {}
    ) : ExtendedBaseType(modIndex, extendedType, traits, linkedTraits, aInstances), parent(parent) {

        StaticLinker[this->parent->typeID][this->parent->refID] = this;

    }

    ~StaticInstance() {
        StaticLinker[this->parent->typeID].erase(this->parent->refID);
        //Shouldn't be destroying these
    }

    TESForm* parent;

    virtual Instance* newInstance(const std::string& key);
    virtual Instance* loadInstance(UInt32 InstID, const std::string& key);

};

struct StaticInstance_WEAP : StaticInstance {

    StaticInstance_WEAP(
        TESForm* parent = nullptr,
        UInt32 modIndex = 0,
        UInt32 extendedType = 40,
        const std::unordered_map<std::string, UInt32>& aBaseAttachments = {},
        const TraitMap& traits = {},
        const SlotMap& linkedTraits = {},
        const InstanceVector& aInstances = {}
    ) : StaticInstance(parent, modIndex, extendedType, traits, linkedTraits, aInstances), aBaseAttachments(aBaseAttachments) {}

    std::unordered_map<std::string, UInt32> aBaseAttachments;
    std::unordered_map<std::string, std::vector<UInt32>> aAllAttachments;

    std::string FirstPersonModelPath;

    Instance* newInstance(const std::string& key) override final;
    Instance* loadInstance(UInt32 InstID, const std::string& key) override final;

};


struct StaticInstance_Akimbo : ExtendedBaseType {

    StaticInstance_Akimbo(
        UInt32 modIndex = 0,
        StaticInstance_WEAP* left = nullptr,
        StaticInstance_WEAP* right = nullptr,
        const TraitMap& traits = {},
        const SlotMap& linkedTraits = {},
        const InstanceVector& aInstances = {}
    ) : ExtendedBaseType(modIndex, 222, traits, linkedTraits, aInstances), left(left), right(right) {

        AkimboSets[this->left->parent->refID][this->right->parent->refID] = this;

    }

    ~StaticInstance_Akimbo() {
        AkimboSets[this->left->parent->refID].erase(this->right->parent->refID);
        if (AkimboSets[this->left->parent->refID].empty()) {
            AkimboSets.erase(this->left->parent->refID);
        }
        //Shouldn't be destroying these
    }

    StaticInstance_WEAP* left;
    StaticInstance_WEAP* right;

    TESForm* newInstance(TESObjectREFR* right, TESObjectREFR* left, const std::string& key);

    Instance_Akimbo* loadInstance(UInt16 InstID, 
        Instance_WEAP* weapRight, Instance_WEAP* weapLeft, 
        ExtraDataList* xDataRight, ExtraDataList* xDataLeft, 
        const std::string& key);

    static StaticInstance_Akimbo* LookupAkimboSet(TESForm* left, TESForm* right);

};

struct TESRefr {

    TESRefr(ExtendedBaseType* baseInstance)
        : TESRefr(baseInstance, baseInstance->baseLifecycle) {}

    TESRefr(ExtendedBaseType* baseInstance, const LifecycleManager& lifecycle)
        : baseInstance(baseInstance), lifecycle(lifecycle) {
    }

    LifecycleManager lifecycle;
    ExtendedBaseType* baseInstance;

};

struct Instance : TESRefr {

    Instance(ExtendedBaseType* baseInstance, TESForm* clone, std::string key)
        : Instance(baseInstance, clone, key, baseInstance->baseLifecycle) {}

    Instance(ExtendedBaseType* baseInstance, TESForm* clone, std::string key, const LifecycleManager& lifecycle)
        : TESRefr(baseInstance, lifecycle), clone(clone), key(key) {
        InstID = baseInstance->aInstances.add(this);
        ++InstanceInterface::cloneCount;
    }

    Instance(UInt16 InstID, ExtendedBaseType* baseInstance, TESForm* clone, std::string key)
        : Instance(InstID, baseInstance, clone, key, baseInstance->baseLifecycle) {}

    Instance(UInt16 InstID, ExtendedBaseType* baseInstance, TESForm* clone, std::string key, const LifecycleManager& lifecycle)
        : TESRefr(baseInstance, lifecycle), clone(clone), key(key), InstID(InstID) {
        baseInstance->aInstances[InstID] = this;
        ++InstanceInterface::cloneCount;
    }

    virtual ~Instance() {
        clone->Destroy(0);
    }

    UInt16 InstID;
    TESForm* clone;
    std::string key;

   //Use if deleting single instances
   void destroy();

};

struct TESInstance : Instance {

    TESInstance(
        StaticInstance* baseInstance,
        TESForm* clone,
        std::string key
    ) : Instance(baseInstance, clone, key) {
        InstanceLinker[this->clone->typeID][this->clone->refID] = this;
    }

    TESInstance(
        UInt16 InstID,
        StaticInstance* baseInstance,
        TESForm* clone,
        std::string key
    ) : Instance(InstID, baseInstance, clone, key) {
        InstanceLinker[this->clone->typeID][this->clone->refID] = this;
    }

    ~TESInstance() override {
        InstanceLinker[this->clone->typeID].erase(this->clone->refID);
        clone->Destroy(0);
    }

};

struct Instance_WEAP : TESInstance {

    Instance_WEAP(
        StaticInstance* baseInstance,
        TESForm* clone,
        std::string key,
        const std::unordered_map<std::string, UInt32>& aAttachments = {}
    ) : TESInstance(baseInstance, clone, key), aAttachments(aAttachments){}

    Instance_WEAP(
        UInt16 InstID,
        StaticInstance* baseInstance,
        TESForm* clone,
        std::string key,
        const std::unordered_map<std::string, UInt32>& aAttachments = {}
    ) : TESInstance(InstID, baseInstance, clone, key), aAttachments(aAttachments) {}

    std::unordered_map<std::string, UInt32> aAttachments;

};

struct Instance_Akimbo : Instance {

    Instance_Akimbo(
        ExtendedBaseType* baseInstance,
        TESForm* clone,
        std::string key,
        Instance_WEAP* left,
        Instance_WEAP* right,
        ExtraDataList* xDataLeft,
        ExtraDataList* xDataRight
    ) : Instance(baseInstance, clone, key),
        right(right),
        left(left),
        xDataLeft(xDataLeft),
        xDataRight(xDataRight)
    {
        InstanceLinker[40][this->clone->refID] = this;
    }

    Instance_Akimbo(
        UInt16 InstID,
        ExtendedBaseType* baseInstance,
        TESForm* clone,
        std::string key,
        Instance_WEAP* left,
        Instance_WEAP* right,
        ExtraDataList* xDataLeft,
        ExtraDataList* xDataRight
    ) : Instance(InstID, baseInstance, clone, key),
        right(right),
        left(left),
        xDataLeft(xDataLeft),
        xDataRight(xDataRight)
    {
        InstanceLinker[40][this->clone->refID] = this;
    }

    ~Instance_Akimbo() override final {
        InstanceLinker[40].erase(this->clone->refID);
        clone->Destroy(0);
    }

    Instance_WEAP* right;   //SetAkimboWeapon
    Instance_WEAP* left;
    ExtraDataList* xDataRight;
    ExtraDataList* xDataLeft;

};
