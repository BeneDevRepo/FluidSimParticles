#pragma once


#include "BMath/vector.h"


class Particle {
private:
	const float mass;
	const float radius_;

public:
	bmath::vec2 pos, vel;

public:
	inline Particle(const float x_, const float y_, const float mass, const float radius_):
		pos(x_, y_), mass(mass), radius_(radius_), vel(0, 0) { }

public:
	inline void applyForce(const float fx, const float fy, const float dt) {
		const bmath::vec2 acc(fx / mass, fy / mass);
		vel += acc * dt;
	}

	inline void update(const float dt) {
		pos += vel * dt;
	}

public:
	inline float& x() {
		return pos[0];
	}

	inline float& y() {
		return pos[1];
	}

	inline float& vx() {
		return vel[0];
	}

	inline float& vy() {
		return vel[1];
	}

	inline float x() const {
		return pos[0];
	}

	inline float y() const {
		return pos[1];
	}

	inline float vx() const {
		return vel[0];
	}

	inline float vy() const {
		return vel[1];
	}

	inline float radius() const {
		return radius_;
	}
};