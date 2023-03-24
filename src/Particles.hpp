#pragma once


#include <algorithm>
#include <unordered_map>
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
		for(size_t i = 0; i < NUM_PARTICLES; i++) {
			particles.push_back(
				Particle(
					1 + rand() * 1.f * (width - 2) / RAND_MAX,
					1 + rand() * 1.f * (height - 2) / RAND_MAX,
					1.f,
					.5f
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
				// p.vx() *= -1;
				p.vx() *= 0;
			}

			if(p.y() - p.radius() < 0 || p.y() + p.radius() > height) {
				p.y() = std::max<float>(p.y(), 0 + p.radius());
				p.y() = std::min<float>(p.y(), height - p.radius());
				// p.vy() *= -1;
				p.vy() *= 0;
			}
		}

		// std::unordered_map<uint64_t, std::vector<Particle*>> map;
		// const auto ind = [this](const bmath::vec2& pos)
		// 	-> uint64_t {
		// 		return 
		// 			((int64_t)pos[0] << 32) |
		// 			((int64_t)pos[1]);
		// 	};

		// for(Particle& p : particles)
		// 	map[ind(p.pos)].push_back(&p);

		for(size_t i = 0; i < particles.size(); i++) {
			Particle& a = particles[i];

			std::vector<Particle*> bpList;

			// for(size_t y = -2; y <= 2; y++) {
			// 	for(size_t x = -2; x <= 2; x++) {
			// 		const bmath::vec2 off(x, y);

			// 		const uint64_t index = ind(a.pos + off);

			// 		if(map.find(index) == map.end())
			// 			continue;

			// 		std::vector<Particle*>& pList = map[index];
			// 		for(Particle *bp : pList)
			// 		// for(Particle *bp : map[index])
			// 			if(bp != &a)
			// 				bpList.push_back(bp);

			// 		pList.clear();
			// 	}
			// }
			
			// for(Particle* bp : bpList) {
			// 	Particle& b = *bp;
	
			for(size_t j = i + 1; j < particles.size(); j++) {
				Particle& b = particles[j];

				// prevent division 0/0 (And thereby NaN epidemic):
				if(a.pos[0] == b.pos[0] && a.pos[1] == b.pos[1])
					continue;

				if((b.pos - a.pos).mag() < a.radius() + b.radius()) {
					bmath::vec2 ab = b.pos - a.pos;
					ab /= ab.mag();

					bmath::vec2 ba = a.pos - b.pos;
					ba /= ba.mag();

					const bmath::vec2 normA = ab * ab.dot(a.vel);
					const bmath::vec2 tangA = a.vel - normA;
					a.vel = -normA + tangA;

					const bmath::vec2 normB = ba * ba.dot(b.vel);
					const bmath::vec2 tangB = b.vel - normB;
					b.vel = -normB + tangB;


					// push apart:
					float delta = (a.radius() + b.radius()) - (b.pos - a.pos).mag();
					a.pos -= ab * delta;
					b.pos -= ba * delta;
				}
			}

			// for(size_t i = 0; i < map[ind(a.pos)].size(); i++) {
			// 	if(map[ind(a.pos)][i] == &a) {
			// 		map[ind(a.pos)].erase(map[ind(a.pos)].begin() + i);
			// 		break;
			// 	}
			// }

			// for(Particle* p : bpList)
			// 	map[ind(p->pos)].push_back(p);
		}
	}
};