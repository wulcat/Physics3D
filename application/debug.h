#pragma once

#include "../engine/debug.h"

#include "../engine/math/vec3.h"
#include "threePhaseBuffer.h"
#include "../util/log.h"


#define ASSERT(x) if (!(x)) __debugbreak();
#define glCall(x) {clearError(); x; ASSERT(logCall(#x, __FILE__, __LINE__));}

void clearError();

bool logCall(const char* func, const char* file, int line);


namespace AppDebug {
	struct ColoredVector {
		Vec3 origin, vector;
		Debug::VectorType type;
		ColoredVector() = default;
		ColoredVector(Vec3 origin, Vec3 vector, Debug::VectorType type) : origin(origin), vector(vector), type(type) {}
	};
	struct ColoredPoint {
		Vec3 point;
		Debug::PointType type;
		ColoredPoint() = default;
		ColoredPoint(Vec3 point, Debug::PointType type) : point(point), type(type) {}
	};

	void setupDebugHooks();

	void logTickStart();
	void logTickEnd();

	void logFrameStart();
	void logFrameEnd();

	AddableBuffer<ColoredVector>& getVectorBuffer();
	AddableBuffer<ColoredPoint>& getPointBuffer();
}
