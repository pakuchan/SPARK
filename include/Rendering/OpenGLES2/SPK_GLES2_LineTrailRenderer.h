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

#ifndef H_SPK_GLES2_LINETRAILRENDERER
#define H_SPK_GLES2_LINETRAILRENDERER

#include "Rendering/OpenGLES2/SPK_GLES2_Renderer.h"
#include "Rendering/OpenGLES2/SPK_GLES2_Buffer.h"

namespace SPK
{
namespace GLES2
{
	/**
	* @class GLES2LineTrailRenderer
	* @brief A Renderer drawing particles as line trails defined by the positions of particles over time
	*
	* The trail coordinates are computed in a procedural manner over time.<br>
	* A trail i defined by a duration. The faster the particle, the longer the trail. It is defined by a numbers of samples.<br>
	* The sampling frequency of the trail is therefore computed by nbSamples / duration and defines its resolution.<br>
	* The higher the sampling frequency, the smoother the trail but the bigger the compution time and the memory consumption.<br>
	* <br>
	* All the particles of a Group are renderered in a single batch of GL_LINE_STRIP,
	* which means every trails belong to the same object to reduce overhead on GPU side.<br>
	* To allow that, invisible lines link trails together. They are defined as degenerated lines.<br>
	* This imposes the alpha value is taken into account and the blending is therefore forced with GLES2LineTrailRenderer.<br>
	* The user has the possibility to set the RGBA values of degenerated lines to keep them invisible function of the blending mode and environment.<br>
	* By default it is set to (0.0f,0.0f,0.0f,0.0f).
	* <br>
	* Below are the parameters of Particle that are used in this Renderer (others have no effects) :
	* <ul>
	* <li>SPK::PARAM_RED</li>
	* <li>SPK::PARAM_GREEN</li>
	* <li>SPK::PARAM_BLUE</li>
	* <li>SPK::PARAM_ALPHA</li>
	* </ul>
	*/
	class SPK_GLES2_PREFIX GLES2LineTrailRenderer : public GLES2Renderer
	{
	public :

		//////////////////
		// Constructors //
		//////////////////

		/**
		* @brief Creates a new GLES2LineTrailRenderer
		* @return A new GLES2LineTrailRenderer
		*/
		static Ref<GLES2LineTrailRenderer> create(uint32 nbSamples = 8,float duration = 1.0f,float width = 1.0f);

		///////////////
		// nbSamples //
		///////////////

		/**
		* @brief Sets the number of samples in a trail
		*
		* The number of samples defines the number of points used to construct the trail.<br>
		* The bigger the number of samples, the smoother the trail but the bigger the compution time and the memory consumption.
		*
		* @param nbSamples : the number of samples to construct the trails
		*/
		void setNbSamples(uint32 nbSamples);

		/**
		* @brief Gets the number of samples per trail
		* @return the number of samples per trail
		*/
		uint32 getNbSamples() const;

		//////////////
		// duration //
		//////////////

		/**
		* @brief Sets the duration of a sample
		*
		* The duration of a sample is defined by its life time from its creation to its destruction.<br>
		* Note that the alpha of a sample will decrease linearly from its initial alpha to 0.
		*
		* @param duration : the duration of a sample
		*/
		void setDuration(float duration);

		/**
		* @brief Gets the duration of a sample
		* @return the duration of a sample
		*/
		float getDuration() const;

		///////////
		// width //
		///////////

		/**
		* @brief Sets the width of a trail
		*
		* Like for GLES2LineRenderer, the width is defined in pixels and is not dependant of the distance of the trail from the camera
		*
		* @param width : the width of trails in pixels
		*/
		void setWidth(float width);

		/**
		* @brief Gets the width of a trail
		* @return the width of a trail (in pixels)
		*/
		float getWidth() const;

		/**
		* @brief Sets the color components of degenerated lines
		* @param color : the color of the degenerated lines
		*/
		void setDegeneratedLines(Color color);

		virtual void enableBlending(bool blendingEnabled);

	public :
		spark_description(GLES2LineTrailRenderer, GLES2Renderer)
		(
		);

	protected :

		virtual void createData(DataSet& dataSet,const Group& group) const;
		virtual void checkData(DataSet& dataSet,const Group& group) const;

	private :

		// Data indices
		static const uint32 NB_DATA = 4;
		static const uint32 VERTEX_BUFFER_INDEX = 0;
		static const uint32 COLOR_BUFFER_INDEX = 1;
		static const uint32 AGE_DATA_INDEX = 2;
		static const uint32 START_ALPHA_DATA_INDEX = 3;

		uint32 nbSamples;

		float width;
		float duration;

		Color degeneratedColor;

		/////////////////
		// Constructor //
		/////////////////

		GLES2LineTrailRenderer(uint32 nbSamples = 8,float duration = 1.0f,float width = 1.0f);
		GLES2LineTrailRenderer(const GLES2LineTrailRenderer& renderer);

		void initShader();

		virtual void init(const Particle& particle,DataSet* dataSet) const;
		virtual void update(const Group& group,DataSet* dataSet) const;

		virtual void render(const Group& group,const DataSet* dataSet,RenderBuffer* renderBuffer) const;
		virtual void computeAABB(Vector3D& AABBMin,Vector3D& AABBMax,const Group& group,const DataSet* dataSet) const;

		static const char* VERTEX_SOURCE;
		static const char* FRAGMENT_SOURCE;
	};

	inline Ref<GLES2LineTrailRenderer> GLES2LineTrailRenderer::create(uint32 nbSamples,float duration,float width)
	{
		return SPK_NEW(GLES2LineTrailRenderer,nbSamples,duration,width);
	}

	inline uint32 GLES2LineTrailRenderer::getNbSamples() const
	{
		return nbSamples;
	}

	inline float GLES2LineTrailRenderer::getDuration() const
	{
		return duration;
	}

	inline void GLES2LineTrailRenderer::setWidth(float width)
	{
		this->width = width;
	}

	inline float GLES2LineTrailRenderer::getWidth() const
	{
		return width;
	}

	inline void GLES2LineTrailRenderer::setDegeneratedLines(Color color)
	{
		degeneratedColor = color;
	}	
}}

#endif
