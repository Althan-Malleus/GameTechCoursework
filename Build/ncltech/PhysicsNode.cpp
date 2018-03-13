#include "PhysicsNode.h"
#include "PhysicsEngine.h"

void PhysicsNode::IntegrateForVelocity(float dt)
{
	/* TUTORIAL 2 CODE */
	/* TUTORIAL 2 CODE */
	if (invMass > 0.0f)
		linVelocity += PhysicsEngine::Instance()->GetGravity() * dt;

	//semi-implicit euler intergration
	linVelocity += force * invMass * dt;

	//apply velocity damping
	linVelocity =
		linVelocity * PhysicsEngine::Instance()->GetDampingFactor();
	//angular rotation
	// mass -> torque
	// velocity -> rotational velocity
	// position -> orientation

	angVelocity += invInertia * torque * dt;

	// apply velocity damping

	angVelocity =
		angVelocity * PhysicsEngine::Instance()->GetDampingFactor();
}

/* Between these two functions the physics engine will solve for velocity
based on collisions/constraints etc. So we need to integrate velocity, solve
constraints, then use final velocity to update position.
*/

void PhysicsNode::IntegrateForPosition(float dt)
{
	/* TUTORIAL 2 CODE */

	//update position
	//euler integration. works on assumption velocity does not change over time (or changes so slightly  it doesn't make a difference)

	position += linVelocity * dt;

	//update orientation, different calculation due to quaternions. leaves a slight error.
	orientation = orientation +
		Quaternion(angVelocity * dt * 0.5f, 0.0f) * orientation;
	/*invInertia = invInertia *
	(quaternion(angVelocity * dt * 0.5f, 0.0f(
	* orientation).ToMatrix3();*/

	orientation.Normalise();


	//Finally: Notify any listener's that this PhysicsNode has a new world transform.
	// - This is used by GameObject to set the worldTransform of any RenderNode's. 
	//   Please don't delete this!!!!!
	FireOnUpdateCallback();
}