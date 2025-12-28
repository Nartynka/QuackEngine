#include "Systems.h"

#include "Scene.h"
#include "Components.h"

#include "Renderer.h" // for debug only

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

namespace Quack
{
	bool CheckCollisionCubeWithCube(const glm::vec3& cube1Position, const glm::vec3& cube1HalfSize, const glm::quat& cube1Orientation, const glm::vec3& cube2Position, const glm::vec3& cube2HalfSize, const glm::quat& cube2Orientation, glm::vec3& shortestAxis, float& shortestOverlap);
	glm::vec3 FindClosestPointToSphereOnOBB(const glm::vec3& spherePosition, float sphereRadius, const glm::vec3& cubePosition, const glm::vec3& cubeHalfSize, const glm::quat& cubeOrientation);
	glm::vec3 FindClosestPointToSphereOnOBB(const glm::vec3& cubePosition, const glm::vec3& cubeHalfSzie, const glm::quat& cubeOrientation, const glm::vec3& spherePosition, float sphereRadius);

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

			// Force-based physics

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

	// Collision with immovable objects (floor)
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
				// For now constraints can only be cube shaped so we don't check the radius of the floor
				if (collision.radius > 0.f)
				{
					glm::vec3 closestPoint = FindClosestPointToSphereOnOBB(physics.position, collision.radius, floor.position, floor.halfSize, floor.orientation);

					glm::vec3 diff = physics.position - closestPoint;
					float distanceSquared = dot(diff, diff); // distance from closes point to the sphere. length of diff without expensive sqrt
					
					// if distance is less or equal then we have a collision!
					if (distanceSquared <= collision.radius * collision.radius)
					{
						// Position-based constrain resolution
						glm::vec3 collisionNormal = normalize(diff); // direction from OBB to sphere

						float penetration = collision.radius - sqrt(distanceSquared); // @TODO: can I omit the sqrt here?






						//physics.position += collisionNormal * penetration;

						// @TODO: Change to real impulse-based physics!!
						//glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
						//glm::vec3 velocityTangental = physics.velocity - velocityNormal;		// parallel to the floor

						//physics.velocity = velocityTangental - velocityNormal * physics.bounce;

						// This line prevents jittering when sphere bounces forever (on not oriented surface) but prevents the sphere from rolling down sloped/oriented surface
						//physics.velocity = physics.velocity.y < 0.21f ? glm::vec3(0.f) : physics.velocity; // to prevent jittering
					}
				}
				else
				{
					// OBB - OBB collision (SAT)
					glm::vec3 shortestAxis;
					float shortestOverlap = 100000.f;

					bool bCollision = CheckCollisionCubeWithCube(physics.position, collision.halfSize, physics.orientation, floor.position, floor.halfSize, floor.orientation, shortestAxis, shortestOverlap);

					if (bCollision)
					{
						glm::vec3 normal = normalize(shortestAxis);

						glm::vec3 contactPoint = physics.position - normal * collision.halfSize;








						// Resolve overlap (Position-based)
						//physics.position += normal * shortestOverlap;

						/// ANGULAR FORCES
						//Renderer::DrawPoint(contactPoint, glm::vec3(0.f, 1.f, 0.f));

						//glm::vec3 r = contactPoint - physics.position;

						// Relative velocity, constraints don't have linear & angular velocity so we only take the entity velocities
						//glm::vec3 relativeVelocity = physics.velocity + cross(physics.angularVelocity, r);

						//// Project relative velocity onto normal to check if the entity is moving away (collision already solved) or is moving towards constrain (collision)
						//float nV = dot(relativeVelocity, normal);

						//// if collision (not moving away) - solve impulse
						//if (nV < 0.f)
						//{
						//	float invMass = 1 / physics.mass;
						//	glm::mat3 rotationMatrix = glm::toMat3(physics.orientation);
						//	glm::mat3 worldInvInertia = rotationMatrix * physics.invInertiaTensor * transpose(rotationMatrix);
						//	float j = (-(1 + physics.bounce) * nV) / (invMass + dot(normal, cross(worldInvInertia * cross(r, normal), r)));
						//	glm::vec3 impulse = j * normal;
						//	physics.velocity += impulse * invMass;
						//	physics.angularVelocity += worldInvInertia * cross(r, impulse);
						//	//QUACK_LOG("{} {} {}", physics.angularVelocity.x, physics.angularVelocity.y, physics.angularVelocity.z);
						//}
						


						// @TODO: Change to real impulse-based!!
						/// LINEAR FORCES
						//glm::vec3 velocityNormal = normal * glm::dot(normal, physics.velocity); // perpendicular to the floor. "Normal to the floor"
						//glm::vec3 velocityTangental = physics.velocity - velocityNormal;		// parallel to the floor

						// TEMP!!! Will change when proper impulses are done
						//glm::vec3 impulse = velocityTangental - velocityNormal * physics.bounce;

						//physics.velocity = impulse;

						// @TODO: FIX temp
						//physics.angularVelocity += glm::cross(impulse, r);

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

	// Collision with other entities
	void SolveCollision(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto collisionView = registry.view<CollisionComponent, PhysicsComponent>();

		for (auto entity : collisionView)
		{
			auto& [collision, physics] = collisionView.get(entity);
			for (auto entity2 : collisionView)
			{
				if (entity == entity2)
					continue;

				auto& [collision2, physics2] = collisionView.get(entity2);

				// @TODO: find better way to determine whether collision shape is a sphere or a cube and make proper sphere - sphere collision
				if (collision.radius > 0.f)
				{
					if (collision2.radius > 0.f)
					{
						// Sphere - Sphere collision
						glm::vec3 diff = physics.position - physics2.position;
						float distanceSquared = dot(diff, diff);
						float radii = collision.radius + collision2.radius; // radiuses

						if (distanceSquared < radii * radii)
						{
							glm::vec3 collisionNormal = normalize(diff); // direction from sphere to sphere

							float penetration = (radii - sqrt(distanceSquared)) / 2.f; // @TODO: can I omit the sqrt here?



							// Position-based
							//physics.position += collisionNormal * penetration;
							//physics2.position += -collisionNormal * penetration;

							//// @TODO: Change to impulse based
							//{
							//	glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
							//	glm::vec3 velocityTangental = physics.velocity - velocityNormal;		// parallel to the floor

							//	physics.velocity = velocityTangental - velocityNormal * physics.bounce;
							//}
							//{
							//	glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics2.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
							//	glm::vec3 velocityTangental = physics2.velocity - velocityNormal;		// parallel to the floor

							//	physics2.velocity = velocityTangental - velocityNormal * physics2.bounce;
							//}
						}
					}
					else // sphere - cube
					{
						glm::vec3 closestPoint = FindClosestPointToSphereOnOBB(physics.position, collision.radius, physics2.position, collision2.halfSize, physics2.orientation);

						glm::vec3 diff = physics.position - closestPoint;
						float distanceSquared = dot(diff, diff); // distance from closes point to the sphere. length of diff without expensive sqrt

						// if distance is less or equal then we have a collision!
						if (distanceSquared <= collision.radius * collision.radius)
						{
							glm::vec3 collisionNormal = normalize(diff); // direction from OBB to sphere

							float penetration = collision.radius - sqrt(distanceSquared); // @TODO: can I omit the sqrt here?







							// Position-based constrain resolution
							//physics.position += (collisionNormal * penetration) / 2.f;
							//physics2.position += (collisionNormal * penetration) / 2.f;

							//// @TODO: Change to real impulse-based physics!!
							//glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics.velocity+physics2.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
							//glm::vec3 velocityTangental = (physics.velocity + physics2.velocity) - velocityNormal;		// parallel to the floor

							//float restitution = (physics.bounce + physics2.bounce) / 2.f;

							//physics.velocity = velocityTangental - velocityNormal * restitution;
							//physics2.velocity = velocityTangental - velocityNormal * restitution;

							// This line prevents jittering when sphere bounces forever (on not oriented surface) but prevents the sphere from rolling down sloped/oriented surface
							//physics.velocity = physics.velocity.y < 0.21f ? glm::vec3(0.f) : physics.velocity; // to prevent jittering
						}
					}
				}

				else
				{
					if (collision2.radius > 0.f)
					{
						// box - sphere
						glm::vec3 closestPoint = FindClosestPointToSphereOnOBB(physics.position, collision.halfSize, physics.orientation, physics2.position, collision2.radius);

						glm::vec3 diff = physics2.position - closestPoint;
						float distanceSquared = dot(diff, diff); // distance from closes point to the sphere. length of diff without expensive sqrt

						// if distance is less or equal then we have a collision!
						if (distanceSquared <= collision.radius * collision.radius)
						{
							glm::vec3 collisionNormal = normalize(diff); // direction from OBB to sphere

							float penetration = collision.radius - sqrt(distanceSquared);



							// Position-based constrain resolution
							//physics.position += (collisionNormal * penetration) / 2.f;
							//physics2.position += (collisionNormal * penetration) / 2.f;

							//// @TODO: Change to real impulse-based physics!!
							//glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics.velocity + physics2.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
							//glm::vec3 velocityTangental = (physics.velocity + physics2.velocity) - velocityNormal;		// parallel to the floor

							//float restitution = (physics.bounce + physics2.bounce) / 2.f;

							//physics.velocity = velocityTangental - velocityNormal * restitution;
							//physics2.velocity = velocityTangental - velocityNormal * restitution;

							// This line prevents jittering when sphere bounces forever (on not oriented surface) but prevents the sphere from rolling down sloped/oriented surface
							//physics.velocity = physics.velocity.y < 0.21f ? glm::vec3(0.f) : physics.velocity; // to prevent jittering
						}
					}
					else
					{
						// box - box collision
						glm::vec3 shortestAxis;
						float shortestOverlap = 100000.f;

						bool bCollision = CheckCollisionCubeWithCube(physics.position, collision.halfSize, physics.orientation, physics2.position, collision2.halfSize, physics2.orientation, shortestAxis, shortestOverlap);

						if (bCollision)
						{
							glm::vec3 normal = normalize(shortestAxis);

							// @TODO: This only works if the bodies are the same size!!!
							glm::vec3 p1 = physics.position - normal * (shortestOverlap * 0.5f);
							glm::vec3 p2 = physics2.position + normal * (shortestOverlap * 0.5f);
							glm::vec3 contactPoint = 0.5f * (p1 + p1);


							// Arm from center of mass to a point of contact
							glm::vec3 r1 = contactPoint - physics.position;
							glm::vec3 r2 = contactPoint - physics2.position;

							// Velocity of a point of the body (v_lin + omega x r)
							glm::vec3 v1 = physics.velocity + cross(physics.angularVelocity, r1);
							glm::vec3 v2 = physics2.velocity + cross(physics2.angularVelocity, r2);

							// Relative velocity of two bodies
							glm::vec3 relativeVelocity = v1 - v2;

							// project relative velocity onto normal to check if the bodies are already separating or moving into one another

							float normalVelocity = dot(relativeVelocity, normal);

							if (normalVelocity > 0.f)
								return; // already separating - do nothing
							

							float invMass1 = 1.f / physics.mass;
							float invMass2 = 1.f / physics2.mass;

							// @TODO: think if this is correct
							float restitution = glm::min(physics.bounce, physics2.bounce);
							
							glm::vec3 effectiveMass1 = cross(physics.invInertiaTensor * cross(r1, normal), r1);
							glm::vec3 effectiveMass2 = cross(physics2.invInertiaTensor * cross(r2, normal), r2);

							// j
							float impulse = (-(1 + restitution) * normalVelocity) / (invMass1 + invMass2 + dot(normal, effectiveMass1 + effectiveMass2));

							// J
							glm::vec3 vectorImpulse = impulse * normal;

							physics.velocity += vectorImpulse * invMass1;
							physics2.velocity -= vectorImpulse * invMass2;

							glm::mat3 R1 = glm::toMat3(physics.orientation);
							glm::mat3 R2 = glm::toMat3(physics2.orientation);
							// transpose is the same as inverse (because the rotation matrix is orthogonal) but transpose is less expensive
							glm::mat3 invInertiaWorld1 = R1 * physics.invInertiaTensor * glm::transpose(R1);
							glm::mat3 invInertiaWorld2 = R2 * physics2.invInertiaTensor * glm::transpose(R2);

							physics.angularVelocity += invInertiaWorld1 * glm::cross(r1, vectorImpulse);
							physics2.angularVelocity -= invInertiaWorld2 * glm::cross(r2, vectorImpulse);

							
							/*
							//physics.position += (normal * shortestOverlap) / 2.f;
							//physics2.position -= (normal * shortestOverlap) / 2.f;

							//glm::vec3 raCrossN = glm::cross(rA, normal);
							//glm::vec3 rbCrossN = glm::cross(rB, normal);

							//float invMass = 1.f / physics.mass;
							//float invMass2 = 1.f / physics.mass;

							//float invMassSum = invMass + invMass2 + dot(normal, cross(physics.invInertiaTensor * raCrossN, rA) + cross(physics2.invInertiaTensor * rbCrossN, rB));

							//float e = (physics.bounce + physics2.bounce) / 2.f;

							//float j = -(1.0f + e) * vn;
							//j /= invMassSum;

							//glm::vec3 impulse = j * normal;

							//physics.velocity += impulse * invMass;
							//physics.angularVelocity += physics.invInertiaTensor * glm::cross(rA, impulse);

							//physics2.velocity -= impulse * invMass2;
							//physics2.angularVelocity -= physics2.invInertiaTensor * glm::cross(rB, impulse);

							//const float percent = 0.8f; // correction strength
							//const float slop = 0.01f;   // penetration allowance

							//float correctionMag =
							//	std::max(shortestOverlap - slop, 0.0f) /
							//	(invMass + invMass2) * percent;

							//glm::vec3 correction = correctionMag * normal;

							//physics.position += correction * invMass;
							//physics2.position -= correction * invMass2;
							*/
						}
					}
					}
			}
		}
	}


	glm::vec3 FindClosestPointToSphereOnOBB(const glm::vec3& spherePosition, float sphereRadius, const glm::vec3& cubePosition, const glm::vec3& cubeHalfSize, const glm::quat& cubeOrientation)
	{
		// vector from sphere to cube
		glm::vec3 d = spherePosition - cubePosition;

		glm::mat3 cubeAxes = glm::toMat3(cubeOrientation) * glm::mat3(1.f);

		// Find the closest point on OBB to the sphere center
		float distX = dot(d, cubeAxes[0]);
		float distY = dot(d, cubeAxes[1]);
		float distZ = dot(d, cubeAxes[2]);

		// Clamp so that the point is on the OBB
		distX = glm::clamp(distX, -cubeHalfSize.x, cubeHalfSize.x);
		distY = glm::clamp(distY, -cubeHalfSize.y, cubeHalfSize.y);
		distZ = glm::clamp(distZ, -cubeHalfSize.z, cubeHalfSize.z);

		// dist- are in floor local space, in that local space they are not rotated so we have to rotate them
		// axes are our "portal"/"bridge" between floor local space and world space, they represent the floor rotated axes
		// - that's why we multiply dist- by corresponding axes
		glm::vec3 closestPoint = cubePosition + distX * cubeAxes[0] + distY * cubeAxes[1] + distZ * cubeAxes[2];

		return closestPoint;
	}
	// @TODO: Which is better, function or a define here?
	glm::vec3 FindClosestPointToSphereOnOBB(const glm::vec3& cubePosition, const glm::vec3& cubeHalfSzie, const glm::quat& cubeOrientation, const glm::vec3& spherePosition, float sphereRadius)
	{
		return FindClosestPointToSphereOnOBB(spherePosition, sphereRadius, cubePosition, cubeHalfSzie, cubeOrientation);
	}

	// @TODO: Change passing shortestAxis and shortestOverlap to returning collision manifold
	bool CheckCollisionCubeWithCube(const glm::vec3& cube1Position, const glm::vec3& cube1HalfSize, const glm::quat& cube1Orientation, const glm::vec3& cube2Position, const glm::vec3& cube2HalfSize, const glm::quat& cube2Orientation, glm::vec3& shortestAxis, float& shortestOverlap)
	{
		// OBB - OBB collision (SAT)
		// !!!! Right now edge - edge will be detected as collision even when there isn't any

		// @TODO: Change manual defined corners to a nice short loop!
		glm::mat3 cube1Axes = glm::toMat3(cube1Orientation) * glm::mat3(1.f);
		glm::mat3 cube2Axes = glm::toMat3(cube2Orientation) * glm::mat3(1.f);

		std::vector<glm::vec3> axes = { cube1Axes[0], cube1Axes[1], cube1Axes[2], cube2Axes[0], cube2Axes[1], cube2Axes[2] };

		// Visualize axes from cube1
		//Renderer::DrawLine(glm::vec3(0.f), cube1Axes[0] * 5.f, glm::vec3(1.f, 0.f, 0.f));
		//Renderer::DrawLine(glm::vec3(0.f), cube1Axes[1] * 5.f, glm::vec3(0.f, 1.f, 0.f));
		//Renderer::DrawLine(glm::vec3(0.f), cube1Axes[2] * 5.f, glm::vec3(0.f, 0.f, 1.f));
		// Visualize axes from cube2
		//Renderer::DrawLine(glm::vec3(0.f), cube2Axes[0] * 5.f, glm::vec3(1.f, 0.f, 0.f));
		//Renderer::DrawLine(glm::vec3(0.f), cube2Axes[1] * 5.f, glm::vec3(0.f, 1.f, 0.f));
		//Renderer::DrawLine(glm::vec3(0.f), cube2Axes[2] * 5.f, glm::vec3(0.f, 0.f, 1.f));


		// Cube1 corners in local space
		glm::vec3 cube1TopRightFrontCorner = cube1HalfSize;
		glm::vec3 cube1TopRightBackCorner = glm::vec3(cube1HalfSize.x, cube1HalfSize.y, -cube1HalfSize.z);

		glm::vec3 cube1BottomRightFrontCorner = glm::vec3(cube1HalfSize.x, -cube1HalfSize.y, cube1HalfSize.z);
		glm::vec3 cube1BottomRightBackCorner = glm::vec3(cube1HalfSize.x, -cube1HalfSize.y, -cube1HalfSize.z);

		glm::vec3 cube1TopLeftFrontCorner = glm::vec3(-cube1HalfSize.x, cube1HalfSize.y, cube1HalfSize.z);
		glm::vec3 cube1TopLeftBackCorner = glm::vec3(-cube1HalfSize.x, cube1HalfSize.y, -cube1HalfSize.z);

		glm::vec3 cube1BottomLeftFrontCorner = glm::vec3(-cube1HalfSize.x, -cube1HalfSize.y, cube1HalfSize.z);
		glm::vec3 cube1BottomLeftBackCorner = -cube1HalfSize;

		// Cube1 corners in world space (rotated and translated)
		// THE ORDER MATTERS!!! vec3 * quat is not the same as quat * vec3!!!!!!!!!!!!!!
		glm::vec3 wCube1TopRightFrontCorner = (cube1Orientation * cube1TopRightFrontCorner) + cube1Position;
		glm::vec3 wCube1TopRightBackCorner = (cube1Orientation * cube1TopRightBackCorner) + cube1Position;

		glm::vec3 wCube1BottomRightFrontCorner = (cube1Orientation * cube1BottomRightFrontCorner) + cube1Position;
		glm::vec3 wCube1BottomRightBackCorner = (cube1Orientation * cube1BottomRightBackCorner) + cube1Position;

		glm::vec3 wCube1TopLeftFrontCorner = (cube1Orientation * cube1TopLeftFrontCorner) + cube1Position;
		glm::vec3 wCube1TopLeftBackCorner = (cube1Orientation * cube1TopLeftBackCorner) + cube1Position;

		glm::vec3 wCube1BottomLeftFrontCorner = (cube1Orientation * cube1BottomLeftFrontCorner) + cube1Position;
		glm::vec3 wCube1BottomLeftBackCorner = (cube1Orientation * cube1BottomLeftBackCorner) + cube1Position;

		std::vector<glm::vec3> cube1Points = { wCube1TopRightFrontCorner, wCube1TopRightBackCorner,
												wCube1BottomRightFrontCorner, wCube1BottomRightBackCorner,
												wCube1TopLeftFrontCorner, wCube1TopLeftBackCorner,
												wCube1BottomLeftFrontCorner, wCube1BottomLeftBackCorner };

		// Visualize points on the cube1
		//Renderer::DrawPoint(wCube1TopRightFrontCorner);
		//Renderer::DrawPoint(wCube1BottomRightFrontCorner);
		//Renderer::DrawPoint(wCube1TopLeftFrontCorner);
		//Renderer::DrawPoint(wCube1BottomLeftFrontCorner);
		//Renderer::DrawPoint(wCube1TopRightBackCorner);
		//Renderer::DrawPoint(wCube1BottomRightBackCorner);
		//Renderer::DrawPoint(wCube1TopLeftBackCorner);
		//Renderer::DrawPoint(wCube1BottomLeftBackCorner);

		// Cube2 corners in local space
		glm::vec3 cube2TopRightFrontCorner = cube2HalfSize;
		glm::vec3 cube2TopRightBackCorner = glm::vec3(cube2HalfSize.x, cube2HalfSize.y, -cube2HalfSize.z);

		glm::vec3 cube2BottomRightFrontCorner = glm::vec3(cube2HalfSize.x, -cube2HalfSize.y, cube2HalfSize.z);
		glm::vec3 cube2BottomRightBackCorner = glm::vec3(cube2HalfSize.x, -cube2HalfSize.y, -cube2HalfSize.z);

		glm::vec3 cube2TopLeftFrontCorner = glm::vec3(-cube2HalfSize.x, cube2HalfSize.y, cube2HalfSize.z);
		glm::vec3 cube2TopLeftBackCorner = glm::vec3(-cube2HalfSize.x, cube2HalfSize.y, -cube2HalfSize.z);

		glm::vec3 cube2BottomLeftFrontCorner = glm::vec3(-cube2HalfSize.x, -cube2HalfSize.y, cube2HalfSize.z);
		glm::vec3 cube2BottomLeftBackCorner = -cube2HalfSize;

		// Cube2 corners in world space (rotated and translated)
		glm::vec3 wCube2TopRightFrontCorner = (cube2Orientation * cube2TopRightFrontCorner) + cube2Position;
		glm::vec3 wCube2TopRightBackCorner = (cube2Orientation * cube2TopRightBackCorner) + cube2Position;

		glm::vec3 wCube2BottomRightFrontCorner = (cube2Orientation * cube2BottomRightFrontCorner) + cube2Position;
		glm::vec3 wCube2BottomRightBackCorner = (cube2Orientation * cube2BottomRightBackCorner) + cube2Position;

		glm::vec3 wCube2TopLeftFrontCorner = (cube2Orientation * cube2TopLeftFrontCorner) + cube2Position;
		glm::vec3 wCube2TopLeftBackCorner = (cube2Orientation * cube2TopLeftBackCorner) + cube2Position;

		glm::vec3 wCube2BottomLeftFrontCorner = (cube2Orientation * cube2BottomLeftFrontCorner) + cube2Position;
		glm::vec3 wCube2BottomLeftBackCorner = (cube2Orientation * cube2BottomLeftBackCorner) + cube2Position;

		std::vector<glm::vec3> cube2Points = { wCube2TopRightFrontCorner, wCube2TopRightBackCorner,
												wCube2BottomRightFrontCorner, wCube2BottomRightBackCorner,
												wCube2TopLeftFrontCorner, wCube2TopLeftBackCorner,
												wCube2BottomLeftFrontCorner, wCube2BottomLeftBackCorner };

		// Project the points onto axes and find the "shadows"
		std::vector<glm::vec3> cube1Mins;
		std::vector<glm::vec3> cube1Maxs;

		std::vector<glm::vec3> cube2Mins;
		std::vector<glm::vec3> cube2Maxs;

		bool bCollision = true;

		for (const auto& axis : axes)
		{
			float c1Min = dot(cube1Points[0], axis);
			float c1Max = c1Min;

			for (const auto& point : cube1Points)
			{
				float p = dot(point, axis);

				c1Min = glm::min(c1Min, p);
				c1Max = glm::max(c1Max, p);
			}

			cube1Mins.push_back(c1Min * axis);
			cube1Maxs.push_back(c1Max * axis);

			// cube2
			float c2Min = dot(cube2Points[0], axis);
			float c2Max = c2Min;

			for (const auto& point : cube2Points)
			{
				float p = dot(point, axis);

				c2Min = glm::min(c2Min, p);
				c2Max = glm::max(c2Max, p);
			}

			cube2Mins.push_back(c2Min * axis);
			cube2Maxs.push_back(c2Max * axis);

			if (c1Max < c2Min || c2Max < c1Min)
				bCollision = false;
			else // There is no gap on current axis. We have an overlap on current axis
			{
				// Amount of overlap
				float amount = glm::min(c1Max, c2Max) - glm::max(c1Min, c2Min);

				if (amount < shortestOverlap)
				{
					shortestOverlap = amount;
					shortestAxis = axis;
				}
			}
		}

		return bCollision;
	}

}
