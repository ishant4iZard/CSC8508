//#include "BehaviourSelector.h"
//#include <iostream>
//BehaviourState BehaviourSelector::Execute(float dt) {
//	//std::cout << "Executing selector " << name << "\n";
//	for (auto& i : childNodes) {
//
//		BehaviourState nodeState = i->Execute(dt);
//		switch (nodeState) {
//			case Failure: continue; 
//			case Success: 
//			case Ongoing:
//			{
//				currentState = nodeState;
//				return currentState;
//			}
//		}
//	}
//	return Failure;
//}