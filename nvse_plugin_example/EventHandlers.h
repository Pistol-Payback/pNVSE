#pragma once
#include <stdarg.h>
#include "ppNVSE.h"

struct Event {
    SInt32 priority;
    Script* script;
    std::vector<void*> filters;

    Event(SInt32 priority, Script* script);
    Event(SInt32 priority, Script* script, std::vector<void*> filters);

    bool CompareFilters(const std::vector<void*>& otherFilters) const;      //Use when dispatching
    static std::vector<void*> EvaluateEventArg(int num_args, ...);
};

struct EventHandler {

    std::vector<Event> handlers;

    void AddEvent(const Event& event);
    void RemoveEvent(Script* script);

    //template<typename... Args>
    //void DispatchEvent(Args&&... args);
    //void DispatchEvent(TESForm* hello, TESForm* no);

    template<typename... Args>
    void DispatchEvent(Args&&... args) {

        std::vector<void*> filter{ std::forward<Args>(args)... };
        for (auto& eventHandler : handlers) {
            if (eventHandler.CompareFilters(filter)) {
                g_scriptInterface->CallFunction(eventHandler.script, nullptr, nullptr, nullptr, filter.size(), args...);
            }
        }

    }
};
