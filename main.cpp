#include <stdio.h>

#include <App.hpp>
#include <Window.hpp>

#include "MainScreen.hpp"

int main(int argc, char** argv) {
	BARE2D::App app;

	unsigned int width = 600, height = 400;

	app.getWindow()->setSize(width, height);
	app.getWindow()->setTitle("Fluids, Baby!");

	app.getScreenList()->addEntryScreen(new MainScreen(width, height, app.getInputManager()));

	app.run();
}
