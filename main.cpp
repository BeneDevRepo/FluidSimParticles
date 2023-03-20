#include <algorithm>
#include <iostream>
#include <cstdint>
#include <cmath>
// #include <thread>
// #include <mutex>

#include "BWindow/GDIWindow.h"

#include "windows.h"


#include "Fluid.hpp"


#include "Particle.hpp"
#include <vector>

#include "Particles.hpp"


int main() {
	GDIWindow win(800, 800);

	const auto drawDir = [&win](const vec2& pos, const vec2& dir, const float size){
			win.graphics.line((int)pos.x, (int)pos.y, (int)(pos.x + dir.x * size), (int)(pos.y + dir.y * size), 0xFFFFFFFF);
			win.graphics.setPixel((int)pos.x, (int)pos.y, 0xFF0000FF);
		};

	static constexpr size_t CELLS_X = 400;
	static constexpr size_t CELLS_Y = 400;

	// TODO: no memory leak :)
	FluidGrid<CELLS_X, CELLS_Y>* vCurrent = new FluidGrid<CELLS_X, CELLS_Y>;
	FluidGrid<CELLS_X, CELLS_Y>* vNext = new FluidGrid<CELLS_X, CELLS_Y>;

	Particles particles(CELLS_X, CELLS_Y, 200, 10);

	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq); // TODO


	LARGE_INTEGER lastTime;
	QueryPerformanceCounter(&lastTime);


	for(size_t x = 0; x < CELLS_X + 1; x++) {
		vCurrent->s[0][x] = 0;
		vCurrent->s[CELLS_Y][x] = 0;
	}
	for(size_t y = 0; y < CELLS_Y + 1; y++)
		vCurrent->s[y][0] = 0;

	for(size_t y = 0; y < CELLS_Y / 8; y++) {
		for(size_t x = 0; x < CELLS_X / 8; x++) {
			vCurrent->s[CELLS_Y / 3 * 1 + y][CELLS_X / 5 + x] = 0;
			vCurrent->s[CELLS_Y / 3 * 2 - y][CELLS_X / 5 + x] = 0;
		}
	}

	// float density = 1000.f;
	while(!win.shouldClose()) {
		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);

		const double dt = (currentTime.QuadPart - lastTime.QuadPart) * 1. / freq.QuadPart;
		lastTime = currentTime;

		std::cout << "dt: " << dt << "s\n";

		// <Update Particles>
		// for(Particle& p : particles)
		// 	p.applyForce(0, 9.81f, dt);
		particles.applyForce(0, 9.81f, dt);

		// for(Particle& p : particles)
		// 	p.update(dt);
		particles.updatePos(dt);

		// for(Particle& p : particles) {
		// 	if(p.x() - p.radius() < 0 || p.x() + p.radius() > CELLS_X) {
		// 		p.x() = std::max<float>(p.x(), 0 + p.radius());
		// 		p.x() = std::min<float>(p.x(), CELLS_X - p.radius());
		// 		p.vx() *= -1;
		// 	}

		// 	if(p.y() - p.radius() < 0 || p.y() + p.radius() > CELLS_Y) {
		// 		p.y() = std::max<float>(p.y(), 0 + p.radius());
		// 		p.y() = std::min<float>(p.y(), CELLS_Y - p.radius());
		// 		p.vy() *= -1;
		// 	}
		// }
		particles.collide();
		// </Update Particles>


		// <Velocity Transfer>
		for(size_t y = 0; y < CELLS_Y; y++) {
			for(size_t x = 0; x < CELLS_X; x++) {
				vCurrent->hor[y][x] = 0;
				vCurrent->vert[y][x] = 0;
			}
		}

		auto num = new size_t[CELLS_Y][CELLS_X]{};
		for(const Particle p : particles.particles) {
			size_t x = std::clamp<size_t>((int)p.pos[0], 0, CELLS_X - 1);
			size_t y = std::clamp<size_t>((int)p.pos[1], 0, CELLS_Y - 1);

			vCurrent->hor[y][x] += p.vel[0] * .5f;
			vCurrent->hor[y][x + 1] += p.vel[0] * .5f;

			vCurrent->vert[y][x] += p.vel[1] * .5f;
			vCurrent->vert[y + 1][x] += p.vel[1] * .5f;

			num[y][x]++;
		}

		// for(size_t y = 0; y < CELLS_Y; y++) {
		// 	for(size_t x = 0; x < CELLS_X; x++) {
		// 		vCurrent->hor[y][x] /= num[y][x];
		// 		vCurrent->vert[y][x] /= num[y][x];
		// 	}
		// }
		// </Velocity Transfer>


		const size_t NUM_SMOKE_TRAILS = 10;
		for(size_t y = CELLS_Y / NUM_SMOKE_TRAILS / 2; y < CELLS_Y; y += CELLS_Y / NUM_SMOKE_TRAILS)
			vCurrent->smoke[y][0] = 3.f;

		for(size_t y = 1; y < CELLS_Y - 3; y++)
			vCurrent->hor[y][1] = 30;
		// for(size_t y = 1; y < CELLS_Y - 2; y++)
		// 	vCurrent->vert[y][1] = 0;

		// vCurrent->hor[54][5] = 50;

		// vCurrent->addVel(0, 9.81f);

		vCurrent->solveDivergence(10, 1.9f);

		// const size_t iter = 1;
		// for(size_t i = 0; i < iter; i++)
		// 	FluidGrid<CELLS_X, CELLS_Y>::update(*vCurrent, *vNext, dt / iter);

		vCurrent->extrapolate();

		FluidGrid<CELLS_X, CELLS_Y>::update(*vCurrent, *vNext, dt);

		FluidGrid<CELLS_X, CELLS_Y>::updateSmoke(*vCurrent, *vNext, dt);


		// --- graphics:
		for(size_t y = 0; y < win.height; y++) {
			for(size_t x = 0; x < win.width; x++) {
				const size_t cellX = x * (CELLS_X + 1) / win.width;
				const size_t cellY = y * (CELLS_Y + 1) / win.height;

				static constexpr auto clamp = [](float v, float min, float max){
						return (v < min) ? min : (v > max) ? max : v;
					};

				const float wall = vCurrent->s[cellY][cellX];

				const float smoke = 1.f - clamp(vCurrent->sample(Field::SMOKE, cellX, cellY), 0.f, 1.f);

				const uint8_t vX = smoke * wall * (clamp(vCurrent->sample(Field::VEL_X, cellX, cellY) * .01f, -.5f, .5f) * 255 + 128);
				const uint8_t vY = smoke * wall * (clamp(vCurrent->sample(Field::VEL_Y, cellX, cellY) * .01f, -.5f, .5f) * 255 + 128);

				win.graphics.setPixel(x, y,
					0xFF << 24
					| vX << 16
					| vY << 8
				);
				// win.graphics.setPixel(x, y, (x % 100 < 50) ? 0xFF00FF00 : 0xFFFF0000);
			}
		}


		if(GetAsyncKeyState('L') & 0x8000) {
			for(size_t y = 0; y < CELLS_Y; y++) {
				for(size_t x = 0; x < CELLS_X; x++) {
					drawDir(
						vec2 {
							(x + .5f) * win.width / CELLS_X,
							(y + .5f) * win.height / CELLS_Y
						},
						vec2 {
							vCurrent->sample(Field::VEL_X, x, y),
							vCurrent->sample(Field::VEL_Y, x, y)
						},
						5.f);
				}
			}
	
			// for(size_t y = 0; y < CELLS_Y; y++) {
			// 	for(size_t x = 0; x < CELLS_X + 1; x++) {
			// 		const int startX = x * win.width / CELLS_X;
			// 		const int startY = (y + .5f) * win.height / CELLS_Y;
			// 		win.graphics.line(
			// 			startX,
			// 			startY,
			// 			startX + std::min<float>(vCurrent->hor[y][x] * 5.f, 15),
			// 			startY,
			// 			0xFFFFFFFF
			// 		);
			// 		win.graphics.fillCircle(startX, startY, 1, 0xFFFF00FF);
			// 	}
			// }
			// for(size_t y = 0; y < CELLS_Y + 1; y++) {
			// 	for(size_t x = 0; x < CELLS_X; x++) {
			// 		const int startX = (x + .5f) * win.width / CELLS_X;
			// 		const int startY = y * win.height / CELLS_Y;
			// 		win.graphics.line(
			// 			startX,
			// 			startY,
			// 			startX,
			// 			startY + std::min<float>(vCurrent->vert[y][x] * 5.f, 15),
			// 			0xFFFFFFFF
			// 		);
			// 		win.graphics.fillCircle(startX, startY, 1, 0xFFFF00FF);
			// 	}
			// }
		}

		for(const Particle p : particles.particles)
			win.graphics.fillCircle(
				(int)p.x() * win.graphics.width / CELLS_X,
				(int)p.y() * win.graphics.height / CELLS_Y,
				(int)p.radius() * win.graphics.width / CELLS_X,
				0xFFFF0000);

		win.updateScreen();

		win.pollMsg();
	}

	return 0;
}