#include "core.h"

#include "modelLayer.h"

#include "view/screen.h"
#include "shader/shaders.h"
#include "extendedPart.h"
#include "worlds.h"

#include "../engine/ecs/registry.h"
#include "ecs/components.h"
#include "ecs/material.h"

#include "../graphics/meshRegistry.h"

#include "../graphics/mesh/indexedMesh.h"
#include "../graphics/debug/visualDebug.h"

#include "../graphics/gui/color.h"

#include "../physics/math/linalg/vec.h"
#include "../physics/misc/filters/visibilityFilter.h"

#include "../util/resource/resourceManager.h"
#include "../layer/shadowLayer.h"

#include "../graphics/batch/instanceBatchManager.h"
#include "picker/tools/selectionTool.h"

namespace P3D::Application {

enum class RelationToSelectedPart {
	NONE,
	SELF,
	DIRECT_ATTACH,
	MAINPART,
	PHYSICAL_ATTACH,
	MAINPHYSICAL_ATTACH
};

static RelationToSelectedPart getRelationToSelectedPart(const Part* selectedPart, const Part* testPart) {
	if (selectedPart == nullptr)
		return RelationToSelectedPart::NONE;

	if (testPart == selectedPart)
		return RelationToSelectedPart::SELF;

	if (selectedPart->parent != nullptr && testPart->parent != nullptr) {
		if (testPart->parent == selectedPart->parent) {
			if (testPart->isMainPart())
				return RelationToSelectedPart::MAINPART;
			else
				return RelationToSelectedPart::DIRECT_ATTACH;
		} else if (testPart->parent->mainPhysical == selectedPart->parent->mainPhysical) {
			if (testPart->parent->isMainPhysical())
				return RelationToSelectedPart::MAINPHYSICAL_ATTACH;
			else
				return RelationToSelectedPart::PHYSICAL_ATTACH;
		}
	}

	return RelationToSelectedPart::NONE;
}

static Color getAmbientForPartForSelected(Screen* screen, Part* part) {
	switch (getRelationToSelectedPart(screen->selectedPart, part)) {
		case RelationToSelectedPart::NONE:
			return Color(0.0f, 0, 0, 0);
		case RelationToSelectedPart::SELF:
			return Color(0.5f, 0, 0, 0);
		case RelationToSelectedPart::DIRECT_ATTACH:
			return Color(0, 0.25f, 0, 0);
		case RelationToSelectedPart::MAINPART:
			return Color(0, 1.0f, 0, 0);
		case RelationToSelectedPart::PHYSICAL_ATTACH:
			return Color(0, 0, 0.25f, 0);
		case RelationToSelectedPart::MAINPHYSICAL_ATTACH:
			return Color(0, 0, 1.0f, 0);
	}

	return Color(0, 0, 0, 0);
}

static Color getAlbedoForPart(Screen* screen, ExtendedPart* part) {
	Color computedAmbient = getAmbientForPartForSelected(screen, part);

	if (part->entity == screen->intersectedEntity)
		computedAmbient = Vec4f(computedAmbient) + Vec4f(-0.1f, -0.1f, -0.1f, 0);

	return computedAmbient;
}

void ModelLayer::onInit(Engine::Registry64& registry) {
	using namespace Graphics;
	Screen* screen = static_cast<Screen*>(this->ptr);

	// Dont know anymore
	Shaders::basicShader.updateTexture(false);
	Shaders::instanceShader.updateTexture(false);

	// Instance batch manager
	manager = new InstanceBatchManager<Uniform>(DEFAULT_UNIFORM_BUFFER_LAYOUT);
}

void ModelLayer::onUpdate(Engine::Registry64& registry) {
	auto view = registry.view<Comp::Light>();

	std::size_t index = 0;
	for (auto entity : view) {
		Ref<Comp::Light> light = view.get<Comp::Light>(entity);

		Position position;
		Ref<Comp::Transform> transform = registry.get<Comp::Transform>(entity);
		if (transform.valid()) 
			position = transform->getCFrame().getPosition();
		
		Shaders::basicShader.updateLight(index, position, *light);
		Shaders::instanceShader.updateLight(index, position, *light);
		
		index += 1;
	}

	Shaders::basicShader.updateLightCount(index);
	Shaders::instanceShader.updateLightCount(index);
}

void ModelLayer::onEvent(Engine::Registry64& registry, Engine::Event& event) {

}
		
void ModelLayer::onRender(Engine::Registry64& registry) {
	using namespace Graphics;
	using namespace Renderer;
	Screen* screen = static_cast<Screen*>(this->ptr);

	beginScene();

	graphicsMeasure.mark(UPDATE);
	Shaders::debugShader.updateProjection(screen->camera.viewMatrix, screen->camera.projectionMatrix, screen->camera.cframe.position);
	Shaders::basicShader.updateProjection(screen->camera.viewMatrix, screen->camera.projectionMatrix, screen->camera.cframe.position);
	Shaders::instanceShader.updateProjection(screen->camera.viewMatrix, screen->camera.projectionMatrix, screen->camera.cframe.position);

	// Shadow
	Vec3f from = { -10, 10, -10 };
	Vec3f to = { 0, 0, 0 };
	Vec3f sunDirection = to - from;
	ShadowLayer::lightView = lookAt(from, to);
	ShadowLayer::lighSpaceMatrix = ShadowLayer::lightProjection * ShadowLayer::lightView;
	activeTexture(1);
	activeTexture(1);
	bindTexture2D(ShadowLayer::depthMap);
	Shaders::instanceShader.setUniform("shadowMap", 1);
	Shaders::instanceShader.setUniform("lightMatrix", ShadowLayer::lighSpaceMatrix);
	Shaders::instanceShader.updateSunDirection(sunDirection);

	graphicsMeasure.mark(PHYSICALS);
	
	// Filter on mesh ID and transparency
	struct EntityInfo {
		Engine::Registry64::entity_type entity = 0;
		Comp::Transform transform;
		Comp::Material material;
		Ref<Comp::Mesh> mesh;
		Ref<Comp::Collider> collider;
	};
	
	std::map<double, EntityInfo> transparentEntities;
	screen->world->syncReadOnlyOperation([this, &transparentEntities, screen, &registry] () {
		VisibilityFilter filter = VisibilityFilter::forWindow(screen->camera.cframe.position, screen->camera.getForwardDirection(), screen->camera.getUpDirection(), screen->camera.fov, screen->camera.aspect, screen->camera.zfar);

		auto view = registry.view<Comp::Mesh>();
		for (auto entity : view) {
			EntityInfo info;
			info.entity = entity;
			
			info.mesh = view.get<Comp::Mesh>(entity);
			if (!info.mesh.valid())
				continue;

			if (info.mesh->id == -1)
				continue;
			
			info.collider = registry.get<Comp::Collider>(entity);
			if (info.collider.valid())
				if (!filter(*info.collider->part))
					continue;

			info.transform = registry.getOr<Comp::Transform>(entity);
			info.material = registry.getOr<Comp::Material>(entity);
			
			if (info.material.albedo.a < 1.0f) {
				double distance = lengthSquared(Vec3(screen->camera.cframe.position - info.transform.getPosition()));
				transparentEntities.insert(std::make_pair(distance, info));
			} else {
				Mat4f modelMatrix = info.transform.getModelMatrix();

				if (info.collider.valid())
					info.material.albedo += getAlbedoForPart(screen, info.collider->part);

				Uniform uniform {
					modelMatrix,
					info.material.albedo,
					info.material.metalness,
					info.material.roughness,
					info.material.ao
				};
				
				manager->add(info.mesh->id, uniform);
			}
		}
		
		Shaders::instanceShader.bind();
		manager->submit();
		

		// Render transparent meshes
		Shaders::basicShader.bind();
		enableBlending();
		for (auto iterator = transparentEntities.rbegin(); iterator != transparentEntities.rend(); ++iterator) {
			EntityInfo info = iterator->second;

			if (info.mesh->id == -1)
				continue;

			if (info.collider.valid())
				info.material.albedo += getAlbedoForPart(screen, info.collider->part);

			Shaders::basicShader.updateMaterial(info.material);
			Shaders::basicShader.updateModel(info.transform.getModelMatrix());
			MeshRegistry::meshes[info.mesh->id]->render(info.mesh->mode);
		}

		// Hitbox drawing
		if (screen->selectedEntity) {
			Ref<Comp::Transform> transform = registry.get<Comp::Transform>(screen->selectedEntity);
			if (transform.valid()) {
				Ref<Comp::Hitbox> hitbox = registry.get<Comp::Hitbox>(screen->selectedEntity);

				if (hitbox.valid()) {
					Shape shape = hitbox->getShape();
					DiagonalMat3 scale = transform->getScale();

					if (!hitbox->isPartAttached())
						scale = scale * hitbox->getScale();		
					
					VisualData data = MeshRegistry::getOrCreateMeshFor(shape.baseShape);

					Shaders::debugShader.updateModel(transform->getCFrame().asMat4WithPreScale(scale));
					MeshRegistry::meshes[data.id]->render();
				}
			}
		}
		auto scf = SelectionTool::selection.getCFrame();
		auto shb = SelectionTool::selection.getHitbox();
		if (scf.has_value() && shb.has_value()) {
			VisualData data = MeshRegistry::getOrCreateMeshFor(shb->baseShape);
			Shaders::debugShader.updateModel(scf.value().asMat4WithPreScale(shb->scale));
			MeshRegistry::meshes[data.id]->render();
		}
		
		// Hitbox drawing
		for (auto entity : SelectionTool::selection) {
			Ref<Comp::Transform> transform = registry.get<Comp::Transform>(entity);
			if (transform.valid()) {
				Ref<Comp::Hitbox> hitbox = registry.get<Comp::Hitbox>(entity);

				if (hitbox.valid()) {
					Shape shape = hitbox->getShape();

					if (!hitbox->isPartAttached())
						shape = shape.scaled(transform->getScale());

					VisualData data = MeshRegistry::getOrCreateMeshFor(shape.baseShape);

					Shaders::debugShader.updateModel(transform->getCFrame().asMat4WithPreScale(shape.scale));
					MeshRegistry::meshes[data.id]->render();
				}
			}
		}
	});

	endScene();
}

void ModelLayer::onClose(Engine::Registry64& registry) {

}

};