//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2011 - Julien Fryer - julienfryer@gmail.com				//
//																				//
// This software is provided 'as-is', without any express or implied			//
// warranty.  In no event will the authors be held liable for any damages		//
// arising from the use of this software.										//
//																				//
// Permission is granted to anyone to use this software for any purpose,		//
// including commercial applications, and to alter it and redistribute it		//
// freely, subject to the following restrictions:								//
//																				//
// 1. The origin of this software must not be misrepresented; you must not		//
//    claim that you wrote the original software. If you use this software		//
//    in a product, an acknowledgment in the product documentation would be		//
//    appreciated but is not required.											//
// 2. Altered source versions must be plainly marked as such, and must not be	//
//    misrepresented as being the original software.							//
// 3. This notice may not be removed or altered from any source distribution.	//
//////////////////////////////////////////////////////////////////////////////////

#include <ctime>
#include <deque>
#include <cstdlib>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

#include <exception>
#include <functional>
#include <SDL.h>

#include <GL/glew.h>

#include <SPARK.h>
#include <SPARK_GLES2.h>


float angleX = 0.0f;
float angleY = 0.0f;
const float CAM_POS_Z = 3.0f;

// Loads a texture
bool loadTexture(GLuint& index, char* path, GLuint type, GLuint clamp)
{
	SDL_Surface* particleImg;
	particleImg = SDL_LoadBMP(path);
	if (particleImg == NULL)
	{
		std::cout << "Unable to load bitmap :" << SDL_GetError() << std::endl;
		return false;
	}

	// converts from BGR to RGB
	if ((type == GL_RGB) || (type == GL_RGBA))
	{
		const int offset = (type == GL_RGB ? 3 : 4);
		unsigned char* iterator = static_cast<unsigned char*>(particleImg->pixels);
		unsigned char* tmp0, *tmp1;
		for (int i = 0; i < particleImg->w * particleImg->h; ++i)
		{
			tmp0 = iterator;
			tmp1 = iterator + 2;
			std::swap(*tmp0, *tmp1);
			iterator += offset;
		}
	}

	glGenTextures(1, &index);
	glBindTexture(GL_TEXTURE_2D, index);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D,
		0,
		type,
		particleImg->w,
		particleImg->h,
		0,
		type,
		GL_UNSIGNED_BYTE,
		particleImg->pixels);

	SDL_FreeSurface(particleImg);

	return true;
}

// Draws the bounding box for a particle system
void drawBoundingBox(const SPK::System& system, const float* modelViewMatrix, const float* projectionMatrix)
{
	if (!system.isAABBComputationEnabled())
		return;

	static const char* vsSrc = R"(
		#version 100
		attribute vec4 a_position;
		attribute vec4 a_color;
		uniform mat4 u_modelView;
		uniform mat4 u_projection;
		varying vec4 v_color;
		void main() {
			mat4 mvp = u_projection * u_modelView;
			gl_Position = mvp * a_position;
			v_color = a_color;
		}
	)";
	static const char* fsSrc = R"(
		#version 100
		precision highp float;
		varying vec4 v_color;
		void main() {
			gl_FragColor = vec4(v_color.xyz, 1.0);
		}
	)";

	static GLuint bbVS = 0;
	static GLuint bbFS = 0;
	static GLuint bbProg = 0;
	static GLint modelViewMatrixUniformLocation = 0;
	static GLint projectionMatrixUniformLocation = 0;
	static GLint positionAttribLocation = 0;
	static GLint colorAttribLocation = 0;
	if (bbVS == 0) {
		bbVS = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(bbVS, 1, &vsSrc, 0);
		glCompileShader(bbVS);
	}
	if (bbFS == 0) {
		bbFS = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(bbFS, 1, &fsSrc, 0);
		glCompileShader(bbFS);
	}
	if (bbProg == 0) {
		bbProg = glCreateProgram();
		glAttachShader(bbProg, bbVS);
		glAttachShader(bbProg, bbFS);
		glLinkProgram(bbProg);

		modelViewMatrixUniformLocation = glGetUniformLocation(bbProg, "u_modelView");
		projectionMatrixUniformLocation = glGetUniformLocation(bbProg, "u_projection");
		positionAttribLocation = glGetAttribLocation(bbProg, "a_position");
		colorAttribLocation = glGetAttribLocation(bbProg, "a_color");
	}

	SPK::Vector3D AABBMin = system.getAABBMin();
	SPK::Vector3D AABBMax = system.getAABBMax();

	float positions[72] = {
		AABBMin.x, AABBMin.y, AABBMin.z,
		AABBMax.x, AABBMin.y, AABBMin.z,
		AABBMin.x, AABBMin.y, AABBMin.z,
		AABBMin.x, AABBMax.y, AABBMin.z,
		AABBMin.x, AABBMin.y, AABBMin.z,
		AABBMin.x, AABBMin.y, AABBMax.z,
		AABBMax.x, AABBMax.y, AABBMax.z,
		AABBMin.x, AABBMax.y, AABBMax.z,
		AABBMax.x, AABBMax.y, AABBMax.z,
		AABBMax.x, AABBMin.y, AABBMax.z,
		AABBMax.x, AABBMax.y, AABBMax.z,
		AABBMax.x, AABBMax.y, AABBMin.z,
		AABBMin.x, AABBMin.y, AABBMax.z,
		AABBMax.x, AABBMin.y, AABBMax.z,
		AABBMin.x, AABBMin.y, AABBMax.z,
		AABBMin.x, AABBMax.y, AABBMax.z,
		AABBMin.x, AABBMax.y, AABBMin.z,
		AABBMax.x, AABBMax.y, AABBMin.z,
		AABBMin.x, AABBMax.y, AABBMin.z,
		AABBMin.x, AABBMax.y, AABBMax.z,
		AABBMax.x, AABBMin.y, AABBMin.z,
		AABBMax.x, AABBMax.y, AABBMin.z,
		AABBMax.x, AABBMin.y, AABBMin.z,
		AABBMax.x, AABBMin.y, AABBMax.z,
	};
	float colors[72] = {
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
	};
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(bbProg);
	glEnableVertexAttribArray(positionAttribLocation);
	if (modelViewMatrixUniformLocation != -1) {
		glUniformMatrix4fv(modelViewMatrixUniformLocation, 1, GL_FALSE, modelViewMatrix);
	}
	if (projectionMatrixUniformLocation != -1) {
		glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, projectionMatrix);
	}
	glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, positions);
	glEnableVertexAttribArray(colorAttribLocation);
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, colors);
	glDrawArrays(GL_LINES, 0, 24);
}

void loadPerspectiveMatrix(float* m, float fovy, float aspect, float zNear, float zFar) {
	float f = 1.0f / std::tanf(fovy * (float)M_PI / 180.0f / 2.0f);

	m[0] = f / aspect; m[4] = 0; m[8] = 0;                                m[12] = 0;
	m[1] = 0;          m[5] = f; m[9] = 0;                                m[13] = 0;
	m[2] = 0;          m[6] = 0; m[10] = (zFar + zNear) / (zNear - zFar); m[14] = (2 * zNear * zFar) / (zNear - zFar);
	m[3] = 0;          m[7] = 0; m[11] = -1;                              m[15] = 0;
}

void loadIdentityMatrix(float* m) {
	m[0] = 1; m[4] = 0; m[8] = 0;  m[12] = 0;
	m[1] = 0; m[5] = 1; m[9] = 0;  m[13] = 0;
	m[2] = 0; m[6] = 0; m[10] = 1; m[14] = 0;
	m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
}

void multMatrix(float* a, const float* b) {
	float r[16];
	r[0]  = a[0] * b[0]  + a[4] * b[1]  + a[8]  * b[2]  + a[12] * b[3];
	r[4]  = a[0] * b[4]  + a[4] * b[5]  + a[8]  * b[6]  + a[12] * b[7];
	r[8]  = a[0] * b[8]  + a[4] * b[9]  + a[8]  * b[10] + a[12] * b[11];
	r[12] = a[0] * b[12] + a[4] * b[13] + a[8]  * b[14] + a[12] * b[15];
	r[1]  = a[1] * b[0]  + a[5] * b[1]  + a[9]  * b[2]  + a[13] * b[3];
	r[5]  = a[1] * b[4]  + a[5] * b[5]  + a[9]  * b[6]  + a[13] * b[7];
	r[9]  = a[1] * b[8]  + a[5] * b[9]  + a[9]  * b[10] + a[13] * b[11];
	r[13] = a[1] * b[12] + a[5] * b[13] + a[9]  * b[14] + a[13] * b[15];
	r[2]  = a[2] * b[0]  + a[6] * b[1]  + a[10] * b[2]  + a[14] * b[3];
	r[6]  = a[2] * b[4]  + a[6] * b[5]  + a[10] * b[6]  + a[14] * b[7];
	r[10] = a[2] * b[8]  + a[6] * b[9]  + a[10] * b[10] + a[14] * b[11];
	r[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
	r[3]  = a[3] * b[0]  + a[7] * b[1]  + a[11] * b[2]  + a[15] * b[3];
	r[7]  = a[3] * b[4]  + a[7] * b[5]  + a[11] * b[6]  + a[15] * b[7];
	r[11] = a[3] * b[8]  + a[7] * b[9]  + a[11] * b[10] + a[15] * b[11];
	r[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
	std::memcpy(a, r, sizeof(r));
}

void translateMatrix(float* m, float x, float y, float z) {
	float t[16];
	loadIdentityMatrix(t);
	t[12] = x;
	t[13] = y;
	t[14] = z;
	multMatrix(m, t);
}

void rotateMatrix(float* m, float angle, float x, float y, float z) {
	float r[16];
	float c = std::cosf(angle * (float)M_PI / 180.0f);
	float s = std::sinf(angle * (float)M_PI / 180.0f);
	r[0] = x*x*(1-c)+c;   r[4] = x*y*(1-c)-z*s; r[8] = x*z*(1-c)+y*s; r[12] = 0;
	r[1] = y*x*(1-c)+z*s; r[5] = y*y*(1-c)+c;   r[9] = y*z*(1-c)-x*s; r[13] = 0;
	r[2] = x*z*(1-c)-y*s; r[6] = y*z*(1-c)+x*s; r[10] = z*z*(1-c)+c;  r[14] = 0;
	r[3] = 0;             r[7] = 0;             r[11] = 0;            r[15] = 1;
	multMatrix(m, r);
}

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	auto wnd(
		SDL_CreateWindow("SPARK 2 test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			800, 600,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN));

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetSwapInterval(0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	auto glc = SDL_GL_CreateContext(wnd);

	glewInit();

	auto rdr = SDL_CreateRenderer(
		wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

	SDL_Surface & screen = *SDL_GetWindowSurface(wnd);
	int screenWidth = screen.w;
	int screenHeight = screen.h;
	float screenRatio = (float)screen.w / (float)screen.h;

	// Loads particle texture
	GLuint textureParticle;
	if (!loadTexture(textureParticle, "res/explosion.bmp", GL_LUMINANCE, GL_CLAMP))
	{
	}//	return 1;

	// inits openGL
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, screenWidth, screenHeight);

	float projectionMatrix[16];
	loadPerspectiveMatrix(projectionMatrix, 45.0f, screenRatio, 0.001f, 10.0f);
	glEnable(GL_DEPTH_TEST);

	SPK::System::setClampStep(true, 0.1f);			// clamp the step to 100 ms
	SPK::System::useAdaptiveStep(0.001f, 0.01f);		// use an adaptive step from 1ms to 10ms (1000fps to 100fps)

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	const char* vsSrc = R"(
		#version 100
		attribute vec4 a_position;
		attribute vec4 a_color;
		uniform mat4 u_modelView;
		uniform mat4 u_projection;
		varying vec4 v_color;
		void main() {
			mat4 mvp = u_projection * u_modelView;
			gl_Position = mvp * a_position;
			v_color = a_color;
		}
	)";
	const char* fsSrc = R"(
		#version 100
		precision highp float;
		varying vec4 v_color;
		void main() {
			gl_FragColor = v_color;
		}
	)";

	glShaderSource(vs, 1, &vsSrc, 0);
	glCompileShader(vs);
	GLint compileStatus;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		GLint infoLogLength = 0;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> infoLog(infoLogLength + 1);
		glGetShaderInfoLog(vs, infoLogLength, NULL, &infoLog[0]);
		SPK_LOG_ERROR("Vertex shader compile error: " << &infoLog[0] << "\n");
		glDeleteShader(vs);
		glDeleteShader(vs);
		return false;
	}
	glShaderSource(fs, 1, &fsSrc, 0);
	glCompileShader(fs);
	if (compileStatus == GL_FALSE)
	{
		GLint infoLogLength = 0;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> infoLog(infoLogLength + 1);
		glGetShaderInfoLog(fs, infoLogLength, NULL, &infoLog[0]);
		SPK_LOG_ERROR("Fragment shader compile error: " << &infoLog[0] << "\n");
		glDeleteShader(fs);
		glDeleteShader(fs);
		return false;
	}
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	float positions[] = {
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
	};
	float colors[] = {
		0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, 0.0f, 1.0f,
	};
	GLuint vboPosition;
	GLuint vboColor;
	glGenBuffers(1, &vboPosition);
	glGenBuffers(1, &vboColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

	{
		SPK::Ref<SPK::System> system = SPK::System::create(true);
		system->setName("Test System");

		SPK::Ref<SPK::GLES2::GLES2QuadRenderer> renderer = SPK::GLES2::GLES2QuadRenderer::create();
		renderer->setBlendMode(SPK::BLEND_MODE_ADD);
		renderer->enableRenderingOption(SPK::RENDERING_OPTION_DEPTH_WRITE, false);
		renderer->setTexture(textureParticle);
		renderer->setTexturingMode(SPK::TEXTURE_MODE_NONE);
		renderer->setAtlasDimensions(2, 2);

		SPK::Ref<SPK::SphericEmitter> emitter = SPK::SphericEmitter::create(SPK::Vector3D(0.0f, 0.0f, -1.0f), 0.0f, 3.14159f / 4.0f, SPK::Point::create(), true, -1, 100.0f, 0.2f, 0.5f);

		SPK::Ref<SPK::Group> phantomGroup = system->createGroup(40);
		SPK::Ref<SPK::Group> trailGroup = system->createGroup(1000);

		SPK::Ref<SPK::Plane> ground = SPK::Plane::create(SPK::Vector3D(0.0f, -1.0f, 0.0f));

		phantomGroup->setName("Phantom Group");
		phantomGroup->setLifeTime(5.0f, 5.0f);
		phantomGroup->setRadius(0.06f);
		phantomGroup->addEmitter(SPK::SphericEmitter::create(SPK::Vector3D(0.0f, 1.0f, 0.0f), 0.0f, 3.14159f / 4.0f, SPK::Point::create(SPK::Vector3D(0.0f, -1.0f, 0.0f)), true, -1, 2.0f, 1.2f, 2.0f));
		phantomGroup->addModifier(SPK::Gravity::create(SPK::Vector3D(0.0f, -1.0f, 0.0f)));
		phantomGroup->addModifier(SPK::Obstacle::create(ground, 0.8f));
		phantomGroup->addModifier(SPK::EmitterAttacher::create(trailGroup, emitter, true));

		trailGroup->setName("Trail");
		trailGroup->setLifeTime(0.5f, 1.0f);
		trailGroup->setRadius(0.06f);
		trailGroup->setRenderer(renderer);
		trailGroup->setColorInterpolator(SPK::ColorSimpleInterpolator::create(0xFF802080, 0xFF000000));
		trailGroup->setParamInterpolator(SPK::PARAM_TEXTURE_INDEX, SPK::FloatRandomInitializer::create(0.0f, 4.0f));
		trailGroup->setParamInterpolator(SPK::PARAM_ROTATION_SPEED, SPK::FloatRandomInitializer::create(-0.1f, 1.0f));
		trailGroup->setParamInterpolator(SPK::PARAM_ANGLE, SPK::FloatRandomInitializer::create(0.0f, 2.0f * 3.14159f));
		trailGroup->addModifier(SPK::Rotator::create());
		trailGroup->addModifier(SPK::Destroyer::create(ground));

		float deltaTime = 0.0f;

		std::deque<clock_t> frameFPS;
		frameFPS.push_back(clock());

		SDL_Event event;
		bool exit = false;
		bool paused = false;
		while (!exit)
		{
			while (SDL_PollEvent(&event))
			{
				// if esc is pressed, exit
				if ((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) || event.type == SDL_QUIT)
					exit = true;

				// if pause is pressed, the system is paused
				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_PAUSE)
					paused = !paused;

				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F4)
					system->enableAABBComputation(!system->isAABBComputationEnabled());

				// Moves the camera with the mouse
				if (event.type == SDL_MOUSEMOTION)
				{
					angleY += event.motion.xrel * 0.5f;
					angleX += event.motion.yrel * 0.5f;
				}
			}

			glDepthMask(GL_TRUE);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (!paused)
				system->updateParticles(deltaTime);

			float modelView[16];
			loadIdentityMatrix(modelView);
			translateMatrix(modelView, 0.0f, 0.0f, -CAM_POS_Z);
			rotateMatrix(modelView, angleX, 1.0f, 0.0f, 0.0f);
			rotateMatrix(modelView, angleY, 0.0f, 1.0f, 0.0f);
			renderer->setModelViewMatrix(modelView);
			renderer->setProjectionMatrix(projectionMatrix);

			GLint positionAttribLoc = glGetAttribLocation(prog, "a_position");
			GLint colorAttribLoc = glGetAttribLocation(prog, "a_color");
			GLint modelViewUniformLoc = glGetUniformLocation(prog, "u_modelView");
			GLint projectionUniformLoc = glGetUniformLocation(prog, "u_projection");
			glDisable(GL_BLEND);
			glUseProgram(prog);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glUniformMatrix4fv(modelViewUniformLoc, 1, GL_FALSE, modelView);
			if (projectionUniformLoc != -1) {
				glUniformMatrix4fv(projectionUniformLoc, 1, GL_FALSE, projectionMatrix);
			}
			if (positionAttribLoc != -1) {
				glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
				glEnableVertexAttribArray(positionAttribLoc);
				glVertexAttribPointer(positionAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
			if (colorAttribLoc != -1) {
				glBindBuffer(GL_ARRAY_BUFFER, vboColor);
				glEnableVertexAttribArray(colorAttribLoc);
				glVertexAttribPointer(colorAttribLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
			}
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			drawBoundingBox(*system, modelView, projectionMatrix);
			system->renderParticles();

			SDL_GL_SwapWindow(wnd);

			clock_t currentTick = clock();
			deltaTime = (float)(currentTick - frameFPS.back()) / CLOCKS_PER_SEC;
			frameFPS.push_back(currentTick);
			while ((frameFPS.back() - frameFPS.front() > 1000) && (frameFPS.size() > 2))
				frameFPS.pop_front();
		};

		SPK_DUMP_MEMORY
	}
	SPK_DUMP_MEMORY

	SDL_Quit();

	return 0;
}

