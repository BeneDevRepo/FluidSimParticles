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

	int32_t pmouseX = 0, pmouseY = 0;

	const auto drawDir = [&win](const vec2& pos, const vec2& dir, const float size){
			win.graphics.line((int)pos.x, (int)pos.y, (int)(pos.x + dir.x * size), (int)(pos.y + dir.y * size), 0xFFFFFFFF);
			win.graphics.setPixel((int)pos.x, (int)pos.y, 0xFF0000FF);
		};

	static constexpr size_t CELLS_X = 100;
	static constexpr size_t CELLS_Y = 100;
	// static constexpr size_t CELLS_X = 20;
	// static constexpr size_t CELLS_Y = 20;

	// TODO: no memory leak :)
	FluidGrid<CELLS_X, CELLS_Y>* vCurrent = new FluidGrid<CELLS_X, CELLS_Y>;
	FluidGrid<CELLS_X, CELLS_Y>* vNext = new FluidGrid<CELLS_X, CELLS_Y>;
	// float (*density)[CELLS_X] = new float[CELLS_Y][CELLS_X]{}; // density at the center of each cell


	Particles particles(CELLS_X, CELLS_Y, 2000);
	// Particles particles(CELLS_X, CELLS_Y, 100);

	enum class CellType : uint8_t {
		SOLID,
		FLUID,
		AIR
	};

	auto cellTypes = new CellType[CELLS_Y][CELLS_X];



	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);


	LARGE_INTEGER lastTime;
	QueryPerformanceCounter(&lastTime);


	

	// float density = 1000.f;
	while(!win.shouldClose()) {
		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);

		double dt = (currentTime.QuadPart - lastTime.QuadPart) * 1. / freq.QuadPart;
		// dt = std::min<double>(dt, .03);
		lastTime = currentTime;

		std::cout << "dt: " << dt << "s\n";




		// --- <Setup static Walls>
		for(size_t y = 0; y < CELLS_Y; y++) {
			for(size_t x = 0; x < CELLS_X; x++) {
				vCurrent->s[y][x] = 1.f;
			}
		}

		for(size_t x = 0; x < CELLS_X + 1; x++) {
			vCurrent->s[0][x] = 0;
			vCurrent->s[CELLS_Y][x] = 0;
		}
		for(size_t y = 0; y < CELLS_Y + 1; y++) {
			vCurrent->s[y][0] = 0;
			// vCurrent->s[y][CELLS_Y] = 0;
		}

		for(size_t y = 0; y < CELLS_Y / 8; y++) {
			for(size_t x = 0; x < CELLS_X / 8; x++) {
				vCurrent->s[CELLS_Y / 3 * 1 + y][CELLS_X / 5 + x] = 0;
				vCurrent->s[CELLS_Y / 3 * 2 - y][CELLS_X / 5 + x] = 0;
			}
		}
		// --- </Setup static Walls>



		// --- <Update Particles>
		particles.applyForce(0, 9.81f, dt);

		particles.updatePos(dt);

		// for(size_t i = 0; i < 10; i++)
			particles.collide();

		// constexpr size_t PARTICLE_ITERS = 10;
		// for(size_t i = 0; i < PARTICLE_ITERS; i++) {
		// 	particles.updatePos(dt / PARTICLE_ITERS);

		// 	particles.collide();
		// }
		// --- </Update Particles>



		// --- <Velocity Transfer>

		// Reset Velocity:
		for(size_t y = 0; y < CELLS_Y; y++) {
			for(size_t x = 0; x < CELLS_X; x++) {
				vCurrent->hor[y][x] = 0;
				vCurrent->vert[y][x] = 0;
			}
		}

		// Set Cell types:
		for(size_t y = 0; y < CELLS_Y; y++) {
			for(size_t x = 0; x < CELLS_X; x++) {
				cellTypes[y][x] = vCurrent->s[y][x] == 0 ? CellType::SOLID : CellType::AIR;
			}
		}

		for(const Particle p : particles.particles) {
			// size_t x = std::clamp<size_t>((int)p.pos[0], 1, CELLS_X - 2);
			// size_t y = std::clamp<size_t>((int)p.pos[1], 1, CELLS_Y - 2);
			size_t x = std::clamp<size_t>((int)p.pos[0], 0, CELLS_X - 1);
			size_t y = std::clamp<size_t>((int)p.pos[1], 0, CELLS_Y - 1);
			// size_t x = (size_t)p.pos[0];
			// size_t y = (size_t)p.pos[1];
			if(cellTypes[y][x] != CellType::SOLID)
				cellTypes[y][x] = CellType::FLUID;
		}

		// auto num = new size_t[CELLS_Y][CELLS_X]{};
		// for(const Particle p : particles.particles) {
		// 	size_t x = std::clamp<size_t>((int)p.pos[0], 0, CELLS_X - 1);
		// 	size_t y = std::clamp<size_t>((int)p.pos[1], 0, CELLS_Y - 1);

		// 	num[y][x]++;
		// }


		for(const Particle p : particles.particles) {
			// size_t xBase = std::clamp<int>((int)p.pos[0], 1, CELLS_X - 2);
			// size_t yBase = std::clamp<int>((int)p.pos[1], 1, CELLS_Y - 2);
			// size_t x = std::clamp<size_t>((int)p.pos[0], 2, CELLS_X - 3);
			// size_t y = std::clamp<size_t>((int)p.pos[1], 2, CELLS_Y - 3);

			// float xRem = p.pos[0] - xBase;
			// float yRem = p.pos[1] - yBase;

			// horizontal velocity:
			// vCurrent->hor[yBase][xBase    ] += p.vel[0] * .5f;
			// vCurrent->hor[yBase][xBase + 1] += p.vel[0] * .5f;
			{
				const size_t xBase = std::clamp<int>((int)p.pos[0], 1, CELLS_X - 2);
				const size_t yBase = std::clamp<int>((int)p.pos[1] - .5f, 1, CELLS_Y - 2);

				const float xRem = p.pos[0] - xBase;
				const float yRem = std::max<float>((p.pos[1] - .5f) - yBase, 0.f);

				vCurrent->hor[yBase    ][xBase    ] += p.vel[0] * (1.f - xRem) * (1.f - yRem);
				vCurrent->hor[yBase    ][xBase + 1] += p.vel[0] * (      xRem) * (1.f - yRem);
				vCurrent->hor[yBase + 1][xBase    ] += p.vel[0] * (1.f - xRem) * (      yRem);
				vCurrent->hor[yBase + 1][xBase + 1] += p.vel[0] * (      xRem) * (      yRem);
			}


			// vertical velocity:
			// vCurrent->vert[yBase    ][xBase] += p.vel[1] * .5f;
			// vCurrent->vert[yBase + 1][xBase] += p.vel[1] * .5f;
			{
				const size_t xBase = std::clamp<int>((int)p.pos[0] - .5f, 1, CELLS_X - 2);
				const size_t yBase = std::clamp<int>((int)p.pos[1], 1, CELLS_Y - 2);

				const float xRem = std::max<float>((p.pos[0] - .5f) - xBase, 0.f);
				const float yRem = p.pos[1] - yBase;

				vCurrent->vert[yBase    ][xBase    ] += p.vel[1] * (1.f - xRem) * (1.f - yRem);
				vCurrent->vert[yBase    ][xBase + 1] += p.vel[1] * (      xRem) * (1.f - yRem);
				vCurrent->vert[yBase + 1][xBase    ] += p.vel[1] * (1.f - xRem) * (      yRem);
				vCurrent->vert[yBase + 1][xBase + 1] += p.vel[1] * (      xRem) * (      yRem);
			}
		}

		// for(size_t y = 0; y < CELLS_Y; y++) {
		// 	for(size_t x = 0; x < CELLS_X; x++) {
		// 		vCurrent->hor[y][x] /= num[y][x];
		// 		vCurrent->vert[y][x] /= num[y][x];
		// 	}
		// }
		// --- </Velocity Transfer>




		const size_t NUM_SMOKE_TRAILS = 10;
		for(size_t y = CELLS_Y / NUM_SMOKE_TRAILS / 2; y < CELLS_Y; y += CELLS_Y / NUM_SMOKE_TRAILS)
			vCurrent->smoke[y][0] = 3.f;

		// for(size_t y = 1; y < CELLS_Y - 3; y++)
		// 	vCurrent->hor[y][1] = 30;
		// for(size_t y = 1; y < CELLS_Y - 2; y++)
		// 	vCurrent->vert[y][1] = 0;

		// vCurrent->hor[54][5] = 50;

		// vCurrent->addVel(0, 9.81f);

		for(size_t y = 0; y < CELLS_Y; y++) {
			for(size_t x = 0; x < CELLS_X; x++) {
				vNext->hor[y][x] = vCurrent->hor[y][x];
				vNext->vert[y][x] = vCurrent->vert[y][x];
			}
		}

		vCurrent->solveDivergence(10, 1.9f);

		// const size_t iter = 1;
		// for(size_t i = 0; i < iter; i++)
		// 	FluidGrid<CELLS_X, CELLS_Y>::update(*vCurrent, *vNext, dt / iter);

		vCurrent->extrapolate(); // set border cells equal to neighboring cells

		// FluidGrid<CELLS_X, CELLS_Y>::update(*vCurrent, *vNext, dt);

		FluidGrid<CELLS_X, CELLS_Y>::updateSmoke(*vCurrent, *vNext, dt);

		// --- <Mouse Interaction>
		if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
			bmath::vec2 mouseProj(
				win.win.mouseX * 1.f * CELLS_X / win.win.width,
				win.win.mouseY * 1.f * CELLS_Y / win.win.height);

			const float radProj = 50.f * CELLS_X / win.win.width;
			for(Particle& particle : particles.particles) {
				if((particle.pos - mouseProj).mag() > radProj)
					continue;
				
				particle.vel = bmath::vec2(
					(win.win.mouseX - pmouseX) * 1.f * CELLS_X / win.win.width / dt,
					(win.win.mouseY - pmouseY) * 1.f * CELLS_Y / win.win.height / dt
				);
			}
			for(size_t y = 0; y < CELLS_Y; y++) {
				for(size_t x = 0; x < CELLS_X; x++) {
					// vCurrent->s[y][x] = 1.f;

					if((bmath::vec2(x, y) - mouseProj).mag() > radProj)
						continue;
					
					vCurrent->hor[y][x] += (win.win.mouseX - pmouseX) * 1.f * CELLS_X / win.win.width / dt * .5f;
					vCurrent->vert[y][x] += (win.win.mouseY - pmouseY) * 1.f * CELLS_Y / win.win.height / dt * .5f;
					vCurrent->s[y][x] = 0.f;
				}
			}
		}
		// --- </Mouse Interaction>

		// --- <Transfer velicities back to particles>
		for(Particle& particle : particles.particles) {
			// var nr0 = x0*n + y0;
			// var nr1 = x1*n + y0;
			// var nr2 = x1*n + y1;
			// var nr3 = x0*n + y1;
			// bool valid0 = cellTypes[ ] != CellType::AIR /*|| this.cellType[nr0 - offset] != AIR_CELL*/ ? 1.0 : 0.0;
			// bool valid1 = cellTypes[nr1] != CellType::AIR /*|| this.cellType[nr1 - offset] != AIR_CELL*/ ? 1.0 : 0.0;
			// bool valid2 = cellTypes[nr2] != CellType::AIR /*|| this.cellType[nr2 - offset] != AIR_CELL*/ ? 1.0 : 0.0;
			// bool valid3 = cellTypes[nr3] != CellType::AIR /*|| this.cellType[nr3 - offset] != AIR_CELL*/ ? 1.0 : 0.0;
			
			const float picX = vCurrent->sample(Field::VEL_X, particle.pos[0], particle.pos[1]);
			const float picY = vCurrent->sample(Field::VEL_Y, particle.pos[0], particle.pos[1]);

			const float flipX = particle.vel[0] + picX - vNext->sample(Field::VEL_X, particle.pos[0], particle.pos[1]);
			const float flipY = particle.vel[1] + picY - vNext->sample(Field::VEL_Y, particle.pos[0], particle.pos[1]);

			// particle.vel[0] = picX;
			// particle.vel[1] = picX;
			// particle.vel[0] = flipX;
			// particle.vel[1] = flipY;
			particle.vel[0] = picX * .1f + flipX * .9f;
			particle.vel[1] = picY * .1f + flipY * .9f;
		}
		// --- <Transfer velicities back to particles/>


		// --- graphics:
		win.graphics.clear(0xFF000000);
		if(false)
		for(size_t y = 0; y < win.height; y++) {
			for(size_t x = 0; x < win.width; x++) {
				const size_t cellX = x * (CELLS_X + 1) / win.width;
				const size_t cellY = y * (CELLS_Y + 1) / win.height;

				// const size_t cellX = x * CELLS_X / win.width;
				// const size_t cellY = y * CELLS_Y / win.height;

				// static constexpr auto clamp = [](float v, float min, float max){
				// 		return (v < min) ? min : (v > max) ? max : v;
				// 	};

				const float wall = vCurrent->s[cellY][cellX];

				// const float smoke = 1.f - clamp(vCurrent->sample(Field::SMOKE, cellX, cellY), 0.f, 1.f);
				const float smoke = 1.f - std::clamp<float>(vCurrent->sample(Field::SMOKE, cellX, cellY), 0.f, 1.f);

				// const uint8_t vX = smoke * wall * (clamp(vCurrent->sample(Field::VEL_X, cellX, cellY) * .01f, -.5f, .5f) * 255 + 128);
				// const uint8_t vY = smoke * wall * (clamp(vCurrent->sample(Field::VEL_Y, cellX, cellY) * .01f, -.5f, .5f) * 255 + 128);
				const uint8_t vX = smoke * wall * (std::clamp<float>(vCurrent->sample(Field::VEL_X, cellX, cellY) * .01f, -.5f, .5f) * 255 + 128);
				const uint8_t vY = smoke * wall * (std::clamp<float>(vCurrent->sample(Field::VEL_Y, cellX, cellY) * .01f, -.5f, .5f) * 255 + 128);

				win.graphics.setPixel(x, y,
					0xFF << 24
					| vX << 16
					| vY << 8
				);
			}
		}


		// -----  Color based on cell type:
		if(GetAsyncKeyState('C') & 0x8000) {
			for(size_t y = 0; y < win.height; y++) {
				for(size_t x = 0; x < win.width; x++) {
					const size_t cellX = x * (CELLS_X + 1) / win.width;
					const size_t cellY = y * (CELLS_Y + 1) / win.height;
					// const size_t cellX = x * CELLS_X / win.width;
					// const size_t cellY = y * CELLS_Y / win.height;
					CellType type = cellTypes[cellY][cellX];
					uint32_t color = 0;
					switch(type) {
						case CellType::AIR:
							color = 0xFF00FF00;
							break;
						case CellType::FLUID:
							color = 0xFF0000FF;
							break;
						case CellType::SOLID:
							color = 0xFF000000;
							break;
					}
					win.graphics.setPixel(x, y, color);
				}
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
				(int)(p.x() * win.graphics.width / CELLS_X),
				(int)(p.y() * win.graphics.height / CELLS_Y),
				(int)(p.radius() * win.graphics.width / CELLS_X),
				0xFFFF0000);

		win.updateScreen();

		pmouseX = win.win.mouseX;
		pmouseY = win.win.mouseY;

		win.pollMsg();

		// while(GetAsyncKeyState(' ') & 0x8000);

		// Sleep(20);
	}

	return 0;
}