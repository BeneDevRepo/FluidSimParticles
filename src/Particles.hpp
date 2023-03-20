#pragma once


#include <algorithm>
#include <vector>

#include "Particle.hpp"


class Particles {
public:
	std::vector<Particle> particles;

private:
	const size_t width;
	const size_t height;
	const size_t CELL_SIZE;

public:
	Particles(const size_t width, const size_t height, const size_t NUM_PARTICLES, const size_t CELL_SIZE = 5):
			width(width),
			height(height),
			CELL_SIZE(CELL_SIZE) {
		for(size_t i = 0; i < 1000; i++) {
			particles.push_back(
				Particle(
					1 + rand() % (width - 2),
					1 + rand() % (height - 2),
					1.f,
					2.f
				)
			);
		}
	}

	inline void applyForce(const float fx, const float fy, const float dt) {
		for(Particle& p : particles)
			p.applyForce(0, 9.81f, dt);
	}

	inline void updatePos(const float dt) {
		for(Particle& p : particles)
			p.update(dt);
	}

	inline void collide() {
		for(Particle& p : particles) {
			if(p.x() - p.radius() < 0 || p.x() + p.radius() > width) {
				p.x() = std::max<float>(p.x(), 0 + p.radius());
				p.x() = std::min<float>(p.x(), width - p.radius());
				p.vx() *= -1;
			}

			if(p.y() - p.radius() < 0 || p.y() + p.radius() > height) {
				p.y() = std::max<float>(p.y(), 0 + p.radius());
				p.y() = std::min<float>(p.y(), height - p.radius());
				p.vy() *= -1;
			}
		}
	}
};