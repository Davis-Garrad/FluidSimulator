#pragma once

#include <Screen.hpp>
#include <MutableTexture.hpp>
#include <BasicRenderer.hpp>
#include <ResourceManager.hpp>
#include <InputManager.hpp>
#include <glm/glm.hpp>

#define CELLS_X 150
#define CELLS_Y 100

struct vec {
	double x = 0.0d, y = 0.0d;
};

class MainScreen : public BARE2D::Screen {
  public:
	MainScreen(unsigned int windowWidth, unsigned int windowHeight, BARE2D::InputManager* input);
	~MainScreen();

	virtual void initScreen() override;
	virtual void destroyScreen() override;
	virtual void onEntry() override;
	virtual void onExit() override;
	virtual void draw() override;
	virtual void update(double dt) override;
	virtual int	 getNextScreenIndex() const override;

  private:
	void		 updateInput();
	void		 diffuseFluid(double dt);
	void		 moveFluid(double dt);
	unsigned int getTextureIndex(unsigned int x, unsigned int y);
	void		 swapDensities();
	void		 setDensityBoundaries();

	BARE2D::BasicRenderer*	m_renderer	   = nullptr;
	BARE2D::MutableTexture* m_fluidTexture = nullptr;

	// unsigned char goes from 0-255. Useful for rendering, so we can just keep it that way for now.
	unsigned char* m_densities	= nullptr;
	vec*		   m_directions = nullptr;
	// swapping counterparts.
	unsigned char* m_swapDensities	= nullptr;
	vec*		   m_swapDirections = nullptr;

	BARE2D::InputManager* m_input = nullptr;

	unsigned int m_windowW, m_windowH;

	double		  m_diffusionRate = 0.5;
	unsigned char m_boundary	  = 0;

	glm::vec2 m_lastMouse;
};
