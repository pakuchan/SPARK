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

#ifndef H_SPK_GLES2_BUFFER
#define H_SPK_GLES2_BUFFER

#include "Rendering/OpenGLES2/SPK_GLES2_DEF.h"

namespace SPK
{
namespace GLES2
{
	class SPK_GLES2_PREFIX GLES2Buffer : public RenderBuffer
	{
	public :

		GLES2Buffer(uint32 nbVertices,uint32 nbTexCoords = 0);
		~GLES2Buffer();

		void positionAtStart();

		void setNextVertex(const Vector3D& vertex);

		void setNextColor(const Color& color);
		void skipNextColors(uint32 nb);

		void setNextTexCoord(float texCoord);
		void skipNextTexCoords(uint32 nb);

		void setNbTexCoords(uint32 nb);
		uint32 getNbTexCoords();

		void render(GLuint primitive, uint32 nbVertices, GLint vertexAttribLocation, GLint colorAttribLocation, GLint texCoordAttribLocation);

	private :

		const uint32 nbVertices;
		uint32 nbTexCoords;

		Vector3D* vertexBuffer;
		Color* colorBuffer;
		float* texCoordBuffer;

		uint32 currentVertexIndex;
		uint32 currentColorIndex;
		uint32 currentTexCoordIndex;
	};

	inline void GLES2Buffer::positionAtStart()
	{
		currentVertexIndex = 0;
		currentColorIndex = 0;
		currentTexCoordIndex = 0;
	}

	inline void GLES2Buffer::setNextVertex(const Vector3D& vertex)
	{
		vertexBuffer[currentVertexIndex++] = vertex;
	}

	inline void GLES2Buffer::setNextColor(const Color& color)
	{
		colorBuffer[currentColorIndex++] = color;
	}

	inline void GLES2Buffer::skipNextColors(uint32 nb)
	{
		currentColorIndex += nb;
	}

	inline void GLES2Buffer::setNextTexCoord(float texCoord)
	{
		texCoordBuffer[currentTexCoordIndex++] = texCoord;
	}

	inline void GLES2Buffer::skipNextTexCoords(uint32 nb)
	{
		currentTexCoordIndex += nb;
	}

	inline uint32 GLES2Buffer::getNbTexCoords()
	{
		return nbTexCoords;
	}
}}

#endif
