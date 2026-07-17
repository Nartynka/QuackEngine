#include "Systems.h"

#include "Scene.h"
#include "Components.h"

#include "Renderer.h" // for debug only

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

namespace Quack
{
	struct ContactManifold;

	void SolveVelocityConstraint(RigidBodyComponent& rigidBody1, RigidBodyComponent& rigidBody2, const TransformComponent& transform1, const TransformComponent& transform2, const glm::vec3& normal, const std::vector<glm::vec3>& contactPoints, std::vector<float>& accumulatedImpulses);
	void SolvePositionConstraint(const RigidBodyComponent& rigidBody1, const RigidBodyComponent& rigidBody2, TransformComponent& transform1, TransformComponent& transform2, const glm::vec3& normal, float shortestOverlap);

	bool CheckCollisionCubeWithCube(TransformComponent& transform1, TransformComponent& transform2, const ColliderComponent& collider1, const ColliderComponent& collider2, ContactManifold& contactManifold);
	glm::vec3 FindClosestPointToSphereOnOBB(const glm::vec3& spherePosition, float sphereRadius, const glm::vec3& cubePosition, const glm::vec3& cubeHalfSize, const glm::quat& cubeOrientation);
	std::vector<glm::vec3> CreateFaceFromNormal(const glm::vec3& faceNormal, const glm::mat3& axes, const glm::vec3& position, const glm::vec3& halfSize, glm::vec3& faceCenter);

	struct ContactManifold
	{
		RigidBodyComponent& rigidBody1;
		RigidBodyComponent& rigidBody2;

		TransformComponent& transform1;
		TransformComponent& transform2;

		glm::vec3 normal;

		std::vector<glm::vec3> contactPoints; // @TODO: change to array of max 4 points
		std::vector<float> accumulatedImpulses;
		float penetration;

		ContactManifold(RigidBodyComponent& r1, RigidBodyComponent& r2, TransformComponent& t1, TransformComponent& t2, glm::vec3 normal = glm::vec3(0.f), std::vector<glm::vec3> contactPoints = {}, float penetration = 0.f)
			: rigidBody1(r1), rigidBody2(r2), transform1(t1), transform2(t2), normal(normal), contactPoints(contactPoints), penetration(penetration) 
		{
			accumulatedImpulses.resize(contactPoints.size(), 0.f);
		}

		void SetContactPoints(const std::vector<glm::vec3>& points)
		{
			contactPoints = points;
			accumulatedImpulses.resize(points.size(), 0.f);
		}
	};

	static std::vector<ContactManifold> contactManifolds;

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

			// mass here is a bit useless because it cancels out when applying forces to acceleration
			// and gravity is the only force here because the engine is impulse based xD
			rigidBody.forces = rigidBody.gravity * (1.f / rigidBody.invMass);
		}
	}

	const float Y_TRESHOLD = -100.f;

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

			if (transform.position.y < Y_TRESHOLD)
				registry.destroy(entity);
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



	//void UpdateConstraints(const std::shared_ptr<Scene> scene, float dt)
	//{
	//	auto& registry = scene->GetRegistry();
	//
	//	auto view = registry.view<ElasticConstraintComponent>();
	//
	//	for (int i = 5; i > 0; i--)
	//	{
	//		for (auto entity : view)
	//		{
	//			auto& [constraint] = view.get(entity);
	//
	//			Entity* a = constraint.entity_a;
	//			Entity* b = constraint.entity_b;
	//
	//			auto& transA = a->GetComponent<TransformComponent>();
	//			auto& transB = b->GetComponent<TransformComponent>();
	//
	//			float massA = 0.f;
	//			float massB = 0.f;
	//			if(a->HasComponent<RigidBodyComponent>())
	//				massA = 1.f / a->GetComponent<RigidBodyComponent>().invMass;
	//		
	//			if (b->HasComponent<RigidBodyComponent>())
	//				massB = 1.f / b->GetComponent<RigidBodyComponent>().invMass;
	//
	//			glm::vec3 diff = transB.position - transA.position;
	//			float distance = length(diff);
	//		
	//			float displacement = constraint.distance - distance;
	//			glm::vec3 dir = normalize(diff);
	//
	//			//float alpha = constraint.compliance / dt;
	//
	//			transA.position += -dir * displacement * (massA / (massA + massB) * constraint.stiffness);
	//			transB.position += dir * displacement * (massB / (massA + massB) * constraint.stiffness);
	//
	//			Renderer::DrawLine(transA.position, transB.position);
	//		}
	//	}
	//}


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
					glm::vec3 diff = transform2.position - transform1.position; // from A to B
					float distanceSquared = dot(diff, diff);
					float radii = collider1.radius + collider2.radius; // radiuses

					//Renderer::DrawLine(transform1.position, transform1.position + diff);

					if (distanceSquared < radii * radii)
					{
						glm::vec3 normal = normalize(diff); // normal from A to B

						float penetration = radii - sqrt(distanceSquared);

						// @TODO what if the spheres have different radiuses?
						glm::vec3 contactPoint = transform1.position + normal * collider1.radius - penetration * 0.5f;

						//Renderer::DrawPoint(contactPoint, glm::vec3(1.f, 0.5f, 1.f));

						ContactManifold manifold = { rigidBody1, rigidBody2, transform1, transform2, normal, {contactPoint}, penetration };
						contactManifolds.push_back(manifold);
					}
				}
				else if (collider1.type == collider2.type && collider1.type == ColliderType::Cube)
				{
					// Cube - Cube
					ContactManifold manifold(rigidBody1, rigidBody2, transform1, transform2);

					if (CheckCollisionCubeWithCube(transform1, transform2, collider1, collider2, manifold))
						contactManifolds.push_back(manifold);
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

					glm::vec3 diff = closestPoint - sphereTransform.position;
					float distanceSquared = dot(diff, diff); // distance from closes point to the sphere. length of diff without expensive sqrt

					// if distance is less or equal then we have a collision!
					if (distanceSquared <= sphereCollider.radius * sphereCollider.radius)
					{
						glm::vec3 normal = normalize(diff); // direction from sphere to point on OBB

						float penetration = sphereCollider.radius - sqrt(distanceSquared);

						// @TODO: Change this!!
						ContactManifold manifold = { isEntity1Sphere ? rigidBody1 : rigidBody2, isEntity1Sphere ? rigidBody2 : rigidBody1, sphereTransform, cubeTransform, normal, {closestPoint}, penetration };
						contactManifolds.push_back(manifold);
					}
				}
			}
		}
	}


	const int SOLVER_ITERATIONS = 10;

	void SolveCollisions()
	{
		if (contactManifolds.empty())
			return;

		for (int i = 0; i < SOLVER_ITERATIONS; i++)
		{
			for (ContactManifold& manifold : contactManifolds)
			{
				// if both bodies are static then skip solving
				if (!manifold.rigidBody1.invMass && !manifold.rigidBody2.invMass)
					continue;

				SolveVelocityConstraint(manifold.rigidBody1, manifold.rigidBody2, manifold.transform1, manifold.transform2, manifold.normal, manifold.contactPoints, manifold.accumulatedImpulses);
				
				SolvePositionConstraint(manifold.rigidBody1, manifold.rigidBody2, manifold.transform1, manifold.transform2, manifold.normal, manifold.penetration);
			}
		}
		contactManifolds.clear();
	}

	void SolveVelocityConstraint(RigidBodyComponent& rigidBody1, RigidBodyComponent& rigidBody2, const TransformComponent& transform1, const TransformComponent& transform2, const glm::vec3& normal, const std::vector<glm::vec3>& contactPoints, std::vector<float>& accumulatedImpulses)
	{
		if (contactPoints.empty())
		{
			QUACK_ERROR("Contact Points empty!!!!!");
			return;
		}

		float restitution = glm::max(rigidBody1.bounce,rigidBody2.bounce);

		glm::mat3 R1 = glm::toMat3(transform1.orientation);
		glm::mat3 R2 = glm::toMat3(transform2.orientation);

		// transpose is the same as inverse (because the rotation matrix is orthogonal) but transpose is less expensive
		glm::mat3 invInertiaWorld1 = R1 * rigidBody1.invInertiaTensor * glm::transpose(R1);
		glm::mat3 invInertiaWorld2 = R2 * rigidBody2.invInertiaTensor * glm::transpose(R2);

		for (int i = 0; i < contactPoints.size(); i++)
		{
			const glm::vec3 contactPoint = contactPoints[i];

			// Arm from center of mass to a point of contact
			glm::vec3 r1 = contactPoint - transform1.position;
			glm::vec3 r2 = contactPoint - transform2.position;

			// Velocity of a point of the body (linear velocity (center of mass) + angular velocity x arm)
			glm::vec3 v1 = rigidBody1.velocity + cross(rigidBody1.angularVelocity, r1);
			glm::vec3 v2 = rigidBody2.velocity + cross(rigidBody2.angularVelocity, r2);

			// Velocity of B relative to A
			glm::vec3 relativeVelocity = v2 - v1;

			// check if the bodies are already separating or moving into one another
			float normalVelocity = dot(relativeVelocity, normal);

			// @TODO introduce a bias or something that even if normal velocity is positive but penetration exists then still apply impulse
			if (normalVelocity > 0.f)
			{
				continue; // already separating - do nothing
			}

			// If resting then no bounce should be present
			if (abs(normalVelocity) < 0.2f)
				restitution = 0.f;

			// arm & normal are in world space so inertia should also be in world space!!!!
			// rotational/angular resistance vector
			glm::vec3 rotResistance1 = cross(invInertiaWorld1 * cross(r1, normal), r1);
			glm::vec3 rotResistance2 = cross(invInertiaWorld2 * cross(r2, normal), r2);

			// "effective mass of this collision"
			float effectiveMass = rigidBody1.invMass + rigidBody2.invMass + dot(normal, rotResistance1 + rotResistance2);

			float deltaImpulse = (-(1.f + restitution) * normalVelocity) / effectiveMass;

			// We want to ensure that the total applied impulse in this frame in not negative 
			// box sitting on a floor can't pull itself, it can only push. If it could the floor would turn into superglue and wouldn't let go of the box
			float& accumulatedImpulse = accumulatedImpulses[i];
			
			float newAccumulated = glm::max(accumulatedImpulse + deltaImpulse, 0.0f);
			float impulseToApply = newAccumulated - accumulatedImpulse;

			accumulatedImpulse = newAccumulated;

			glm::vec3 vectorImpulse = impulseToApply * normal;

			rigidBody1.velocity -= vectorImpulse * rigidBody1.invMass;
			rigidBody2.velocity += vectorImpulse * rigidBody2.invMass;

			// apply angular momentum change (calculate torque)
			rigidBody1.angularVelocity -= invInertiaWorld1 * glm::cross(r1, vectorImpulse);
			rigidBody2.angularVelocity += invInertiaWorld2 * glm::cross(r2, vectorImpulse);
		}
	}



	void SolvePositionConstraint(const RigidBodyComponent& rigidBody1, const RigidBodyComponent& rigidBody2, TransformComponent& transform1, TransformComponent& transform2, const glm::vec3& normal, float shortestOverlap)
	{
		float slop = 0.01f;   // allowed penetration
		float percent = 0.2f; // how aggressive the correction is

		float correctionMag = glm::max(shortestOverlap - slop, 0.0f) * percent;
		glm::vec3 correction = correctionMag * normal;

		if(rigidBody1.invMass) // prevents from static bodies disappearing and setting position to NaN
			transform1.position -= correction * rigidBody1.invMass / (rigidBody1.invMass + rigidBody2.invMass);
		if (rigidBody2.invMass)
			transform2.position += correction * rigidBody2.invMass / (rigidBody1.invMass + rigidBody2.invMass);
	}


	glm::vec3 FindClosestPointToSphereOnOBB(const glm::vec3& spherePosition, float sphereRadius, const glm::vec3& cubePosition, const glm::vec3& cubeHalfSize, const glm::quat& cubeOrientation)
	{
		// vector from cube to sphere
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


	bool CheckCollisionCubeWithCube(TransformComponent& transform1, TransformComponent& transform2, const ColliderComponent& collider1, const ColliderComponent& collider2, ContactManifold& contactManifold)
	{
		// OBB - OBB collision (SAT)
		glm::mat3 cube1Axes = glm::toMat3(transform1.orientation);
		glm::mat3 cube2Axes = glm::toMat3(transform2.orientation);

		// orientation quat is normalized every frame so no need for normalizing these axes
		std::vector<glm::vec3> axes = { cube1Axes[0], cube1Axes[1], cube1Axes[2], cube2Axes[0], cube2Axes[1], cube2Axes[2] };

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				glm::vec3 crossedAxis = cross(cube1Axes[i], cube2Axes[j]);

				// if the axis is zero or near zero (e.g. parallel to another) then it will cause division by 0 when trying to normalize it
				if (glm::length2(crossedAxis) < 0.00001f)
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

		bool isAxisFirstBody = false;
		bool isAxisCrossProduct = false;
		int whichCrossProductAxis = -1;

		glm::vec3 shortestAxis;
		float shortestOverlap = 100000.f;

		for (int i = 0; i < axes.size(); i++)
		{
			const glm::vec3& axis = axes[i];
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
				// Which body was the axis that won
				isAxisFirstBody = i < 3;
				isAxisCrossProduct = i > 5;
				whichCrossProductAxis = i;
			}
		}

		// Ensure that normal points from A to B
		glm::vec3 centerToCenter = transform2.position - transform1.position;
		if (glm::dot(shortestAxis, centerToCenter) < 0.f)
		{
			shortestAxis = -shortestAxis;
		}

		// Normal always points from A to B so we have to flip it if the first body is not ref
		glm::vec3 clippingNormal = shortestAxis;
		if (!isAxisFirstBody)
		{
			clippingNormal *= -1;
		}

		// Find points of contact

		if (isAxisCrossProduct)
		{
			// @TODO: fix this, i have had several existential crisis doing this, will fix in the future

			//QUACK_LOG("Axis is from cross product");
			int crossAxisIndex = whichCrossProductAxis - 6;

			int i = crossAxisIndex / 3;
			int j = crossAxisIndex % 3;

			glm::vec3 crossAxis1 = cube1Axes[i];
			glm::vec3 crossAxis2 = cube2Axes[j];

			glm::vec3 otherA1 = cube1Axes[(i + 1) % 3];
			glm::vec3 otherA2 = cube1Axes[(i + 2) % 3];

			float sa1 = glm::sign(glm::dot(shortestAxis, otherA1));
			float sa2 = glm::sign(glm::dot(shortestAxis, otherA2));

			glm::vec3 edgeCenterA = transform1.position + sa1 * otherA1 * collider1.halfSize[(i + 1) % 3] + sa2 * otherA2 * collider1.halfSize[(i + 2) % 3];

			glm::vec3 otherB1 = cube2Axes[(j + 1) % 3];
			glm::vec3 otherB2 = cube2Axes[(j + 2) % 3];

			float sb1 = glm::sign(glm::dot(shortestAxis, -otherB1));
			float sb2 = glm::sign(glm::dot(shortestAxis, -otherB2));

			glm::vec3 edgeCenterB = transform2.position + sb1 * otherB1 * collider2.halfSize[(j + 1) % 3] + sb2 * otherB2 * collider2.halfSize[(j + 2) % 3];

			glm::vec3 contactPoint = (edgeCenterA + edgeCenterB) * 0.5f;

			Renderer::DrawPoint(contactPoint, glm::vec3(1.f, 0.5f, 0.5f));


			contactManifold.normal = shortestAxis;
			contactManifold.penetration = shortestOverlap;
			contactManifold.SetContactPoints({ contactPoint });

			return true;
		}



		TransformComponent& incTransform = isAxisFirstBody ? transform2 : transform1;
		TransformComponent& refTransform = isAxisFirstBody ? transform1 : transform2;
		const ColliderComponent& incCollider = isAxisFirstBody ? collider2 : collider1;
		const ColliderComponent& refCollider = isAxisFirstBody ? collider1 : collider2;

		glm::mat3 incAxes = glm::toMat3(incTransform.orientation);
		glm::mat3 refAxes = glm::toMat3(refTransform.orientation);

		glm::vec3 incFaceNormal = incAxes[0];
		float incDot = 1.f;

		glm::vec3 refFaceNormal = refAxes[0];
		float refDot = -1.f;

		// Find which of the 6 faces is the incident (most anti parallel) & reference (most parallel) face
		for (int i = 0; i < 3; i++)
		{
			float incDir = glm::dot(clippingNormal, incAxes[i]);

			if (incDir < incDot)
			{
				incDot = incDir;
				incFaceNormal = incAxes[i];
			}

			if (-incDir < incDot)
			{
				incDot = -incDir;
				incFaceNormal = -incAxes[i];
			}

			float refDir = glm::dot(clippingNormal, refAxes[i]);

			if (refDir > refDot)
			{
				refDot = refDir;
				refFaceNormal = refAxes[i];
			}

			if (-refDir > refDot)
			{
				refDot = -refDir;
				refFaceNormal = -refAxes[i];
			}
		}

		// Faces from normals
		glm::vec3 incFaceCenter;
		std::vector<glm::vec3> incFace = CreateFaceFromNormal(incFaceNormal, incAxes, incTransform.position, incCollider.halfSize, incFaceCenter);

		glm::vec3 refFaceCenter;
		std::vector<glm::vec3> refFace = CreateFaceFromNormal(refFaceNormal, refAxes, refTransform.position, refCollider.halfSize, refFaceCenter);

		// Clipping
		// Side planes from edges
		std::vector<glm::vec3> planePoints;
		std::vector<glm::vec3> planeNormals;

		// Colors for debugging
		//glm::vec3 color[4] = { glm::vec3(1.f, 0.5f, 0.5f), glm::vec3(0.5f, 1.f, 0.5f), glm::vec3(0.5f, 0.5f, 1.f), glm::vec3(1.f, 1.f, 1.f) };
		for (int i = 0; i < 4; i++)
		{
			glm::vec3 planePoint = refFace[i];
			glm::vec3 edgeDir = normalize(refFace[(i + 1) % 4] - refFace[i]);
			glm::vec3 planeNormal = cross(edgeDir, clippingNormal);
			
			// Check if the plane points inward
			if (dot(refFaceCenter - planePoint, planeNormal) < 0)
				planeNormal = -planeNormal;

			planePoints.push_back(planePoint);
			planeNormals.push_back(planeNormal);

			//Renderer::DrawLine(planePoint + edgeDir * 0.5f, planePoint + edgeDir * 0.5f + planeNormal * 0.5f, color[i]);
			//Renderer::DrawLine(planePoint, planePoint + edgeDir * 5.f, color[i]);
			//Renderer::DrawLine(planePoint, planePoint - edgeDir * 5.f, color[i]);
			//Renderer::DrawLine(planePoint, planePoint + clippingNormal * 5.f, color[i]);
			//Renderer::DrawLine(planePoint, planePoint - clippingNormal * 5.f, color[i]);
		}


		std::vector<glm::vec3> contactPolygon = incFace;

		for (int i = 0; i < 4; i++)
		{
			std::vector<glm::vec3> clippedPolygon;

			for (int j = 0; j < contactPolygon.size(); j++)
			{
				// Break loop if there is only one contact point
				if (contactPolygon.size() < 2)
					break;

				// Check if point is on the cutting plane
				glm::vec3 currentVertex = contactPolygon[j];
				glm::vec3 nextVertex = contactPolygon[(j + 1) % contactPolygon.size()];

				float distanceCurrent = dot(currentVertex - planePoints[i], planeNormals[i]);
				float distanceNext = dot(nextVertex - planePoints[i], planeNormals[i]);

				if (distanceCurrent >= 0)
				{
					// Current point inside
					if (distanceNext >= 0)
					{
						clippedPolygon.push_back(nextVertex);
					}
					else
					{
						float t = distanceCurrent / (distanceCurrent - distanceNext);
						glm::vec3 intersectionPoint = currentVertex + t * (nextVertex - currentVertex);
						clippedPolygon.push_back(intersectionPoint);
					}
				}
				else
				{
					// Current point outside
					if (distanceNext >= 0)
					{
						float t = distanceCurrent / (distanceCurrent - distanceNext);
						glm::vec3 intersectionPoint = currentVertex + t * (nextVertex - currentVertex);
						clippedPolygon.push_back(intersectionPoint);

						clippedPolygon.push_back(nextVertex);
					}
					// If both points outside then we keep nothing
				}
			}

			contactPolygon = clippedPolygon;
		}

		std::vector<glm::vec3> contactPoints;

		for (glm::vec3 point : contactPolygon)
		{
			bool isPointPenetrating = dot(point - refFace[0], clippingNormal) <= shortestOverlap + 0.01f;

			if (isPointPenetrating)
			{
				contactPoints.push_back(point);
				Renderer::DrawPoint(point, glm::vec3(0.f, 1.f, 0.f));
			}
		}
		
		contactManifold.normal = shortestAxis;
		contactManifold.penetration = shortestOverlap;
		contactManifold.contactPoints = contactPoints;
		contactManifold.SetContactPoints(contactPoints);

		return true;
	}


	std::vector<glm::vec3> CreateFaceFromNormal(const glm::vec3& faceNormal, const glm::mat3& axes, const glm::vec3& position, const glm::vec3& halfSize, glm::vec3& faceCenter)
	{
		float dx = dot(faceNormal, axes[0]);
		float dy = dot(faceNormal, axes[1]);
		float dz = dot(faceNormal, axes[2]);

		glm::vec3 topLeftVertex, bottomLeftVertex, bottomRightVertex, topRightVertex;

		if (abs(dx) > abs(dy) && abs(dx) > abs(dz))
		{
			faceCenter = position + axes[0] * halfSize.x * glm::sign(dx);

			topLeftVertex = faceCenter + axes[1] * halfSize.y + axes[2] * halfSize.z;
			bottomLeftVertex = faceCenter - axes[1] * halfSize.y + axes[2] * halfSize.z;
			bottomRightVertex = faceCenter - axes[1] * halfSize.y - axes[2] * halfSize.z;
			topRightVertex = faceCenter + axes[1] * halfSize.y - axes[2] * halfSize.z;

			//QUACK_LOG("x face");
		}
		else if (abs(dy) > abs(dx) && abs(dy) > abs(dz))
		{
			faceCenter = position + axes[1] * halfSize.y * glm::sign(dy);

			topLeftVertex = faceCenter + axes[0] * halfSize.x + axes[2] * halfSize.z;
			bottomLeftVertex = faceCenter - axes[0] * halfSize.x + axes[2] * halfSize.z;
			bottomRightVertex = faceCenter - axes[0] * halfSize.x - axes[2] * halfSize.z;
			topRightVertex = faceCenter + axes[0] * halfSize.x - axes[2] * halfSize.z;

			//QUACK_LOG("y face");
		}
		else if (abs(dz) > abs(dx) && abs(dz) > abs(dy))
		{
			faceCenter = position + axes[2] * halfSize.z * glm::sign(dz);

			topLeftVertex = faceCenter + axes[1] * halfSize.y + axes[0] * halfSize.x;
			bottomLeftVertex = faceCenter - axes[1] * halfSize.y + axes[0] * halfSize.x;
			bottomRightVertex = faceCenter - axes[1] * halfSize.y - axes[0] * halfSize.x;
			topRightVertex = faceCenter + axes[1] * halfSize.y - axes[0] * halfSize.x;

			//QUACK_LOG("z face");
		}

		return { topLeftVertex, bottomLeftVertex, bottomRightVertex, topRightVertex };
	}
}
