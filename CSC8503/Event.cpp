#include "Event.h"

EventEmitter* EventEmitter::emitter = nullptr;

EventEmitter* EventEmitter::GetInstance() {
	if (emitter == nullptr)
		emitter = new EventEmitter();

	return emitter;
}

void EventEmitter::Destory() {
	if (emitter == nullptr)
		return;

	delete emitter;
}