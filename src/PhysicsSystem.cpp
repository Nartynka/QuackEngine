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
	const float PERSISTENT_CONTACT_THRESHOLD_SR = 1e-4f; // square root = 0.01

	const float Y_TRESHOLD = -100.f;
	const float EPSILON = 1e-3f;

	struct ContactManifold;
	struct Hit;
	struct Face;
	struct Plane;
	struct Line;

	void SolveVelocityConstraint(RigidBodyComponent& rigidBody1, RigidBodyComponent& rigidBody2, const TransformComponent& transform1, const TransformComponent& transform2, const glm::vec3& normal, const std::vector<glm::vec3>& contactPoints, std::vector<float>& accumulatedImpulses, std::vector<float>& accumulatedFrictions1, std::vector<float>& accumulatedFrictions2);
	void SolvePositionConstraint(RigidBodyComponent& rigidBody1, RigidBodyComponent& rigidBody2, TransformComponent& transform1, TransformComponent& transform2, const glm::vec3& normal, float penetration, const std::vector<glm::vec3>& contactPoints);

	bool CheckCollisionCubeWithCube(TransformComponent& transform1, TransformComponent& transform2, const ColliderComponent& collider1, const ColliderComponent& collider2, Hit& hit);
	std::vector<glm::vec3> GenerateContactPoints(const Hit& hit, RigidBodyComponent& r1, RigidBodyComponent& r2);

	Face BuildFace(const glm::vec3& position, const glm::vec3& faceNormal, const glm::vec3 axes[3], int normalIndex, const glm::vec3& halfSize);
	std::vector<glm::vec3> PolygonClipping(const Plane* sidePlanes, int planeCount, const glm::vec3& clippingNormal, const glm::vec3* faceToBeClipped);

	Line FindEdgeEndPoints(int index, const glm::mat3& axes, const glm::vec3& normal, const glm::vec3& position, const glm::vec3& halfSize);
	glm::vec3 ClosestPointOfTwoLines(const Line& line1, const Line& line2);

	glm::vec3 FindClosestPointToSphereOnOBB(const glm::vec3& spherePosition, float sphereRadius, const glm::vec3& cubePosition, const glm::vec3& cubeHalfSize, const glm::quat& cubeOrientation);
	std::vector<glm::vec3> GetVerticesFromSize(const glm::vec3& halfSize);
	glm::vec3 CreateIntersectionPoint(const glm::vec3& v1, const glm::vec3& v2, const Plane& plane);

	glm::vec3 WorldToLocalSpace(const glm::vec3& point, const glm::quat& orientation, const glm::vec3& position);

	struct ContactManifold
	{
		RigidBodyComponent& rigidBody1;
		RigidBodyComponent& rigidBody2;

		TransformComponent& transform1;
		TransformComponent& transform2;

		glm::vec3 normal;
		float penetration;

		std::vector<glm::vec3> contactPoints; // @TODO: change to array of max 4 points

		std::vector<glm::vec3> localPoints1;
		std::vector<glm::vec3> localPoints2;

		std::vector<float> accumulatedImpulses; // normal

		std::vector<float> accumulatedFrictions1; // tangential 1
		std::vector<float> accumulatedFrictions2; // tangential 2

		ContactManifold(RigidBodyComponent& r1, RigidBodyComponent& r2, TransformComponent& t1, TransformComponent& t2, glm::vec3 normal, float penetration, std::vector<glm::vec3> contactPoints = {})
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
		glm::vec3 worldPosition;
		glm::vec3 localPosition1;
		glm::vec3 localPosition2;

		float accumulatedImpulse; // normal
		float accumulatedFriction1; // tangential
		float accumulatedFriction2; // tangential
	};

	struct Face
	{
		glm::vec3 vertices[4];
		glm::vec3 normal;
	};

	struct Plane
	{
		//glm::vec3 point;
		float distance;
		glm::vec3 normal;

		Plane(float distance, glm::vec3 normal) : distance(distance), normal(normal) {}
		Plane(glm::vec3 point, glm::vec3 normal) : normal(normal)
		{
			distance = glm::dot(normal, point);
		}
	};

	struct Line
	{
		glm::vec3 start;
		glm::vec3 end;
	};

	struct Hit
	{
		bool isAxisCrossProduct;

		glm::vec3 normal;
		float penetrationDepth;

		Face refFace;
		Face incFace;

		std::pair<Line, Line> crossEdgePair;

		TransformComponent* incTransform;
		TransformComponent* refTransform;
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


	void CheckCollisions(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();

		auto view = registry.view<ColliderComponent, TransformComponent>();

		for (auto entity1 : view)
		{
			auto& [collider1, transform1] = view.get(entity1);

			QUACK_ASSERT(transform1.position == transform1.position, "Transform1 is NaN!!!");

			for (auto entity2 : view)
			{
				// entt entity is just an uint_32 so we can skip the reversed pairs e.g. (2,1) when (1,2) was already checked
				if (entity1 <= entity2)
					continue;

				auto& [collider2, transform2] = view.get(entity2);

				Entity e1(entity1, scene.get());
				Entity e2(entity2, scene.get());

				bool isBody1Dynamic = e1.HasComponent<RigidBodyComponent>();
				bool isBody2Dynamic = e2.HasComponent<RigidBodyComponent>();

				if(!isBody1Dynamic && !isBody2Dynamic)
					continue;

				RigidBodyComponent& rigidBody1 = isBody1Dynamic ? e1.GetComponent<RigidBodyComponent>() : staticRigidBody;
				RigidBodyComponent& rigidBody2 = isBody2Dynamic ? e2.GetComponent<RigidBodyComponent>() : staticRigidBody;

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
						
						ContactManifold manifold = { rigidBody1, rigidBody2, transform1, transform2, normal, penetration, {contactPoint} };

						manifold.localPoints1.push_back(contactPoint);
						manifold.localPoints2.push_back(contactPoint);

						//for (CachedContact& oldContactPoint : prevFrameContacts)
						//{
						//	if (glm::length2(contactPoint - oldContactPoint.worldPosition) < PERSISTENT_CONTACT_THRESHOLD_SR)
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

					Hit collisionData;

					QUACK_ASSERT(transform1.position == transform1.position, "Transform1 is NaN!!!");
					QUACK_ASSERT(transform2.position == transform2.position, "Transform2 is NaN!!!");

					if (CheckCollisionCubeWithCube(transform1, transform2, collider1, collider2, collisionData))
					{
						ContactManifold manifold(rigidBody1, rigidBody2, transform1, transform2, collisionData.normal, collisionData.penetrationDepth);

						std::vector<glm::vec3> worldContactPoints = GenerateContactPoints(collisionData, rigidBody1, rigidBody2);

						std::vector<glm::vec3> local1contactPoints;
						std::vector<glm::vec3> local2contactPoints;

						for (const glm::vec3& contactPoint : worldContactPoints)
						{
							local1contactPoints.push_back(WorldToLocalSpace(contactPoint, collisionData.refTransform->orientation, collisionData.refTransform->position));
							local2contactPoints.push_back(WorldToLocalSpace(contactPoint, collisionData.incTransform->orientation, collisionData.incTransform->position));
						}

						manifold.SetContactPoints(worldContactPoints);
						manifold.localPoints1 = local1contactPoints;
						manifold.localPoints2 = local2contactPoints;
						//rigidBody1.velocity = rigidBody2.velocity = rigidBody1.angularVelocity = rigidBody2.angularVelocity = rigidBody1.gravity = rigidBody2.gravity = glm::vec3(0.f);

						for (int i = 0; i < manifold.contactPoints.size(); i++)
						{
							glm::vec3& newLocalContact1 = manifold.localPoints1[i];
							glm::vec3& newLocalContact2 = manifold.localPoints2[i];

							glm::vec3& newContact = manifold.contactPoints[i];

							for (CachedContact& oldContactPoint : prevFrameContacts)
							{
								if (glm::length2(newContact - oldContactPoint.worldPosition) < PERSISTENT_CONTACT_THRESHOLD_SR ||
									glm::length2(newLocalContact1 - oldContactPoint.localPosition1) < PERSISTENT_CONTACT_THRESHOLD_SR ||
									glm::length2(newLocalContact2 - oldContactPoint.localPosition2) < PERSISTENT_CONTACT_THRESHOLD_SR)
								{
									//QUACK_LOG("Found point match!");
									manifold.accumulatedImpulses[i] = oldContactPoint.accumulatedImpulse;
									manifold.accumulatedFrictions1[i] = oldContactPoint.accumulatedFriction1;
									manifold.accumulatedFrictions2[i] = oldContactPoint.accumulatedFriction2;

									//oldContactPoint.accumulatedImpulse = 0.f; // to prevent double-assigning
									//oldContactPoint.accumulatedFriction1 = 0.f;
									//oldContactPoint.accumulatedFriction2 = 0.f;
									break;
								}
								//QUACK_LOG("To far away :( {}", glm::length2(newLocalContact1 - oldContactPoint.localPosition1));
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
						ContactManifold manifold = { isEntity1Sphere ? rigidBody1 : rigidBody2, isEntity1Sphere ? rigidBody2 : rigidBody1, sphereTransform, cubeTransform, normal, penetration, {closestPoint} };
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
				//if(manifold.contactPoints.size() > 4)
				//	QUACK_LOG("{}", manifold.contactPoints.size());

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

				QUACK_ASSERT(manifold.normal != glm::vec3(0.f), "Degenerate normal!!!");

				SolveVelocityConstraint(manifold.rigidBody1, manifold.rigidBody2, manifold.transform1, manifold.transform2, manifold.normal, manifold.contactPoints, manifold.accumulatedImpulses, manifold.accumulatedFrictions1, manifold.accumulatedFrictions2);
				
				if(scene->bPositionalCorrection)
					SolvePositionConstraint(manifold.rigidBody1, manifold.rigidBody2, manifold.transform1, manifold.transform2, manifold.normal, manifold.penetration, manifold.contactPoints);
			}
		}


		for (const ContactManifold& manifold : contactManifolds)
		{
			for (int i = 0; i < manifold.contactPoints.size(); i++)
			{
				prevFrameContacts.push_back({ manifold.contactPoints[i], manifold.localPoints1[i], manifold.localPoints2[i], manifold.accumulatedImpulses[i], manifold.accumulatedFrictions1[i], manifold.accumulatedFrictions2[i] });
				//prevFrameContacts.push_back({ manifold.contactPoints[i], manifold.accumulatedImpulses[i], manifold.accumulatedFrictions1[i], manifold.accumulatedFrictions2[i] });
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

		float restitution = glm::max(rigidBody1.bounce, rigidBody2.bounce);
		float friction = glm::sqrt(rigidBody1.frictionCoef * rigidBody2.frictionCoef);

		// @TODO: something is wrong with restitution because bodies won't jump and behave weird
		restitution = 0.f;
		//friction = 0.5f;

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
			if (glm::abs(normalVelocity) < 0.2f)
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

			//Renderer::DrawLine(transform1.position, transform1.position + normal, glm::vec3(0.f, 1.f, 0.f));
			//Renderer::DrawLine(transform1.position, transform1.position + tangent1, glm::vec3(1.f, 0.f, 0.f));
			//Renderer::DrawLine(transform1.position, transform1.position + tangent2, glm::vec3(1.f, 1.f, 1.f));

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


			glm::vec3 tangetImpulse1 = frictionToApply1 * tangent1;
			glm::vec3 tangetImpulse2 = frictionToApply2 * tangent2;

			glm::vec3 normalImpulse = impulseToApply * normal;
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
		float slop = 0.001f; // allowed penetration
		float beta = 0.1f;  // how aggressive the correction is, 1 = remove all overlap in one timestep
	
		// @TODO: recalculate penetration after each iteration or calculate penetration for each point manually
		float correctionMag = glm::max(penetration * 0.5f - slop, 0.0f) * beta;

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


	bool CheckCollisionCubeWithCube(TransformComponent& transform1, TransformComponent& transform2, const ColliderComponent& collider1, const ColliderComponent& collider2, Hit& hit)
	{
		// OBB - OBB collision (SAT)
		glm::mat3 cube1Axes = glm::toMat3(transform1.orientation);
		glm::mat3 cube2Axes = glm::toMat3(transform2.orientation);

		// orientation quat is normalized every frame so no need for normalizing these axes
		std::vector<glm::vec3> axes = { cube1Axes[0], cube1Axes[1], cube1Axes[2], cube2Axes[0], cube2Axes[1], cube2Axes[2] };

		std::vector<std::pair<int, int>> crossAxisPairs; // parallel to the tail of axes

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				glm::vec3 crossedAxis = cross(cube1Axes[i], cube2Axes[j]);

				// if the axis is zero or near zero (e.g. parallel to another) then it will cause division by 0 when trying to normalize it
				if (glm::length2(crossedAxis) < 0.00001f)
					continue;

				axes.push_back(normalize(crossedAxis));
				crossAxisPairs.emplace_back(i, j);
			}
		}

		std::vector<glm::vec3> cube1LocalVertices = GetVerticesFromSize(collider1.halfSize);
		std::vector<glm::vec3> cube2LocalVertices = GetVerticesFromSize(collider2.halfSize);

		std::vector<glm::vec3> cube1Vertices;
		std::vector<glm::vec3> cube2Vertices;

		for (const auto& v : cube1LocalVertices)
		{
			// Cube1 corners in world space (rotated and translated)
			// THE ORDER MATTERS!!! vec3 * quat is not the same as quat * vec3!!!!!!!!!!!!!!
			cube1Vertices.emplace_back(transform1.orientation * v + transform1.position);
		}

		for (const auto& v : cube2LocalVertices)
		{
			cube2Vertices.emplace_back(transform2.orientation * v + transform2.position);
		}


		bool isAxisFirstBody = false;
		bool isAxisCrossProduct = false;

		int index = 0;

		glm::vec3 shortestAxis;
		float shortestOverlap = FLT_MAX;

		for (int i = 0; i < axes.size(); i++)
		{
			const glm::vec3& axis = axes[i];
			float c1Min = dot(cube1Vertices[0], axis);
			float c1Max = c1Min;

			for (const auto& point : cube1Vertices)
			{
				float p = dot(point, axis);

				c1Min = glm::min(c1Min, p);
				c1Max = glm::max(c1Max, p);
			}

			// cube2
			float c2Min = dot(cube2Vertices[0], axis);
			float c2Max = c2Min;

			for (const auto& point : cube2Vertices)
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

			// add bias to favor normal axes and not crossed ones
			if (i > 5)
				amount += 0.005f;

			if (amount < shortestOverlap)
			{
				if (i > 5)
				{
					isAxisCrossProduct = true;
					amount -= 0.005f;
				}
				shortestOverlap = amount;
				index = i;
			}
		}
		
		shortestAxis = axes[index];

		// Ensure that normal points from first body to second body
		glm::vec3 centerToCenter = transform2.position - transform1.position;
		if (glm::dot(shortestAxis, centerToCenter) < 0.f) // if the shortestAxis is pointing in opposite way -> flip it
		{
			shortestAxis = -shortestAxis;
		}

		hit.normal = shortestAxis;
		hit.penetrationDepth = shortestOverlap;

		if (isAxisCrossProduct)
		{
			// axis index - 6 not cross product axes from both bodies gives us index of the cross product axis
			auto crossPair = crossAxisPairs[index-6];

			//Renderer::DrawLine(transform1.position, transform1.position+shortestAxis, glm::vec3(1.f, 1.f, 1.f));

			// Normal points from first body to the second body
			// For the first body we want the edge which second & third axis mostly points in the same direction as the normal
			// For the second body we want the edge which second & third axis mostly points in the opposite direction as the normal so we flip the normal
			Line edge1 = FindEdgeEndPoints(crossPair.first, cube1Axes, shortestAxis, transform1.position, collider1.halfSize);
			Line edge2 = FindEdgeEndPoints(crossPair.second, cube2Axes, -shortestAxis, transform2.position, collider2.halfSize);


			hit.isAxisCrossProduct = true;
			hit.crossEdgePair = { edge1, edge2 };

			hit.refTransform = &transform1;
			hit.incTransform = &transform2;

			return true;
		}



		// Which body was the axis that won
		isAxisFirstBody = index < 3;

		TransformComponent& refTransform = isAxisFirstBody ? transform1 : transform2;
		TransformComponent& incTransform = isAxisFirstBody ? transform2 : transform1;

		const ColliderComponent& refCollider = isAxisFirstBody ? collider1 : collider2;
		const ColliderComponent& incCollider = isAxisFirstBody ? collider2 : collider1;

		//refCollider.shapeColor = glm::vec3(1.f, 0.f, 1.f);


		// Collision normal always points from first body to second body so we have to flip it if the first body is not ref 
		// (we want if pointing away from ref, from the ref to inc)
		glm::vec3 refFaceNormal = shortestAxis;
		if (!isAxisFirstBody)
		{
			refFaceNormal *= -1;
		}


		// Reference face

		glm::vec3* refAxes = isAxisFirstBody ? &axes[0] : &axes[3];

		/*
		// The axes are calculated in BuildFace function, here only for debug
		glm::vec3 secondAxis = refAxes[(index + 1) % 3];
		glm::vec3 thirdAxis = refAxes[(index + 2) % 3];

		Renderer::DrawLine(refTransform.position, refTransform.position + refFaceNormal * shortestOverlap, glm::vec3(0.f, 1.f, 1.f));
		Renderer::DrawAxes(axes[index], secondAxis, thirdAxis, refTransform.position);
		*/
		
		Face refFace = BuildFace(refTransform.position, refFaceNormal, refAxes, index % 3, refCollider.halfSize);
		//Renderer::DrawPolygon(4, refFace.vertices, glm::vec3(0.f, 1.f, 1.f));


		// Incident face

		glm::vec3* incAxes = isAxisFirstBody ? &axes[3] : &axes[0];
		glm::vec3 incNormals[] = { incAxes[0], incAxes[1], incAxes[2],  -incAxes[0], -incAxes[1], -incAxes[2] };

		/*
		"To find the incident face simply iterate all faces on the other hull and compute the
		dot product of each face normal with the normal of the reference face.The face
		with the smallest dot product defines the incident face!" ~ Robust Contact Creation for Physics Simulations - Dirk Gregorius (Valve Software)
		*/

		float smallestDot = FLT_MAX;
		int incFaceIndex = 0;

		for (int i = 0; i < 6; i++)
		{
			float dot = glm::dot(incNormals[i], refFaceNormal);
			if (dot < smallestDot)
			{
				smallestDot = dot;
				incFaceIndex = i;
			}
		}

		// We use 0-2 indexes from incNormals because the sign of the other two axes doesn't matter
		// both of them will have the same sign (both positive or negative)
		// so when calculating face vertices + & + is plus and - & - is also
		
		/*
		// The axes are calculated in BuildFace function, here only for debug
		glm::vec3 incFaceNormal = incNormals[incFaceIndex];
		glm::vec3 incSecondAxis = incNormals[(incFaceIndex + 1) % 3];
		glm::vec3 incThirdAxis = incNormals[(incFaceIndex + 2) % 3];

		Renderer::DrawLine(incTransform.position, incTransform.position + incFaceNormal * shortestOverlap, glm::vec3(1.f, 0.f, 1.f));
		Renderer::DrawAxes(incFaceNormal, incSecondAxis, incThirdAxis, incTransform.position);
		*/

		Face incFace = BuildFace(incTransform.position, incNormals[incFaceIndex], incAxes, incFaceIndex % 3, incCollider.halfSize);

		QUACK_ASSERT(incFace.vertices[0] == incFace.vertices[0], "incFace vectices are NaN!!!");

		//Renderer::DrawPolygon(4, incFace.vertices);

		// Build hit return value for not edge-edge collision
		hit.isAxisCrossProduct = false;
		
		hit.refFace = refFace;
		hit.incFace = incFace;

		hit.refTransform = &refTransform;
		hit.incTransform = &incTransform;

		return true;
	}


	// Return cube vertices from half size in local space
	std::vector<glm::vec3> GetVerticesFromSize(const glm::vec3& halfSize)
	{
		// i in binary is:
		// 0000, 0001, 0010, 0011, 0100, 0101, ...
		// the bit shift shifts the bit that we are interested in (3rd for x, 2nd for y, 1st for z) to the right (is now the fist bit)
		// With AND operator we make sure that we get only one single bit from it
		// x doesn't need AND mask because the third bit is the highest bit we will get
		// z doesn't need bit shift because the bit we are interested in is already the first bit
		// 
		// Now to convert [0; 1] to [1; -1] we multiply by 2 and get [0; 2] and subtract it from 1 to get [1; -1]! 
		// (if we were to substract 1 from it and not it from 1 then we would get [-1; 1] and not [1; -1])

		std::vector<glm::vec3> localVertices;

		for (int i = 0; i < 8; i++)
		{
			glm::vec3 v = { (1 - (i >> 2) * 2) * halfSize.x, (1 - (i >> 1 & 1) * 2) * halfSize.y, (1 - (i & 1) * 2) * halfSize.z };

			localVertices.push_back(v);
		}

		return localVertices;
	}



	Face BuildFace(const glm::vec3& position, const glm::vec3& faceNormal, const glm::vec3 axes[3], int normalIndex, const glm::vec3& halfSize)
	{
		int secondIndex = (normalIndex + 1) % 3;
		int thirdIndex = (normalIndex + 2) % 3;

		glm::vec3 faceCenter = position + halfSize[normalIndex] * faceNormal;

		glm::vec3 v1 = faceCenter + halfSize[secondIndex] * axes[secondIndex] + halfSize[thirdIndex] * axes[thirdIndex];
		glm::vec3 v2 = faceCenter + halfSize[secondIndex] * axes[secondIndex] - halfSize[thirdIndex] * axes[thirdIndex];
		glm::vec3 v3 = faceCenter - halfSize[secondIndex] * axes[secondIndex] - halfSize[thirdIndex] * axes[thirdIndex];
		glm::vec3 v4 = faceCenter - halfSize[secondIndex] * axes[secondIndex] + halfSize[thirdIndex] * axes[thirdIndex];

		return { {v1, v2, v3, v4}, faceNormal };
	}

	std::vector<glm::vec3> GenerateContactPoints(const Hit& hit, RigidBodyComponent& r1, RigidBodyComponent& r2)
	{
		// After a lot of pain, suffering and floating point errors it's fixed!! \o/
		// Well almost xD
		// Same size cube stacks sometimes still generate 6 points instead of 4 due to some floating point error / dividing by almost zero / idk why
		// But getting same size cube stack to work perfectly it beyond my current knowledge and I did my best to get it working multiple times
		
		if (hit.isAxisCrossProduct)
		{
			glm::vec3 contactPoint = ClosestPointOfTwoLines(hit.crossEdgePair.first, hit.crossEdgePair.second);

			return { contactPoint };
		}

		// Clipping
		
		// Clipping planes from refFace
		std::vector<Plane> planes;

		const glm::vec3* refFace = hit.refFace.vertices;

		// Colors for debugging
		glm::vec3 color[4] = { glm::vec3(1.f, 0.5f, 0.5f), glm::vec3(0.5f, 1.f, 0.5f), glm::vec3(0.5f, 0.5f, 1.f), glm::vec3(1.f, 1.f, 1.f) };
		
		glm::vec3 clippingNormal = hit.refFace.normal;

		for (int i = 0; i < 4; i++)
		{
			glm::vec3 planePoint = refFace[i];
			glm::vec3 edgeDir = normalize(refFace[(i + 1) % 4] - refFace[i]);
			glm::vec3 planeNormal = glm::normalize(cross(edgeDir, clippingNormal)); // normalize???

			// Make sure that the plane points outwards
			if (dot(hit.refTransform->position - planePoint, planeNormal) > 0)
				planeNormal = -planeNormal;

			planes.emplace_back(planePoint, planeNormal);

			//Renderer::DrawLine(planePoint + edgeDir * 0.5f, planePoint + planeNormal + edgeDir * 0.5f, color[i]);
			//Renderer::DrawLine(planePoint, planePoint + edgeDir * 3.f, color[i]);
			//Renderer::DrawLine(planePoint, planePoint - edgeDir * 3.f, color[i]);
		}

		std::vector<glm::vec3> clippedPolygon = PolygonClipping(planes.data(), (int)planes.size(), clippingNormal, hit.incFace.vertices);

		std::vector<glm::vec3> contactPoints;

		// Only keep points bellow ref face & then project points onto ref face
		for (auto p : clippedPolygon)
		{
			// signed distance "t" from point on a plane (refFace) to clipped polygon point
			float distance = glm::dot(clippingNormal, p - refFace[0]);

			if (distance <= 0.006f)
			{
				glm::vec3 projection = p - distance * clippingNormal;
				contactPoints.push_back(projection);
				//Renderer::DrawPoint(projection, glm::vec3(0.f, 1.f, 1.f));
			}
			//else
			//{
			//	QUACK_LOG("Point above ref face: {}", distance);
			//}
		}

		Renderer::DrawPolygon(contactPoints.size(), contactPoints.data(), glm::vec3(0.f, 1.f, 1.f));
		//Renderer::DrawPolygon(clippedPolygon.size(), clippedPolygon.data(), glm::vec3(1.f, 1.f, 1.f));

		// There shouldn't be a situation where there are no contact points
		// So if there are no contact points it probably means that there is a bug somewhere, or something went wrong
		// e.g. when if(distance <= 0) was if(distance < 0) then if bodies were perfectly perpendicular (dot = 0) all the points were discarded
		//QUACK_ASSERT(!contactPoints.empty(), "Contact points empty!! Number of points after clipping: {}", clippedPolygon.size());
		if (contactPoints.empty())
		{
			QUACK_ERROR("Contact points empty!! Number of points after clipping: {}", clippedPolygon.size());
		}

		if (contactPoints.size() > 4)
		{
			QUACK_WARN("More than 4 contact points!!!! {}", contactPoints.size());
			// @TODO: remove this xD
			//contactPoints.erase(contactPoints.begin()+4, contactPoints.end());
			contactPoints.erase(contactPoints.end()-1);
		}

		//if (contactPoints.size() < 4)
		//{
		//	QUACK_WARN("Less than 4 contact points!! {} Number of points after clipping: {}", contactPoints.size(), clippedPolygon.size());
		//}

		return contactPoints;
	}

	Line FindEdgeEndPoints(int index, const glm::mat3& axes, const glm::vec3& normal, const glm::vec3& position, const glm::vec3& halfSize)
	{
		glm::vec3 edgeDir = axes[index];

		int secondIndex = (index + 1) % 3;
		int thirdIndex = (index + 2) % 3;

		glm::vec3 secondAxis = axes[secondIndex];
		glm::vec3 thirdAxis = axes[thirdIndex];

		if (glm::dot(secondAxis, normal) < 0.f)
		{
			secondAxis = -secondAxis;
		}
		if (glm::dot(thirdAxis, normal) < 0.f)
		{
			thirdAxis = -thirdAxis;
		}

		glm::vec3 edgeCenter = position;

		edgeCenter += secondAxis * halfSize[secondIndex];
		edgeCenter += thirdAxis * halfSize[thirdIndex];

		glm::vec3 A = edgeCenter - edgeDir * halfSize[index];
		glm::vec3 B = edgeCenter + edgeDir * halfSize[index];

		Renderer::DrawPoint(edgeCenter, glm::vec3(1.f, 0.f, 0.f));
		Renderer::DrawLine(A, B, glm::vec3(1.f, 1.f, 0.f));

		return { A, B };
	}


	glm::vec3 ClosestPointOfTwoLines(const Line& line1, const Line& line2)
	{
		glm::vec3 A = line1.start;
		glm::vec3 C = line2.start;

		//Renderer::DrawLine(A, line1End, glm::vec3(1.f, 1.f, 1.f));
		//Renderer::DrawLine(C, line2End, glm::vec3(1.f, 1.f, 0.f));

		glm::vec3 ab = line1.end - A;
		glm::vec3 cd = line2.end - C;

		// Point on a line: 
		// L1(s) = A + s*ab
		// L2(t) = C + t*cd
		//
		// We have to find s & t so that vector between these points (L1-L2) will be perpendicular to both lines
		// dot(L1(s) - L2(t), ab) = 0  &  dot(L1(s) - L2(t), cd) = 0

		glm::vec3 r = A - C;

		float a = glm::dot(ab, ab);
		float b = glm::dot(cd, ab);
		float c = glm::dot(r, ab);
		float e = glm::dot(cd, cd);
		float f = glm::dot(r, cd);

		float det = a * e - b * b;

		float s = (b * f - c * e) / det;
		float t = (a * f - b * c) / det;

		// !! Clamping breaks the perpendicularity to both lines !!
		//s = glm::clamp(s, 0.f, 1.f);
		//t = glm::clamp(t, 0.f, 1.f);
		// But with two edges of a cube the point will always be on both lines so no need for clamping 
		// or projecting the second point if clamping were needed

		glm::vec3 L1 = A + s * ab;
		glm::vec3 L2 = C + t * cd;

		glm::vec3 v = L1 - L2;

		float x = glm::dot(v, ab);
		float y = glm::dot(v, cd);

		//if (glm::epsilonNotEqual(x, 0.f, 0.00001f) || glm::epsilonNotEqual(y, 0.f, 0.00001f))
		//	QUACK_LOG("{}, {}", x, y);

		Renderer::DrawLine(L1, L2, glm::vec3(1.f, 0.f, 1.f));

		glm::vec3 point = (L1+L2) * 0.5f;

		Renderer::DrawPoint(point, glm::vec3(0.f, 1.f, 1.f));

		return point;
	}


	// planeCount is also its size because planes are defined as one point and a normal
	// faceToBeClipped is incident face which we clip against reference face planes
	std::vector<glm::vec3> PolygonClipping(const Plane* sidePlanes, int planeCount, const glm::vec3& clippingNormal, const glm::vec3* faceToBeClipped)
	{
		std::vector<glm::vec3> polygonToBeClipped(faceToBeClipped, faceToBeClipped + 4);

		for (int i = 0; i < planeCount; i++)
		{
			glm::vec3 normal = sidePlanes[i].normal;
			float distance = sidePlanes[i].distance;

			std::vector<glm::vec3> newClippedPolygon;

			for (int j = 0; j < polygonToBeClipped.size(); j++)
			{
				glm::vec3 v1 = polygonToBeClipped[j]; // A
				glm::vec3 v2 = polygonToBeClipped[(j + 1) % polygonToBeClipped.size()]; // B

				float d1 = glm::dot(normal, v1) - distance;
				float d2 = glm::dot(normal, v2) - distance;

				//if (glm::abs(d1) < EPSILON)
				//	d1 = 0.0f;
				//if (glm::abs(d2) < EPSILON)
				//	d2 = 0.0f;

				bool v1_outside = d1 > EPSILON;
				bool v2_outside = d2 > EPSILON;
				bool v1_inside = d1 < -EPSILON;
				bool v2_inside = d2 < -EPSILON;

				// if all are false then the edge is on the plane which is a special case

				if (v1_outside && v2_inside)
				{
					// Outside inside (keep intersection & v2)
					glm::vec3 intersection = CreateIntersectionPoint(v1, v2, sidePlanes[i]);

					newClippedPolygon.push_back(intersection);
					newClippedPolygon.push_back(v2);
				}
				else if (v1_outside && v2_outside) { /*Both outside (keep nothing)*/ }
				else if (v1_inside && v2_outside)
				{
					// inside outside (keep only intersection)
					glm::vec3 intersection = CreateIntersectionPoint(v1, v2, sidePlanes[i]);

					newClippedPolygon.push_back(intersection);
				}
				else
				{
					// inside-inside (keep v2)
					// inside-onPlane
					// onPlane-inside 
					// onPlane-onPlane
					newClippedPolygon.push_back(v2);
				}

				/*
				if (d1 > 0.f)
				{
					// Both outside (keep nothing)
					if (d2 > 0.f) {}

					// Outside inside (keep intersection & v2)
					else
					{
						//QUACK_LOG("Outside-inside, keeping v2 & new intersection point {} {}", i, j);
						glm::vec3 intersection = CreateIntersectionPoint(v1, v2, sidePlanes[i]);

						//if(intersection-v1)

						newClippedPolygon.push_back(intersection);
						newClippedPolygon.push_back(v2);
					}
				}
				else
				{
					// inside outside (keep only intersection)
					if (d2 > 0.f)
					{
						//QUACK_LOG("Inside-outside, keeping only intersection");

						float denom = glm::dot(normal, (v2 - v1));

						if(glm::abs(denom) < EPSILON)
							continue;

						glm::vec3 intersection = CreateIntersectionPoint(v1, v2, sidePlanes[i]);
						
						newClippedPolygon.push_back(intersection);
					}
					// inside inside (keep v2)
					else
					{
						//QUACK_LOG("Inside-inside, keeping v2 {} {}", i, j);
						newClippedPolygon.push_back(v2);
					}
				}
				*/
			}

			polygonToBeClipped = newClippedPolygon;
		}

		//Renderer::DrawPolygon((int)polygonToBeClipped.size(), polygonToBeClipped.data(), glm::vec3(0.f, 0.f, 1.f));
		//Renderer::DrawPolygon(4, faceToBeClipped, glm::vec3(1.f, 0.f, 0.f));


		// Check for duplicates / almost duplicates due to floating point error
		std::vector<glm::vec3> clippedPolygon;

		int skipped = 0;

		for (const glm::vec3& point : polygonToBeClipped)
		{
			bool isDuplicate = false;

			for (const glm::vec3& uniquePoint : clippedPolygon)
			{
				// sqrt(0.1) = 0.31f
				// sqrt(0.01) = 0.1f
				// sqrt(0.001) = 0.031f
				// sqrt(0.0001) = 0.01f
				// sqrt(0.00001) = 0.0031f
				// sqrt(0.000001) = 0.001f
				if (glm::length2(point - uniquePoint) < 1e-6f)
				{
					isDuplicate = true;
					break;
				}
			}

			if (!isDuplicate)
				clippedPolygon.push_back(point);
			//else
			//{
			//	//QUACK_LOG("duplicate, skipping!!!");
			//	skipped++;
			//}
		}

		//if(skipped > 0 || clippedPolygon.size() != 4)
		//	QUACK_LOG("size: {}, skiped: {}", clippedPolygon.size(), skipped);

		return clippedPolygon;
	}

	glm::vec3 CreateIntersectionPoint(const glm::vec3& v1, const glm::vec3& v2, const Plane& plane)
	{
		glm::vec3 ab = v2 - v1;

		float num = plane.distance - dot(plane.normal, v1);
		float denom = dot(plane.normal, ab);

		// Denominator close to 0 = the line is perpendicular to plane normal (so parallel to plane)
		// Numerator close to 0 = start of the line lays on the plane
		// Denmo & Num close to 0 = the whole line (both points) is laying on the plan
		// And floating points are a ... something

		// If denom & num is 0/close to 0 then the line is laying on the plane which shouldn't happen in the first place
		QUACK_ASSERT(glm::abs(denom) > EPSILON || glm::abs(num) > EPSILON, "The line lays on the plane!!");

		// If denom & num is 0/close to 0 then the line is laying on the plane, treat as inside-inside (keep v2)
		//if (glm::abs(denom) < EPSILON && glm::abs(num) < EPSILON)
		//	return v2;

		// If the point lays on the / close to plane then return the point as an intersection
		// which will be discarded when removing duplicates
		if (glm::abs(num) < EPSILON || glm::abs(denom) < EPSILON)
			return v1;

		float t = num / denom;


		//t = glm::abs(t) < EPSILON ? 0.f : t;

		//QUACK_LOG("t: {}   ; v1: {} {} {}; v2: {} {} {};  int: {} {} {}", t, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, intersection.x, intersection.y, intersection.z);
		//if(glm::abs(t) < 0.001f || glm::abs(denom) < 0.001f || glm::abs(num) < 0.001f)
			//QUACK_LOG("t: {}; num: {}; denom: {}", t, num, denom);

		return v1 + t * ab;
	}

	glm::vec3 WorldToLocalSpace(const glm::vec3& point, const glm::quat& orientation, const glm::vec3& position)
	{
		return glm::transpose(glm::toMat3(orientation)) * (point - position);
	}



}

