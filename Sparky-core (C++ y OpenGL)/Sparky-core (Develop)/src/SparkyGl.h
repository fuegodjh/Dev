#pragma once

#ifdef SPARKY_EMSCRIPTEN
	#define GLFW_INCLUDE_ES3
#else
	#include <GL\glew.h>
#endif

#include <GLFW/glfw3.h>