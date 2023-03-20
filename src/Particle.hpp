#pragma once


#include "modules/BMath/vector.h"


class Particle {
private:
	// float x_, y_;
	// float vx_, vy_;
	const float mass;
	const float radius_;

public:
	bmath::vec2 pos, vel;

public:
	inline Particle(const float x_, const float y_, const float mass, const float radius_):
		// x_(x_), y_(y_), mass(mass), radius_(radius_), vx_(0), vy_(0) { }
		pos(x_, y_), mass(mass), radius_(radius_), vel(0, 0) { }

public:
	inline void applyForce(const float fx, const float fy, const float dt) {
		// const float ax = fx / mass;
		// const float ay = fy / mass;
		const bmath::vec2 acc(fx / mass, fy / mass);
		// vx_ += ax * dt;
		// vy_ += ay * dt;
		vel += acc * dt;
	}

	inline void update(const float dt) {
		// x_ += vx_ * dt;
		// y_ += vy_ * dt;
		pos += vel * dt;
	}

public:
	inline float& x() {
		// return x_;
		return pos[0];
	}

	inline float& y() {
		// return y_;
		return pos[1];
	}

	inline float& vx() {
		// return vx_;
		return vel[0];
	}

	inline float& vy() {
		// return vy_;
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