#include "serialization.h"

#include "../geometry/polyhedron.h"
#include "../geometry/builtinShapeClasses.h"
#include "../geometry/shape.h"
#include "../geometry/shapeClass.h"
#include "../part.h"
#include "../world.h"
#include "../hardconstraints/hardConstraint.h"
#include "../hardconstraints/fixedConstraint.h"
#include "../hardconstraints/motorConstraint.h"
#include "../hardconstraints/sinusoidalPistonConstraint.h"

#include "../constraints/ballConstraint.h"
#include "../constraints/hingeConstraint.h"
#include "../constraints/barConstraint.h"

#include "../externalforces/gravityForce.h"

#include <map>
#include <set>
#include <limits.h>
#include <string>
#include <iostream>


#define CURRENT_VERSION_ID 2

#pragma region serializeComponents

void serializePolyhedron(const Polyhedron& poly, std::ostream& ostream) {
	::serialize<int>(poly.vertexCount, ostream);
	::serialize<int>(poly.triangleCount, ostream);

	for(int i = 0; i < poly.vertexCount; i++) {
		::serialize<Vec3f>(poly.getVertex(i), ostream);
	}
	for(int i = 0; i < poly.triangleCount; i++) {
		::serialize<Triangle>(poly.getTriangle(i), ostream);
	}
}
Polyhedron deserializePolyhedron(std::istream& istream) {
	uint32_t vertexCount = ::deserialize<uint32_t>(istream);
	uint32_t triangleCount = ::deserialize<uint32_t>(istream);

	Vec3f* vertices = new Vec3f[vertexCount];
	Triangle* triangles = new Triangle[triangleCount];

	for(uint32_t i = 0; i < vertexCount; i++) {
		vertices[i] = ::deserialize<Vec3f>(istream);
	}
	for(uint32_t i = 0; i < triangleCount; i++) {
		triangles[i] = ::deserialize<Triangle>(istream);
	}

	Polyhedron result(vertices, triangles, vertexCount, triangleCount);
	delete[] vertices;
	delete[] triangles;
	return result;
}

void ShapeSerializer::include(const Shape& shape) {
	sharedShapeClassSerializer.include(shape.baseShape);
}

void ShapeSerializer::serializeShape(const Shape& shape, std::ostream& ostream) const {
	sharedShapeClassSerializer.serializeIDFor(shape.baseShape, ostream);
	::serialize<double>(shape.getWidth(), ostream);
	::serialize<double>(shape.getHeight(), ostream);
	::serialize<double>(shape.getDepth(), ostream);
}

Shape ShapeDeserializer::deserializeShape(std::istream& istream) const {
	const ShapeClass* baseShape = sharedShapeClassDeserializer.deserializeObject(istream);
	double width = ::deserialize<double>(istream);
	double height = ::deserialize<double>(istream);
	double depth = ::deserialize<double>(istream);
	return Shape(baseShape, width, height, depth);
}

void serializeFixedConstraint(const FixedConstraint& object, std::ostream& ostream) {}
FixedConstraint* deserializeFixedConstraint(std::istream& istream) { return new FixedConstraint(); }

void serializeMotorConstraint(const ConstantSpeedMotorConstraint& constraint, std::ostream& ostream) {
	::serialize<double>(constraint.speed, ostream);
	::serialize<double>(constraint.currentAngle, ostream);
}
ConstantSpeedMotorConstraint* deserializeMotorConstraint(std::istream& istream) {
	double speed = ::deserialize<double>(istream);
	double currentAngle = ::deserialize<double>(istream);

	return new ConstantSpeedMotorConstraint(speed, currentAngle);
}

void serializePistonConstraint(const SinusoidalPistonConstraint& constraint, std::ostream& ostream) {
	::serialize<double>(constraint.minValue, ostream);
	::serialize<double>(constraint.maxValue, ostream);
	::serialize<double>(constraint.period, ostream);
	::serialize<double>(constraint.currentStepInPeriod, ostream);
}
SinusoidalPistonConstraint* deserializePistonConstraint(std::istream& istream) {
	double minLength = ::deserialize<double>(istream);
	double maxLength = ::deserialize<double>(istream);
	double period = ::deserialize<double>(istream);
	double currentStepInPeriod = ::deserialize<double>(istream);

	SinusoidalPistonConstraint* newConstraint = new SinusoidalPistonConstraint(minLength, maxLength, period);
	newConstraint->currentStepInPeriod = currentStepInPeriod;

	return newConstraint;
}
void serializeSinusoidalMotorConstraint(const MotorConstraintTemplate<SineWaveController>& constraint, std::ostream& ostream) {
	::serialize<double>(constraint.minValue, ostream);
	::serialize<double>(constraint.maxValue, ostream);
	::serialize<double>(constraint.period, ostream);
	::serialize<double>(constraint.currentStepInPeriod, ostream);
}
MotorConstraintTemplate<SineWaveController>* deserializeSinusoidalMotorConstraint(std::istream& istream) {
	double minLength = ::deserialize<double>(istream);
	double maxLength = ::deserialize<double>(istream);
	double period = ::deserialize<double>(istream);
	double currentStepInPeriod = ::deserialize<double>(istream);

	MotorConstraintTemplate<SineWaveController>* newConstraint = new MotorConstraintTemplate<SineWaveController>(minLength, maxLength, period);
	newConstraint->currentStepInPeriod = currentStepInPeriod;

	return newConstraint;
}

void serializeBallConstraint(const BallConstraint& constraint, std::ostream& ostream) {
	::serialize<Vec3>(constraint.attachA, ostream);
	::serialize<Vec3>(constraint.attachB, ostream);
}

BallConstraint* deserializeBallConstraint(std::istream& istream) {
	Vec3 attachA = ::deserialize<Vec3>(istream);
	Vec3 attachB = ::deserialize<Vec3>(istream);

	return new BallConstraint(attachA, attachB);
}

void serializeHingeConstraint(const HingeConstraint& constraint, std::ostream& ostream) {
	::serialize<Vec3>(constraint.attachA, ostream);
	::serialize<Vec3>(constraint.axisA, ostream);
	::serialize<Vec3>(constraint.attachB, ostream);
	::serialize<Vec3>(constraint.axisB, ostream);
}

HingeConstraint* deserializeHingeConstraint(std::istream& istream) {
	Vec3 attachA = ::deserialize<Vec3>(istream);
	Vec3 axisA = ::deserialize<Vec3>(istream);
	Vec3 attachB = ::deserialize<Vec3>(istream);
	Vec3 axisB = ::deserialize<Vec3>(istream);

	return new HingeConstraint(attachA, axisA, attachB, axisB);
}

void serializeBarConstraint(const BarConstraint& constraint, std::ostream& ostream) {
	::serialize<Vec3>(constraint.attachA, ostream);
	::serialize<Vec3>(constraint.attachB, ostream);
	::serialize<double>(constraint.length, ostream);
}

BarConstraint* deserializeBarConstraint(std::istream& istream) {
	Vec3 attachA = ::deserialize<Vec3>(istream);
	Vec3 attachB = ::deserialize<Vec3>(istream);
	double length = ::deserialize<double>(istream);

	return new BarConstraint(attachA, attachB, length);
}

void serializePolyhedronShapeClass(const PolyhedronShapeClass& polyhedron, std::ostream& ostream) {
	::serializePolyhedron(polyhedron.asPolyhedron(), ostream);
}
PolyhedronShapeClass* deserializePolyhedronShapeClass(std::istream& istream) {
	Polyhedron poly = ::deserializePolyhedron(istream);
	PolyhedronShapeClass* result = new PolyhedronShapeClass(std::move(poly));
	return result;
}

void serializeDirectionalGravity(const DirectionalGravity& gravity, std::ostream& ostream) {
	::serialize<Vec3>(gravity.gravity, ostream);
}
DirectionalGravity* deserializeDirectionalGravity(std::istream& istream) {
	Vec3 g = ::deserialize<Vec3>(istream);
	return new DirectionalGravity(g);
}

#pragma endregion

#pragma region serializePartPhysicalAndRelated

static void serializeLayer(const Part& part, std::ostream& ostream) {
	::serialize<uint32_t>(part.getLayerID(), ostream);
}
static WorldLayer* deserializeLayer(std::vector<ColissionLayer>& knownLayers, std::istream& istream) {
	uint32_t id = ::deserialize<uint32_t>(istream);
	return getLayerByID(knownLayers, id);
}


void SerializationSessionPrototype::serializePartData(const Part& part, std::ostream& ostream) {
	shapeSerializer.serializeShape(part.hitbox, ostream);
	::serialize<PartProperties>(part.properties, ostream);
	this->serializePartExternalData(part, ostream);
}
void SerializationSessionPrototype::serializePartExternalData(const Part& part, std::ostream& ostream) {
	// no extra data by default
}
Part* DeSerializationSessionPrototype::deserializePartData(const GlobalCFrame& cframe, WorldLayer* layer, std::istream& istream) {
	Shape shape = shapeDeserializer.deserializeShape(istream);
	PartProperties properties = ::deserialize<PartProperties>(istream);
	Part* result = this->deserializePartExternalData(Part(shape, cframe, properties), istream);
	result->layer = layer;
	return result;
}
Part* DeSerializationSessionPrototype::deserializePartExternalData(Part&& part, std::istream& istream) {
	return new Part(std::move(part));
}

void SerializationSessionPrototype::serializeRigidBodyInContext(const RigidBody& rigidBody, std::ostream& ostream) {
	serializeLayer(*rigidBody.mainPart, ostream);
	serializePartData(*rigidBody.mainPart, ostream);
	::serialize<uint32_t>(static_cast<uint32_t>(rigidBody.parts.size()), ostream);
	for(const AttachedPart& atPart : rigidBody.parts) {
		::serialize<CFrame>(atPart.attachment, ostream);
		serializeLayer(*atPart.part, ostream);
		serializePartData(*atPart.part, ostream);
	}
}

RigidBody DeSerializationSessionPrototype::deserializeRigidBodyWithContext(const GlobalCFrame& cframeOfMain, std::vector<ColissionLayer>& layers, std::istream& istream) {
	WorldLayer* layer = deserializeLayer(layers, istream);
	Part* mainPart = deserializePartData(cframeOfMain, layer, istream);
	RigidBody result(mainPart);
	uint32_t size = ::deserialize<uint32_t>(istream);
	result.parts.reserve(size);
	for(uint32_t i = 0; i < size; i++) {
		CFrame attach = ::deserialize<CFrame>(istream);
		WorldLayer* layer = deserializeLayer(layers, istream); 
		Part* newPart = deserializePartData(cframeOfMain.localToGlobal(attach), layer, istream);
		result.parts.push_back(AttachedPart{attach, newPart});
	}
	return result;
}


void SerializationSessionPrototype::serializeConstraintInContext(const PhysicalConstraint& constraint, std::ostream& ostream) {
	std::uint32_t indexA = this->physicalIndexMap[constraint.physA];
	std::uint32_t indexB = this->physicalIndexMap[constraint.physB];

	serialize<std::uint32_t>(indexA, ostream);
	serialize<std::uint32_t>(indexB, ostream);

	dynamicConstraintSerializer.serialize(*constraint.constraint, ostream);
}

PhysicalConstraint DeSerializationSessionPrototype::deserializeConstraintInContext(std::istream& istream) {
	std::uint32_t indexA = deserialize<std::uint32_t>(istream);
	std::uint32_t indexB = deserialize<std::uint32_t>(istream);

	Physical* physA = indexToPhysicalMap[indexA];
	Physical* physB = indexToPhysicalMap[indexB];

	return PhysicalConstraint(physA, physB, dynamicConstraintSerializer.deserialize(istream));
}


static void serializeHardPhysicalConnection(const HardPhysicalConnection& connection, std::ostream& ostream) {
	::serialize<CFrame>(connection.attachOnChild, ostream);
	::serialize<CFrame>(connection.attachOnParent, ostream);

	dynamicHardConstraintSerializer.serialize(*connection.constraintWithParent, ostream);
}

static HardPhysicalConnection deserializeHardPhysicalConnection(std::istream& istream) {
	CFrame attachOnChild = ::deserialize<CFrame>(istream);
	CFrame attachOnParent = ::deserialize<CFrame>(istream);

	HardConstraint* constraint = dynamicHardConstraintSerializer.deserialize(istream);

	return HardPhysicalConnection(std::unique_ptr<HardConstraint>(constraint), attachOnChild, attachOnParent);
}

void SerializationSessionPrototype::serializePhysicalInContext(const Physical& phys, std::ostream& ostream) {
	physicalIndexMap.emplace(&phys, currentPhysicalIndex++);
	serializeRigidBodyInContext(phys.rigidBody, ostream);
	::serialize<uint32_t>(static_cast<uint32_t>(phys.childPhysicals.size()), ostream);
	for(const ConnectedPhysical& p : phys.childPhysicals) {
		serializeHardPhysicalConnection(p.connectionToParent, ostream);
		serializePhysicalInContext(p, ostream);
	}
}

void SerializationSessionPrototype::serializeMotorizedPhysicalInContext(const MotorizedPhysical& phys, std::ostream& ostream) {
	::serialize<Motion>(phys.motionOfCenterOfMass, ostream);
	::serialize<GlobalCFrame>(phys.getMainPart()->getCFrame(), ostream);

	serializePhysicalInContext(phys, ostream);
}

void DeSerializationSessionPrototype::deserializeConnectionsOfPhysicalWithContext(std::vector<ColissionLayer>& layers, Physical& physToPopulate, std::istream& istream) {
	uint32_t childrenCount = ::deserialize<uint32_t>(istream);
	physToPopulate.childPhysicals.reserve(childrenCount);
	for(uint32_t i = 0; i < childrenCount; i++) {
		HardPhysicalConnection connection = deserializeHardPhysicalConnection(istream);
		GlobalCFrame cframeOfConnectedPhys = physToPopulate.getCFrame().localToGlobal(connection.getRelativeCFrameToParent());
		RigidBody b = deserializeRigidBodyWithContext(cframeOfConnectedPhys, layers, istream);
		physToPopulate.childPhysicals.emplace_back(std::move(b), &physToPopulate, std::move(connection));
		ConnectedPhysical& currentlyWorkingOn = physToPopulate.childPhysicals.back();
		indexToPhysicalMap.push_back(static_cast<Physical*>(&currentlyWorkingOn));
		deserializeConnectionsOfPhysicalWithContext(layers, currentlyWorkingOn, istream);
	}
}

MotorizedPhysical* DeSerializationSessionPrototype::deserializeMotorizedPhysicalWithContext(std::vector<ColissionLayer>& layers, std::istream& istream) {
	Motion motion = ::deserialize<Motion>(istream);
	GlobalCFrame cf = ::deserialize<GlobalCFrame>(istream);
	MotorizedPhysical* mainPhys = new MotorizedPhysical(deserializeRigidBodyWithContext(cf, layers, istream));
	indexToPhysicalMap.push_back(static_cast<Physical*>(mainPhys));
	mainPhys->motionOfCenterOfMass = motion;

	deserializeConnectionsOfPhysicalWithContext(layers, *mainPhys, istream);

	mainPhys->refreshPhysicalProperties();
	return mainPhys;
}

#pragma endregion

#pragma region serializeWorld

#pragma region information collection

void SerializationSessionPrototype::collectPartInformation(const Part& part) {
	this->shapeSerializer.include(part.hitbox);
}

void SerializationSessionPrototype::collectPhysicalInformation(const Physical& phys) {
	for(const Part& p : phys.rigidBody) {
		collectPartInformation(p);
	}

	for(const ConnectedPhysical& p : phys.childPhysicals) {
		collectConnectedPhysicalInformation(p);
	}
}

void SerializationSessionPrototype::collectMotorizedPhysicalInformation(const MotorizedPhysical& motorizedPhys) {
	collectPhysicalInformation(motorizedPhys);
}
void SerializationSessionPrototype::collectConnectedPhysicalInformation(const ConnectedPhysical& connectedPhys) {
	collectPhysicalInformation(connectedPhys);
}

#pragma endregion

static void serializeVersion(std::ostream& ostream) {
	::serialize<int32_t>(CURRENT_VERSION_ID, ostream);
}

static void assertVersionCorrect(std::istream& istream) {
	uint32_t readVersionID = ::deserialize<uint32_t>(istream);
	if(readVersionID != CURRENT_VERSION_ID) {
		throw SerializationException(
			"This serialization version is outdated and cannot be read! Current " +
			std::to_string(CURRENT_VERSION_ID) +
			" version from stream: " +
			std::to_string(readVersionID)
		);
	}
}


void SerializationSessionPrototype::serializeWorldLayer(const WorldLayer& layer, std::ostream& ostream) {
	uint32_t numberOfUnPhysicaledPartsInLayer = 0;
	layer.tree.forEach([&numberOfUnPhysicaledPartsInLayer](const Part& p) {
		if(p.parent == nullptr) {
			numberOfUnPhysicaledPartsInLayer++;
		}
	});

	::serialize<uint32_t>(numberOfUnPhysicaledPartsInLayer, ostream);
	layer.tree.forEach([this, &ostream](const Part& p) {
		if(p.parent == nullptr) {
			::serialize<GlobalCFrame>(p.getCFrame(), ostream);
			this->serializePartData(p, ostream);
		}
	});
}

void SerializationSessionPrototype::serializeWorld(const WorldPrototype& world, std::ostream& ostream) {
	for(const MotorizedPhysical* p : world.physicals) {
		collectMotorizedPhysicalInformation(*p);
	}
	for(const ColissionLayer& clayer : world.layers) {
		for(const WorldLayer& layer : clayer.subLayers) {
			layer.tree.forEach([this](const Part& p) {
				if(p.parent == nullptr) {
					collectPartInformation(p);
				}
			});
		}
	}

	serializeCollectedHeaderInformation(ostream);
	

	// actually serialize the world

	::serialize<uint64_t>(world.age, ostream);

	::serialize<uint32_t>(world.getLayerCount(), ostream);
	for(int i = 0; i < world.getLayerCount(); i++) {
		for(int j = 0; j <= i; j++) {
			::serialize<bool>(world.doLayersCollide(i, j), ostream);
		}
	}

	for(const ColissionLayer& layer : world.layers) {
		serializeWorldLayer(layer.subLayers[ColissionLayer::TERRAIN_PARTS_LAYER], ostream);
	}

	::serialize<uint32_t>(world.physicals.size(), ostream);
	for(const MotorizedPhysical* p : world.physicals) {
		serializeMotorizedPhysicalInContext(*p, ostream);
	}

	::serialize<std::uint32_t>(static_cast<std::uint32_t>(world.constraints.size()), ostream);
	for(const ConstraintGroup& cg : world.constraints) {
		::serialize<std::uint32_t>(static_cast<std::uint32_t>(cg.constraints.size()), ostream);
		for(const PhysicalConstraint& c : cg.constraints) {
			this->serializeConstraintInContext(c, ostream);
		}
	}
	::serialize<uint32_t>(world.externalForces.size(), ostream);
	for(ExternalForce* force : world.externalForces) {
		dynamicExternalForceSerializer.serialize(*force, ostream);
	}
}

void DeSerializationSessionPrototype::deserializeWorldLayer(WorldLayer& layer, std::istream& istream) {
	uint32_t extraPartsInLayer = ::deserialize<uint32_t>(istream);
	for(uint32_t i = 0; i < extraPartsInLayer; i++) {
		GlobalCFrame cf = ::deserialize<GlobalCFrame>(istream);
		layer.tree.add(deserializePartData(cf, &layer, istream));
	}
}

void DeSerializationSessionPrototype::deserializeWorld(WorldPrototype& world, std::istream& istream) {
	this->deserializeAndCollectHeaderInformation(istream);

	world.age = ::deserialize<uint64_t>(istream);

	world.layers.clear();
	uint32_t layerCount = ::deserialize<uint32_t>(istream);
	world.layers.reserve(layerCount);
	for(uint32_t i = 0; i < layerCount; i++) {
		world.layers.emplace_back(&world, false);
	}
	for(int i = 0; i < world.getLayerCount(); i++) {
		for(int j = 0; j <= i; j++) {
			bool layersCollide = ::deserialize<bool>(istream);
			world.setLayersCollide(i, j, layersCollide);
		}
	}
	for(ColissionLayer& layer : world.layers) {
		deserializeWorldLayer(layer.subLayers[ColissionLayer::TERRAIN_PARTS_LAYER], istream);
	}

	uint32_t numberOfPhysicals = ::deserialize<uint32_t>(istream);
	world.physicals.reserve(numberOfPhysicals);
	for(uint32_t i = 0; i < numberOfPhysicals; i++) {
		world.addPhysicalWithExistingLayers(deserializeMotorizedPhysicalWithContext(world.layers, istream));
	}

	std::uint32_t constraintCount = ::deserialize<std::uint32_t>(istream);
	world.constraints.reserve(constraintCount);
	for(std::uint32_t cg = 0; cg < constraintCount; cg++) {
		ConstraintGroup group;
		std::uint32_t numberOfConstraintsInGroup = ::deserialize<std::uint32_t>(istream);
		for(std::uint32_t c = 0; c < numberOfConstraintsInGroup; c++) {
			group.constraints.push_back(this->deserializeConstraintInContext(istream));
		}
		world.constraints.push_back(std::move(group));
	}
	uint32_t forceCount = ::deserialize<uint32_t>(istream);
	world.externalForces.reserve(forceCount);
	for(uint32_t i = 0; i < forceCount; i++) {
		ExternalForce* force = dynamicExternalForceSerializer.deserialize(istream);
		world.externalForces.push_back(force);
	}
}

void SerializationSessionPrototype::serializeParts(const Part* const parts[], size_t partCount, std::ostream& ostream) {
	for(size_t i = 0; i < partCount; i++) {
		collectPartInformation(*(parts[i]));
	}
	serializeCollectedHeaderInformation(ostream);
	::serialize<uint32_t>(static_cast<uint32_t>(partCount), ostream);
	for(size_t i = 0; i < partCount; i++) {
		::serialize<GlobalCFrame>(parts[i]->getCFrame(), ostream);
		serializePartData(*(parts[i]), ostream);
	}
}

std::vector<Part*> DeSerializationSessionPrototype::deserializeParts(std::istream& istream) {
	deserializeAndCollectHeaderInformation(istream);
	size_t numberOfParts = ::deserialize<uint32_t>(istream);
	std::vector<Part*> result;
	result.reserve(numberOfParts);
	for(size_t i = 0; i < numberOfParts; i++) {
		GlobalCFrame cframeOfPart = ::deserialize<GlobalCFrame>(istream);
		Part* newPart = deserializePartData(cframeOfPart, nullptr, istream);
		result.push_back(newPart);
	}
	return result;
}


void SerializationSessionPrototype::serializeCollectedHeaderInformation(std::ostream& ostream) {
	serializeVersion(ostream);
	this->shapeSerializer.sharedShapeClassSerializer.serializeRegistry([](const ShapeClass* sc, std::ostream& ostream) {dynamicShapeClassSerializer.serialize(*sc, ostream); }, ostream);
}

void DeSerializationSessionPrototype::deserializeAndCollectHeaderInformation(std::istream& istream) {
	assertVersionCorrect(istream);
	shapeDeserializer.sharedShapeClassDeserializer.deserializeRegistry([](std::istream& istream) {return dynamicShapeClassSerializer.deserialize(istream); }, istream);
}

static const ShapeClass* builtinKnownShapeClasses[]{&CubeClass::instance, &SphereClass::instance, &CylinderClass::instance};
SerializationSessionPrototype::SerializationSessionPrototype(const std::vector<const ShapeClass*>& knownShapeClasses) : shapeSerializer(builtinKnownShapeClasses) {
	for(const ShapeClass* sc : knownShapeClasses) {
		shapeSerializer.sharedShapeClassSerializer.addPredefined(sc);
	}
}

DeSerializationSessionPrototype::DeSerializationSessionPrototype(const std::vector<const ShapeClass*>& knownShapeClasses) : shapeDeserializer(builtinKnownShapeClasses) {
	for(const ShapeClass* sc : knownShapeClasses) {
		shapeDeserializer.sharedShapeClassDeserializer.addPredefined(sc);
	}
}

#pragma endregion

#pragma region dynamic serializers

static DynamicSerializerRegistry<HardConstraint>::ConcreteDynamicSerializer<FixedConstraint> fixedConstraintSerializer
(serializeFixedConstraint, deserializeFixedConstraint, 0);
static DynamicSerializerRegistry<HardConstraint>::ConcreteDynamicSerializer<ConstantSpeedMotorConstraint> motorConstraintSerializer
(serializeMotorConstraint, deserializeMotorConstraint, 1);
static DynamicSerializerRegistry<HardConstraint>::ConcreteDynamicSerializer<SinusoidalPistonConstraint> pistonConstraintSerializer
(serializePistonConstraint, deserializePistonConstraint, 2);
static DynamicSerializerRegistry<HardConstraint>::ConcreteDynamicSerializer<MotorConstraintTemplate<SineWaveController>> sinusiodalMotorConstraintSerializer
(serializeSinusoidalMotorConstraint, deserializeSinusoidalMotorConstraint, 3);

static DynamicSerializerRegistry<Constraint>::ConcreteDynamicSerializer<BallConstraint> ballConstraintSerializer
(serializeBallConstraint, deserializeBallConstraint, 0);
static DynamicSerializerRegistry<Constraint>::ConcreteDynamicSerializer<HingeConstraint> hingeConstraintSerializer
(serializeHingeConstraint, deserializeHingeConstraint, 1);
static DynamicSerializerRegistry<Constraint>::ConcreteDynamicSerializer<BarConstraint> barConstraintSerializer
(serializeBarConstraint, deserializeBarConstraint, 2);

static DynamicSerializerRegistry<ShapeClass>::ConcreteDynamicSerializer<PolyhedronShapeClass> polyhedronSerializer
(serializePolyhedronShapeClass, deserializePolyhedronShapeClass, 0);

static DynamicSerializerRegistry<ExternalForce>::ConcreteDynamicSerializer<DirectionalGravity> gravitySerializer
(serializeDirectionalGravity, deserializeDirectionalGravity, 0);

DynamicSerializerRegistry<Constraint> dynamicConstraintSerializer{
	{typeid(BallConstraint), &ballConstraintSerializer},
	{typeid(HingeConstraint), &hingeConstraintSerializer},
	{typeid(BarConstraint), &barConstraintSerializer}
};
DynamicSerializerRegistry<HardConstraint> dynamicHardConstraintSerializer{
	{typeid(FixedConstraint), &fixedConstraintSerializer},
	{typeid(ConstantSpeedMotorConstraint), &motorConstraintSerializer},
	{typeid(SinusoidalPistonConstraint), &pistonConstraintSerializer},
	{typeid(MotorConstraintTemplate<SineWaveController>), &sinusiodalMotorConstraintSerializer}
};
DynamicSerializerRegistry<ShapeClass> dynamicShapeClassSerializer{
	{typeid(PolyhedronShapeClass), &polyhedronSerializer}
};
DynamicSerializerRegistry<ExternalForce> dynamicExternalForceSerializer{
	{typeid(DirectionalGravity), &gravitySerializer}
};

#pragma endregion
