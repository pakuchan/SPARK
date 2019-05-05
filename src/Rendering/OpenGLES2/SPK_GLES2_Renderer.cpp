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

#include <SPARK_Core.h>
#include "Rendering/OpenGLES2/SPK_GLES2_Renderer.h"
#include "SPK_GLES2_ErrorChecker.h"

namespace SPK
{
namespace GLES2
{
	void GLES2Renderer::setBlendMode(BlendMode blendMode)
	{
		switch(blendMode)
		{
		case BLEND_MODE_NONE :
			srcBlending = GL_ONE;
			destBlending = GL_ZERO;
			blendingEnabled = false;
			break;

		case BLEND_MODE_ADD :
			srcBlending = GL_SRC_ALPHA;
			destBlending = GL_ONE;
			blendingEnabled = true;
			break;

		case BLEND_MODE_ALPHA :
			srcBlending = GL_SRC_ALPHA;
			destBlending = GL_ONE_MINUS_SRC_ALPHA;
			blendingEnabled = true;
			break;

		default :
			SPK_LOG_WARNING("GLES2Renderer::setBlendMode(BlendMode) - Unsupported blending mode. Nothing happens");
			break;
		}
	}

	bool GLES2Renderer::initBaseShader(const char* vertSrc, const char* fragSrc)
	{
		GLint compileStatus = GL_FALSE;
		GLint linkStatus = GL_FALSE;

		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertShader, 1, &vertSrc, 0);
		glCompileShader(vertShader);
		glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus == GL_FALSE)
		{
			GLint infoLogLength = 0;
			glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<char> infoLog(infoLogLength + 1);
			glGetShaderInfoLog(vertShader, infoLogLength, NULL, &infoLog[0]);
			SPK_LOG_ERROR("Vertex shader compile error: " << &infoLog[0] << "\n");
			glDeleteShader(vertShader);
			return false;
		}

		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, &fragSrc, 0);
		glCompileShader(fragShader);
		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus == GL_FALSE)
		{
			GLint infoLogLength = 0;
			glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<char> infoLog(infoLogLength + 1);
			glGetShaderInfoLog(fragShader, infoLogLength, NULL, &infoLog[0]);
			SPK_LOG_ERROR("Fragment shader compile error: " << &infoLog[0] << "\n");
			glDeleteShader(vertShader);
			glDeleteShader(fragShader);
			return false;
		}

		GLuint prog = glCreateProgram();
		glAttachShader(prog, vertShader);
		glAttachShader(prog, fragShader);
		glLinkProgram(prog);
		glGetProgramiv(prog, GL_LINK_STATUS, &linkStatus);
		if (linkStatus == GL_FALSE)
		{
			GLint infoLogLength = 0;
			glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<char> infoLog(infoLogLength + 1);
			glGetProgramInfoLog(prog, infoLogLength, NULL, &infoLog[0]);
			SPK_LOG_ERROR("Shader link error: " << &infoLog[0] << "\n");
			glDeleteProgram(prog);
			glDeleteShader(vertShader);
			glDeleteShader(fragShader);
			return false;
		}

		// Retrieve uniform location
		GLint modelViewMatrixUniformLoc = glGetUniformLocation(prog, "u_modelViewMatrix");
		GLint projectionMatrixUniformLoc = glGetUniformLocation(prog, "u_projectionMatrix");
		GLint alphaTestEnabledUniformLoc = glGetUniformLocation(prog, "u_alphaTestEnabled");
		GLint alphaTestThresholdUniformLoc = glGetUniformLocation(prog, "u_alphaTestThreshold");
		GLint textureSamplerUniformLoc = glGetUniformLocation(prog, "u_texSampler");
		GLint vertexAttribLoc = glGetAttribLocation(prog, "a_vertexAttrib");
		GLint colorAttribLoc = glGetAttribLocation(prog, "a_colorAttrib");
		GLint texCoordAttribLoc = glGetAttribLocation(prog, "a_texCoordAttrib");

		if (programName != 0)
			glDeleteProgram(programName);

		programName = prog;
		modelViewMatrixUniformLocation = modelViewMatrixUniformLoc;
		projectionMatrixUniformLocation = projectionMatrixUniformLoc;
		alphaTestEnabledUniformLocation = alphaTestEnabledUniformLoc;
		alphaTestThresholdUniformLocation = alphaTestThresholdUniformLoc;
		vertexAttribLocation = vertexAttribLoc;
		colorAttribLocation = colorAttribLoc;
		texCoordAttribLocation = texCoordAttribLoc;

		checkError(__FILE__, __LINE__);

		return true;
	}

	void GLES2Renderer::initRenderingOptions() const
	{
		// alpha test
		if (isRenderingOptionEnabled(RENDERING_OPTION_ALPHA_TEST) && alphaTestEnabledUniformLocation > 0 && alphaTestThresholdUniformLocation > 0)
		{
			glUniform1f(alphaTestThresholdUniformLocation, getAlphaTestThreshold());
			glUniform1i(alphaTestEnabledUniformLocation, GL_TRUE);
		}
		else
			glUniform1i(alphaTestEnabledUniformLocation, GL_FALSE);
		checkError(__FILE__, __LINE__);

		// depth write
		glDepthMask(isRenderingOptionEnabled(RENDERING_OPTION_DEPTH_WRITE));
		checkError(__FILE__, __LINE__);

		// model-view matrix
		glUniformMatrix4fv(modelViewMatrixUniformLocation, 1, GL_FALSE, modelViewMatrix);

		// projection matrix
		glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, projectionMatrix);
	}

}}
