#include "Systems.h"

#include "Scene.h"
#include "Components.h"

#include "Renderer.h" // for debug only

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

namespace Quack
{
	void ApplyForces(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto physicsView = registry.view<PhysicsComponent>();

		for (auto entity : physicsView)
		{
			auto& [physics] = physicsView.get(entity);

			physics.forces = physics.gravity * physics.mass; // For now only one force, gravity
		}
	}


	void Update(const std::shared_ptr<Scene> scene, float dt)
	{
		auto& registry = scene->GetRegistry();
		auto physicsView = registry.view<PhysicsComponent>();

		for (auto entity : physicsView)
		{
			auto& [physics] = physicsView.get(entity);

			// linear motion
			physics.oldPosition = physics.position;
			glm::vec3 oldVelocity = physics.velocity;

			glm::vec3 acceleration = physics.forces * (1.0f / physics.mass); // forces * inverse of mass

			physics.velocity = physics.velocity * physics.friction + acceleration * dt;
			physics.position += (oldVelocity + physics.velocity) * 0.5f * dt;

			// angular motion

			//glm::mat3 R = glm::mat3(physics.orientation);
			//glm::mat3 wInvI = R * physics.invInertiaTensor * glm::transpose(R);
			//glm::mat3 wInvI = glm::mat3(1.f) * (1 / (1.0f / 6.0f) * physics.mass * 0.25f);

			glm::vec3 angularAcceleration = physics.invInertiaTensor * physics.torques;
			physics.angularVelocity += angularAcceleration * dt;

			// TEMP!!!
			physics.angularVelocity *= physics.friction;

			physics.orientation += 0.5f * glm::quat(0.f, physics.angularVelocity) * physics.orientation * dt;
			physics.orientation = glm::normalize(physics.orientation);

			physics.torques = glm::vec3(0.f);
		}
	}

	void UpdateTransform(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto movableView = registry.view<PhysicsComponent, TransformComponent>();

		for (auto entity : movableView)
		{
			auto& [physics, transform] = movableView.get(entity);

			glm::vec3 scale;
			scale.x = glm::length(transform.transform[0]);
			scale.y = glm::length(transform.transform[1]);
			scale.z = glm::length(transform.transform[2]);

			transform.transform = glm::translate(glm::mat4(1.0f), physics.position);
			transform.transform *= glm::toMat4(physics.orientation);
			transform.transform *= glm::scale(glm::mat4(1.f), scale);
		}

		auto constraintView = registry.view<ConstraintComponent, TransformComponent>();

		for (auto entity : constraintView)
		{
			auto& [constraint, transform] = constraintView.get(entity);
			transform.transform = glm::translate(glm::mat4(1.0f), constraint.position);
			transform.transform *= glm::toMat4(constraint.orientation);
		}

	}

	void SolveConstraints(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto collisionView = registry.view<CollisionComponent, PhysicsComponent>();

		auto constraints = registry.view<ConstraintComponent>();

		for (auto entity : collisionView)
		{
			auto& [collision, physics] = collisionView.get(entity);

			for (auto constraint : constraints)
			{
				// each constraint component is a floor
				const auto& floor = constraints.get<ConstraintComponent>(constraint);
				glm::mat3 floorAxes = glm::toMat3(floor.orientation) * glm::mat3(1.f);

				// @TODO: find better way to distinct if collision shape is sphere or cube
				if (collision.radius > 0.f)
				{
					// vector from sphere to floor
					glm::vec3 d = physics.position - floor.position;

					// Find the closest point on OBB to the sphere center
					float distX = dot(d, floorAxes[0]);
					float distY = dot(d, floorAxes[1]);
					float distZ = dot(d, floorAxes[2]);

					// Without clamping the point would the sphere center
					distX = glm::clamp(distX, -floor.halfSize.x, floor.halfSize.x);
					distY = glm::clamp(distY, -floor.halfSize.y, floor.halfSize.y);
					distZ = glm::clamp(distZ, -floor.halfSize.z, floor.halfSize.z);

					// dist- are in floor local space, in that local space they are not rotated so we have to rotate them
					// axes are our "portal"/"bridge" between floor local space and world space, they represent the floor rotated axes
					// - that's why we multiply dist- by corresponding axes
					glm::vec3 closestPoint = floor.position + distX * floorAxes[0] + distY * floorAxes[1] + distZ * floorAxes[2];

					//Renderer::DrawPoint(closestPoint);

					glm::vec3 diff = physics.position - closestPoint;
					float distanceSquared = dot(diff, diff); // distance from closes point to the sphere. length of diff without expensive sqrt

					if (distanceSquared <= collision.radius * collision.radius)
					{
						glm::vec3 collisionNormal = normalize(diff); // direction from OBB to sphere

						float penetration = collision.radius - sqrt(distanceSquared); // @TODO: can I omit the sqrt here?

						physics.position += collisionNormal * penetration;

						glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
						glm::vec3 velocityTangental = physics.velocity - velocityNormal;		// parallel to the floor

						physics.velocity = velocityTangental - velocityNormal * physics.bounce;

						// This line prevents jittering when sphere bounces forever (on not oriented surface) but prevents the sphere from rolling down sloped/oriented surface
						//physics.velocity = physics.velocity.y < 0.21f ? glm::vec3(0.f) : physics.velocity; // to prevent jittering
					}
				}
				else
				{
					// OBB - OBB collision (SAT)
					// !!!! Right now edge - edge will be detected as collision even when there isn't any

					// @TODO: Change manual defined corners to a nice short loop!

					glm::mat3 entityAxes = glm::toMat3(physics.orientation) * glm::mat3(1.f);

					std::vector<glm::vec3> axes = { entityAxes[0], entityAxes[1], entityAxes[2], floorAxes[0], floorAxes[1], floorAxes[2] };

					// Visualize axes from entity
					//Renderer::DrawLine(glm::vec3(0.f), entityAxes[0] * 5.f, glm::vec3(1.f, 0.f, 0.f));
					//Renderer::DrawLine(glm::vec3(0.f), entityAxes[1] * 5.f, glm::vec3(0.f, 1.f, 0.f));
					//Renderer::DrawLine(glm::vec3(0.f), entityAxes[2] * 5.f, glm::vec3(0.f, 0.f, 1.f));
					// Visualize axes from floor
					//Renderer::DrawLine(glm::vec3(0.f), floorAxes[0] * 5.f, glm::vec3(1.f, 0.f, 0.f));
					//Renderer::DrawLine(glm::vec3(0.f), floorAxes[1] * 5.f, glm::vec3(0.f, 1.f, 0.f));
					//Renderer::DrawLine(glm::vec3(0.f), floorAxes[2] * 5.f, glm::vec3(0.f, 0.f, 1.f));


					// Entity corners in local space
					glm::vec3 topRightFrontCorner = collision.halfSize;
					glm::vec3 topRightBackCorner = glm::vec3(collision.halfSize.x, collision.halfSize.y, -collision.halfSize.z);

					glm::vec3 bottomRightFrontCorner = glm::vec3(collision.halfSize.x, -collision.halfSize.y, collision.halfSize.z);
					glm::vec3 bottomRightBackCorner = glm::vec3(collision.halfSize.x, -collision.halfSize.y, -collision.halfSize.z);

					glm::vec3 topLeftFrontCorner = glm::vec3(-collision.halfSize.x, collision.halfSize.y, collision.halfSize.z);
					glm::vec3 topLeftBackCorner = glm::vec3(-collision.halfSize.x, collision.halfSize.y, -collision.halfSize.z);

					glm::vec3 bottomLeftFrontCorner = glm::vec3(-collision.halfSize.x, -collision.halfSize.y, collision.halfSize.z);
					glm::vec3 bottomLeftBackCorner = -collision.halfSize;

					// Entity corners in world space (rotated and translated)
					// THE ORDER MATTERS!!! vec3 * quat is not the same as quat * vec3!!!!!!!!!!!!!!
					glm::vec3 wTopRightFrontCorner = (physics.orientation * topRightFrontCorner) + physics.position;
					glm::vec3 wTopRightBackCorner = (physics.orientation * topRightBackCorner) + physics.position;

					glm::vec3 wBottomRightFrontCorner = (physics.orientation * bottomRightFrontCorner) + physics.position;
					glm::vec3 wBottomRightBackCorner = (physics.orientation * bottomRightBackCorner) + physics.position;

					glm::vec3 wTopLeftFrontCorner = (physics.orientation * topLeftFrontCorner) + physics.position;
					glm::vec3 wTopLeftBackCorner = (physics.orientation * topLeftBackCorner) + physics.position;

					glm::vec3 wBottomLeftFrontCorner = (physics.orientation * bottomLeftFrontCorner) + physics.position;
					glm::vec3 wBottomLeftBackCorner = (physics.orientation * bottomLeftBackCorner) + physics.position;

					std::vector<glm::vec3> entityPoints = { wTopRightFrontCorner, wTopRightBackCorner,
															wBottomRightFrontCorner, wBottomRightBackCorner,
															wTopLeftFrontCorner, wTopLeftBackCorner,
															wBottomLeftFrontCorner, wBottomLeftBackCorner };

					// Visualize points on the cube
					//Renderer::DrawPoint(wTopRightFrontCorner);
					//Renderer::DrawPoint(wBottomRightFrontCorner);
					//Renderer::DrawPoint(wTopLeftFrontCorner);
					//Renderer::DrawPoint(wBottomLeftFrontCorner);
					//Renderer::DrawPoint(wTopRightBackCorner);
					//Renderer::DrawPoint(wBottomRightBackCorner);
					//Renderer::DrawPoint(wTopLeftBackCorner);
					//Renderer::DrawPoint(wBottomLeftBackCorner);

					// Floor corners in local space
					glm::vec3 floorTopRightFrontCorner = floor.halfSize;
					glm::vec3 floorTopRightBackCorner = glm::vec3(floor.halfSize.x, floor.halfSize.y, -floor.halfSize.z);

					glm::vec3 floorBottomRightFrontCorner = glm::vec3(floor.halfSize.x, -floor.halfSize.y, floor.halfSize.z);
					glm::vec3 floorBottomRightBackCorner = glm::vec3(floor.halfSize.x, -floor.halfSize.y, -floor.halfSize.z);

					glm::vec3 floorTopLeftFrontCorner = glm::vec3(-floor.halfSize.x, floor.halfSize.y, floor.halfSize.z);
					glm::vec3 floorTopLeftBackCorner = glm::vec3(-floor.halfSize.x, floor.halfSize.y, -floor.halfSize.z);

					glm::vec3 floorBottomLeftFrontCorner = glm::vec3(-floor.halfSize.x, -floor.halfSize.y, floor.halfSize.z);
					glm::vec3 floorBottomLeftBackCorner = -floor.halfSize;

					// Floor corners in world space (rotated and translated)
					glm::vec3 wFloorTopRightFrontCorner = (floor.orientation * floorTopRightFrontCorner) + floor.position;
					glm::vec3 wFloorTopRightBackCorner = (floor.orientation * floorTopRightBackCorner) + floor.position;

					glm::vec3 wFloorBottomRightFrontCorner = (floor.orientation * floorBottomRightFrontCorner) + floor.position;
					glm::vec3 wFloorBottomRightBackCorner = (floor.orientation * floorBottomRightBackCorner) + floor.position;

					glm::vec3 wFloorTopLeftFrontCorner = (floor.orientation * floorTopLeftFrontCorner) + floor.position;
					glm::vec3 wFloorTopLeftBackCorner = (floor.orientation * floorTopLeftBackCorner) + floor.position;

					glm::vec3 wFloorBottomLeftFrontCorner = (floor.orientation * floorBottomLeftFrontCorner) + floor.position;
					glm::vec3 wFloorBottomLeftBackCorner = (floor.orientation * floorBottomLeftBackCorner) + floor.position;

					std::vector<glm::vec3> floorPoints = { wFloorTopRightFrontCorner, wFloorTopRightBackCorner,
															wFloorBottomRightFrontCorner, wFloorBottomRightBackCorner,
															wFloorTopLeftFrontCorner, wFloorTopLeftBackCorner,
															wFloorBottomLeftFrontCorner, wFloorBottomLeftBackCorner };

					// Project the points onto axes and find the "shadows"
					std::vector<glm::vec3> entityMins;
					std::vector<glm::vec3> entityMaxs;

					std::vector<glm::vec3> floorMins;
					std::vector<glm::vec3> floorMaxs;

					bool bCollision = true;

					float shortestOverlap = 100000.f;
					glm::vec3 shortestAxis;

					for (const auto& axis : axes)
					{
						float eMin = dot(entityPoints[0], axis);
						float eMax = eMin;

						for (const auto& point : entityPoints)
						{
							float p = dot(point, axis);

							eMin = glm::min(eMin, p);
							eMax = glm::max(eMax, p);
						}

						entityMins.push_back(eMin * axis);
						entityMaxs.push_back(eMax * axis);

						// floor
						float fMin = dot(floorPoints[0], axis);
						float fMax = fMin;

						for (const auto& point : floorPoints)
						{
							float p = dot(point, axis);

							fMin = glm::min(fMin, p);
							fMax = glm::max(fMax, p);
						}

						floorMins.push_back(fMin * axis);
						floorMaxs.push_back(fMax * axis);


						if (eMax < fMin || fMax < eMin)
							bCollision = false;
						else // There is no gap on current axis. We have an overlap on current axis
						{
							// Amount of overlap
							float amount = glm::min(eMax, fMax) - glm::max(eMin, fMin);

							if (amount < shortestOverlap)
							{
								shortestOverlap = amount;
								shortestAxis = axis;
							}
						}
					}


					if (bCollision)
					{
						// Resolve overlap
						glm::vec3 normal = normalize(shortestAxis);
						physics.position += normal * shortestOverlap;

						/// LINEAR FORCES
						glm::vec3 velocityNormal = normal * glm::dot(normal, physics.velocity); // perpendicular to the floor. "Normal to the floor"
						glm::vec3 velocityTangental = physics.velocity - velocityNormal;		// parallel to the floor

						// TEMP!!! Will change when proper impulses are done
						glm::vec3 impulse = velocityTangental - velocityNormal * physics.bounce;

						physics.velocity = impulse;

						//physics.velocity = physics.velocity.y < 0.21f ? glm::vec3(0.f) : physics.velocity; // to prevent jittering

						/// ANGULAR FORCES
						glm::vec3 contactPoint = physics.position - normal * collision.halfSize;
						//Renderer::DrawPoint(contactPoint, glm::vec3(0.f, 1.f, 0.f));

						glm::vec3 r = contactPoint - physics.position;

						// @TODO: FIX temp
						physics.angularVelocity += glm::cross(impulse, r);

					}

					// Visualize the projected intervals for a box
					//for (int i = 0; i < entityMins.size(); i++)
					//{
					//	Renderer::DrawLine(entityMins[i], entityMaxs[i], bCollision ? glm::vec3(1.f, 0.f, 0.f) : glm::vec3(1.f, 1.f, 0.f));
					//}
					// Visualize the projected intervals for the floor
					//for (int i = 0; i < floorMins.size(); i++)
					//{
					//	Renderer::DrawLine(floorMins[i], floorMaxs[i], bCollision ? glm::vec3(1.f, 0.f, 0.f) : glm::vec3(0.f, 1.f, 1.f));
					//}
				}
			}
		}
	}

	void SolveCollision(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto collisionView = registry.view<CollisionComponent, PhysicsComponent>();

		for (auto entity : collisionView)
		{
			auto& [collision, physics] = collisionView.get(entity);

			// @TODO: find better way to determine whether collision shape is a sphere or a cube
			// @TODO: make proper sphere - sphere collision
			if (collision.radius > 0.f)
			{
				for (auto entity2 : collisionView)
				{
					if (entity == entity2)
						continue;

					auto& [collision2, physics2] = collisionView.get(entity2);

					if (collision2.radius > 0.f)
					{
						glm::vec3 diff = physics.position - physics2.position;
						float distanceSquared = dot(diff, diff);
						float radii = collision.radius + collision2.radius; // radiuses

						if (distanceSquared < radii * radii)
						{
							glm::vec3 collisionNormal = normalize(diff); // direction from sphere to sphere

							float penetration = (radii - sqrt(distanceSquared)) / 2.f; // @TODO: can I omit the sqrt here?
							physics.position += collisionNormal * penetration;
							physics2.position += -collisionNormal * penetration;

							{
								glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
								glm::vec3 velocityTangental = physics.velocity - velocityNormal;		// parallel to the floor

								physics.velocity = velocityTangental - velocityNormal * physics.bounce;
							}
							{
								glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics2.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
								glm::vec3 velocityTangental = physics2.velocity - velocityNormal;		// parallel to the floor

								physics2.velocity = velocityTangental - velocityNormal * physics2.bounce;
							}
						}
					}
				}
			}
		}
	}
}
