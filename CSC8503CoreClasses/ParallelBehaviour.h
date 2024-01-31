#pragma once
#include "BehaviourNodeWithChildren.h"

class ParallelBehaviour : public BehaviourNodeWithChildren {
public:
	ParallelBehaviour(const std::string nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~ParallelBehaviour() {}
	BehaviourState Execute(float dt) override {
		//std::cout << "Executing selector " << name << "\n";
		for (auto& i : childNodes) {
			BehaviourState nodeState = i->Execute(dt);
			switch (nodeState) {
				case Failure:
				case Success:
				case Ongoing:
				{
					//currentState = nodeState;
					return nodeState;
				}
			}
		}
		return Failure;
	}
};
