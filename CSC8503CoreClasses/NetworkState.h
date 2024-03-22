#pragma once

namespace NCL {
	using namespace Maths;
	namespace CSC8503 {
		class NetworkState	{
		public:
			NetworkState();
			virtual ~NetworkState();

			Vector3		position;
			Quaternion	orientation;
			int			stateID;
		};
	}
}

