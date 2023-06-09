#pragma once


struct vec2 {
	float x, y;
};


enum class Field : uint8_t {
	VEL_X,
	VEL_Y,
	SMOKE
};

template<size_t NUM_CELLS_X, size_t NUM_CELLS_Y>
struct FluidGrid {
	float hor  [NUM_CELLS_Y + 1][NUM_CELLS_X + 1]{}; // horizontal velocities
	float vert [NUM_CELLS_Y + 1][NUM_CELLS_X + 1]{}; // vertical velocities
	float s    [NUM_CELLS_Y + 1][NUM_CELLS_X + 1]{}; // scaling (0 = wall, 1 = fluid)
	float smoke[NUM_CELLS_Y + 1][NUM_CELLS_X + 1]{};
	// float pressure[NUM_CELLS_Y][NUM_CELLS_X]{};

	inline FluidGrid() {
		for(size_t y = 0; y < NUM_CELLS_Y + 1; y++)
			for(size_t x = 0; x < NUM_CELLS_X + 1; x++)
				s[y][x] = 1.f;
	}

	inline void addVel(const float vHor, const float vVert) {
		for(size_t y = 1; y < NUM_CELLS_Y; y++)
			for(size_t x = 1; x < NUM_CELLS_X + 1; x++)
				hor[y][x] += vHor;
		for(size_t y = 1; y < NUM_CELLS_Y + 1; y++)
			for(size_t x = 1; x < NUM_CELLS_X; x++)
				vert[y][x] += vVert;
	}

	inline float divergence(const size_t x, const size_t y) const { // calculate divergence (outflow at specified cell)
		return hor[y][x + 1] - hor[y][x] // horizontal divergence
			+ vert[y + 1][x] - vert[y][x]; // vertical divergence
	}

	inline void solveDivergence(const size_t num_iter = 100, const float overRelaxation = 1.9f) { // change velocities so divergence becomes 0
		for(size_t i = 0; i < num_iter; i++) {
			for(size_t y = 1; y < NUM_CELLS_Y; y++) {
				for(size_t x = 1; x < NUM_CELLS_X; x++) {
					if(s[y][x] == 0.f) continue; // inside wall

					const float sx0 = s[y][x-1];
					const float sx1 = s[y][x+1];
					const float sy0 = s[y-1][x];
					const float sy1 = s[y+1][x];
					const float s = sx0 + sx1 + sy0 + sy1;

					if (s == 0.f) continue; // surrounded by walls

					const float currentDivergence = divergence(x, y);
					const float correctionFactor = -currentDivergence / s * overRelaxation; // TODO: adapt for walls (replace 4)

					hor[y][x  ] -= correctionFactor * sx0;
					hor[y][x+1] += correctionFactor * sx1;
					vert[y  ][x] -= correctionFactor * sy0;
					vert[y+1][x] += correctionFactor * sy1;
				}
			}
		}
	}

	inline void extrapolate() {
		for (size_t x = 0; x < NUM_CELLS_X; x++) {
			hor[0][x] = hor[1][x];
			hor[NUM_CELLS_Y][x] = hor[NUM_CELLS_Y - 1][x]; 
		}
		for (size_t y = 0; y < NUM_CELLS_Y; y++) {
			vert[y][0] = vert[y][1];
			vert[y][NUM_CELLS_X] = vert[y][NUM_CELLS_X - 1];
		}
	}

	inline float sample(const Field field, float x, float y) const {
		const float h = 1.f;
		const float h1 = 1.f / h;
		const float h2 = .5f * h;

		// x = std::max<float>(std::min<float>(x, NUM_CELLS_X + 1), 1);
		// y = std::max<float>(std::min<float>(y, NUM_CELLS_Y + 1), 1);
		x = std::clamp<float>(x, 1, NUM_CELLS_X - 2);
		y = std::clamp<float>(y, 1, NUM_CELLS_Y - 2);

		float dx = 0.f;
		float dy = 0.f;

		// const float (*f)[NUM_CELLS_Y + 1][NUM_CELLS_X + 1];
		const float (*f)[NUM_CELLS_X + 1];

		switch (field) {
			case Field::VEL_X: f = hor; dy = h2; break;
			case Field::VEL_Y: f = vert; dx = h2; break;
			case Field::SMOKE: f = smoke; dx = h2; dy = h2; break;
		}

		const size_t x0 = std::min<size_t>(std::floor((x-dx)*h1), NUM_CELLS_X-1);
		const float tx = ((x-dx) - x0*h) * h1;
		const size_t x1 = std::min<size_t>(x0 + 1, NUM_CELLS_X-1);
		
		const size_t y0 = std::min<size_t>(std::floor((y-dy)*h1), NUM_CELLS_Y-1);
		const float ty = ((y-dy) - y0*h) * h1;
		const size_t y1 = std::min<size_t>(y0 + 1, NUM_CELLS_Y-1);

		const float sx = 1.0 - tx;
		const float sy = 1.0 - ty;

		const float val =
			sx*sy * f[y0][x0] +
			tx*sy * f[y0][x1] +
			tx*ty * f[y1][x1] +
			sx*ty * f[y1][x0];
		
		return val;
	}

	inline float avgVelX(const size_t x, const size_t y) const {
		return (hor[y][x] + hor[y][x + 1]
			+ hor[y + 1][x] + hor[y + 1][x + 1]) * .25f;
	}

	inline float avgVelY(const size_t x, const size_t y) const {
		return (vert[y][x] + vert[y + 1][x]
			+ vert[y][x + 1] + vert[y + 1][x + 1]) * .25f;
	}

	static inline void update(FluidGrid& prev, FluidGrid& next, const float dt) {
		for(size_t y = 1; y < NUM_CELLS_Y; y++) {
			for(size_t x = 1; x < NUM_CELLS_X; x++) {
				// update horizontal velocities:
				{
					float px = x;
					float py = y + .5f;
					const float velX = prev.hor[y][x];
					const float velY = prev.avgVelY(x, y);
					px -= dt*velX;
					py -= dt*velY;
					const float newVelX = prev.sample(Field::VEL_X, px, py);
					next.hor[y][x] = newVelX;
				}

				// update vertical velocities:
				{
					float px = x + .5f;
					float py = y;
					const float velX = prev.avgVelX(x, y);
					const float velY = prev.vert[y][x];
					px -= dt*velX;
					py -= dt*velY;
					const float newVelY = prev.sample(Field::VEL_Y, px, py);
					next.vert[y][x] = newVelY;
				}
			}
		}

		// prev = next;
		memcpy(prev.hor, next.hor, sizeof(float) * (NUM_CELLS_X + 1) * (NUM_CELLS_Y + 1));
		memcpy(prev.vert, next.vert, sizeof(float) * (NUM_CELLS_X + 1) * (NUM_CELLS_Y + 1));
	}

	static inline void updateSmoke(FluidGrid& prev, FluidGrid& next, const float dt) {
		for(size_t y = 1; y < NUM_CELLS_Y; y++) {
			for(size_t x = 1; x < NUM_CELLS_X; x++) {
				if(prev.s[y][x] == 0) continue;

				const float velX = (prev.hor[y][x] + prev.hor[y][x + 1]) * .5f;
				const float velY = (prev.vert[y][x] + prev.vert[y + 1][x]) * .5f;

				const float sourceX = x + .5f - velX * dt;
				const float sourceY = y + .5f - velY * dt;

				next.smoke[y][x] = prev.sample(Field::SMOKE, sourceX, sourceY);
			}
		}

		memcpy(prev.smoke, next.smoke, sizeof(float) * (NUM_CELLS_X + 1) * (NUM_CELLS_Y + 1));
	}
};