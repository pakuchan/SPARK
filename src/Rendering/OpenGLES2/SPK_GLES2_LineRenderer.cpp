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
#include "Rendering/OpenGLES2/SPK_GLES2_LineRenderer.h"

namespace SPK
{
namespace GLES2
{
	const char* GLES2LineRenderer::VERTEX_SOURCE = R"(#version 100

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

	const char* GLES2LineRenderer::FRAGMENT_SOURCE = R"(#version 100

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

	GLES2LineRenderer::GLES2LineRenderer(float length,float width) :
		GLES2Renderer(false),
		LineRenderBehavior(length,width)
	{
		initShader();
	}

	GLES2LineRenderer::GLES2LineRenderer(const GLES2LineRenderer& renderer) :
		GLES2Renderer(renderer),
		LineRenderBehavior(renderer)
	{
		initShader();
	}

	void GLES2LineRenderer::initShader()
	{
		initBaseShader(VERTEX_SOURCE, FRAGMENT_SOURCE);
	}

	RenderBuffer* GLES2LineRenderer::attachRenderBuffer(const Group& group) const
	{
		return SPK_NEW(GLES2Buffer,group.getCapacity() << 1);
	}

	void GLES2LineRenderer::render(const Group& group,const DataSet* dataSet,RenderBuffer* renderBuffer) const
	{
		SPK_ASSERT(renderBuffer != NULL,"GLLinesRenderer::render(const Group&,const DataSet*,RenderBuffer*) - renderBuffer must not be NULL");
		GLES2Buffer& buffer = static_cast<GLES2Buffer&>(*renderBuffer);
		buffer.positionAtStart();

		GLuint prog = getProgramName();
		glUseProgram(prog);

		initBlending();
		initRenderingOptions();

		glLineWidth(width);

		for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
		{
			const Particle& particle = *particleIt;

			buffer.setNextVertex(particle.position());
			buffer.setNextVertex(particle.position() + particle.velocity() * length);

			buffer.setNextColor(particle.getColor());
			buffer.setNextColor(particle.getColor());
		}

		buffer.render(GL_LINES, group.getNbParticles() << 1, getVertexAttribLocation(), getColorAttribLocation(), getTexCoordAttribLocation());
	}

	void GLES2LineRenderer::computeAABB(Vector3D& AABBMin,Vector3D& AABBMax,const Group& group,const DataSet* dataSet) const
	{
		for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
		{
			Vector3D v = particleIt->position() + particleIt->velocity() * length;
			AABBMin.setMin(particleIt->position());
			AABBMin.setMin(v);
			AABBMax.setMax(particleIt->position());
			AABBMax.setMax(v);
		}
	}
}}
