#include "world.h"

#include <algorithm>
#include "../util/log.h"

#ifndef NDEBUG
#define ASSERT_VALID if (!isValid()) {throw "World not valid!";}
#else
#define ASSERT_VALID
#endif


class Layer {
	BoundsTree<Part>& tree;
public:
	Layer(BoundsTree<Part>& tree) : tree(tree) {}
};


WorldPrototype::WorldPrototype(double deltaT) : 
	deltaT(deltaT), 
	layers{Layer{objectTree}, Layer{terrainTree}},
	colissionMatrix(2) {
	colissionMatrix.get(0, 0) = true; // free-free
	colissionMatrix.get(1, 0) = true; // free-terrain
	colissionMatrix.get(1, 1) = false; // terrain-terrain
}

WorldPrototype::~WorldPrototype() {

}

BoundsTree<Part>& WorldPrototype::getTreeForPart(const Part* part) {
	return (part->isTerrainPart) ? this->terrainTree : this->objectTree;
}

const BoundsTree<Part>& WorldPrototype::getTreeForPart(const Part* part) const {
	return (part->isTerrainPart) ? this->terrainTree : this->objectTree;
}

static void addToNode(TreeNode& nodeToAddTo, const Physical* physicalToAdd) {
	nodeToAddTo.addInside(TreeNode(physicalToAdd->mainPart, physicalToAdd->mainPart->getStrictBounds(), false));
	for(const AttachedPart& p : physicalToAdd->parts) {
		nodeToAddTo.addInside(TreeNode(p.part, p.part->getStrictBounds(), false));
	}
	for(const ConnectedPhysical& conPhys : physicalToAdd->childPhysicals) {
		addToNode(nodeToAddTo, &conPhys);
	}
}

static TreeNode createNodeFor(const MotorizedPhysical* phys) {
	TreeNode newNode(phys->mainPart, phys->mainPart->getStrictBounds(), true);
	for(const AttachedPart& p : phys->parts) {
		newNode.addInside(TreeNode(p.part, p.part->getStrictBounds(), false));
	}
	for(const ConnectedPhysical& conPhys : phys->childPhysicals) {
		addToNode(newNode, &conPhys);
	}
	return newNode;
}

void WorldPrototype::addPart(Part* part, bool anchored) {
	ASSERT_VALID;
	part->ensureHasParent();
	if (part->parent->mainPhysical->world == this) {
		Log::warn("Attempting to readd part to world");
		ASSERT_VALID;
		return;
	}
	
	objectTree.add(std::move(createNodeFor(part->parent->mainPhysical)));
	physicals.push_back(part->parent->mainPhysical);

	objectCount += part->parent->getPartCount();
	
	part->parent->mainPhysical->world = this;
	part->parent->setAnchored(anchored);

	ASSERT_VALID;
}
void WorldPrototype::removePart(Part* part) {
	ASSERT_VALID;
	Physical* parent = part->parent;
	parent->detachPart(part);

	

	objectCount--;

	ASSERT_VALID;
}
void WorldPrototype::removeMainPhysical(MotorizedPhysical* phys) {
	objectCount -= phys->getPartCount();

	NodeStack stack = objectTree.findGroupFor(phys->mainPart, phys->mainPart->getStrictBounds());
	stack.remove();
	physicals.erase(std::remove(physicals.begin(), physicals.end(), phys));

	ASSERT_VALID;
}
void WorldPrototype::addTerrainPart(Part* part) {
	objectCount++;

	terrainTree.add(part, part->getStrictBounds());
	part->isTerrainPart = true;
}
void WorldPrototype::optimizeTerrain() {
	for(int i = 0; i < 5; i++)
		terrainTree.improveStructure();
}




void WorldPrototype::setPartCFrame(Part* part, const GlobalCFrame& newCFrame) {
	Bounds oldBounds = part->getStrictBounds();

	part->parent->setPartCFrame(part, newCFrame);

	objectTree.updateObjectGroupBounds(part, oldBounds);
}

void WorldPrototype::updatePartBounds(const Part* updatedPart, const Bounds& oldBounds) {
	objectTree.updateObjectBounds(updatedPart, oldBounds);
}

void WorldPrototype::updatePartGroupBounds(const Part* mainPart, const Bounds& oldMainPartBounds) {
	objectTree.updateObjectGroupBounds(mainPart, oldMainPartBounds);
}

void WorldPrototype::removePartFromTrees(const Part* part) {
	getTreeForPart(part).remove(part);
}



void WorldPrototype::addExternalForce(ExternalForce* force) {
	externalForces.push_back(force);
}

void WorldPrototype::removeExternalForce(ExternalForce* force) {
	externalForces.erase(std::remove(externalForces.begin(), externalForces.end(), force));
}

//using WorldPartIter = IteratorGroup<IteratorFactory<BoundsTreeIter<TreeIterator, Part>, IteratorEnd>, 2>;
IteratorFactoryWithEnd<WorldPartIter> WorldPrototype::iterParts(int partsMask) {
	size_t size = 0;
	IteratorFactoryWithEnd<BoundsTreeIter<TreeIterator, Part>> iters[2]{};
	if(partsMask & FREE_PARTS) {
		BoundsTreeIter<TreeIterator, Part> i(objectTree.begin());
		iters[size++] = IteratorFactoryWithEnd<BoundsTreeIter<TreeIterator, Part>>(i);
	}
	if(partsMask & TERRAIN_PARTS) {
		BoundsTreeIter<TreeIterator, Part> i(terrainTree.begin());
		iters[size++] = IteratorFactoryWithEnd<BoundsTreeIter<TreeIterator, Part>>(i);
	}

	WorldPartIter group(iters, size);

	return IteratorFactoryWithEnd<WorldPartIter>(std::move(group));
}
IteratorFactoryWithEnd<ConstWorldPartIter> WorldPrototype::iterParts(int partsMask) const {
	size_t size = 0;
	IteratorFactoryWithEnd<BoundsTreeIter<ConstTreeIterator, const Part>> iters[2]{};
	if(partsMask & FREE_PARTS) {
		BoundsTreeIter<ConstTreeIterator, const Part> i(objectTree.begin());
		iters[size++] = IteratorFactoryWithEnd<BoundsTreeIter<ConstTreeIterator, const Part>>(i);
	}
	if(partsMask & TERRAIN_PARTS) {
		BoundsTreeIter<ConstTreeIterator, const Part> i(terrainTree.begin());
		iters[size++] = IteratorFactoryWithEnd<BoundsTreeIter<ConstTreeIterator, const Part>>(i);
	}

	ConstWorldPartIter group(iters, size);

	return IteratorFactoryWithEnd<ConstWorldPartIter>(std::move(group));
}


void recursiveTreeValidCheck(const TreeNode& node) {
	if (node.isLeafNode()) return;
	
	Bounds bounds = node[0].bounds;
	for (int i = 1; i < node.nodeCount; i++) {
		bounds = unionOfBounds(bounds, node[i].bounds);
	}
	if (bounds != node.bounds) {
		throw "A node in the tree does not have valid bounds!";
	}

	for (TreeNode& n : node) {
		recursiveTreeValidCheck(n);
	}
}
static bool isConnectedPhysicalValid(const ConnectedPhysical* phys, const MotorizedPhysical* mainPhys);

static bool isPhysicalValid(const Physical* phys, const MotorizedPhysical* mainPhys) {
	if(phys->mainPhysical != mainPhys) {
		Log::error("Physical's parent is not mainPhys!");
		__debugbreak();
		return false;
	}
	for(const Part& part : *phys) {
		if(part.parent != phys) {
			Log::error("part's parent's child is not part");
			__debugbreak();
			return false;
		}
	}
	for(const ConnectedPhysical& subPhys : phys->childPhysicals) {
		if(!isConnectedPhysicalValid(&subPhys, mainPhys)) return false;
	}
	return true;
}

static bool isConnectedPhysicalValid(const ConnectedPhysical* phys, const MotorizedPhysical* mainPhys) {
	return isPhysicalValid(phys, mainPhys);
}

bool WorldPrototype::isValid() const {
	for (const MotorizedPhysical* phys : iterPhysicals()) {
		if (phys->world != this) {
			Log::error("physicals's world is not correct!");
			__debugbreak();
			return false;
		}

		if(!isPhysicalValid(phys, phys)) return false;
	}

	recursiveTreeValidCheck(objectTree.rootNode);
	recursiveTreeValidCheck(terrainTree.rootNode);

	return true;
}
