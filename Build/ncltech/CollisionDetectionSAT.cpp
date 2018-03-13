#include "CollisionDetectionSAT.h"
#include <nclgl\NCLDebug.h>
#include "GeometryUtils.h"

using namespace GeometryUtils;

CollisionDetectionSAT::CollisionDetectionSAT()
{
}

void CollisionDetectionSAT::BeginNewPair(
	PhysicsNode* obj1,
	PhysicsNode* obj2,
	CollisionShape* shape1,
	CollisionShape* shape2)
{
	possibleColAxes.clear();

	pnodeA = obj1;
	pnodeB = obj2;
	cshapeA = obj1->GetCollisionShape();
	cshapeB = obj2->GetCollisionShape();

	areColliding = false;
}



bool CollisionDetectionSAT::AreColliding(CollisionData* out_coldata)
{
	/* TUTORIAL 4 CODE */
	if (!cshapeA || !cshapeB)
		return false;

	areColliding = false;
	possibleColAxes.clear();

	//default axes

	//GetCollisionAxes takes in the /other/ object as a parameter here

	std::vector<Vector3> axes1, axes2;

	cshapeA->GetCollisionAxes(pnodeB, axes1);
	for (const Vector3& axis : axes1)
		AddPossibleCollisionAxis(axis);

	cshapeB->GetCollisionAxes(pnodeA, axes2);
	for (const Vector3& axis : axes2)
		AddPossibleCollisionAxis(axis);

	//edge-edge cases

	for (const Vector3& norm1 : axes1)
	{
		for (const Vector3& norm2 : axes2)
		{
			AddPossibleCollisionAxis(
				Vector3::Cross(norm1, norm2).Normalise());
		}
	}

	//SAT says that if a single axis is found where the two objects arent colliding then they cannot be colliding, so we have to check each possible axis until
	// we return false, or return the best possible axis until we either return false, or return the best axis(with the least penetration) found.

	CollisionData cur_colData;

	bestColData._penetration = -FLT_MAX;
	for (const Vector3& axis : possibleColAxes)
	{
		//if the collision axis does not interset then return immediately as we know that at least in one direction/axis the two objects do not intersect

		if (!CheckCollisionAxis(axis, cur_colData))
			return false;

		if (cur_colData._penetration >= bestColData._penetration)
		{
			bestColData = cur_colData;
		}
	}

	if (out_coldata) *out_coldata = bestColData;

	areColliding = true;
	return true;
	//return false;//Temporary - to avoid compiler errors

}


bool CollisionDetectionSAT::CheckCollisionAxis(const Vector3& axis, CollisionData& out_coldata)
{
	//Overlap Test
	// Points go: 
	//          +-------------+
	//  +-------|-----+   2   |
	//  |   1   |     |       |
	//  |       +-----|-------+
	//  +-------------+
	//  A ------C --- B ----- D 
	//
	//	IF	 A < C AND B > C (Overlap in order object 1 -> object 2)
	//	IF	 C < A AND D > A (Overlap in order object 2 -> object 1)

	/* TUTORIAL 4 CODE */


	Vector3 min1, min2, max1, max2;

	//get the min/max vertices along the axis from shape1 and shape2

	cshapeA->GetMinMaxVertexOnAxis(axis, min1, max1);
	cshapeB->GetMinMaxVertexOnAxis(axis, min2, max2);

	float A = Vector3::Dot(axis, min1);
	float B = Vector3::Dot(axis, max1);
	float C = Vector3::Dot(axis, min2);
	float D = Vector3::Dot(axis, max2);

	//overlap test (order: object 1 -> object 2

	if (A <= C && B >= C)
	{
		out_coldata._normal = axis;
		out_coldata._penetration = C - B;

		//smallest overlap distance is between B->C
		//compute closest point on edge of the object
		out_coldata._pointOnPlane =
			max1 + out_coldata._normal * out_coldata._penetration;

		return true;
	}

	//overlap test (order: object2 -> object1)
	if (C <= A && D >= A)
	{
		out_coldata._normal = -axis;
		//invert axis here so we can do all our resolution phase as object 1 -> object 2
		out_coldata._penetration = A - D;
		//smallest overlap distance is between D->A
		//compute closest point on edge of the object
		out_coldata._pointOnPlane =
			min1 + out_coldata._normal * out_coldata._penetration;
		return true;
	}



	return false;


	return false;
}






void CollisionDetectionSAT::GenContactPoints(Manifold* out_manifold)
{
	/* TUTORIAL 5 CODE */

	if (!out_manifold || !areColliding)
		return;
	if (bestColData._penetration >= 0.0f)
		return;

	//get the required face information for the two shapoes around the colliusion normal

	std::list<Vector3> polygon1, polygon2;
	Vector3 normal1, normal2;
	std::vector<Plane> adjPlanes1, adjPlanes2;

	cshapeA->GetIncidentReferencePolygon(
		bestColData._normal, polygon1, normal1, adjPlanes1);
	cshapeB->GetIncidentReferencePolygon(
		-bestColData._normal, polygon2, normal2, adjPlanes2);

	//if either shape1 or shape2 returned a single point, then it must be on a curve and this the only contact point to generate is already available

	if (polygon1.size() == 0 || polygon2.size() == 0)
	{
		return; //no points returned, resulting in no possible contact points
	}
	else if (polygon1.size() == 1)
	{
		out_manifold->AddContact(
			polygon1.front(), //polygon1 -> polygon2
			polygon1.front() + bestColData._normal
			* bestColData._penetration, bestColData._normal,
			bestColData._penetration);
	}
	else if (polygon2.size() == 1)
	{
		out_manifold->AddContact(
			polygon2.front() - bestColData._normal
			* bestColData._penetration,
			polygon2.front(), //polygon2 <- polygon 1
			bestColData._normal,
			bestColData._penetration);
	}
	else
	{
		/* Otherwise use clipping to cut down the incident face to fit inside the reference planes using the surrounding face planes

		first we need to know if we have to flip the incident and reference faces around for clipping */

		bool flipped = fabs(Vector3::Dot(bestColData._normal, normal1))
			< fabs(Vector3::Dot(bestColData._normal, normal2));

		if (flipped)
		{
			std::swap(polygon1, polygon2);
			std::swap(normal1, normal2);
			std::swap(adjPlanes1, adjPlanes2);
		}

		//clip the incident face to the adjacent edges of the reference face

		if (adjPlanes1.size() > 0)
			SutherlandHodgmanClipping(polygon2, adjPlanes1.size(),
				&adjPlanes1[0], &polygon2, false);

		//finally clip (and remove) any contact points that are above the reference face

		Plane refPlane =
			Plane(-normal1, -Vector3::Dot(-normal1, polygon1.front()));
		SutherlandHodgmanClipping(polygon2, 1, &refPlane, &polygon2, true);

		//now we are left with a selection of valid contact points to be used for the manifold

		for (const Vector3& point : polygon2)
		{
			//compute distance to reference plane

			Vector3 pointDiff =
				point - GetClosestPointPolygon(point, polygon1);
			float contact_penetration =
				Vector3::Dot(pointDiff, bestColData._normal);

			//set contact data

			Vector3 globalOnA = point;
			Vector3 globalOnB =
				point - bestColData._normal * contact_penetration;

			/*if we flipped incident and reference planes, we will need to flip it back before sending it to the manifold, e.g. turn it from talking about object2 -> object1 into
			object1 -> obbject2*/

			if (flipped)
			{
				contact_penetration = -contact_penetration;
				globalOnA =
					point + bestColData._normal * contact_penetration;

				globalOnB = point;
			}

			//just make a final sanity check that the contact point
			//is actually a point of contact not just a clipping bug

			//		if (contact_penetration > -0.01f) //was 0 
			{
				out_manifold->AddContact(
					globalOnA,
					globalOnB,
					bestColData._normal,
					contact_penetration);
			}
		}
	}

}


bool CollisionDetectionSAT::AddPossibleCollisionAxis(Vector3 axis)
{
	const float epsilon = 1e-6f;

	//is axis 0,0,0??
	if (Vector3::Dot(axis, axis) < epsilon)
		return false;

	axis.Normalise();

	for (const Vector3& p_axis : possibleColAxes)
	{
		//Is axis very close to the same as a previous axis already in the list of axes??
		if (Vector3::Dot(axis, p_axis) >= 1.0f - epsilon)
			return false;
	}

	possibleColAxes.push_back(axis);
	return true;
}