#pragma once
#include "BehaviourNodeWithChildren.h"
#include <vector>

class BehaviourSequence : public BehaviourNodeWithChildren {
public:
	BehaviourSequence(const std::string& nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~BehaviourSequence() {}
	BehaviourState Execute(float dt) override {
		//std::cout << "Executing sequence " << name << "\n";
		for (auto& i : childNodes) {
			BehaviourState nodeState = i->Execute(dt);
			switch (nodeState) {
				case Success: continue;
				case Failure:
				case Ongoing:
				{
					currentState = nodeState;
					return nodeState;
				}
			}
		}
		return Success;
	}
};

class ParallelBehaviour : public BehaviourNodeWithChildren {
public:
	ParallelBehaviour(const std::string nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~ParallelBehaviour() {}
	BehaviourState Execute(float dt) override {
		//std::cout << "Executing selector " << name << "\n";
		std::vector<BehaviourState> nodeState;
		for (auto& i : childNodes) {
			nodeState.push_back(i->Execute(dt));
		}
		//BehaviourState cnodeState = Failure;
		for (auto& i : nodeState) {
			switch (i) {
			case Failure: continue;
			case Success: return Success;
			case Ongoing: return Ongoing;
			}
		}
		/*for (int i = 0; i < childNodes.size(); i++) {
			if (nodeState[i] == Ongoing)
				return Ongoing;
		}
		for (int i = 0; i < childNodes.size();i++) {
			if (nodeState[i] == Success)
				return Success;
		}*/
		/*if (cnodeState == Ongoing)
			return Ongoing;*/
		return Failure;
	}
};