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
#include "Rendering/OpenGLES2/SPK_GLES2_Buffer.h"
#include "SPK_GLES2_ErrorChecker.h"

namespace SPK
{
namespace GLES2
{
	GLES2Buffer::GLES2Buffer(uint32 nbVertices,uint32 nbTexCoords) :
		nbVertices(nbVertices),
		nbTexCoords(nbTexCoords),
		texCoordBuffer(NULL)
	{
		SPK_ASSERT(nbVertices > 0,"GLES2Buffer::GLES2Buffer(uint32,uint32) - The number of vertices cannot be 0");

		vertexBuffer = SPK_NEW_ARRAY(Vector3D,nbVertices);
		colorBuffer = SPK_NEW_ARRAY(Color,nbVertices);

		if (nbTexCoords > 0)
		{
			texCoordBuffer = SPK_NEW_ARRAY(float,nbVertices * nbTexCoords);
		}
		checkError(__FILE__, __LINE__);
	}

	GLES2Buffer::~GLES2Buffer()
	{
		SPK_DELETE_ARRAY(vertexBuffer);
		SPK_DELETE_ARRAY(colorBuffer);
		SPK_DELETE_ARRAY(texCoordBuffer);
	}

	void GLES2Buffer::setNbTexCoords(uint32 nb)
	{
		if (nbTexCoords != nb)
		{
			nbTexCoords = nb;
			SPK_DELETE_ARRAY(texCoordBuffer);
			if (nbTexCoords > 0)
				texCoordBuffer = SPK_NEW_ARRAY(float,nbVertices * nbTexCoords);
		}
	}

	void GLES2Buffer::render(GLuint primitive, uint32 nbVertices, GLint vertexAttribLocation, GLint colorAttribLocation, GLint texCoordAttribLocation)
	{
		if (nbVertices == 0)
			return;

		checkError(__FILE__, __LINE__);
		glEnableVertexAttribArray(vertexAttribLocation);
		checkError(__FILE__, __LINE__);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glVertexAttribPointer(vertexAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, vertexBuffer);
		glEnableVertexAttribArray(colorAttribLocation);
		checkError(__FILE__, __LINE__);
		glVertexAttribPointer(colorAttribLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, colorBuffer);
		if (texCoordAttribLocation >= 0 && nbTexCoords > 0)
		{
			glEnableVertexAttribArray(texCoordAttribLocation);
			checkError(__FILE__, __LINE__);
			glVertexAttribPointer(texCoordAttribLocation, static_cast<GLint>(nbTexCoords), GL_FLOAT, GL_FALSE, 0, texCoordBuffer);
		}
		checkError(__FILE__, __LINE__);

		glDrawArrays(primitive,0,static_cast<GLsizei>(nbVertices));
		checkError(__FILE__, __LINE__);
	}
}}
