#include "visibilityFilter.h"

#include "../../../util/log.h"
#include "../../math/linalg/trigonometry.h"

using namespace P3D::OldBoundsTree;

VisibilityFilter::VisibilityFilter(const Position& origin, Vec3 normals[5], double maxDepth) :
	origin(origin), 
	up(normals[0]), down(normals[1]), left(normals[2]), right(normals[3]), forward(normals[4]),
	maxDepth(maxDepth) {}

VisibilityFilter VisibilityFilter::fromSteps(const Position& origin, const Vec3& stepForward, const Vec3& stepUp, const Vec3& stepRight, double maxDepth) {
	Vec3 upNormal = (stepForward + stepUp) % stepRight;
	Vec3 downNormal = -(stepForward - stepUp) % stepRight;
	Vec3 leftNormal = -(stepForward + stepRight) % stepUp;
	Vec3 rightNormal = (stepForward - stepRight) % stepUp;

	Vec3 normals[5]{upNormal, downNormal, leftNormal, rightNormal, stepForward};

	return VisibilityFilter(origin, normals, maxDepth);
}

VisibilityFilter VisibilityFilter::fromSteps(const Position& origin, const Vec3& stepForward, const Vec3& stepUp, const Vec3& stepRight, double maxDepth, double left, double right, double down, double up) {
	Vec3 upNormal = (stepForward + stepUp * up) % stepRight;
	Vec3 downNormal = -(stepForward + stepUp * down) % stepRight;
	Vec3 leftNormal = -(stepForward - stepRight * left) % stepUp;
	Vec3 rightNormal = (stepForward - stepRight * right) % stepUp;

	Vec3 normals[5]{upNormal, downNormal, leftNormal, rightNormal, stepForward};

	return VisibilityFilter(origin, normals, maxDepth);
}

VisibilityFilter VisibilityFilter::forWindow(const Position& origin, const Vec3& cameraForward, const Vec3& cameraUp, double fov, double aspect, double maxDepth) {
	double tanFov = tan(fov / 2);
	
	Vec3 stepForward = normalize(cameraForward);
	Vec3 stepUp = normalize(cameraUp) * tanFov;
	Vec3 stepRight = normalize(cameraUp % cameraForward) * tanFov * aspect;

	return fromSteps(
		origin,
		stepForward,
		stepUp,
		stepRight,
		maxDepth
	);
}

VisibilityFilter VisibilityFilter::forSubWindow(const Position& origin, const Vec3& cameraForward, const Vec3& cameraUp, double fov, double aspect, double maxDepth, double left, double right, double down, double up) {
	double tanFov = tan(fov / 2);

	Vec3 stepForward = normalize(cameraForward);
	Vec3 stepUp = normalize(cameraUp) * tanFov;
	Vec3 stepRight = normalize(cameraUp % cameraForward) * tanFov * aspect;
	
	return fromSteps(
		origin,
		stepForward,
		stepUp,
		stepRight,
		maxDepth,
		left, right, down, up
	);
}

bool VisibilityFilter::operator()(const TreeNode& node) const {
	return operator()(node.bounds);
}

bool VisibilityFilter::operator()(const Position& point) const {
	double offsets[5] { 0,0,0,0,maxDepth };
	Vec3 normals[5] { up, down, left, right, forward };
	for (int i = 0; i < 5; i++) {
		Vec3& normal = normals[i];

		Vec3 relativePos = point - origin;
		if (relativePos * normal > offsets[i])
			return false;
	}
	return true;
}

bool VisibilityFilter::operator()(const Bounds& bounds) const {
	double offsets[5] { 0,0,0,0,maxDepth };
	Vec3 normals[5] { up, down, left, right, forward };
	for (int i = 0; i < 5; i++) {
		Vec3& normal = normals[i];
		// we're checking that *a* corner of the TreeNode's bounds is within the viewport, basically similar to rectangle-rectangle colissions, google it!
		// cornerOfInterest is the corner that is the furthest positive corner relative to the normal, so if it is not visible (eg above the normal) then the whole box must be invisible
		Position cornerOfInterest(
			(normal.x >= 0) ? bounds.min.x : bounds.max.x,
			(normal.y >= 0) ? bounds.min.y : bounds.max.y, // let's look at the top of the viewport, if the bottom of the box is above this then the whole box must be above it. 
			(normal.z >= 0) ? bounds.min.z : bounds.max.z
		);

		Vec3 relativePos = cornerOfInterest - origin;
		if (relativePos * normal > offsets[i])
			return false;
	}
	return true;
}


bool VisibilityFilter::operator()(const Part& part) const {
	return true;
}

/*   A
	/
   / o---o  <-- cornerOfInterest for B
  /  |   |
 /   |   |
o    o---o  <-- cornerOfInterest for A
 \
  \
   \
	\
	 B


A               B
 \       COI B /
  \        v  /
   \       o---o <- COI A
	\      |/  |
	 \     |   |
	  \   /o---o
	   \ /
		o

*/