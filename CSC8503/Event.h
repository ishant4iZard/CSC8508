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
    void RegisterForEvent(const EventType eventType, EventListener* l) {
        listeners.insert(std::make_pair(eventType, l));
    }

    void EmitEvent(const EventType eventType) {
        std::pair<EventIterator, EventIterator> ii = listeners.equal_range(eventType);
        for (EventIterator i = ii.first; i != ii.second; ++i) {
            i->second->ReceiveEvent(eventType);
        }
    }

    static EventEmitter*    GetInstance();
    static void             Destory();

protected:
    static EventEmitter* emitter;
    std::multimap<EventType, EventListener*> listeners;
};