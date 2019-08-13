#pragma once

#include "math/vec.h"
#include "math/position.h"
#include "math/cframe.h"
#include "math/globalCFrame.h"

struct Shape;

namespace Debug {
	
	enum VectorType {
		INFO_VEC,
		FORCE,
		MOMENT,
		IMPULSE,
		ANGULAR_IMPULSE,
		POSITION,
		VELOCITY,
		ACCELERATION,
		ANGULAR_VELOCITY,
	};

	enum PointType {
		INFO_POINT,
		CENTER_OF_MASS,
		INTERSECTION,
	};

	enum CFrameType {
		OBJECT_CFRAME,
		INERTIAL_CFRAME
	};

	void logVector(Position origin, Vec3 vec, VectorType type);
	void logPoint(Position point, PointType type);
	void logCFrame(CFrame frame, CFrameType type);
	void logShape(const Shape& shape);

	void setVectorLogAction(void(*logger)(Position origin, Vec3 vec, VectorType type));
	void setPointLogAction(void(*logger)(Position point, PointType type));
	void setCFrameLogAction(void(*logger)(CFrame frame, CFrameType type));
	void setShapeLogAction(void(*logger)(const Shape& shape, const GlobalCFrame& location));
}
