#pragma once
#include "BehaviourNodeWithChildren.h"

class BehaviourSelector: public BehaviourNodeWithChildren {
public:
	BehaviourSelector(const std::string& nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~BehaviourSelector() {}
	BehaviourState Execute(float dt) override {
		//std::cout << "Executing selector " << name << "\n";
		int i = rand() % childNodes.size();
		BehaviourState nodeState = childNodes[i]->Execute(dt);
		switch (nodeState) {
			case Failure:
			case Success:
			case Ongoing:
			{
				currentState = nodeState;
				return nodeState;
			}
		}
		return Failure;
	}
};