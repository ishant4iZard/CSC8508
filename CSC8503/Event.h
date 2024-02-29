#pragma once
#include <string>
#include <map>
#include "EventType.h"

// Classes that want to be notified of events must extend this class and overide the Receive Event function
class EventListener {
public:
    virtual void ReceiveEvent(const EventType eventType) = 0;
};

typedef std::multimap<EventType, EventListener*>::iterator EventIterator;

class EventEmitter {
public:
    static void RegisterForEvent(const EventType& eventType, EventListener* l) {
        listeners.insert(std::make_pair(eventType, l));
    }

    static void EmitEvent(const EventType& eventType) {
        std::pair<EventIterator, EventIterator> ii = listeners.equal_range(eventType);
        for (EventIterator i = ii.first; i != ii.second; ++i) {
            if (i->second == nullptr) continue;
            i->second->ReceiveEvent(eventType);
        }
    }

    static void RemoveListner(EventListener* l) {
        for (auto it = listeners.begin(); it != listeners.end();) {
            if (it->second == l)
                it = listeners.erase(it);
            else
                ++it;
        }
    }

protected:
    static std::multimap<EventType, EventListener*> listeners;
};