#pragma once
#include "EventHandlers.h"

Event::Event(SInt32 priority, Script* script) : priority(priority), script(script) {}

Event::Event(SInt32 priority, Script* script, std::vector<void*> filters)
    : priority(priority), script(script), filters(filters) {}

bool Event::CompareFilters(const std::vector<void*>& pfilters) const {      //Use when dispatching

    if (filters.size() != pfilters.size()) {
        return false;
    }

    for (size_t i = 0; i < filters.size(); ++i) {
 
        if (filters[i] == nullptr) {
            continue;
        }

        if (filters[i] != pfilters[i]) {
            return false;
        }
    }

    return true;

}

std::vector<void*> Event::EvaluateEventArg(int num_args, ...) {
    va_list args;
    va_start(args, num_args);
    std::vector<void*> filter(num_args, nullptr);

    for (int i = 0; i < num_args; i++) {
        UInt32 index = va_arg(args, UInt32);
        if (index != 0) {                           //Filter skipped.
            if (index <= num_args) {
                filter[index - 1] = va_arg(args, void*);
            }
            else {
                va_arg(args, void*);    //Skip, priority out of scope.
            }
        }
        else {
            break;
        }
    }

    va_end(args);
    return filter;
}

void EventHandler::AddEvent(const Event& event) {
    auto it = std::find_if(handlers.begin(), handlers.end(), [script = event.script](const Event& handler) {
        return handler.script == script;
        });

    if (it == handlers.end()) {
        auto insertPos = std::lower_bound(handlers.begin(), handlers.end(), event, [](const Event& a, const Event& b) {
            return a.priority < b.priority;
            });
        handlers.insert(insertPos, event);
    }
    else {
        it->priority = event.priority;
        it->filters = event.filters;
    }
}

void EventHandler::RemoveEvent(Script* script) {
    handlers.erase(std::remove_if(handlers.begin(), handlers.end(), [script](const Event& handler) {
        return handler.script == script;
        }), handlers.end());
}

//template<typename... Args>
//void EventHandler::DispatchEvent(Args&&... args) {

   // std::vector<void*> filter{ std::forward<Args>(args)... };
    //for (auto& eventHandler : handlers) {
        //if (eventHandler.CompareFilters(filter)) {
            //g_scriptInterface->CallFunction(eventHandler.script, nullptr, nullptr, nullptr, filter.size(), args...);
       // }
    //}

//}