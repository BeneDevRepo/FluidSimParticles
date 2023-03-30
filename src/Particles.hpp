#pragma once


#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Particle.hpp"

#include "SpatialHashGrid.hpp"


class Particles {
public:
	std::vector<Particle> particles;

private:
	static constexpr float RADIUS = .5f;
	static constexpr float CELL_SIZE = RADIUS * 2;
	const size_t width;
	const size_t height;
	const size_t tableSize;

public:
	Particles(const size_t width, const size_t height, const size_t NUM_PARTICLES):
			width(width),
			height(height),
			tableSize(2 * NUM_PARTICLES) {
			// tableSize(NUM_PARTICLES) {

		for(size_t i = 0; i < NUM_PARTICLES; i++) {
			particles.push_back(
				Particle(
					1 + rand() * 1.f * (width - 2) / RAND_MAX,
					1 + rand() * 1.f * (height - 2) / RAND_MAX,
					1.f,
					RADIUS
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

	inline size_t hash_i(const size_t xi, const size_t yi, const size_t zi = 0) {
		// const size_t h = (xi * 92837111) ^ (yi * 689287499) ^ (zi * 283923481);	// fantasy function
		// return abs((int)h) % tableSize;
		const size_t h = yi * width + xi;	// fantasy function
		return h % tableSize;
	}

	inline size_t hash(const bmath::vec2& pos) {
		// const float spacing = CELL_SIZE;
		const float spacing = 1;
		return hash_i(
			// (int)std::floorf(pos[0] / spacing),
			// (int)std::floorf(pos[1] / spacing),
			// (size_t)std::floorf(std::max<float>(pos[0], 0) / spacing),
			// (size_t)std::floorf(std::max<float>(pos[1], 0) / spacing),
			// (size_t)(std::max<float>(pos[0], 0) * spacing),
			// (size_t)(std::max<float>(pos[1], 0) * spacing),
			(size_t)(std::max<float>(pos[0], 0) / spacing),
			(size_t)(std::max<float>(pos[1], 0) / spacing),
			0
		);
	};

	inline std::vector<std::tuple<Particle*, Particle*>> getCollisionsPrimitive() {
		std::vector<std::tuple<Particle*, Particle*>> collisionPairs;

		for(size_t i = 0; i < particles.size(); i++) {
			Particle& a = particles[i];

	
			for(size_t j = i + 1; j < particles.size(); j++) {
				Particle& b = particles[j];

				if((b.pos - a.pos).mag() < a.radius() + b.radius())
					collisionPairs.push_back({&a, &b});
			}
		}

		return collisionPairs;
	}

	inline std::vector<std::tuple<Particle*, Particle*>> getCollisionsUnorderedMap() {
		std::vector<std::tuple<Particle*, Particle*>> collisionPairs;

		std::unordered_map<uint64_t, std::vector<Particle*>> map;
		// std::unordered_map<uint64_t, Particle*> map;

		const auto ind = [](const bmath::vec2& pos)
			-> uint64_t {
				// static constexpr float GRID_SCALE = .5f * 2;
				return 
					((int64_t)(pos[0] * CELL_SIZE) << 32) |
					((int64_t)(pos[1] * CELL_SIZE));
			};

		constexpr int max_dist = 1;
		for(Particle& p : particles)
			for(int32_t dy = -max_dist; dy <= max_dist; dy++)
				for(int32_t dx = -max_dist; dx <= max_dist; dx++)
					map[ind(p.pos + bmath::vec2(dx, dy))].push_back(&p);
					// map.insert({ind(p.pos + bmath::vec2(dx, dy)), &p});

		std::unordered_set<Particle*> collided;

		for(size_t i = 0; i < particles.size(); i++) {
			Particle& a = particles[i];

			const uint64_t index = ind(a.pos);

			for(Particle* bp : map.at(index)) {

			// auto range = map.equal_range(index);
			// for(auto it = range.first; it != range.second; ++it) {
			// 	Particle* bp = it->second;

				if(collided.find(bp) != collided.end())
					continue;

				Particle& b = *bp;
				// if((b.pos - a.pos).mag() < a.radius() + b.radius()) {
					collisionPairs.push_back({&a, &b});
				// }
			}

			collided.insert(particles.data() + i);
		}
		
		return collisionPairs;
	}




	inline std::vector<std::tuple<Particle*, Particle*>> getCollisionsDenseHash() {
		std::vector<std::tuple<Particle*, Particle*>> collisionPairs;

		// static const size_t maxNumObjects = particles.size();
		static const size_t maxNumObjects = particles.size() * 2;

		// static const float spacing = CELL_SIZE;
		
		// static size_t* numCellParticles = new size_t[tableSize];
		// static size_t* cellStart = new size_t[tableSize + 1];
		// static size_t* cellEntries = new size_t[maxNumObjects];
		static std::vector<size_t> numCellParticles(tableSize);
		static std::vector<size_t> cellStart(tableSize + 1);
		static std::vector<size_t> cellEntries(maxNumObjects);
		// size_t* queryIds = new size_t[maxNumObjects];
		// size_t querySize = 0;


		memset(cellStart.data(), 0, (tableSize + 1) * sizeof(size_t));
		memset(cellEntries.data(), 0, maxNumObjects * sizeof(size_t));



		// count particles per cell:
		memset(numCellParticles.data(), 0, tableSize * sizeof(size_t));
		for (const Particle& particle : particles) {
			const size_t cell = hash(particle.pos);
			numCellParticles[cell]++;
		}

		// determine cells starts (partial sums):
		{
			size_t start = 0;
			for (size_t i = 0; i < tableSize; i++) {
				start += numCellParticles[i];
				cellStart[i] = start;
			}
			cellStart[tableSize] = start;	// guard
		}

		// fill in objects ids:
		for (size_t i = 0; i < particles.size(); i++) {
			const size_t h = hash(particles[i].pos);
			cellStart[h]--;
			cellEntries[cellStart[h]] = i;
		}

		// --- </dense hash grid>

		// std::unordered_set<Particle*> collided;

		for(size_t ai = 0; ai < particles.size(); ai++) {
			Particle& a = particles[ai];

			const float maxDist = 1;

			const float xf = std::floorf(a.pos[0]);
			const float yf = std::floorf(a.pos[1]);

			const float x0 = std::max<float>(xf - maxDist, 0);
			const float y0 = std::max<float>(yf - maxDist, 0);

			const float x1 = std::min<float>(xf + maxDist, width - 1);
			const float y1 = std::min<float>(yf + maxDist, height - 1);

			for(float y = y0; y <= y1; y++) {
				for(float x = x0; x <= x1; x++) {
					const size_t h = hash_i(x, y, 0);
					// const size_t h = hash(bmath::vec2(x, y));
					const size_t start = cellStart[h];
					const size_t end = cellStart[h + 1];

					// if(start >= particles.size())
					// 	continue;
					for (size_t bi = start; bi < end; bi++) {
						// if(bi == ai) continue;
						// if(cellEntries[bi] == ai) continue;
						// if(collided.find(particles.data() + bi) != collided.end()) continue;

						// Particle& b = particles[bi];
						Particle& b = particles[cellEntries[bi]];

						// if((b.pos - a.pos).mag() < a.radius() + b.radius()) {
							collisionPairs.push_back({
								particles.data() + ai,
								// particles.data() + bi
								particles.data() + cellEntries[bi]
							});
						// }
					}
				}
			}

			// collided.insert(particles.data() + ai);
		}

		
		return collisionPairs;
	}

	inline void collide() {
		for(Particle& p : particles) {
			if(p.x() - p.radius() < 0 || p.x() + p.radius() > width) {
				p.x() = std::max<float>(p.x(), 0 + p.radius());
				p.x() = std::min<float>(p.x(), width - p.radius());
				p.vx() *= -1;
				// p.vx() *= 0;
			}

			if(p.y() - p.radius() < 0 || p.y() + p.radius() > height) {
				p.y() = std::max<float>(p.y(), 0 + p.radius());
				p.y() = std::min<float>(p.y(), height - p.radius());
				p.vy() *= -1;
				// p.vy() *= 0;
			}
		}

		// std::vector<std::tuple<Particle*, Particle*>> collisionPairs = getCollisionsPrimitive();
		// std::vector<std::tuple<Particle*, Particle*>> collisionPairs = getCollisionsUnorderedMap();
		std::vector<std::tuple<Particle*, Particle*>> collisionPairs = getCollisionsDenseHash();

		// std::cout << "Num pairs collided: " << collisionPairs.size() << "\n";

		for(auto [ap, bp] : collisionPairs) {
			Particle& a = *ap;
			Particle& b = *bp;

			// prevent division 0/0 (And thereby NaN epidemic):
			if(a.pos[0] == b.pos[0] && a.pos[1] == b.pos[1])
				continue;

			const float dist = (b.pos - a.pos).mag();

			if(dist > a.radius() + b.radius())
				continue;

			const bmath::vec2 ab = (b.pos - a.pos) / dist;
			const bmath::vec2 ba = (a.pos - b.pos) / dist;

			const bmath::vec2 normA = ab * ab.dot(a.vel);
			const bmath::vec2 tangA = a.vel - normA;
			a.vel = -normA + tangA;

			const bmath::vec2 normB = ba * ba.dot(b.vel);
			const bmath::vec2 tangB = b.vel - normB;
			b.vel = -normB + tangB;


			// push apart:
			const float delta = (a.radius() + b.radius()) - dist;
			a.pos -= ab * delta;
			b.pos -= ba * delta;
		}
	}
};