#pragma once


class Particle {
private:
	float x_, y_;
	float vx_, vy_;
	float mass;
	float radius_;

public:
	inline Particle(const float x_, const float y_, const float mass, const float radius_):
		x_(x_), y_(y_), mass(mass), radius_(radius_), vx_(0), vy_(0) { }

public:
	inline void applyForce(const float fx, const float fy, const float dt) {
		const float ax = fx / mass;
		const float ay = fy / mass;
		vx_ += ax * dt;
		vy_ += ay * dt;
	}

	inline void update(const float dt) {
		x_ += vx_ * dt;
		y_ += vy_ * dt;
	}

public:
	inline float& x() {
		return x_;
	}

	inline float& y() {
		return y_;
	}

	inline float& vx() {
		return vx_;
	}

	inline float& vy() {
		return vy_;
	}

	inline float x() const {
		return x_;
	}

	inline float y() const {
		return y_;
	}

	inline float vx() const {
		return vx_;
	}

	inline float vy() const {
		return vy_;
	}

	inline float radius() const {
		return radius_;
	}
};