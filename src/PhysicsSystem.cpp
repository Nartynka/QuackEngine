#include "Systems.h"

#include "Scene.h"
#include "Components.h"

#include "Renderer.h" // for debug only

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

namespace Quack
{
	void SolveVelocityConstraint(RigidBodyComponent& rigidBody1, RigidBodyComponent& rigidBody2, const TransformComponent& transform1, const TransformComponent& transform2, const glm::vec3& normal, const glm::vec3& contactPoint);
	void SolvePositionConstraint(const RigidBodyComponent& rigidBody1, const RigidBodyComponent& rigidBody2, TransformComponent& transform1, TransformComponent& transform2, const glm::vec3& normal, float shortestOverlap);

	bool CheckCollisionCubeWithCube(const TransformComponent& transform1, const ColliderComponent& collider1, const TransformComponent& transform2, const ColliderComponent& collider2, glm::vec3& shortestAxis, float& shortestOverlap);
	glm::vec3 FindClosestPointToSphereOnOBB(const glm::vec3& spherePosition, float sphereRadius, const glm::vec3& cubePosition, const glm::vec3& cubeHalfSize, const glm::quat& cubeOrientation);

	struct CollisionManifold
	{
		RigidBodyComponent& rigidBody1;
		RigidBodyComponent& rigidBody2;

		TransformComponent& transform1;
		TransformComponent& transform2;

		glm::vec3 normal;

		glm::vec3 contactPoint; // later will change to an array
		float penetration;
	};

	static std::vector<CollisionManifold> collisionManifolds;

	static RigidBodyComponent staticRigidBody;


	void ApplyForces(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto view = registry.view<RigidBodyComponent>();

		for (auto entity : view)
		{
			auto& [rigidBody] = view.get(entity);

			// if mass is 0 then skip because it would cause division by 0
			if(!rigidBody.invMass)
				continue;

			rigidBody.forces = rigidBody.gravity * (1.f / rigidBody.invMass);
		}
	}


	void Update(const std::shared_ptr<Scene> scene, float dt)
	{
		auto& registry = scene->GetRegistry();
		auto view = registry.view<RigidBodyComponent, TransformComponent>();

		for (auto entity : view)
		{
			auto& [rigidBody, transform] = view.get(entity);

			// linear motion
			glm::vec3 oldVelocity = rigidBody.velocity;

			glm::vec3 acceleration = rigidBody.forces * rigidBody.invMass;

			rigidBody.velocity = rigidBody.velocity * rigidBody.friction + acceleration * dt;
			transform.position += (oldVelocity + rigidBody.velocity) * 0.5f * dt;

			// angular motion
			rigidBody.angularVelocity *= rigidBody.friction;

			transform.orientation += 0.5f * glm::quat(0.f, rigidBody.angularVelocity) * transform.orientation * dt;
			transform.orientation = glm::normalize(transform.orientation);
		}
	}

	void UpdateTransform(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto view = registry.view<TransformComponent>();

		// @TODO: room for improvement because entities without RigidBodyComponent never change their transformation matrix
		for (auto entity : view)
		{
			auto& [transform] = view.get(entity);

			//glm::vec3 scale;
			//scale.x = glm::length(transform.transform[0]);
			//scale.y = glm::length(transform.transform[1]);
			//scale.z = glm::length(transform.transform[2]);

			transform.transform = glm::translate(glm::mat4(1.0f), transform.position);
			transform.transform *= glm::toMat4(transform.orientation);
			//transform.transform *= glm::scale(glm::mat4(1.f), scale);
		}
	}


	void CheckCollisions(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();

		auto view = registry.view<ColliderComponent, TransformComponent>();

		for (auto entity1 : view)
		{
			auto& [collider1, transform1] = view.get(entity1);

			for (auto entity2 : view)
			{
				// entt entity is just an uint_32 so we can skip the reversed pairs e.g. (2,1) when (1,2) was already checked
				if (entity1 <= entity2)
					continue;

				auto& [collider2, transform2] = view.get(entity2);

				Entity e1(entity1, scene.get());
				Entity e2(entity2, scene.get());

				RigidBodyComponent& rigidBody1 = e1.HasComponent<RigidBodyComponent>() ? e1.GetComponent<RigidBodyComponent>() : staticRigidBody;
				RigidBodyComponent& rigidBody2 = e2.HasComponent<RigidBodyComponent>() ? e2.GetComponent<RigidBodyComponent>() : staticRigidBody;

				// @TODO: find better way to determine whether collision shape is a sphere or a cube and make proper sphere - sphere collision
				if (collider1.type == collider2.type && collider1.type == ColliderType::Sphere)
				{
					// Sphere - Sphere collision
					glm::vec3 diff = transform1.position - transform2.position;
					float distanceSquared = dot(diff, diff);
					float radii = collider1.radius + collider2.radius; // radiuses

					if (distanceSquared < radii * radii)
					{
						glm::vec3 normal = normalize(diff); // direction from sphere to sphere

						float penetration = (radii - sqrt(distanceSquared));

						glm::vec3 contactPoint = transform1.position - normal * collider1.radius - penetration * 0.5f;

						CollisionManifold manifold = { rigidBody1, rigidBody2, transform1, transform2, normal, contactPoint, penetration };
						collisionManifolds.push_back(manifold);
					}
				}
				else if (collider1.type == collider2.type && collider1.type == ColliderType::Cube)
				{
					// Cube - Cube
					glm::vec3 shortestAxis;
					float shortestOverlap = 100000.f;

					bool bCollision = CheckCollisionCubeWithCube(transform1, collider1, transform2, collider2, shortestAxis, shortestOverlap);

					if (bCollision)
					{
						glm::vec3 normal = normalize(shortestAxis);

						// @TODO: This only works if the bodies are the same size!!!
						glm::vec3 p1 = transform1.position + normal * (shortestOverlap * 0.5f);
						glm::vec3 p2 = transform2.position - normal * (shortestOverlap * 0.5f);
						glm::vec3 contactPoint = 0.5f * (p1 + p2);

						CollisionManifold manifold = { rigidBody1, rigidBody2, transform1, transform2, normal, contactPoint, shortestOverlap };
						collisionManifolds.push_back(manifold);
					}
				}
				else
				{
					// Sphere - Cube collision / Cube - Sphere collision

					// @TODO: Find better way to check if first or second entity is a sphere
					bool isEntity1Sphere = collider1.type == ColliderType::Sphere;
					TransformComponent& sphereTransform = isEntity1Sphere ? transform1 : transform2;
					TransformComponent& cubeTransform = isEntity1Sphere ? transform2 : transform1;

					ColliderComponent& sphereCollider = isEntity1Sphere ? collider1 : collider2;
					ColliderComponent& cubeCollider = isEntity1Sphere ? collider2 : collider1;

					glm::vec3 closestPoint = FindClosestPointToSphereOnOBB(sphereTransform.position, sphereCollider.radius, cubeTransform.position, cubeCollider.halfSize, cubeTransform.orientation);

					glm::vec3 diff = sphereTransform.position - closestPoint;
					float distanceSquared = dot(diff, diff); // distance from closes point to the sphere. length of diff without expensive sqrt

					// if distance is less or equal then we have a collision!
					if (distanceSquared <= sphereCollider.radius * sphereCollider.radius)
					{
						glm::vec3 normal = normalize(diff); // direction from point on OBB to sphere

						float penetration = sphereCollider.radius - sqrt(distanceSquared);

						// @TODO: Change this!!
						CollisionManifold manifold = { isEntity1Sphere ? rigidBody1 : rigidBody2, isEntity1Sphere ? rigidBody2 : rigidBody1, sphereTransform, cubeTransform, normal, closestPoint, penetration };
						collisionManifolds.push_back(manifold);
					}
				}
			}
		}
	}


	void SolveCollisions()
	{
		for (const CollisionManifold& manifold : collisionManifolds)
		{
			// if both bodies are static then skip solving
			if (!manifold.rigidBody1.invMass && !manifold.rigidBody2.invMass)
				continue;

			SolveVelocityConstraint(manifold.rigidBody1, manifold.rigidBody2, manifold.transform1, manifold.transform2, manifold.normal, manifold.contactPoint);
			SolvePositionConstraint(manifold.rigidBody1, manifold.rigidBody2, manifold.transform1, manifold.transform2, manifold.normal, manifold.penetration);
		}

		collisionManifolds.clear();
	}

	
	void SolveVelocityConstraint(RigidBodyComponent& rigidBody1, RigidBodyComponent& rigidBody2, const TransformComponent& transform1, const TransformComponent& transform2, const glm::vec3& normal, const glm::vec3& contactPoint)
	{
		// Arm from center of mass to a point of contact
		glm::vec3 r1 = contactPoint - transform1.position;
		glm::vec3 r2 = contactPoint - transform2.position;

		// Velocity of a point of the body (v_lin + omega x r)
		glm::vec3 v1 = rigidBody1.velocity + cross(rigidBody1.angularVelocity, r1);
		glm::vec3 v2 = rigidBody2.velocity + cross(rigidBody2.angularVelocity, r2);

		// Relative velocity of two bodies
		glm::vec3 relativeVelocity = v1 - v2;

		// project relative velocity onto normal to check if the bodies are already separating or moving into one another
		float normalVelocity = dot(relativeVelocity, normal);

		// @TODO introduce a bias or something that even if normal velocity is positive but penetration exists then still apply impulse
		if (normalVelocity > 0.f)
		{
			return; // already separating - do nothing
		}

		// @TODO: think if this is correct
		float restitution = glm::min(rigidBody1.bounce, rigidBody2.bounce);

		glm::vec3 effectiveMass1 = cross(rigidBody1.invInertiaTensor * cross(r1, normal), r1);
		glm::vec3 effectiveMass2 = cross(rigidBody2.invInertiaTensor * cross(r2, normal), r2);

		// j
		float impulse = (-(1 + restitution) * normalVelocity) / (rigidBody1.invMass + rigidBody2.invMass + dot(normal, effectiveMass1 + effectiveMass2));

		// J
		glm::vec3 vectorImpulse = impulse * normal;

		rigidBody1.velocity += vectorImpulse * rigidBody1.invMass;
		rigidBody2.velocity -= vectorImpulse * rigidBody2.invMass;

		glm::mat3 R1 = glm::toMat3(transform1.orientation);
		glm::mat3 R2 = glm::toMat3(transform2.orientation);
		// transpose is the same as inverse (because the rotation matrix is orthogonal) but transpose is less expensive
		glm::mat3 invInertiaWorld1 = R1 * rigidBody1.invInertiaTensor * glm::transpose(R1);
		glm::mat3 invInertiaWorld2 = R2 * rigidBody2.invInertiaTensor * glm::transpose(R2);

		// apply angular momentum change (calculate torque)
		rigidBody1.angularVelocity += invInertiaWorld1 * glm::cross(r1, vectorImpulse);
		rigidBody2.angularVelocity -= invInertiaWorld2 * glm::cross(r2, vectorImpulse);
	}

	void SolvePositionConstraint(const RigidBodyComponent& rigidBody1, const RigidBodyComponent& rigidBody2, TransformComponent& transform1, TransformComponent& transform2, const glm::vec3& normal, float shortestOverlap)
	{
		float slop = 0.001f;   // allowed penetration
		float percent = 0.8f; // how aggressive the correction is

		float correctionMag = glm::max(shortestOverlap - slop, 0.0f) * percent;
		glm::vec3 correction = correctionMag * normal;

		transform1.position += correction * rigidBody1.invMass / (rigidBody1.invMass + rigidBody2.invMass);
		transform2.position -= correction * rigidBody2.invMass / (rigidBody1.invMass + rigidBody2.invMass);
	}


	glm::vec3 FindClosestPointToSphereOnOBB(const glm::vec3& spherePosition, float sphereRadius, const glm::vec3& cubePosition, const glm::vec3& cubeHalfSize, const glm::quat& cubeOrientation)
	{
		// vector from sphere to cube
		glm::vec3 d = spherePosition - cubePosition;

		glm::mat3 cubeAxes = glm::toMat3(cubeOrientation);

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

	// @TODO: Change passing shortestAxis and shortestOverlap to returning collision manifold
	bool CheckCollisionCubeWithCube(const TransformComponent& transform1, const ColliderComponent& collider1, const TransformComponent& transform2, const ColliderComponent& collider2, glm::vec3& shortestAxis, float& shortestOverlap)
	{
		// OBB - OBB collision (SAT)
		glm::mat3 cube1Axes = glm::toMat3(transform1.orientation) * glm::mat3(1.f);
		glm::mat3 cube2Axes = glm::toMat3(transform2.orientation) * glm::mat3(1.f);

		// orientation quat is normalized every frame so no need for normalizing these axes
		std::vector<glm::vec3> axes = { cube1Axes[0], cube1Axes[1], cube1Axes[2], cube2Axes[0], cube2Axes[1], cube2Axes[2] };

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				glm::vec3 crossedAxis = cross(cube1Axes[i], cube2Axes[j]);
				
				// if the axis is zero or near zero (e.g. parallel to another) then it will cause division by 0 when trying to normalize it
				if(glm::length2(crossedAxis) < 0.00001f)
					continue;

				axes.push_back(normalize(crossedAxis));
			}
		}

		// @TODO: Change manual defined corners to a nice short loop!

		// Cube1 corners in local space
		glm::vec3 cube1TopRightFrontCorner = collider1.halfSize;
		glm::vec3 cube1TopRightBackCorner = glm::vec3(collider1.halfSize.x, collider1.halfSize.y, -collider1.halfSize.z);

		glm::vec3 cube1BottomRightFrontCorner = glm::vec3(collider1.halfSize.x, -collider1.halfSize.y, collider1.halfSize.z);
		glm::vec3 cube1BottomRightBackCorner = glm::vec3(collider1.halfSize.x, -collider1.halfSize.y, -collider1.halfSize.z);

		glm::vec3 cube1TopLeftFrontCorner = glm::vec3(-collider1.halfSize.x, collider1.halfSize.y, collider1.halfSize.z);
		glm::vec3 cube1TopLeftBackCorner = glm::vec3(-collider1.halfSize.x, collider1.halfSize.y, -collider1.halfSize.z);

		glm::vec3 cube1BottomLeftFrontCorner = glm::vec3(-collider1.halfSize.x, -collider1.halfSize.y, collider1.halfSize.z);
		glm::vec3 cube1BottomLeftBackCorner = -collider1.halfSize;

		// Cube1 corners in world space (rotated and translated)
		// THE ORDER MATTERS!!! vec3 * quat is not the same as quat * vec3!!!!!!!!!!!!!!
		glm::vec3 wCube1TopRightFrontCorner = (transform1.orientation * cube1TopRightFrontCorner) + transform1.position;
		glm::vec3 wCube1TopRightBackCorner = (transform1.orientation * cube1TopRightBackCorner) + transform1.position;

		glm::vec3 wCube1BottomRightFrontCorner = (transform1.orientation * cube1BottomRightFrontCorner) + transform1.position;
		glm::vec3 wCube1BottomRightBackCorner = (transform1.orientation * cube1BottomRightBackCorner) + transform1.position;

		glm::vec3 wCube1TopLeftFrontCorner = (transform1.orientation * cube1TopLeftFrontCorner) + transform1.position;
		glm::vec3 wCube1TopLeftBackCorner = (transform1.orientation * cube1TopLeftBackCorner) + transform1.position;

		glm::vec3 wCube1BottomLeftFrontCorner = (transform1.orientation * cube1BottomLeftFrontCorner) + transform1.position;
		glm::vec3 wCube1BottomLeftBackCorner = (transform1.orientation * cube1BottomLeftBackCorner) + transform1.position;

		std::vector<glm::vec3> cube1Points = { wCube1TopRightFrontCorner, wCube1TopRightBackCorner,
												wCube1BottomRightFrontCorner, wCube1BottomRightBackCorner,
												wCube1TopLeftFrontCorner, wCube1TopLeftBackCorner,
												wCube1BottomLeftFrontCorner, wCube1BottomLeftBackCorner };

		// Cube2 corners in local space
		glm::vec3 cube2TopRightFrontCorner = collider2.halfSize;
		glm::vec3 cube2TopRightBackCorner = glm::vec3(collider2.halfSize.x, collider2.halfSize.y, -collider2.halfSize.z);

		glm::vec3 cube2BottomRightFrontCorner = glm::vec3(collider2.halfSize.x, -collider2.halfSize.y, collider2.halfSize.z);
		glm::vec3 cube2BottomRightBackCorner = glm::vec3(collider2.halfSize.x, -collider2.halfSize.y, -collider2.halfSize.z);

		glm::vec3 cube2TopLeftFrontCorner = glm::vec3(-collider2.halfSize.x, collider2.halfSize.y, collider2.halfSize.z);
		glm::vec3 cube2TopLeftBackCorner = glm::vec3(-collider2.halfSize.x, collider2.halfSize.y, -collider2.halfSize.z);

		glm::vec3 cube2BottomLeftFrontCorner = glm::vec3(-collider2.halfSize.x, -collider2.halfSize.y, collider2.halfSize.z);
		glm::vec3 cube2BottomLeftBackCorner = -collider2.halfSize;

		// Cube2 corners in world space (rotated and translated)
		glm::vec3 wCube2TopRightFrontCorner = (transform2.orientation * cube2TopRightFrontCorner) + transform2.position;
		glm::vec3 wCube2TopRightBackCorner = (transform2.orientation * cube2TopRightBackCorner) + transform2.position;

		glm::vec3 wCube2BottomRightFrontCorner = (transform2.orientation * cube2BottomRightFrontCorner) + transform2.position;
		glm::vec3 wCube2BottomRightBackCorner = (transform2.orientation * cube2BottomRightBackCorner) + transform2.position;

		glm::vec3 wCube2TopLeftFrontCorner = (transform2.orientation * cube2TopLeftFrontCorner) + transform2.position;
		glm::vec3 wCube2TopLeftBackCorner = (transform2.orientation * cube2TopLeftBackCorner) + transform2.position;

		glm::vec3 wCube2BottomLeftFrontCorner = (transform2.orientation * cube2BottomLeftFrontCorner) + transform2.position;
		glm::vec3 wCube2BottomLeftBackCorner = (transform2.orientation * cube2BottomLeftBackCorner) + transform2.position;

		std::vector<glm::vec3> cube2Points = { wCube2TopRightFrontCorner, wCube2TopRightBackCorner,
												wCube2BottomRightFrontCorner, wCube2BottomRightBackCorner,
												wCube2TopLeftFrontCorner, wCube2TopLeftBackCorner,
												wCube2BottomLeftFrontCorner, wCube2BottomLeftBackCorner };

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

			// cube2
			float c2Min = dot(cube2Points[0], axis);
			float c2Max = c2Min;

			for (const auto& point : cube2Points)
			{
				float p = dot(point, axis);

				c2Min = glm::min(c2Min, p);
				c2Max = glm::max(c2Max, p);
			}

			if (c1Max < c2Min || c2Max < c1Min)
				return false;

			// There is no gap on current axis. We have an overlap
			// Amount of overlap
			float amount = glm::min(c1Max, c2Max) - glm::max(c1Min, c2Min);

			if (amount < shortestOverlap)
			{
				shortestOverlap = amount;
				shortestAxis = axis;
			}
		}

		return true;
	}

}
