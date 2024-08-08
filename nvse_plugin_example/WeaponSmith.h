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

struct ExpirationTimer {

    ExpirationTimer(TESObjectREFR* ref, TESForm* baseform, UInt8 type, ExtraDataList* xDataList)
    :
     ref(ref), baseform(baseform), type(type), xDataList(xDataList){}

    TESObjectREFR* ref;         //Either container or world ref
    TESForm* baseform;          //For error checking
    UInt8 type;                 //ExtraContainerChanges, or ExtraTimeLeft
    ExtraDataList* xDataList;   //Either Containers ExtraContainerChanges, or direct objects ExtraTimeLeft

};

extern std::vector<ExpirationTimer*> lifecycleTimer;
extern std::vector<TESObjectREFR*> newlyCreatedReferences;
//extern std::unordered_set<TESObjectREFR*, TESForm*>, UInt32> lifecycleLookup;

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

    void insertAt(UInt32 index, Instance* instance) {
        if (index >= this->size()) {
            this->resize(index + 1, nullptr);  // Resize and fill with nullptrs
        }
        (*this)[index] = instance;
    }

};

class LifecycleManager {
public:
    enum DestructionPolicy {
        Timed = 1,
        OnUnload = 2,
        Recycle = 4,
        None = 0
    };

    LifecycleManager(int policies = None, float lifetime = 0.0)
        : policies(policies), lifetime(lifetime) {}

    void enablePolicy(DestructionPolicy policy) {
        policies |= policy;
    }

    void disablePolicy(DestructionPolicy policy) {
        policies &= ~policy;
    }

    bool isPolicyEnabled(DestructionPolicy policy) const {
        return (policies & policy) != 0;
    }

    int getPolicies() const {
        return policies;
    }

    float getLifeTime() const {
        return lifetime;
    }

    void setLifeTime(float time) {
        lifetime = time;
    }

    void unload() {
        if (policies & OnUnload) {
            requestDestruction();
        }
    }
    /*
    void requestImmediateDestruction() {
        if (policies & Immediate) {
            requestDestruction();
        }
    }
    */
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

    static void addLifecycleTimer(TESObjectREFR* worldRef, TESForm* baseForm, float time, ExtraDataList* xDataList, bool insideWorldRef = false, bool set = false);

    bool saveBehavior = true;

private:
    UInt8 policies;
    float lifetime;
    std::function<void()> destroyCallback;
};

struct ExtendedBaseType {

protected:

    ExtendedBaseType(
        TESForm* parent = nullptr,
        UInt32 modIndex = 0,
        UInt32 extendedType = 0,
        const TraitMap& traits = {},
        const SlotMap& linkedTraits = {},
        const InstanceVector& aInstances = {}
    ) : edits({ modIndex }), parent(parent), extendedType(extendedType), traits(traits), linkedTraits(linkedTraits), aInstances(aInstances) {}

public:

    ~ExtendedBaseType() {
        clearInstances();
    }

    TESForm* parent;

    LifecycleManager baseLifecycle; //All instances will inherit this

    InstanceVector aInstances;

    TraitMap traits; //Maybe move traits outside so we don't have multi maps on every form
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
        TESForm* parent,
        UInt32 modIndex = 0,
        UInt32 extendedType = 0,
        const TraitMap& traits = {},
        const SlotMap& linkedTraits = {},
        const InstanceVector& aInstances = {}
    ) : ExtendedBaseType(parent, modIndex, extendedType, traits, linkedTraits, aInstances) {
        if (parent) {
            StaticLinker[this->parent->typeID][this->parent->refID] = this;
        }
    }

    ~StaticInstance() {
        StaticLinker[this->parent->typeID].erase(this->parent->refID);
        //Shouldn't be destroying these
    }

    TESForm* createInstance(std::string key);

    //Shouldn't be used at runtime.
    void setParent(TESForm* form, bool doFree = false);

    virtual Instance* newInstance(const std::string& key, UInt32 modIndex);
    virtual Instance* loadInstance(UInt32 InstID, UInt32 modIndex, const std::string& key, LifecycleManager* lifecycle);

};

struct StaticInstance_WEAP : StaticInstance {

    StaticInstance_WEAP(
        TESForm* parent,
        UInt32 modIndex = 0,
        UInt32 extendedType = 40,
        const std::unordered_map<std::string, UInt32>& aBaseAttachments = {},
        const TraitMap& traits = {},
        const SlotMap& linkedTraits = {},
        const InstanceVector& aInstances = {}
    ) : StaticInstance(parent, modIndex, extendedType, traits, linkedTraits, aInstances), aBaseAttachments(aBaseAttachments) {}

    std::unordered_map<std::string, UInt32> aBaseAttachments;
    std::unordered_map<std::string, std::vector<UInt32>> aAllAttachments;

    Instance* newInstance(const std::string& key, UInt32 modIndex) override final;
    Instance* loadInstance(UInt32 InstID, UInt32 modIndex, const std::string& key, LifecycleManager* lifecycle) override final;

};


struct StaticInstance_Akimbo : StaticInstance {

    StaticInstance_Akimbo(
        TESForm* parent,
        UInt32 modIndex,
        TESForm* left,
        TESForm* right,
        const TraitMap traits = {},         //Used to copy traits
        const SlotMap linkedTraits = {},
        const InstanceVector& aInstances = {}
    ) : StaticInstance(parent, modIndex, 222, traits, linkedTraits, aInstances), left(left), right(right) {

        AkimboSets[this->left->refID][this->right->refID] = this;

    }

    ~StaticInstance_Akimbo() {
        AkimboSets[this->left->refID].erase(this->right->refID);
        if (AkimboSets[this->left->refID].empty()) {
            AkimboSets.erase(this->left->refID);
        }
        //Shouldn't be destroying these
    }

    TESForm* left;
    TESForm* right;

    Instance* newInstance(const std::string& key, UInt32 modIndex) override final;
    TESForm* newInstance(TESObjectREFR* right, TESObjectREFR* left, UInt32 modIndex, const std::string& key);

    Instance_Akimbo* loadInstance(UInt16 InstID, UInt32 modIndex,
        TESObjectREFR* weapRight, TESObjectREFR* weapLeft,
        const std::string& key, LifecycleManager* lifecycle);

    static StaticInstance_Akimbo* LookupAkimboSet(TESForm* left, TESForm* right);

};

struct TESRefr {

    TESRefr(StaticInstance* baseInstance, UInt32 modIndex)
        : TESRefr(baseInstance, modIndex, baseInstance->baseLifecycle) {}

    TESRefr(StaticInstance* baseInstance, UInt32 modIndex, const LifecycleManager& lifecycle)
        : baseInstance(baseInstance), modIndex(modIndex), lifecycle(lifecycle) {
    }

    //Direct loading
    TESRefr(StaticInstance* baseInstance, UInt32 modIndex, LifecycleManager&& lifecycle)
        : baseInstance(baseInstance), modIndex(modIndex), lifecycle(std::move(lifecycle)) {}

    LifecycleManager lifecycle;
    StaticInstance* baseInstance;
    UInt32 modIndex;

};

struct Instance : TESRefr {

    Instance(StaticInstance* baseInstance, TESForm* clone, UInt32 modIndex, std::string key)
        : Instance(baseInstance, clone, modIndex, key, baseInstance->baseLifecycle) {}

    Instance(StaticInstance* baseInstance, TESForm* clone, UInt32 modIndex, std::string key, LifecycleManager& lifecycle)
        : TESRefr(baseInstance, modIndex, lifecycle), clone(clone), key(key) {
        InstID = baseInstance->aInstances.add(this);
        lifecycle.enablePolicy(LifecycleManager::Recycle);  //Cleans up instances that don't exist in the world
        ++InstanceInterface::cloneCount;
    }

    //Used to load direct info
    Instance(UInt16 InstID, StaticInstance* baseInstance, TESForm* clone, UInt32 modIndex, std::string key, LifecycleManager&& lifecycle_p)
        : TESRefr(baseInstance, modIndex, std::move(lifecycle_p)), clone(clone), key(key), InstID(InstID) {
        baseInstance->aInstances.insertAt(InstID, this);
        lifecycle.enablePolicy(LifecycleManager::Recycle);  //Cleans up instances that don't exist in the world
        ++InstanceInterface::cloneCount;
    }

    virtual ~Instance() {
        PluginFunctions::c_RemoveFormAnimations(clone);
        clone->Destroy(false);
    }

    UInt16 InstID;
    TESForm* clone;
    std::string key; //Maybe don't store a key on every instance, instead use a set

   //Use if deleting single instances
   void destroy();

};

struct TESInstance : Instance {

    TESInstance(
        StaticInstance* baseInstance,
        TESForm* clone,
        UInt32 modIndex,
        std::string key
    ) : Instance(baseInstance, clone, modIndex, key) {
        InstanceLinker[this->clone->typeID][this->clone->refID] = this;
    }

    //Used to load direct info
    TESInstance(
        UInt16 InstID,
        StaticInstance* baseInstance,
        TESForm* clone,
        UInt32 modIndex,
        std::string key,
        LifecycleManager&& lifecycle
    ) : Instance(InstID, baseInstance, clone, modIndex, key, std::move(lifecycle)) {
        InstanceLinker[this->clone->typeID][this->clone->refID] = this;
    }

    ~TESInstance() override {
        PluginFunctions::c_RemoveFormAnimations(clone);
        InstanceLinker[this->clone->typeID].erase(this->clone->refID);
        clone->Destroy(false);
    }

};

struct Instance_WEAP : TESInstance {

    Instance_WEAP(
        StaticInstance* baseInstance,
        TESForm* clone,
        UInt32 modIndex,
        std::string key,
        const std::unordered_map<std::string, UInt32>& aAttachments = {}
    ) : TESInstance(baseInstance, clone, modIndex, key), aAttachments(aAttachments){}

    //Used to load direct info
    Instance_WEAP(
        UInt16 InstID,
        StaticInstance* baseInstance,
        TESForm* clone,
        UInt32 modIndex,
        std::string key,
        LifecycleManager&& lifecycle,
        const std::unordered_map<std::string, UInt32>& aAttachments = {}
    ) : TESInstance(InstID, baseInstance, clone, modIndex, key, std::move(lifecycle)), aAttachments(aAttachments) {}

    std::unordered_map<std::string, UInt32> aAttachments; //Use a map of cad objects, to avoid dupe strings.

};

struct Instance_Akimbo : Instance {

    Instance_Akimbo(
        StaticInstance* baseInstance,
        TESForm* clone,
        UInt32 modIndex,
        std::string key,
        TESObjectREFR* left,
        TESObjectREFR* right
    ) : Instance(baseInstance, clone, modIndex, key),
        right(right->refID),
        left(left->refID)
    {
        InstanceLinker[40][this->clone->refID] = this;
    }

    //Used to load direct info
    Instance_Akimbo(
        UInt16 InstID,
        StaticInstance* baseInstance,
        TESForm* clone,
        UInt32 modIndex,
        std::string key,
        LifecycleManager&& lifecycle,
        TESObjectREFR* left,
        TESObjectREFR* right
    ) : Instance(InstID, baseInstance, clone, modIndex, key, std::move(lifecycle)),
        right(right->refID),
        left(left->refID)
    {
        InstanceLinker[40][this->clone->refID] = this;
    }

    ~Instance_Akimbo() override final {
        PluginFunctions::c_RemoveFormAnimations(clone);
        InstanceLinker[40].erase(this->clone->refID);
        clone->Destroy(false);
    }

    UInt32 right;   //SetAkimboWeapon
    UInt32 left;

};
