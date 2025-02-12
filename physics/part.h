#pragma once


// this is a central class to everything else, but applications using the library don't need to include all these, as they are implementation details. 
class Part;
class HardConstraint;
class RigidBody;
class Physical;
class ConnectedPhysical;
class MotorizedPhysical;
class WorldLayer;
class WorldPrototype;

#include "geometry/shape.h"
#include "math/linalg/mat.h"
#include "math/position.h"
#include "math/globalCFrame.h"
#include "math/bounds.h"
#include "motion.h"

struct PartProperties {
	double density;
	double friction;
	double bouncyness;

	/*
		This is extra velocity that should be added to any colission
		if this part is anchored, this gives the velocity of another part sliding on top of it, with perfect friction

		In other words, this is the desired relative velocity for there to be no friction
	*/
	Vec3 conveyorEffect{0, 0, 0};
};

struct PartIntersection {
	bool intersects;
	Position intersection;
	Vec3 exitVector;

	PartIntersection() : intersects(false) {}
	PartIntersection(const Position& intersection, const Vec3& exitVector) :
		intersects(true),
		intersection(intersection),
		exitVector(exitVector) {}
};

class Part {
	friend class RigidBody;
	friend class Physical;
	friend class ConnectedPhysical;
	friend class MotorizedPhysical;
	friend class WorldPrototype;
	friend class ConstraintGroup;

	GlobalCFrame cframe;

public:
	WorldLayer* layer = nullptr;
	Physical* parent = nullptr;
	Shape hitbox;
	double maxRadius;
	PartProperties properties;

	Part() = default;
	Part(const Shape& shape, const GlobalCFrame& position, const PartProperties& properties);
	Part(const Shape& shape, Part& attachTo, const CFrame& attach, const PartProperties& properties);
	Part(const Shape& shape, Part& attachTo, HardConstraint* constraint, const CFrame& attachToParent, const CFrame& attachToThis, const PartProperties& properties);
	~Part();

	Part(const Part& other) = delete;
	Part& operator=(const Part& other) = delete;
	Part(Part&& other) noexcept;
	Part& operator=(Part&& other) noexcept;

	WorldPrototype* getWorld();

	PartIntersection intersects(const Part& other) const;
	void scale(double scaleX, double scaleY, double scaleZ);
	void setScale(const DiagonalMat3& scale);
	
#ifdef USE_NEW_BOUNDSTREE
	BoundsTemplate<float> getBounds() const;
#else
	Bounds getBounds() const;
#endif
	BoundingBox getLocalBounds() const;

	Position getPosition() const { return cframe.getPosition(); }
	double getMass() const { return hitbox.getVolume() * properties.density; }
	void setMass(double mass);
	Vec3 getLocalCenterOfMass() const { return hitbox.getCenterOfMass(); }
	Position getCenterOfMass() const { return cframe.localToGlobal(this->getLocalCenterOfMass()); }
	SymmetricMat3 getInertia() const { return hitbox.getInertia() * properties.density; }
	const GlobalCFrame& getCFrame() const { return cframe; }
	void setCFrame(const GlobalCFrame& newCFrame);

	CFrame transformCFrameToParent(const CFrame& cframeRelativeToPart);

	Vec3 getVelocity() const;
	Vec3 getAngularVelocity() const;
	Motion getMotion() const;

	// does not modify angular velocity
	void setVelocity(Vec3 velocity);
	// modifies velocity
	void setAngularVelocity(Vec3 angularVelocity);
	void setMotion(Vec3 velocity, Vec3 angularVelocity);


	bool isMainPart() const;
	void makeMainPart();

	void translate(Vec3 translation);

	double getWidth() const;
	double getHeight() const;
	double getDepth() const;

	void setWidth(double newWidth);
	void setHeight(double newHeight);
	void setDepth(double newDepth);

	double getFriction();
	double getDensity();
	double getBouncyness();
	Vec3 getConveyorEffect();
	
	void setFriction(double friction);
	void setDensity(double density);
	void setBouncyness(double bouncyness);
	void setConveyorEffect(const Vec3& conveyorEffect);
	
	void applyForce(Vec3 relativeOrigin, Vec3 force);
	void applyForceAtCenterOfMass(Vec3 force);
	void applyMoment(Vec3 moment);

	void ensureHasParent();

	int getLayerID() const;

	void attach(Part* other, const CFrame& relativeCFrame);
	void attach(Part* other, HardConstraint* constraint, const CFrame& attachToThis, const CFrame& attachToThat);
	void detach();
	void removeFromWorld();

	bool isValid() const;
};
