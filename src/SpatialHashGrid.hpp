#pragma once

#include "Particle.hpp"


#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility> // std::pair
#include <vector>


class SpatialHashGrid {
private:
	const size_t width;
	const size_t height;
	const size_t cellSize;
	const size_t tableSize;

	std::vector<size_t> numCellParticles;
	std::vector<size_t> cellStart;
	std::vector<size_t> cellEntries;

public:
	inline SpatialHashGrid(const size_t width, const size_t height, const size_t maxNumObjects, const size_t cellSize):
			width(width),
			height(height),
			cellSize(cellSize),
			// tableSize(maxNumObjects * 2),
			// tableSize(maxNumObjects),
			tableSize(width * height),

			numCellParticles(tableSize + 1),
			cellStart(tableSize + 1),
			cellEntries(maxNumObjects) {
		// static std::vector<size_t> numCellParticles(tableSize);
		// static std::vector<size_t> cellStart(tableSize + 1);
		// static std::vector<size_t> cellEntries(maxNumObjects);
	}

	inline void clear() {
		memset(cellStart.data(), 0, cellStart.size() * sizeof(size_t));
		memset(cellEntries.data(), 0, cellEntries.size() * sizeof(size_t));
	}

	inline size_t hash(const size_t xi, const size_t yi) {
		return yi * width + xi;
	}

	inline size_t hash(const bmath::vec2& pos) {
		return hash(
			std::floorf(pos[0]),
			std::floorf(pos[1])
		);
	}

	inline void addAll(const std::vector<Particle>& particles) {

		// count particles per cell:
		// std::vector<size_t> numCellParticles(tableSize + 1);
		// std::vector<size_t> numCellParticles(width * height);
		memset(numCellParticles.data(), 0, tableSize * sizeof(size_t));

		for (const Particle& particle : particles) {
			const size_t cell = hash(particle.pos);
			// const auto nop = [](auto a){a = a;};
			// nop(cell);
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
	}

	// inline std::vector<std::tuple<Particle*, Particle*>> queryCollisions(const size_t ai, std::vector<Particle>& particles) {
	// 	std::vector<std::tuple<Particle*, Particle*>> collisionPairs;
	inline std::vector<std::pair<Particle*, Particle*>> queryCollisions(const size_t ai, std::vector<Particle>& particles) {
		std::vector<std::pair<Particle*, Particle*>> collisionPairs;

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
					const size_t h = hash(x, y);
					// const size_t h = hash(bmath::vec2(x, y));
					const size_t start = cellStart[h];
					const size_t end = cellStart[h + 1];

					// if(start >= particles.size())
					// 	continue;
					for (size_t bii = start; bii < end; bii++) {
						const size_t bi = cellEntries[bii];

						// if(bi == ai) continue;
						// if(particles.data()+bi == &a) continue;
						// if(collided.find(particles.data() + bi) != collided.end()) continue;

						Particle& b = particles[bi];

						// if((b.pos - a.pos).mag() < a.radius() + b.radius()) {
							collisionPairs.push_back({
								particles.data() + ai,
								particles.data() + bi
							});
						// }
					}
				}
			}

			// collided.insert(particles.data() + ai);
		// }

		
		return collisionPairs;
	}
};