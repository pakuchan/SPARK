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
#include "Rendering/OpenGLES2/SPK_GLES2_QuadRenderer.h"
#include "SPK_GLES2_ErrorChecker.h"

namespace SPK
{
namespace GLES2
{
	const char* GLES2QuadRenderer::VERTEX_SOURCE = R"(#version 100

attribute vec4 a_vertexAttrib;
attribute vec4 a_colorAttrib;
attribute vec3 a_texCoordAttrib;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_projectionMatrix;
varying vec4 v_color;
varying vec3 v_texCoord;

void main() {
	mat4 mvp = u_projectionMatrix * u_modelViewMatrix;
	gl_Position = mvp * a_vertexAttrib;
	v_color = a_colorAttrib;
	v_texCoord = a_texCoordAttrib;
}
	)";

	const char* GLES2QuadRenderer::FRAGMENT_SOURCE = R"(#version 100

precision highp float;

uniform sampler2D u_texSampler;
uniform int u_useTexture;
uniform bool u_alphaTestEnabled;
uniform float u_alphaTestThreshold;
varying vec4 v_color;
varying vec3 v_texCoord;

void main() {
	vec4 color = vec4(1.0);

	if (bool(u_useTexture)) {
		color = texture2D(u_texSampler, v_texCoord.xy);
	} else {
		color = v_color;
	}

	gl_FragColor = v_color * color;

	if (u_alphaTestEnabled && gl_FragColor.a <= u_alphaTestThreshold)
		discard;
}
	)";

	GLES2QuadRenderer::GLES2QuadRenderer(float scaleX, float scaleY) :
		GLES2Renderer(false),
		QuadRenderBehavior(scaleX, scaleY),
		Oriented3DRenderBehavior(),
		textureIndex(0),
		useTextureUniformLocation(-1),
		texSamplerUniformLocation(-1)
	{
		initShader();
	}

	GLES2QuadRenderer::GLES2QuadRenderer(const GLES2QuadRenderer& renderer) :
		GLES2Renderer(renderer),
		QuadRenderBehavior(renderer),
		Oriented3DRenderBehavior(renderer),
		textureIndex(renderer.textureIndex),
		useTextureUniformLocation(-1),
		texSamplerUniformLocation(-1)
	{
		initShader();
	}

	void GLES2QuadRenderer::initShader()
	{
		initBaseShader(VERTEX_SOURCE, FRAGMENT_SOURCE);
		checkError(__FILE__, __LINE__);

		GLuint prog = getProgramName();
		useTextureUniformLocation = glGetUniformLocation(prog, "u_useTexture");
		texSamplerUniformLocation = glGetUniformLocation(prog, "u_texSampler");
	}

	bool GLES2QuadRenderer::setTexturingMode(TextureMode mode)
	{
		if (mode == TEXTURE_MODE_3D)
			return false;

		texturingMode = mode;
		return true;
	}

	RenderBuffer* GLES2QuadRenderer::attachRenderBuffer(const Group & group) const
	{
		return SPK_NEW(GLES2Buffer, group.getCapacity() * 6);
	}

	void GLES2QuadRenderer::render(const Group & group, const DataSet * dataSet, RenderBuffer * renderBuffer) const
	{
		SPK_ASSERT(renderBuffer != NULL, "GLES2QuadRenderer::render(const Group&,const DataSet*,RenderBuffer*) - renderBuffer must not be NULL");
		checkError(__FILE__, __LINE__);
		GLES2Buffer& buffer = static_cast<GLES2Buffer&>(*renderBuffer);
		buffer.positionAtStart(); // Repositions all the buffers at the start

		GLuint prog = getProgramName();
		glUseProgram(prog);

		float oldModelView[16];
		for (int i = 0; i < 16; ++i)
			oldModelView[i] = modelView[i];
		std::memcpy(modelView, getModelViewMatrix(), sizeof(modelView));
		for (int i = 0; i < 16; ++i)
			if (oldModelView[i] != modelView[i])
			{
				invertModelView();
				break;
			}

		initBlending();
		checkError(__FILE__, __LINE__);
		initRenderingOptions();
		checkError(__FILE__, __LINE__);

		switch (texturingMode)
		{
		case TEXTURE_MODE_2D:
			// Creates and inits the 2D TexCoord buffer if necessary
			if (buffer.getNbTexCoords() != 2)
			{
				buffer.setNbTexCoords(2);
				if (!group.isEnabled(PARAM_TEXTURE_INDEX))
				{
					float t[8] = { 1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f };
					for (uint32 i = 0; i < group.getCapacity() << 3; ++i)
						buffer.setNextTexCoord(t[i & 7]);
				}
			}

			// Binds the texture
			glUniform1i(useTextureUniformLocation, GL_TRUE);
			glUniform1i(texSamplerUniformLocation, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureIndex);

			// Selects the correct function
			if (!group.isEnabled(PARAM_TEXTURE_INDEX))
			{
				if (!group.isEnabled(PARAM_ANGLE))
					renderParticle = &GLES2QuadRenderer::render2D;
				else
					renderParticle = &GLES2QuadRenderer::render2DRot;
			}
			else
			{
				if (!group.isEnabled(PARAM_ANGLE))
					renderParticle = &GLES2QuadRenderer::render2DAtlas;
				else
					renderParticle = &GLES2QuadRenderer::render2DAtlasRot;
			}
			break;

		case TEXTURE_MODE_NONE:
			if (buffer.getNbTexCoords() != 0)
				buffer.setNbTexCoords(0);

			glUniform1i(useTextureUniformLocation, GL_FALSE);

			// Selects the correct function
			if (!group.isEnabled(PARAM_ANGLE))
				renderParticle = &GLES2QuadRenderer::render2D;
			else
				renderParticle = &GLES2QuadRenderer::render2DRot;
			break;
		}

		bool globalOrientation = precomputeOrientation3D(
			group,
			Vector3D(-invModelView[8],-invModelView[9],-invModelView[10]),
			Vector3D(invModelView[4],invModelView[5],invModelView[6]),
			Vector3D(invModelView[12],invModelView[13],invModelView[14]));

		// Fills the buffers
		if (globalOrientation)
		{
			computeGlobalOrientation3D(group);

			for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
				(this->*renderParticle)(*particleIt,buffer);
		}
		else
		{
			for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
			{
				computeSingleOrientation3D(*particleIt);
				(this->*renderParticle)(*particleIt,buffer);
			}
		}

		checkError(__FILE__, __LINE__);
		buffer.render(GL_TRIANGLES, group.getNbParticles() * 6, getVertexAttribLocation(), getColorAttribLocation(), getTexCoordAttribLocation());
	}

	void GLES2QuadRenderer::computeAABB(Vector3D & AABBMin, Vector3D & AABBMax, const Group & group, const DataSet * dataSet) const
	{
		float diagonal = group.getGraphicalRadius() * std::sqrt(scaleX * scaleX + scaleY * scaleY);
		Vector3D diagV(diagonal, diagonal, diagonal);

		if (group.isEnabled(PARAM_SCALE))
			for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
			{
				Vector3D scaledDiagV = diagV * particleIt->getParamNC(PARAM_SCALE);
				AABBMin.setMin(particleIt->position() - scaledDiagV);
				AABBMax.setMax(particleIt->position() + scaledDiagV);
			}
		else
		{
			for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
			{
				AABBMin.setMin(particleIt->position());
				AABBMax.setMax(particleIt->position());
			}
			AABBMin -= diagV;
			AABBMax += diagV;
		}
	}

	void GLES2QuadRenderer::render2D(const Particle & particle, GLES2Buffer & renderBuffer) const
	{
		scaleQuadVectors(particle, scaleX, scaleY);
		GLCallColorAndVertex(particle, renderBuffer);
	}

	void GLES2QuadRenderer::render2DRot(const Particle & particle, GLES2Buffer & renderBuffer) const
	{
		rotateAndScaleQuadVectors(particle, scaleX, scaleY);
		GLCallColorAndVertex(particle, renderBuffer);
	}

	void GLES2QuadRenderer::render3D(const Particle & particle, GLES2Buffer & renderBuffer) const
	{
		scaleQuadVectors(particle, scaleX, scaleY);
		GLCallColorAndVertex(particle, renderBuffer);
		GLCallTexture3D(particle, renderBuffer);
	}

	void GLES2QuadRenderer::render3DRot(const Particle & particle, GLES2Buffer & renderBuffer) const
	{
		rotateAndScaleQuadVectors(particle, scaleX, scaleY);
		GLCallColorAndVertex(particle, renderBuffer);
		GLCallTexture3D(particle, renderBuffer);
	}

	void GLES2QuadRenderer::render2DAtlas(const Particle & particle, GLES2Buffer & renderBuffer) const
	{
		scaleQuadVectors(particle, scaleX, scaleY);
		GLCallColorAndVertex(particle, renderBuffer);
		GLCallTexture2DAtlas(particle, renderBuffer);
	}

	void GLES2QuadRenderer::render2DAtlasRot(const Particle & particle, GLES2Buffer & renderBuffer) const
	{
		rotateAndScaleQuadVectors(particle, scaleX, scaleY);
		GLCallColorAndVertex(particle, renderBuffer);
		GLCallTexture2DAtlas(particle, renderBuffer);
	}
}
}
