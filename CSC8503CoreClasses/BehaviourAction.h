#pragma once
#include "BehaviourNode.h"

typedef std::function<BehaviourState(float, BehaviourState)> BehaviourActionFunc;

class BehaviourAction : public BehaviourNode	{
public:
	BehaviourAction(const std::string& nodeName, BehaviourActionFunc f) : BehaviourNode(nodeName) {
		function = f;
	}
	BehaviourState Execute(float dt) override {
		currentState = function(dt, currentState);
		return currentState;
	}
protected:
	BehaviourActionFunc function;
};