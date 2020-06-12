#include "inertia.h"

#include "math/linalg/trigonometry.h"
#include "math/predefinedTaylorExpansions.h"

SymmetricMat3 getRotatedInertia(const SymmetricMat3& originalInertia, const Rotation& rotation) {
	return rotation.localToGlobal(originalInertia);
}

SymmetricMat3 getTranslatedInertia(const SymmetricMat3& originalInertia, double mass, const Vec3& translation, const Vec3& centerOfMass) {
	SymmetricMat3 translationFactor = skewSymmetricSquared(translation + centerOfMass) - skewSymmetricSquared(centerOfMass);
	return originalInertia - translationFactor * mass;
}

/*
computes a translated inertial matrix,
COMOffset is the offset of the object's center of mass from the resulting center of mass
== localCenterOfMass - totalCenterOfMass
*/
SymmetricMat3 getTranslatedInertiaAroundCenterOfMass(const SymmetricMat3& originalInertia, double mass, const Vec3& COMOffset) {
	SymmetricMat3 translationFactor = skewSymmetricSquared(COMOffset);
	return originalInertia - translationFactor * mass;
}

SymmetricMat3 getTransformedInertia(const SymmetricMat3& originalInertia, double mass, const CFrame& cframe, const Vec3& centerOfMass) {
	return getTranslatedInertia(cframe.getRotation().localToGlobal(originalInertia), mass, cframe.getPosition(), centerOfMass);
}


/*
computes a translated inertial matrix, and it's derivatives
COMOffset is the offset of the object's center of mass from the resulting center of mass
motionOfOffset is the change of COMOffset over time, this is relative to the motion of the COM towards which this is computed
*/
FullTaylor<SymmetricMat3> getTranslatedInertiaDerivativesAroundCenterOfMass(const SymmetricMat3& originalInertia, double mass, const Vec3& COMOffset, const TranslationalMotion& motionOfOffset) {
	FullTaylor<SymmetricMat3> result = -generateFullTaylorForSkewSymmetricSquared(FullTaylor<Vec3>{COMOffset, motionOfOffset.translation});

	result *= mass;
	result.constantValue += originalInertia;
	return result;
}

/*
computes a rotated inertial matrix, where originalInertia is the inertia around the center of mass of the transformed object
rotation is the starting rotation, and rotationMotion gives the change in rotation, both expressed in global space
*/
FullTaylor<SymmetricMat3> getRotatedInertiaTaylor(const SymmetricMat3& originalInertia, const Rotation& rotation, const RotationalMotion& rotationMotion) {
	Mat3 rotationMat = rotation.asRotationMatrix();
	Taylor<Mat3> rotationDerivs = generateTaylorForRotationMatrix(rotationMotion.rotation, rotationMat);
	
	FullTaylor<SymmetricMat3> result(rotation.localToGlobal(originalInertia));

	// rotation(t) * originalInertia * rotation(t).transpose()
	// diff  => rotation(t) * originalInertia * rotation'(t).transpose() + rotation'(t) * originalInertia * rotation(t).transpose()
	// diff2 => 2 * rotation'(t) * originalInertia * rotation'(t).transpose() + rotation(t) * originalInertia * rotation''(t).transpose() + rotation''(t) * originalInertia * rotation(t).transpose()

	result.derivatives[0] = addTransposed(rotationMat * originalInertia * rotationDerivs[0].transpose());
	result.derivatives[1] = addTransposed(rotationMat * originalInertia * rotationDerivs[1].transpose()) + 2.0*mulSymmetricLeftRightTranspose(originalInertia, rotationDerivs[0]);
	
	return result;
}

/*
computes a transformed inertial matrix, where originalInertia is the inertia around the center of mass of the transformed object
totalCenterOfMass is the center around which the new inertia must be calculated
localCenterOfMass is the center of mass of the transformed object
offsetCFrame is the offset of the object to it's new position
*/
SymmetricMat3 getTransformedInertiaAroundCenterOfMass(const SymmetricMat3& originalInertia, double mass, const Vec3& localCenterOfMass, const CFrame& offsetCFrame, const Vec3& totalCenterOfMass) {
	Vec3 resultingOffset = offsetCFrame.localToGlobal(localCenterOfMass) - totalCenterOfMass;
	return getTransformedInertiaAroundCenterOfMass(originalInertia, mass, CFrame(resultingOffset, offsetCFrame.getRotation()));
}

/*
computes a transformed inertial matrix, where originalInertia is the inertia around the center of mass of the transformed object
offsetCFrame is the offset of the object's center of mass and rotation relative to the COM of it's parent.
*/
SymmetricMat3 getTransformedInertiaAroundCenterOfMass(const SymmetricMat3& originalInertia, double mass, const CFrame& offsetCFrame) {
	SymmetricMat3 translationFactor = skewSymmetricSquared(offsetCFrame.getPosition());
	return getRotatedInertia(originalInertia, offsetCFrame.getRotation()) - translationFactor * mass;
}

/*
computes a transformed inertial matrix, where originalInertia is the inertia around the center of mass of the transformed object
newCenterOfMass is the center around which the new inertia must be calculated
startingCFrame is the current relative position
motion is the relative motion of the offset object's center of mass relative to the total center of mass, in the coordinate system of the total center of mass.
*/
FullTaylor<SymmetricMat3> getTransformedInertiaDerivativesAroundCenterOfMass(const SymmetricMat3& originalInertia, double mass, const CFrame& startingCFrame, const Motion& motion) {
	FullTaylor<Vec3> translation(startingCFrame.getPosition(), motion.translation.translation);
	Rotation originalRotation = startingCFrame.getRotation();
	RotationalMotion rotationMotion = motion.rotation;
	
	FullTaylor<SymmetricMat3> translationFactor = -generateFullTaylorForSkewSymmetricSquared(translation) * mass;
	//translationFactor.constantValue += originalInertia;

	FullTaylor<SymmetricMat3> rotationFactor = getRotatedInertiaTaylor(originalInertia, originalRotation, rotationMotion);

	return translationFactor + rotationFactor;
}
