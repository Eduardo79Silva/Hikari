#include <assert.h>
#include "particle.h"

using namespace cyclone;

void Particle::integrate(real duration)
{
	assert(duration > 0.0);

	// Update linear position.
	position.addScaledVector(velocity, duration);

	// Work out the acceleration from the force.
	Vector3 resultingAcc = acceleration;
	resultingAcc.addScaledVector(forceAccum, inverseMass);

	// Update linear velocity from the acceleration.
	velocity.addScaledVector(resultingAcc, duration);
	// Impose drag.
	velocity *= real_pow(damping, duration);
}