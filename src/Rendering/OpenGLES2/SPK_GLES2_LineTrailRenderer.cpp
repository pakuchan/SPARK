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

#include <cstring> // for memmove and memcpy

#include <SPARK_Core.h>
#include "Rendering/OpenGLES2/SPK_GLES2_LineTrailRenderer.h"

namespace SPK
{
namespace GLES2
{
	const char* GLES2LineTrailRenderer::VERTEX_SOURCE = R"(#version 100

attribute vec4 a_vertexAttrib;
attribute vec4 a_colorAttrib;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_projectionMatrix;
varying vec4 v_color;

void main() {
	gl_Position = u_projectionMatrix * u_modelViewMatrix * a_vertexAttrib;
	v_color = a_colorAttrib;
}
	)";

	const char* GLES2LineTrailRenderer::FRAGMENT_SOURCE = R"(#version 100

precision highp float;

uniform bool u_alphaTestEnabled;
uniform float u_alphaTestThreshold;
varying vec4 v_color;

void main() {
	if (u_alphaTestEnabled && v_color.a <= u_alphaTestThreshold)
		discard;
	gl_FragColor = v_color;
}
	)";

	GLES2LineTrailRenderer::GLES2LineTrailRenderer(uint32 nbSamples,float duration,float width) :
		GLES2Renderer(true),
		width(width),
		degeneratedColor(0x00000000)
	{
		setNbSamples(nbSamples);
		setDuration(duration);
		initShader();
	}

	GLES2LineTrailRenderer::GLES2LineTrailRenderer(const GLES2LineTrailRenderer& renderer) :
		GLES2Renderer(renderer),
		width(renderer.width),
		degeneratedColor(renderer.degeneratedColor),
		nbSamples(renderer.nbSamples),
		duration(renderer.duration)
	{
		initShader();
	}

	void GLES2LineTrailRenderer::initShader()
	{
		initBaseShader(VERTEX_SOURCE, FRAGMENT_SOURCE);
	}

	void GLES2LineTrailRenderer::setNbSamples(uint32 nbSamples)
	{
		SPK_ASSERT(nbSamples >= 2,"GLES2LineTrailRenderer::setNbSamples(uint32) - The number of samples cannot be less than 2");
		this->nbSamples = nbSamples;
	}

	void GLES2LineTrailRenderer::setDuration(float duration)
	{
		SPK_ASSERT(nbSamples > 0.0f,"GLES2LineTrailRenderer::setDuration(float) - The duration cannot be less or equal to 0.0f");
		this->duration = duration;
	}

	void GLES2LineTrailRenderer::enableBlending(bool blendingEnabled)
	{
		if (!blendingEnabled)
			SPK_LOG_WARNING("GLES2LineTrailRenderer::enableBlending(bool) - The blending cannot be disabled for this renderer");
		GLES2Renderer::enableBlending(true);
	}

	void GLES2LineTrailRenderer::createData(DataSet& dataSet,const Group& group) const
	{
		dataSet.init(NB_DATA);
		dataSet.setData(VERTEX_BUFFER_INDEX,SPK_NEW(Vector3DArrayData,group.getCapacity(),nbSamples + 2));
		dataSet.setData(COLOR_BUFFER_INDEX,SPK_NEW(ColorArrayData,group.getCapacity(),nbSamples + 2));
		dataSet.setData(AGE_DATA_INDEX,SPK_NEW(FloatArrayData,group.getCapacity(),nbSamples));
		dataSet.setData(START_ALPHA_DATA_INDEX,SPK_NEW(ArrayData<unsigned char>,group.getCapacity(),nbSamples));

		// Inits the buffers
		for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
			init(*particleIt,&dataSet);
	}

	void GLES2LineTrailRenderer::checkData(DataSet& dataSet,const Group& group) const
	{
		// If the number of samples has changed, we must recreate the buffers
		if (SPK_GET_DATA(FloatArrayData,&dataSet,AGE_DATA_INDEX).getSizePerParticle() != nbSamples)
		{
			dataSet.destroyAllData();
			createData(dataSet,group);
		}
	}

	void GLES2LineTrailRenderer::init(const Particle& particle,DataSet* dataSet) const
	{
		uint32 index = particle.getIndex();
		Vector3D* vertexIt = SPK_GET_DATA(Vector3DArrayData,dataSet,VERTEX_BUFFER_INDEX).getParticleData(index);
		Color* colorIt = SPK_GET_DATA(ColorArrayData,dataSet,COLOR_BUFFER_INDEX).getParticleData(index);
		float* ageIt = SPK_GET_DATA(FloatArrayData,dataSet,AGE_DATA_INDEX).getParticleData(index);
		unsigned char* startAlphaIt = SPK_GET_DATA(ArrayData<unsigned char>,dataSet,START_ALPHA_DATA_INDEX).getParticleData(index);

		// Gets the particle's values
		const Vector3D& pos = particle.position();
		const Color& color = particle.getColor();
		float age = particle.getAge();

		// Inits position
		for (uint32 i = 0; i < nbSamples + 2; ++i)
			*(vertexIt++) = pos;

		// Inits color
		*(colorIt++) = degeneratedColor; // degenerate pre vertex
		for (uint32 i = 0; i < nbSamples; ++i)
			*(colorIt++) = color;
		*colorIt = degeneratedColor; // degenerate post vertex

		// Inits age
		for (uint32 i = 0; i < nbSamples; ++i)
			*(ageIt++) = age;

		// Inits start alpha
		for (uint32 i = 0; i < nbSamples; ++i)
			*(startAlphaIt++) = color.a;
	}

	void GLES2LineTrailRenderer::update(const Group& group,DataSet* dataSet) const
	{
		Vector3D* vertexIt = SPK_GET_DATA(Vector3DArrayData,dataSet,VERTEX_BUFFER_INDEX).getData();
		Color* colorIt = SPK_GET_DATA(ColorArrayData,dataSet,COLOR_BUFFER_INDEX).getData();
		float* ageIt = SPK_GET_DATA(FloatArrayData,dataSet,AGE_DATA_INDEX).getData();
		unsigned char* startAlphaIt = SPK_GET_DATA(ArrayData<unsigned char>,dataSet,START_ALPHA_DATA_INDEX).getData();

		float ageStep = duration / (nbSamples - 1);
		for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
		{
			const Particle& particle = *particleIt;
			float age = particle.getAge();

			if (age - *(ageIt + 1) >= ageStep) // shifts the data by one
			{
				std::memmove(vertexIt + 2,vertexIt + 1,(nbSamples - 1) * sizeof(Vector3D));
				std::memmove(colorIt + 2,colorIt + 1,(nbSamples - 1) * sizeof(Color));
				std::memmove(ageIt + 1,ageIt,(nbSamples - 1) * sizeof(float));
				std::memmove(startAlphaIt + 1,startAlphaIt,(nbSamples - 1) * sizeof(unsigned char));

				// post degenerated vertex copy
				std::memcpy(vertexIt + (nbSamples + 1),vertexIt + nbSamples,sizeof(Vector3D));
			}

			// Updates the current sample
			*(vertexIt++) = particle.position();
			std::memcpy(vertexIt,vertexIt - 1,sizeof(Vector3D));
			vertexIt += nbSamples + 1;

			++colorIt; // skips post degenerated vertex color
			*(colorIt++) = particle.getColor();
			*(startAlphaIt++) = particle.getColor().a;

			*(ageIt++) = age;

			// Updates alpha
			for (uint32 i = 0; i < nbSamples - 1; ++i)
			{
				float ratio = 1.0f - (age - *(ageIt++)) / duration;
				(colorIt++)->a = static_cast<unsigned char>(*(startAlphaIt++) * (ratio > 0.0f ? ratio : 0.0f));
			}
			++colorIt;
		}
	}

	void GLES2LineTrailRenderer::render(const Group& group,const DataSet* dataSet,RenderBuffer* renderBuffer) const
	{
		// RenderBuffer is not used as dataset already contains organized data for rendering
		const Vector3D* vertexBuffer = SPK_GET_DATA(const Vector3DArrayData,dataSet,VERTEX_BUFFER_INDEX).getData();
		const Color* colorBuffer = SPK_GET_DATA(const ColorArrayData,dataSet,COLOR_BUFFER_INDEX).getData();

		GLuint prog = getProgramName();
		glUseProgram(prog);

		initBlending();
		initRenderingOptions();

		// Inits lines' parameters
		glLineWidth(width);

		GLint vertexAttribLocation = getVertexAttribLocation();
		GLint colorAttribLocation = getColorAttribLocation();

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(vertexAttribLocation);
		glVertexAttribPointer(vertexAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, vertexBuffer);

		glEnableVertexAttribArray(colorAttribLocation);
		glVertexAttribPointer(colorAttribLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, colorBuffer);

		glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(group.getNbParticles()) * (nbSamples + 2));
	}

	void GLES2LineTrailRenderer::computeAABB(Vector3D& AABBMin,Vector3D& AABBMax,const Group& group,const DataSet* dataSet) const
	{
		const Vector3D* vertexIt = SPK_GET_DATA(const Vector3DArrayData,dataSet,VERTEX_BUFFER_INDEX).getData();

		for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
		{
			++vertexIt; // skips pre degenerated vertex
			for (uint32 i = 0; i < nbSamples; ++i)
			{
				AABBMin.setMin(*vertexIt);
				AABBMax.setMax(*vertexIt);
				++vertexIt;
			}
			++vertexIt; // skips post degenerated vertex
		}
	}
}}
