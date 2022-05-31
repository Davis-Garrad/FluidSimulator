#include "MainScreen.hpp"

#include <stdio.h>
#include <random>
#include <time.h>

#include <SDL2/SDL.h>

MainScreen::MainScreen(unsigned int windowWidth, unsigned int windowHeight, BARE2D::InputManager* input) :
	m_windowW(windowWidth), m_windowH(windowHeight), m_input(input) {
}

MainScreen::~MainScreen() {
}

void MainScreen::initScreen() {
	// Initialize all our stuff
	// First, the renderer+shaders.
	std::string frag = "shader.frag", vert = "shader.vert";
	m_renderer = new BARE2D::BasicRenderer(frag, vert, m_windowW, m_windowH);
	m_renderer->init();

	// Allocate our texture, which should only pass one channel to OpenGL, the density of the cell.
	std::string texName = "fluidTexture";
	m_fluidTexture = BARE2D::ResourceManager::createMutableTexture(texName, CELLS_X, CELLS_Y, GL_NEAREST, 1, GL_RED);

	// Allocate space for the actual fluids.
	m_densities		 = new unsigned char[CELLS_X * CELLS_Y];
	m_directions	 = new vec[CELLS_X * CELLS_Y];
	m_swapDensities	 = new unsigned char[CELLS_X * CELLS_Y];
	m_swapDirections = new vec[CELLS_X * CELLS_Y];

	std::srand(time(NULL));

	for(unsigned int i = 0; i < CELLS_X * CELLS_Y; i++) {
		m_densities[i] = 0;
	}
}
void MainScreen::destroyScreen() {
}
void MainScreen::onEntry() {
}
void MainScreen::onExit() {
}
void MainScreen::draw() {
	// Clear all the colour, we don't use depth in this app.
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Start the rendering process
	m_renderer->begin();

	// Add the texture to the renderbatches
	m_renderer->draw(glm::vec4(0.0f, 0.0f, m_windowW, m_windowH),
					 glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
					 m_fluidTexture->id,
					 0.0f);

	// End the rendering process and draw to screen.
	m_renderer->end();
	m_renderer->render();
}
void MainScreen::update(double dt) {
	m_fluidTexture->setData(m_densities);

	diffuseFluid(dt);
	moveFluid(dt);

	updateInput();
}

void MainScreen::updateInput() {
	// Get mouse position
	glm::vec2 mousePos = m_input->getMousePosition();

	// Now get the position in the cells:
	unsigned int cellPosX = mousePos.x / m_windowW * CELLS_X;
	unsigned int cellPosY = mousePos.y / m_windowH * CELLS_Y;

	// get the last mouse position to compare too.
	unsigned int lastCellPosX = m_lastMouse.x / m_windowW * CELLS_X;
	unsigned int lastCellPosY = m_lastMouse.y / m_windowH * CELLS_Y;

	if(m_input->isKeyDown(SDL_BUTTON_LEFT)) {
		// "Draw" a line from the last position to this position, updating the directions field.
		double xDiff = (mousePos.x - m_lastMouse.x) / (double)m_windowW * (double)CELLS_X;
		double yDiff = (mousePos.y - m_lastMouse.y) / (double)m_windowH * (double)CELLS_Y;

		// Realistically we can expect the mouse to move no more than 25 pixels per frame.
		// That equates to CELLS_DIM/window_dim * 25
		// Since number of cells should never reasonably exceed number of pixels, we can just divide by 25.
		for(unsigned int i = 0; i < 25; i++) {
			// Step forward one CELLS_DIM/window_dim * mouseDiff/25
			unsigned int x = lastCellPosX + (double)i * xDiff / 25.0;
			unsigned int y = lastCellPosY + (double)i * yDiff / 25.0;

			// Set the velocity.
			m_directions[getTextureIndex(x, y)].x = xDiff;
			m_directions[getTextureIndex(x, y)].y = yDiff;
		}
	}

	if(m_input->isKeyDown(SDLK_SPACE)) {
		for(int i = -1; i < 2; i++) {
			for(int j = -1; j < 2; j++) {
				if(cellPosX + i < 0 || cellPosX + i >= CELLS_X)
					continue;
				if(cellPosY + j < 0 || cellPosY + j >= CELLS_Y)
					continue;
				m_densities[getTextureIndex(cellPosX + i, cellPosY + j)] = 255;
			}
		}
	}

	m_lastMouse = mousePos;
}

void MainScreen::diffuseFluid(double dt) {
	// Apply a Gauss-Seidel relaxation to avoid divergent diffusion.

	// First, diffuse.
	double a = CELLS_X * CELLS_Y * m_diffusionRate * dt;
	for(unsigned int i = 1; i < CELLS_X - 1; i++) {
		for(unsigned int j = 1; j < CELLS_Y - 1; j++) {
			unsigned int cur   = getTextureIndex(i, j);
			unsigned int left  = getTextureIndex(i - 1, j);
			unsigned int right = getTextureIndex(i + 1, j);
			unsigned int up	   = getTextureIndex(i, j - 1);
			unsigned int down  = getTextureIndex(i, j + 1);

			m_swapDensities[cur] = (m_densities[cur] + a * (m_densities[left] + m_densities[right] + m_densities[up] +
															m_densities[down])) /
								   (1 + 4 * a);
		}
	}
	setDensityBoundaries();

	// Complete the swap.
	swapDensities();
}

void MainScreen::moveFluid(double dt) {
	double dtX = CELLS_X * dt;
	double dtY = CELLS_Y * dt;

	for(unsigned int i = 1; i < CELLS_X - 1; i++) {
		for(unsigned int j = 1; j < CELLS_Y - 1; j++) {
			// For future reference
			vec dir = m_directions[getTextureIndex(i, j)];
			// Find where the cell would've been before the time.
			double x = i - dtX * dir.x;
			double y = j - dtY * dir.y;
			// Set boundaries
			if(x < 0.5)
				x = 0.5;
			if(x > 0.5 + CELLS_X)
				x = 0.5 + CELLS_X;
			if(y < 0.5)
				y = 0.5;
			if(y > 0.5 + CELLS_Y)
				y = 0.5 + CELLS_Y;

			// Get floored cell positions.
			unsigned int i0 = (unsigned int)x;
			unsigned int j0 = (unsigned int)y;
			unsigned int i1 = i0 + 1;
			unsigned int j1 = j0 + 1;

			// The decimal difference
			double s1 = x - i0;
			double s0 = 1.0 - s1;
			double t1 = y - j0;
			double t0 = 1.0 - t1;

			// Now assign some stuff to the cell based on surroundings.
			m_swapDensities[getTextureIndex(i, j)] =
				s0 * (t0 * m_densities[getTextureIndex(i0, j0)] + t1 * m_densities[getTextureIndex(i0, j1)]) +
				s1 * (t0 * m_densities[getTextureIndex(i1, j0)] + t1 * m_densities[getTextureIndex(i1, j1)]);
		}
	}

	setDensityBoundaries();

	swapDensities();
}

void MainScreen::swapDensities() {
	for(unsigned int i = 0; i < CELLS_X * CELLS_Y; i++) {
		m_densities[i] = m_swapDensities[i];
	}
}

void MainScreen::setDensityBoundaries() {
	for(unsigned int i = 0; i < CELLS_X; i++) {
		m_swapDensities[getTextureIndex(i, 0)]			 = m_boundary;
		m_swapDensities[getTextureIndex(i, CELLS_Y - 1)] = m_boundary;
	}
	for(unsigned int j = 0; j < CELLS_Y; j++) {
		m_swapDensities[getTextureIndex(0, j)]			 = m_boundary;
		m_swapDensities[getTextureIndex(CELLS_X - 1, j)] = m_boundary;
	}
}

unsigned int MainScreen::getTextureIndex(unsigned int x, unsigned int y) {
	return y * CELLS_X + x;
}

int MainScreen::getNextScreenIndex() const {
	return 0;
}
