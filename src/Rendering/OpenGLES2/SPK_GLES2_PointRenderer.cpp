//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2009 - Julien Fryer - julienfryer@gmail.com				//
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
#include "Rendering/OpenGLES2/SPK_GLES2_PointRenderer.h"
#include "Rendering/OpenGLES2/SPK_GLES2_Buffer.h"

namespace SPK
{
namespace GLES2
{
	const char* GLES2PointRenderer::VERTEX_SOURCE = R"(#version 100
		
attribute vec4 a_vertexAttrib;
attribute vec4 a_colorAttrib;
attribute vec3 a_texCoordAttrib;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_projectionMatrix;
uniform float u_pointSize;
uniform bool u_useWorldSize;
uniform vec3 u_distanceAttenuation;
uniform float u_sizeThreshold;
varying vec4 v_color;
		
float distAtten(float a, float b, float c, float d) {
	return 1.0 / (a + b*d + c*d*d);
}
		
void main() {
	vec4 Pe = u_modelViewMatrix * a_vertexAttrib;
	gl_Position = u_projectionMatrix * Pe;
	if (u_useWorldSize) {
		float dist = length(Pe);
		float derivedSize = u_pointSize * sqrt(distAtten(u_distanceAttenuation.x, u_distanceAttenuation.y, u_distanceAttenuation.z, dist));
		float diameter = u_sizeThreshold;
		float alpha = a_colorAttrib.w;
		if (derivedSize >= u_sizeThreshold) {
			diameter = derivedSize;
		} else {
			float s = (derivedSize / u_sizeThreshold);
			alpha *= s * s;
		}
		gl_PointSize = diameter;
		v_color = vec4(a_colorAttrib.xyz, alpha);
	} else {
		gl_PointSize = u_pointSize;
		v_color = a_colorAttrib;
	}
}
	)";

	const char* GLES2PointRenderer::FRAGMENT_SOURCE = R"(#version 100
		
precision highp float;
		
uniform sampler2D u_texSampler;
uniform int u_useTexture;
uniform bool u_useCircularPoint;
uniform bool u_alphaTestEnabled;
uniform float u_alphaTestThreshold;
varying vec4 v_color;
		
void main() {
	vec4 color = vec4(1.0);
		
	if (bool(u_useTexture)) {
		color = texture2D(u_texSampler, gl_PointCoord);
	}
		
	gl_FragColor = v_color * color;
		
	if (u_useCircularPoint && length(gl_PointCoord - vec2(0.5)) > 0.5)
		discard;
	if (u_alphaTestEnabled && gl_FragColor.a <= u_alphaTestThreshold)
		discard;
}
	)";

	float GLES2PointRenderer::pixelPerUnit = 1024.0f;

	GLES2PointRenderer::GLES2PointRenderer(float screenSize) :
		GLES2Renderer(false),
		PointRenderBehavior(POINT_TYPE_SQUARE, screenSize),
		textureIndex(0),
		useWorldSizeUniformLocation(-1),
		sizeThresholdUniformLocation(-1),
		distanceAttenuationUniformLocation(-1),
		useTextureUniformLocation(-1),
		texSamplerUniformLocation(-1),
		pointSizeUniformLocation(-1),
		useCircularPointUniformLocation(-1)
	{
		initShader();
	}

	GLES2PointRenderer::GLES2PointRenderer(const GLES2PointRenderer& renderer) :
		GLES2Renderer(renderer),
		PointRenderBehavior(renderer),
		textureIndex(renderer.textureIndex),
		useWorldSizeUniformLocation(-1),
		sizeThresholdUniformLocation(-1),
		distanceAttenuationUniformLocation(-1),
		useTextureUniformLocation(-1),
		texSamplerUniformLocation(-1),
		pointSizeUniformLocation(-1),
		useCircularPointUniformLocation(-1)
	{
		initShader();
	}

	void GLES2PointRenderer::initShader()
	{
		initBaseShader(VERTEX_SOURCE, FRAGMENT_SOURCE);

		GLuint prog = getProgramName();
		useWorldSizeUniformLocation = glGetUniformLocation(prog, "u_useWorldSize");
		sizeThresholdUniformLocation = glGetUniformLocation(prog, "u_sizeThreshold");
		distanceAttenuationUniformLocation = glGetUniformLocation(prog, "u_distanceAttenuation");
		useTextureUniformLocation = glGetUniformLocation(prog, "u_useTexture");
		texSamplerUniformLocation = glGetUniformLocation(prog, "u_texSampler");
		pointSizeUniformLocation = glGetUniformLocation(prog, "u_pointSize");
		useCircularPointUniformLocation = glGetUniformLocation(prog, "u_useCircularPoint");
	}

	bool GLES2PointRenderer::setType(PointType type)
	{
		this->type = type;
		return true;
	}

	bool GLES2PointRenderer::enableWorldSize(bool worldSizeEnabled)
	{
		worldSize = worldSizeEnabled;
		if (worldSize != worldSizeEnabled)
			SPK_LOG_WARNING("GLES2PointRenderer::enableWorldSize(true) - World size for points is not available on the hardware");
		return worldSize;
	}

	RenderBuffer* GLES2PointRenderer::attachRenderBuffer(const Group& group) const
	{
		return SPK_NEW(GLES2Buffer, group.getCapacity() << 1);
	}

	void GLES2PointRenderer::render(const Group& group,const DataSet* dataSet,RenderBuffer* renderBuffer) const
	{
		SPK_ASSERT(renderBuffer != NULL, "GLLinesRenderer::render(const Group&,const DataSet*,RenderBuffer*) - renderBuffer must not be NULL");
		GLES2Buffer& buffer = static_cast<GLES2Buffer&>(*renderBuffer);
		buffer.positionAtStart();

		GLuint prog = getProgramName();
		glUseProgram(prog);

		// Sets the different states for rendering
		initBlending();
		initRenderingOptions();

		for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
		{
			buffer.setNextVertex(particleIt->position());
			buffer.setNextColor(particleIt->getColor());
		}

		switch(type)
		{
		case POINT_TYPE_SQUARE :
			glUniform1i(useTextureUniformLocation, GL_FALSE);
			glUniform1i(useCircularPointUniformLocation, GL_FALSE);
			break;

		case POINT_TYPE_SPRITE :
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureIndex);
			glUniform1i(texSamplerUniformLocation, 0);
			glUniform1i(useTextureUniformLocation, GL_TRUE);
			glUniform1i(useCircularPointUniformLocation, GL_FALSE);
			break;

		case POINT_TYPE_CIRCLE :
			glUniform1i(useTextureUniformLocation, GL_FALSE);
			glUniform1i(useCircularPointUniformLocation, GL_TRUE);
			break;
		}

		if (worldSize)
		{
			// derived size = size * sqrt(1 / (A + B * distance + C * distance))
			const float POINT_SIZE_CURRENT = 32.0f;
			const float POINT_SIZE_MIN = 1.0f;
			const float POINT_SIZE_MAX = 1024.0f;
			const float sqrtC = POINT_SIZE_CURRENT / (group.getGraphicalRadius() * worldScale * 2.0f * pixelPerUnit);
			const float QUADRATIC_WORLD[3] = {0.0f,0.0f,sqrtC * sqrtC}; // A = 0; B = 0; C = (POINT_SIZE_CURRENT / (size * pixelPerUnit))
			glUniform3fv(distanceAttenuationUniformLocation, 1, QUADRATIC_WORLD);
			glUniform1f(sizeThresholdUniformLocation, 1.0f);
			glUniform1i(useWorldSizeUniformLocation, GL_TRUE);
			glUniform1f(pointSizeUniformLocation, POINT_SIZE_CURRENT);
		}
		else
		{
			glUniform1i(useWorldSizeUniformLocation, GL_FALSE);
			glUniform1f(pointSizeUniformLocation, screenSize);
		}

		buffer.render(GL_POINTS, group.getNbParticles(), getVertexAttribLocation(), getColorAttribLocation(), getTexCoordAttribLocation());
	}

	void GLES2PointRenderer::computeAABB(Vector3D& AABBMin,Vector3D& AABBMax,const Group& group,const DataSet* dataSet) const
	{
		if (isWorldSizeEnabled())
		{
			float radius = group.getGraphicalRadius();
			for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
			{
				AABBMin.setMin(particleIt->position() - radius);
				AABBMax.setMax(particleIt->position() + radius);
			}
		}
		else
			for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
			{
				AABBMin.setMin(particleIt->position());
				AABBMax.setMax(particleIt->position());
			}
	}
}}
