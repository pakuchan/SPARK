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

#ifndef H_SPK_GLES2_POINTRENDERER
#define H_SPK_GLES2_POINTRENDERER

#include "Rendering/OpenGLES2/SPK_GLES2_Renderer.h"
#include "Extensions/Renderers/SPK_PointRenderBehavior.h"

namespace SPK
{
namespace GLES2
{
	/**
	* @class GLES2PointRenderer
	* @brief A Renderer drawing drawing particles as OpenGL points
	*
	* OpenGL points can be configured to render them in 3 different ways :
	* <ul>
	* <li>SPK::POINT_SQUARE : standard OpenGL points</li>
	* <li>SPK::POINT_CIRCLE : antialiased OpenGL points</li>
	* <li>SPK::POINT_SPRITE : OpenGL point sprites (must be supported by the hardware)</li>
	* </ul>
	* Moreover, points size can either be defined in screen space (in pixels) or in the universe space (must be supported by the hardware).
	* The advantage of the universe space is that points size on the screen will be dependant to their distance from the camera, whereas in screen space
	* all points will have the same size on the screen no matter what their distance from the camera is.
	* <br>
	* This renderer do not use any parameters of particles.
	*/
	class SPK_GLES2_PREFIX GLES2PointRenderer :	public GLES2Renderer,
												public PointRenderBehavior
	{
	public :

		/**
		* @brief Creates and registers a new GLES2PointRenderer
		* @param size : the size of the points
		* @return A new registered GLES2PointRenderer
		*/
		static  Ref<GLES2PointRenderer> create(float screenSize = 1.0f);

		virtual bool setType(PointType type);

		/**
		* @brief Sets the way size of points is computed in this GLES2PointRenderer
		*
		* if universe size is used (true), the extension is checked.<br>
		* if universe size is not supported by the hardware, false is returned and nothing happens.<br>
		* <br>
		* If world size is enabled, the static method setPixelPetUnit(float,int) must be called to set the conversion between pixels and world units.
		*
		* @param worldSizeEnabled : true to enable universe size, false to use screen size
		* @return true the type of size can be set, false otherwise
		*/
		bool enableWorldSize(bool worldSizeEnabled);

		/**
		* @brief Sets the texture of this GLES2PointRenderer
		*
		* Note that the texture is only used if point sprites are used (type is set to SPK::POINT_SPRITE)
		*
		* @param textureIndex : the index of the OpenGL texture of this GLES2PointRenderer
		*/
		void setTexture(GLuint textureIndex);

		/**
		* @brief Gets the texture of this GLES2PointRenderer
		* @return the texture of this GLES2PointRenderer
		*/
		GLuint getTexture() const;

		/**
		* @brief Computes a conversion ratio between pixels and universe units
		*
		* This method must be called when using GLES2PointRenderer with world size enabled.<br>
		* It allows to well transpose world size to pixel size by setting the right OpenGL parameters.<br>
		* <br>
		* Note that fovy can be replaced by fovx if screenHeight is replaced by screenWidth.
		*
		* @param fovy : the field of view in the y axis in radians
		* @param screenHeight : the height of the viewport in pixels
		*/
		static  void setPixelPerUnit(float fovy,int screenHeight);

		static bool isPointSpriteSupported();
		static bool isWorldSizeSupported();

	public :
		spark_description(GLES2PointRenderer, GLES2Renderer)
		(
		);

	private :

		static float pixelPerUnit;

		GLuint textureIndex;
		GLint useWorldSizeUniformLocation;
		GLint sizeThresholdUniformLocation;
		GLint distanceAttenuationUniformLocation;
		GLint useTextureUniformLocation;
		GLint texSamplerUniformLocation;
		GLint pointSizeUniformLocation;
		GLint useCircularPointUniformLocation;

		GLES2PointRenderer(float screenSize = 1.0f);
		GLES2PointRenderer(const GLES2PointRenderer& renderer);

		virtual void initShader();

		virtual RenderBuffer* attachRenderBuffer(const Group& group) const;

		virtual void render(const Group& group,const DataSet* dataSet,RenderBuffer* renderBuffer) const;
		virtual void computeAABB(Vector3D& AABBMin,Vector3D& AABBMax,const Group& group,const DataSet* dataSet) const;

		static const char* VERTEX_SOURCE;
		static const char* FRAGMENT_SOURCE;
	};

	inline Ref<GLES2PointRenderer> GLES2PointRenderer::create(float screenSize)
	{
		return SPK_NEW(GLES2PointRenderer,screenSize);
	}
		
	inline void GLES2PointRenderer::setTexture(GLuint textureIndex)
	{
		this->textureIndex = textureIndex;
	}

	inline GLuint GLES2PointRenderer::getTexture() const
	{
		return textureIndex;
	}

	inline void GLES2PointRenderer::setPixelPerUnit(float fovy,int screenHeight)
	{
		// the pixel per unit is computed for a distance from the camera of screenHeight
		pixelPerUnit = screenHeight / (2.0f * tan(fovy * 0.5f));
	}
}}

#endif
