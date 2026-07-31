#include "Systems.h"

#include "Scene.h"
#include "Components.h"

#include "Renderer.h" // for debug only

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

namespace Quack
{
	const int SOLVER_ITERATIONS = 10;
	const float PERSISTENT_CONTACT_THRESHOLD_SQ = 0.02f;

	const float Y_TRESHOLD = -100.f;

	struct ContactManifold;

	void SolveVelocityConstraint(RigidBodyComponent& rigidBody1, RigidBodyComponent& rigidBody2, const TransformComponent& transform1, const TransformComponent& transform2, const glm::vec3& normal, const std::vector<glm::vec3>& contactPoints, std::vector<float>& accumulatedImpulses, std::vector<float>& accumulatedFrictions1, std::vector<float>& accumulatedFrictions2);
	void SolvePositionConstraint(RigidBodyComponent& rigidBody1, RigidBodyComponent& rigidBody2, TransformComponent& transform1, TransformComponent& transform2, const glm::vec3& normal, float penetration, const std::vector<glm::vec3>& contactPoints);

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

		std::vector<glm::vec3> localPoints1;
		std::vector<glm::vec3> localPoints2;

		std::vector<float> accumulatedImpulses; // normal
		float penetration;

		std::vector<float> accumulatedFrictions1; // tangential 1
		std::vector<float> accumulatedFrictions2; // tangential 2

		ContactManifold(RigidBodyComponent& r1, RigidBodyComponent& r2, TransformComponent& t1, TransformComponent& t2, glm::vec3 normal = glm::vec3(0.f), std::vector<glm::vec3> contactPoints = {}, float penetration = 0.f)
			: rigidBody1(r1), rigidBody2(r2), transform1(t1), transform2(t2), normal(normal), contactPoints(contactPoints), penetration(penetration) 
		{
			accumulatedImpulses.resize(contactPoints.size(), 0.f);
			accumulatedFrictions1.resize(contactPoints.size(), 0.f);
			accumulatedFrictions2.resize(contactPoints.size(), 0.f);
		}

		void SetContactPoints(const std::vector<glm::vec3>& points)
		{
			contactPoints = points;
			accumulatedImpulses.resize(points.size(), 0.f);
			accumulatedFrictions1.resize(points.size(), 0.f);
			accumulatedFrictions2.resize(points.size(), 0.f);
		}
	};

	struct CachedContact
	{
		glm::vec3 localPosition1;
		glm::vec3 localPosition2;

		float accumulatedImpulse; // normal
		float accumulatedFriction1; // tangential
		float accumulatedFriction2; // tangential
	};

	static std::vector<ContactManifold> contactManifolds;
	static std::vector<CachedContact> prevFrameContacts;

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
			rigidBody.velocity = rigidBody.velocity * rigidBody.damping + acceleration * dt;
			transform.position += rigidBody.velocity * dt;
			//transform.position += (oldVelocity + rigidBody.velocity) * 0.5f * dt;
			//Renderer::DrawPoint(transform.position);

			// angular motion
			rigidBody.angularVelocity *= rigidBody.damping;

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

						glm::vec3 contactPoint = transform1.position + normal * collider1.radius - penetration * 0.5f;

						Renderer::DrawPoint(contactPoint, glm::vec3(1.f, 0.5f, 1.f));
						
						ContactManifold manifold = { rigidBody1, rigidBody2, transform1, transform2, normal, {contactPoint}, penetration };

						manifold.localPoints1.push_back(contactPoint);
						manifold.localPoints2.push_back(contactPoint);

						//for (CachedContact& oldContactPoint : prevFrameContacts)
						//{
						//	if (glm::length2(contactPoint - oldContactPoint.worldPosition) < PERSISTENT_CONTACT_THRESHOLD_SQ)
						//	{
						//		//QUACK_LOG("Found point match!");
						//		manifold.accumulatedImpulses[0] = oldContactPoint.accumulatedImpulse;
						//		manifold.accumulatedFrictions1[0] = oldContactPoint.accumulatedFriction1;
						//		manifold.accumulatedFrictions2[0] = oldContactPoint.accumulatedFriction2;
						//		oldContactPoint.accumulatedImpulse = 0.f; // to prevent double-assigning
						//		oldContactPoint.accumulatedFriction1 = 0.f;
						//		oldContactPoint.accumulatedFriction2 = 0.f;
						//		break;
						//	}
						//	//QUACK_LOG("To far away :(");
						//}

						contactManifolds.push_back(manifold);
					}
				}
				else if (collider1.type == collider2.type && collider1.type == ColliderType::Cube)
				{
					// Cube - Cube
					ContactManifold manifold(rigidBody1, rigidBody2, transform1, transform2);

					if (CheckCollisionCubeWithCube(transform1, transform2, collider1, collider2, manifold))
					{
						for (int i = 0; i < manifold.contactPoints.size(); i++)
						{
							glm::vec3& newLocalContact1 = manifold.localPoints1[i];
							glm::vec3& newLocalContact2 = manifold.localPoints2[i];
							for (CachedContact& oldContactPoint : prevFrameContacts)
							{
								if (glm::length2(newLocalContact1 - oldContactPoint.localPosition1) < PERSISTENT_CONTACT_THRESHOLD_SQ &&
									glm::length2(newLocalContact2 - oldContactPoint.localPosition2) < PERSISTENT_CONTACT_THRESHOLD_SQ)
								{
									//QUACK_LOG("Found point match!");
									manifold.accumulatedImpulses[i] = oldContactPoint.accumulatedImpulse;
									manifold.accumulatedFrictions1[i] = oldContactPoint.accumulatedFriction1;
									manifold.accumulatedFrictions2[i] = oldContactPoint.accumulatedFriction2;
									oldContactPoint.accumulatedImpulse = 0.f; // to prevent double-assigning
									oldContactPoint.accumulatedFriction1 = 0.f;
									oldContactPoint.accumulatedFriction2 = 0.f;
									break;
								}
								//QUACK_LOG("To far away :( {}", glm::length2(newContactPoint - oldContactPoint.worldPosition));
							}
						}
						contactManifolds.push_back(manifold);
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

					glm::vec3 diff = closestPoint - sphereTransform.position;
					float distanceSquared = dot(diff, diff); // distance from closes point to the sphere. length of diff without expensive sqrt

					// if distance is less or equal then we have a collision!
					if (distanceSquared <= sphereCollider.radius * sphereCollider.radius)
					{
						glm::vec3 normal = normalize(diff); // direction from sphere to point on OBB

						float penetration = sphereCollider.radius - sqrt(distanceSquared);

						// @TODO: Change this!!
						ContactManifold manifold = { isEntity1Sphere ? rigidBody1 : rigidBody2, isEntity1Sphere ? rigidBody2 : rigidBody1, sphereTransform, cubeTransform, normal, {closestPoint}, penetration };
						manifold.localPoints1.push_back(closestPoint);
						manifold.localPoints2.push_back(closestPoint);
						contactManifolds.push_back(manifold);
					}
				}
			}
		}

		prevFrameContacts.clear();
	}


	void SolveCollisions(const std::shared_ptr<Scene> scene)
	{
		if (contactManifolds.empty())
			return;

		// Warm start
		if(scene->bWarmStart)
			for (const ContactManifold& manifold : contactManifolds)
			{
				// Right now there can be more than 4 contact points and it breaks the simulation
				if(manifold.contactPoints.size() > 4)
					QUACK_LOG("{}", manifold.contactPoints.size());

				glm::mat3 R1 = glm::toMat3(manifold.transform1.orientation);
				glm::mat3 R2 = glm::toMat3(manifold.transform2.orientation);

				glm::mat3 invInertiaWorld1 = R1 * manifold.rigidBody1.invInertiaTensor * glm::transpose(R1);
				glm::mat3 invInertiaWorld2 = R2 * manifold.rigidBody2.invInertiaTensor * glm::transpose(R2);

				for (int i = 0; i < manifold.contactPoints.size(); i++)
				{
					if (manifold.accumulatedImpulses[i] <= 0.f)
						continue;

					//if (i >= 4)
					//	break;

					const glm::vec3& contactPoint = manifold.contactPoints[i];

					glm::vec3 r1 = contactPoint - manifold.transform1.position;
					glm::vec3 r2 = contactPoint - manifold.transform2.position;

					glm::vec3 tangent1, tangent2;

					if (glm::abs(manifold.normal.x) < 0.577f)
					{
						tangent1 = glm::vec3(0.f, manifold.normal.z, -manifold.normal.y);
					}
					else
					{

						tangent1 = glm::vec3(-manifold.normal.z, 0.f, manifold.normal.x);
					}

					tangent1 = glm::normalize(tangent1);

					tangent2 = glm::cross(manifold.normal, tangent1);

					glm::vec3 normalImpulse = manifold.accumulatedImpulses[i] * manifold.normal;
					glm::vec3 tangetImpulse1 = manifold.accumulatedFrictions1[i] * tangent1;
					glm::vec3 tangetImpulse2 = manifold.accumulatedFrictions2[i] * tangent2;

					glm::vec3 vectorImpulse = normalImpulse + tangetImpulse1 + tangetImpulse2;

					manifold.rigidBody1.velocity -= vectorImpulse * manifold.rigidBody1.invMass;
					manifold.rigidBody2.velocity += vectorImpulse * manifold.rigidBody2.invMass;

					manifold.rigidBody1.angularVelocity -= invInertiaWorld1 * glm::cross(r1, vectorImpulse);
					manifold.rigidBody2.angularVelocity += invInertiaWorld2 * glm::cross(r2, vectorImpulse);
				}
			}


		for (int i = 0; i < SOLVER_ITERATIONS; i++)
		{
			for (ContactManifold& manifold : contactManifolds)
			{
				// if both bodies are static then skip solving
				if (!manifold.rigidBody1.invMass && !manifold.rigidBody2.invMass)
					continue;

				SolveVelocityConstraint(manifold.rigidBody1, manifold.rigidBody2, manifold.transform1, manifold.transform2, manifold.normal, manifold.contactPoints, manifold.accumulatedImpulses, manifold.accumulatedFrictions1, manifold.accumulatedFrictions2);
				
				if(scene->bPositionalCorrection)
					SolvePositionConstraint(manifold.rigidBody1, manifold.rigidBody2, manifold.transform1, manifold.transform2, manifold.normal, manifold.penetration, manifold.contactPoints);
			}
		}


		for (const ContactManifold& manifold : contactManifolds)
		{
			for (int i = 0; i < manifold.contactPoints.size(); i++)
			{
				prevFrameContacts.push_back({ manifold.localPoints1[i], manifold.localPoints2[i], manifold.accumulatedImpulses[i], manifold.accumulatedFrictions1[i], manifold.accumulatedFrictions2[i] });
			}
		}

		contactManifolds.clear();
	}

	void SolveVelocityConstraint(RigidBodyComponent& rigidBody1, RigidBodyComponent& rigidBody2, const TransformComponent& transform1, const TransformComponent& transform2, const glm::vec3& normal, const std::vector<glm::vec3>& contactPoints, std::vector<float>& accumulatedImpulses, std::vector<float>& accumulatedFrictions1, std::vector<float>& accumulatedFrictions2)
	{
		if (contactPoints.empty())
		{
			QUACK_ERROR("Contact Points empty!!!!!");
			return;
		}

		float restitution = glm::max(rigidBody1.bounce,rigidBody2.bounce);
		float friction = glm::sqrt(rigidBody1.frictionCoef * rigidBody2.frictionCoef);

		restitution = 0.f;
		friction = 0.5f;

		glm::mat3 R1 = glm::toMat3(transform1.orientation);
		glm::mat3 R2 = glm::toMat3(transform2.orientation);

		// transpose is the same as inverse (because the rotation matrix is orthogonal) but transpose is less expensive
		glm::mat3 invInertiaWorld1 = R1 * rigidBody1.invInertiaTensor * glm::transpose(R1);
		glm::mat3 invInertiaWorld2 = R2 * rigidBody2.invInertiaTensor * glm::transpose(R2);

		for (int i = 0; i < contactPoints.size(); i++)
		{
			const glm::vec3& contactPoint = contactPoints[i];

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
			// box sitting on a floor can't pull itself, it can only push. If it could the floor would turn into super glue and wouldn't let go of the box
			float& accumulatedImpulse = accumulatedImpulses[i];
			
			float newAccumulated = glm::max(accumulatedImpulse + deltaImpulse, 0.0f);
			float impulseToApply = newAccumulated - accumulatedImpulse;

			accumulatedImpulse = newAccumulated;

			glm::vec3 tangent1, tangent2;

			// find a vector that is NOT parallel to normal
			if (glm::abs(normal.x) < 0.577f) // 1/sqrt(3)
			{
				// if normal.x is small = x is not a major component of normal (doesn't point somewhat in X direction)
				// so we can use (1, 0, 0) to cross with normal

				tangent1 = glm::vec3(0.f, normal.z, -normal.y);

				/*
				N cross (1, 0, 0) = {
					Ny * 0 - Nz * 0,
					Nz * 1 - Nx * 0,
					Nx * 0 - Ny * 1,
					};

				N cross (1, 0, 0) = (0, Nz, -Ny)
				e.g.: (0, -1, 0) cross (1, 0, 0) = (0, 0, 1)
				*/
			}
			else
			{
				// if normal.x is large = x is major component of normal (points somewhat in X direction)
				// so we can't use (1, 0, 0) because we could have e.g. (-1, 0, 0) cross (1, 0, 0) which gives (0, 0, 0)
				// could crash when trying to normalize it, so we use (0, 1, 0)

				tangent1 = glm::vec3(-normal.z, 0.f, normal.x);

				/*
				N cross (0, 1, 0) = {
					Ny * 0 - Nz * 1,
					Nz * 0 - Nx * 0,
					Nx * 1 - Ny * 0,
					};

				N cross (0, 1, 0) = (-Nz, 0, Nx)
				e.g.: (-1, 0, 0) cross (0, 1, 0) = (0, 0, -1)
				*/
			}

			tangent1 = glm::normalize(tangent1);

			tangent2 = glm::cross(normal, tangent1);

			Renderer::DrawLine(transform1.position, transform1.position + normal, glm::vec3(0.f, 1.f, 0.f));
			Renderer::DrawLine(transform1.position, transform1.position + tangent1, glm::vec3(1.f, 0.f, 0.f));
			Renderer::DrawLine(transform1.position, transform1.position + tangent2, glm::vec3(1.f, 1.f, 1.f));

			float tangentVelocity1 = dot(relativeVelocity, tangent1);
			float tangentVelocity2 = dot(relativeVelocity, tangent2);

			glm::vec3 rotResistance12 = cross(invInertiaWorld1 * cross(r1, tangent1), r1);
			glm::vec3 rotResistance22 = cross(invInertiaWorld2 * cross(r2, tangent1), r2);

			glm::vec3 rotResistance13 = cross(invInertiaWorld1 * cross(r1, tangent2), r1);
			glm::vec3 rotResistance23 = cross(invInertiaWorld2 * cross(r2, tangent2), r2);

			float effectiveMass2 = rigidBody1.invMass + rigidBody2.invMass + dot(tangent1, rotResistance12 + rotResistance22);
			float effectiveMass3 = rigidBody1.invMass + rigidBody2.invMass + dot(tangent2, rotResistance13 + rotResistance23);

			float deltaFriction1 = -tangentVelocity1 / effectiveMass2;
			float deltaFriction2 = -tangentVelocity2 / effectiveMass3;

			// Coulomb's friction law
			float maxFriction = friction * accumulatedImpulse;

			float& accumulatedFriction1 = accumulatedFrictions1[i];
			float newFriction1 = glm::clamp(accumulatedFriction1 + deltaFriction1, -maxFriction, maxFriction);
			float frictionToApply1 = newFriction1 - accumulatedFriction1;
			accumulatedFriction1 = newFriction1;

			float& accumulatedFriction2 = accumulatedFrictions2[i];
			float newFriction2 = glm::clamp(accumulatedFriction2 + deltaFriction2, -maxFriction, maxFriction);
			float frictionToApply2 = newFriction2 - accumulatedFriction2;
			accumulatedFriction2 = newFriction2;

			glm::vec3 normalImpulse = impulseToApply * normal;
			glm::vec3 tangetImpulse1 = frictionToApply1 * tangent1;
			glm::vec3 tangetImpulse2 = frictionToApply2 * tangent2;
			glm::vec3 vectorImpulse = normalImpulse + tangetImpulse1 + tangetImpulse2;

			rigidBody1.velocity -= vectorImpulse * rigidBody1.invMass;
			rigidBody2.velocity += vectorImpulse * rigidBody2.invMass;

			// apply angular momentum change (calculate torque)
			rigidBody1.angularVelocity -= invInertiaWorld1 * glm::cross(r1, vectorImpulse);
			rigidBody2.angularVelocity += invInertiaWorld2 * glm::cross(r2, vectorImpulse);
		}
	}



	void SolvePositionConstraint(RigidBodyComponent& rigidBody1, RigidBodyComponent& rigidBody2, TransformComponent& transform1, TransformComponent& transform2, const glm::vec3& normal, float penetration, const std::vector<glm::vec3>& contactPoints)
	{
		float slop = 0.002f; // allowed penetration
		float beta = 0.2f;  // how aggressive the correction is, 1 = remove all overlap in one timestep
	
		float correctionMag = glm::max(penetration - slop, 0.0f) * beta;

		glm::mat3 R1 = glm::toMat3(transform1.orientation);
		glm::mat3 R2 = glm::toMat3(transform2.orientation);

		glm::mat3 invInertiaWorld1 = R1 * rigidBody1.invInertiaTensor * glm::transpose(R1);
		glm::mat3 invInertiaWorld2 = R2 * rigidBody2.invInertiaTensor * glm::transpose(R2);

		for (const glm::vec3& contactPoint : contactPoints)
		{
			glm::vec3 r1 = contactPoint - transform1.position;
			glm::vec3 r2 = contactPoint - transform2.position;

			glm::vec3 rotResistance1 = cross(invInertiaWorld1 * cross(r1, normal), r1);
			glm::vec3 rotResistance2 = cross(invInertiaWorld2 * cross(r2, normal), r2);

			float effectiveMass = rigidBody1.invMass + rigidBody2.invMass + dot(normal, rotResistance1 + rotResistance2);

			glm::vec3 vectorImpulse = correctionMag * normal;

			if (rigidBody1.invMass) // prevents from static bodies disappearing and setting position to NaN
			{
				transform1.position -= vectorImpulse * rigidBody1.invMass / effectiveMass;

				//glm::vec3 theta = -invInertiaWorld1 * glm::cross(r1, vectorImpulse);
				//transform1.orientation += 0.5f * glm::quat(0.f, theta) * transform1.orientation;
				//transform1.orientation = glm::normalize(transform1.orientation);
			}

			if (rigidBody2.invMass)
			{
				transform2.position += vectorImpulse * rigidBody2.invMass / effectiveMass;

				//glm::vec3 theta = invInertiaWorld2 * glm::cross(r2, vectorImpulse);
				//transform2.orientation += 0.5f * glm::quat(0.f, theta) * transform2.orientation;
				//transform2.orientation = glm::normalize(transform2.orientation);
			}
		}
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

			glm::vec3 newPointLocal1 = glm::transpose(glm::toMat3(transform1.orientation)) * (contactPoint - transform1.position);
			glm::vec3 newPointLocal2 = glm::transpose(glm::toMat3(transform2.orientation)) * (contactPoint - transform2.position);

			contactManifold.localPoints1.push_back(newPointLocal1);
			contactManifold.localPoints2.push_back(newPointLocal2);

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
			bool isPointPenetrating = dot(point - refFace[0], clippingNormal) <= shortestOverlap + 0.001f;

			if (isPointPenetrating)
			{
				contactPoints.push_back(point);
				Renderer::DrawPoint(point, glm::vec3(0.f, 1.f, 0.f));
			}
		}

		if (contactPoints.empty())
		{
			// @TODO: what should i do when no points
			return false;
		}
		
		contactManifold.normal = shortestAxis;
		contactManifold.penetration = shortestOverlap;
		contactManifold.contactPoints = contactPoints;
		contactManifold.SetContactPoints(contactPoints);

		for (const glm::vec3& contactPoint : contactPoints)
		{
			glm::vec3 newPointLocal1 = glm::transpose(glm::toMat3(transform1.orientation)) * (contactPoint - transform1.position);
			glm::vec3 newPointLocal2 = glm::transpose(glm::toMat3(transform2.orientation)) * (contactPoint - transform2.position);

			contactManifold.localPoints1.push_back(newPointLocal1);
			contactManifold.localPoints2.push_back(newPointLocal2);
		}

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
