//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2013 - Julien Fryer - julienfryer@gmail.com				//
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

#ifndef H_SPK_GLES2_RENDERER
#define H_SPK_GLES2_RENDERER

#include "Rendering/OpenGLES2/SPK_GLES2_DEF.h"

namespace SPK
{
namespace GLES2
{
	/**
	* @class GLES2Renderer
	* @brief An abstract Renderer for the OpenGL renderers
	*/
	class SPK_GLES2_PREFIX GLES2Renderer : public Renderer
	{
	public :

		////////////////
		// Destructor //
		////////////////

		/** @brief Destructor of GLES2Renderer */
		virtual  ~GLES2Renderer();

		/////////////
		// Setters //
		/////////////

		/**
		* @brief Enables or disables the blending of this GLES2Renderer
		* @param blendingEnabled true to enable the blending, false to disable it
		*/
		virtual  void enableBlending(bool blendingEnabled);

		/**
		* @brief Sets the blending functions of this GLES2Renderer
		*
		* the blending functions are one of the OpenGL blending functions.
		*
		* @param src : the source blending function of this GLES2Renderer
		* @param dest : the destination blending function of this GLES2Renderer
		*/
		void setBlendingFunctions(GLuint src,GLuint dest);
		virtual void setBlendMode(BlendMode blendMode);

		/**
		 * @brief Sets the model-view matrix
		 *
		 * @param matrix Column-major matrix.
		 */
		void setModelViewMatrix(const float* matrix);

		/**
		 * @brief Sets the projection matrix
		 *
		 * @param matrix Column-major matrix.
		 */
		void setProjectionMatrix(const float* matrix);

		/////////////
		// Getters //
		/////////////

		/**
		* @brief Tells whether blending is enabled for this GLES2Renderer
		* @return true if blending is enabled, false if it is disabled
		*/
		bool isBlendingEnabled() const;

		/**
		* @brief Gets the source blending function of this GLES2Renderer
		* @return the source blending function of this GLES2Renderer
		*/
		GLuint getSrcBlendingFunction() const;

		/**
		* @brief Gets the destination blending function of this GLES2Renderer
		* @return the source destination function of this GLES2Renderer
		*/
		GLuint getDestBlendingFunction() const;

		/**
		 * @brief Gets the model-view matrix
		 *
		 * @return Column-major matrix.
		 */
		const float* getModelViewMatrix() const;

		/**
		 * @brief Gets the projection matrix
		 *
		 * @return Column-major matrix.
		 */
		const float* getProjectionMatrix() const;

		///////////////
		// Interface //
		///////////////

	public :
		spark_description(GLES2Renderer, Renderer)
		(
		);

	protected :

		GLES2Renderer(bool NEEDS_DATASET);
		GLES2Renderer(const GLES2Renderer& renderer);

		/**
		 * @brief Inits shader program
		 * @return true if succeeded, otherwise false.
		 */
		bool initBaseShader(const char* vertSrc, const char* fragSrc);

		/** @brief Inits the blending of this GLES2Renderer */
		void initBlending() const;

		/** @brief Inits the rendering hints of this GLES2Renderer */
		void initRenderingOptions() const;

		/** @brief Obtain a GLSL program name */
		GLuint getProgramName() const;

		/** @brief Obtain a GLSL alpha test uniform location */
		GLint getAlphaTestEnabledUniformLocation() const;

		/** @brief Obtain a GLSL alpha test threshold uniform location */
		GLint getAlphaTestThresholdUniformLocation() const;

		/** @brief Obtain a GLSL vertex attrib location */
		GLint getVertexAttribLocation() const;

		/** @brief Obtain a GLSL color attrib location */
		GLint getColorAttribLocation() const;

		/** @brief Obtain a GLSL texcoord attrib location */
		GLint getTexCoordAttribLocation() const;

	private :

		bool blendingEnabled;
		GLuint srcBlending;
		GLuint destBlending;
		float modelViewMatrix[16];
		float projectionMatrix[16];
		GLuint programName;
		GLint modelViewMatrixUniformLocation;
		GLint projectionMatrixUniformLocation;
		GLint alphaTestEnabledUniformLocation;
		GLint alphaTestThresholdUniformLocation;
		GLint vertexAttribLocation;
		GLint colorAttribLocation;
		GLint texCoordAttribLocation;
	};

	inline GLES2Renderer::GLES2Renderer(bool NEEDS_DATASET) :
		Renderer(NEEDS_DATASET),
		blendingEnabled(false),
		srcBlending(GL_SRC_ALPHA),
		destBlending(GL_ONE_MINUS_SRC_ALPHA),
		modelViewMatrix{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 },
		projectionMatrix{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 },
		programName(0),
		modelViewMatrixUniformLocation(-1),
		projectionMatrixUniformLocation(-1),
		alphaTestEnabledUniformLocation(-1),
		alphaTestThresholdUniformLocation(-1),
		vertexAttribLocation(-1),
		colorAttribLocation(-1),
		texCoordAttribLocation(-1)
	{}

	inline GLES2Renderer::GLES2Renderer(const GLES2Renderer& renderer) :
		Renderer(renderer),
		blendingEnabled(renderer.blendingEnabled),
		srcBlending(renderer.srcBlending),
		destBlending(renderer.destBlending),
		modelViewMatrix{0},
		projectionMatrix{0},
		programName(0),
		modelViewMatrixUniformLocation(-1),
		projectionMatrixUniformLocation(-1),
		alphaTestEnabledUniformLocation(-1),
		alphaTestThresholdUniformLocation(-1),
		vertexAttribLocation(-1),
		colorAttribLocation(-1),
		texCoordAttribLocation(-1)
	{
		std::memcpy(modelViewMatrix, renderer.modelViewMatrix, sizeof(modelViewMatrix));
		std::memcpy(projectionMatrix, renderer.projectionMatrix, sizeof(projectionMatrix));
	}

	inline GLES2Renderer::~GLES2Renderer()
	{
		if (programName != 0)
		{
			glDeleteProgram(programName);
			programName = 0;
		}
	}

	inline void GLES2Renderer::enableBlending(bool blendingEnabled)
	{
		this->blendingEnabled = blendingEnabled;
	}

	inline void GLES2Renderer::setBlendingFunctions(GLuint src,GLuint dest)
	{
		srcBlending = src;
		destBlending = dest;
	}

	inline void GLES2Renderer::setModelViewMatrix(const float* matrix)
	{
		std::memcpy(modelViewMatrix, matrix, sizeof(float) * 16);
	}

	inline void GLES2Renderer::setProjectionMatrix(const float* matrix)
	{
		std::memcpy(projectionMatrix, matrix, sizeof(float) * 16);
	}

	inline bool GLES2Renderer::isBlendingEnabled() const
	{
		return blendingEnabled;
	}

	inline GLuint GLES2Renderer::getSrcBlendingFunction() const
	{
		return srcBlending;
	}

	inline GLuint GLES2Renderer::getDestBlendingFunction() const
	{
		return destBlending;
	}

	inline const float* GLES2Renderer::getModelViewMatrix() const
	{
		return modelViewMatrix;
	}

	inline const float* GLES2Renderer::getProjectionMatrix() const
	{
		return projectionMatrix;
	}

	inline void GLES2Renderer::initBlending() const
	{
		if (blendingEnabled)
		{
			glBlendFunc(srcBlending,destBlending);
			glEnable(GL_BLEND);
		}
		else
			glDisable(GL_BLEND);
	}

	inline GLuint GLES2Renderer::getProgramName() const
	{
		return programName;
	}

	inline GLint GLES2Renderer::getAlphaTestEnabledUniformLocation() const
	{
		return alphaTestEnabledUniformLocation;
	}

	inline GLint GLES2Renderer::getAlphaTestThresholdUniformLocation() const
	{
		return alphaTestThresholdUniformLocation;
	}

	inline GLint GLES2Renderer::getVertexAttribLocation() const
	{
		return vertexAttribLocation;
	}

	inline GLint GLES2Renderer::getColorAttribLocation() const
	{
		return colorAttribLocation;
	}

	inline GLint GLES2Renderer::getTexCoordAttribLocation() const
	{
		return texCoordAttribLocation;
	}
}}

#endif
