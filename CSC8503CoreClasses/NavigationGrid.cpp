#include "NavigationGrid.h"
#include "Assets.h"
#include "PhysicsSystem.h"

#include <fstream>

using namespace NCL;
using namespace CSC8503;

const int LEFT_NODE		= 0;
const int RIGHT_NODE	= 1;
const int TOP_NODE		= 2;
const int BOTTOM_NODE	= 3;

const char WALL_NODE	= 'x';
const char FLOOR_NODE	= '.';

NavigationGrid::NavigationGrid() {
	nodeSize = 0;
	gridWidth = 0;
	gridHeight = 0;
	allNodes = nullptr;
}


//Create a Grid that can detect there are obstacles or not.
NavigationGrid::NavigationGrid(GameWorld* world)	{
	nodeSize	= 2;

	// NOTE : width and height do NOT refer to length but the number of cells
	gridWidth	= 200 / nodeSize;
	gridHeight	= 200 / nodeSize;

	allNodes	= new GridNode[gridWidth * gridHeight];

	startpos = Vector3(-100, 0, -100);
	RayCollision  obstaclesCollision;

	int z = startpos.z;
	for (int j = 0; j < gridHeight; j++)
	{
		int x = startpos.x;
		for (int i = 0; i < gridWidth; i++)
		{
			rays.push_back(Ray(Vector3(x+1, 100, z+1), (Vector3(0, -20, 0) - Vector3(0, 20, 0)).Normalised()));//+1 is to get the centre of the grid
			x += nodeSize;
		}
		z += nodeSize;
	}

	// Identify grid cell type
	int currentIndx = 0;
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {

			GridNode& n = allNodes[(gridWidth * y) + x];

			if (world->Raycast(rays[currentIndx], obstaclesCollision, true)) {
				GameObject* ObjectDetected = (GameObject*)obstaclesCollision.node;

				if (!ObjectDetected) continue;

				if (ObjectDetected->gettag() == "walls" || ObjectDetected->gettag() == "bouncePads") {//use isKinematic
					n.type = 'x';
					n.position = Vector3(startpos.x + (float)(x * nodeSize), startpos.y, startpos.z + (float)(y * nodeSize));
				}
				else {
					n.type = '.';
					n.position = Vector3(startpos.x + (float)(x * nodeSize), startpos.y, startpos.z + (float)(y * nodeSize));
				}
			}

			currentIndx++;
		}
	}

	// Connect cells to neighbouring cells
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {

			GridNode& n = allNodes[(gridWidth * y) + x];

			if (y > 0) { //get the above node
				n.connected[0] = &allNodes[(gridWidth * (y - 1)) + x];
			}
			if (y < gridHeight - 1) { //get the below node
				n.connected[1] = &allNodes[(gridWidth * (y + 1)) + x];
			}
			if (x > 0) { //get left node
				n.connected[2] = &allNodes[(gridWidth * (y)) + (x - 1)];
			}
			if (x < gridWidth - 1) { //get right node
				n.connected[3] = &allNodes[(gridWidth * (y)) + (x + 1)];
			}

			for (int i = 0; i < 4; ++i) {
				if (n.connected[i]) {
					if (n.connected[i]->type == '.') {//not dectected
						n.costs[i] = 1;
					}
					if (n.connected[i]->type == 'x') {
						n.connected[i] = nullptr; //actually a wall, disconnect!
					}
				}
			}
		}
	}
}

NavigationGrid::NavigationGrid(const std::string&filename) : NavigationGrid() {
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	allNodes = new GridNode[gridWidth * gridHeight];
	Vector3 startpos = Vector3(5, -17, -30);

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];
			char type = 0;
			infile >> type;
			n.type = type;
			n.position = Vector3(startpos.x+(float)(x * nodeSize), startpos.y, startpos.z + (float)(y * nodeSize));
		}
	}
	
	//now to build the connectivity between the nodes
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode&n = allNodes[(gridWidth * y) + x];		

			if (y > 0) { //get the above node
				n.connected[0] = &allNodes[(gridWidth * (y - 1)) + x];
			}
			if (y < gridHeight - 1) { //get the below node
				n.connected[1] = &allNodes[(gridWidth * (y + 1)) + x];
			}
			if (x > 0) { //get left node
				n.connected[2] = &allNodes[(gridWidth * (y)) + (x - 1)];
			}
			if (x < gridWidth - 1) { //get right node
				n.connected[3] = &allNodes[(gridWidth * (y)) + (x + 1)];
			}
			for (int i = 0; i < 4; ++i) {
				if (n.connected[i]) {
					if (n.connected[i]->type == '.') {//not dectected
						n.costs[i]		= 1;
					}
					if (n.connected[i]->type == 'x') {
						n.connected[i] = nullptr; //actually a wall, disconnect!
					}
				}
			}
		}	
	}
}

NavigationGrid::~NavigationGrid()	{
	delete[] allNodes;
}

bool NavigationGrid::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
	//need to work out which node 'from' sits in, and 'to' sits in
	int fromX = ((int)(from.x - startpos.x) / nodeSize);
	int fromZ = ((int)(from.z - startpos.z) / nodeSize);

	int toX = ((int)(to.x - startpos.x) / nodeSize);
	int toZ = ((int)(to.z - startpos.z) / nodeSize);

	if (fromX < 0 || fromX > gridWidth - 1 ||
		fromZ < 0 || fromZ > gridHeight - 1) {
		return false; //outside of map region!
	}

	if (toX < 0 || toX > gridWidth - 1 ||
		toZ < 0 || toZ > gridHeight - 1) {
		return false; //outside of map region!
	}

	GridNode* startNode = &allNodes[(fromZ * gridWidth) + fromX];
	GridNode* endNode	= &allNodes[(toZ * gridWidth) + toX];

	std::vector<GridNode*>  openList;
	std::vector<GridNode*>  closedList;

	openList.push_back(startNode);

	startNode->f = 0;
	startNode->g = 0;
	startNode->parent = nullptr;

	GridNode* currentBestNode = nullptr;
	int j = 1;

	while (!openList.empty()) {
		currentBestNode = RemoveBestNode(openList);
		if (currentBestNode == endNode) {			//we've found the path!
			GridNode* node = endNode;
			while (node != nullptr) {
				outPath.PushWaypoint(node->position);
				node = node->parent;
			}
			return true;
		}
		else {
			for (int i = 0; i < 4; ++i) {
				GridNode* neighbour = currentBestNode->connected[i];
				if (!neighbour) { //might not be connected...
					continue;
				}	
				bool inClosed	= NodeInList(neighbour, closedList);
				if (inClosed) {
					continue; //already discarded this neighbour...
				}

				float h = Heuristic(neighbour, endNode);				
				float g = currentBestNode->g + currentBestNode->costs[i];
				float f = h + g;

				bool inOpen		= NodeInList(neighbour, openList);

				if (!inOpen) { //first time we've seen this neighbour

					openList.emplace_back(neighbour);
				}
				if (!inOpen || f < neighbour->f) {//might be a better route to this neighbour

					neighbour->parent = currentBestNode;
					neighbour->f = f;
					neighbour->g = g;
				}
			}
			closedList.emplace_back(currentBestNode);
		}
	}

	return false; //open list emptied out with no path!
}

bool NavigationGrid::AIFindPath(const std::vector<Vector3>& waypoints, NavigationPath& outPath) {
	if (waypoints.empty()) {
		return false;
	}
	std::vector<Vector3>::const_iterator it = waypoints.begin();
	//need to work out which node 'from' sits in, and 'to' sits in

	while (it != waypoints.end()) {
		Vector3 from = *it; 

		++it;
		if (it == waypoints.end()) {
			break; 
		}

		Vector3 to = *it;

		int fromX = ((int)from.x / nodeSize);
		int fromZ = ((int)from.z / nodeSize);
		int toX = ((int)to.x / nodeSize);
		int toZ = ((int)to.z / nodeSize);

		if (fromX < 0 || fromX > gridWidth - 1 ||
			fromZ < 0 || fromZ > gridHeight - 1) {
			return false; //outside of map region!
		}

		if (toX < 0 || toX > gridWidth - 1 ||
			toZ < 0 || toZ > gridHeight - 1) {
			return false; //outside of map region!
		}

		GridNode* startNode = &allNodes[(fromZ * gridWidth) + fromX];
		GridNode* endNode = &allNodes[(toZ * gridWidth) + toX];

		std::vector<GridNode*>  openList;
		std::vector<GridNode*>  closedList;

		openList.push_back(startNode);

		startNode->f = 0;
		startNode->g = 0;
		startNode->parent = nullptr;

		GridNode* currentBestNode = nullptr;
		int j = 1;

		while (!openList.empty()) {
			currentBestNode = RemoveBestNode(openList);
			if (currentBestNode == endNode) {			//we've found the path!
				GridNode* node = endNode;
				while (node != nullptr) {
					outPath.PushWaypoint(node->position);
					node = node->parent;
				}
				return true;
			}
			else {
				for (int i = 0; i < 4; ++i) {
					GridNode* neighbour = currentBestNode->connected[i];
					if (!neighbour) { //might not be connected...
						continue;
					}
					bool inClosed = NodeInList(neighbour, closedList);
					if (inClosed) {
						continue; //already discarded this neighbour...
					}

					float h = Heuristic(neighbour, endNode);
					float g = currentBestNode->g + currentBestNode->costs[i];
					float f = h + g;

					bool inOpen = NodeInList(neighbour, openList);

					if (!inOpen) { //first time we've seen this neighbour

						openList.emplace_back(neighbour);
					}
					if (!inOpen || f < neighbour->f) {//might be a better route to this neighbour

						neighbour->parent = currentBestNode;
						neighbour->f = f;
						neighbour->g = g;
					}
				}
				closedList.emplace_back(currentBestNode);
			}
		}
		return false; //open list emptied out with no path!
	}
}

bool NavigationGrid::NodeInList(GridNode* n, std::vector<GridNode*>& list) const {
	std::vector<GridNode*>::iterator i = std::find(list.begin(), list.end(), n);
	return i == list.end() ? false : true;
}

GridNode*  NavigationGrid::RemoveBestNode(std::vector<GridNode*>& list) const {
	std::vector<GridNode*>::iterator bestI = list.begin();

	GridNode* bestNode = *list.begin();

	for (auto i = list.begin(); i != list.end(); ++i) {
		if ((*i)->f < bestNode->f) {
			bestNode	= (*i);
			bestI		= i;
		}
	}
	list.erase(bestI);

	return bestNode;
}

float NavigationGrid::Heuristic(GridNode* hNode, GridNode* endNode) const {
	return (hNode->position - endNode->position).Length();
}

void NavigationGrid::PrintGrid() {	
	for (int j = 0; j < gridWidth; j++) {
		for (int i = 0; i < gridHeight; i++) {
			GridNode& n = allNodes[gridWidth * j + i];

			Debug::DrawLine(n.position, n.position + Vector4(nodeSize, 0, 0, 0), (n.type == '.' ? Debug::RED : Debug::CYAN));
			Debug::DrawLine(n.position, n.position + Vector4(-nodeSize, 0, 0, 0), (n.type == '.' ? Debug::RED : Debug::CYAN));
			Debug::DrawLine(n.position, n.position + Vector4(0, 0, nodeSize, 0), (n.type == '.' ? Debug::RED : Debug::CYAN));
			Debug::DrawLine(n.position, n.position + Vector4(0, 0, -nodeSize, 0), (n.type == '.' ? Debug::RED : Debug::CYAN));
		}
	}
}