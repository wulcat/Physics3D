#pragma once

#include "extendedPart.h"
#include "../physics/math/position.h"
#include "../physics/threading/synchonizedWorld.h"

namespace P3D::Application {

class PlayerWorld : public SynchronizedWorld<ExtendedPart> {
public:
	PlayerWorld(double deltaT);

	Part* selectedPart = nullptr;
	Vec3 localSelectedPoint;
	Position magnetPoint;
	
	void applyExternalForces() override;

	void onPartAdded(ExtendedPart* part) override;
	void onPartRemoved(ExtendedPart* part) override;
};

};