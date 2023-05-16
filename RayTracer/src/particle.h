#pragma once
#include "core.h"
namespace cyclone {
	/**
	* A particle is the simplest object that can be simulated in the
	* physics system.
	*/
	class Particle
	{
	public:
		/**
		* Holds the linear position of the particle in
		* world space.
		*/
		Vector3 position;
		/**
		* Holds the linear velocity of the particle in
		* world space.
		*/
		Vector3 velocity;
		/**
		* Holds the acceleration of the particle. This value
		* can be used to set acceleration due to gravity (its primary
		* use) or any other constant acceleration.
		*/
		Vector3 acceleration;
		/**
		* Holds the amount of damping applied to linear
		* motion. Damping is required to remove energy added
		* through numerical instability in the integrator.
		*/
		real damping;
		/**
		* Holds the inverse of the mass of the particle. It
		* is more useful to hold the inverse mass because
		* integration is simpler and because in real-time
		* simulation it is more useful to have objects with
		* infinite mass(immovable) than zero mass
		* (completely unstable in numerical simulation).
		*/
		real inverseMass;
		/**
		 * Holds the accumulated force to be applied at the next
		 * simulation iteration only. This value is zeroed at each
		 * integration step.
		 */
		Vector3 forceAccum;
		/**
		* Integrates the particle forward in time by the given amount.
		* This function uses a Newton-Euler integration method, which is a
		* linear approximation of the correct integral. For this reason it
		* may be inaccurate in some cases.
		*/
		void integrate(real duration);
	};
}