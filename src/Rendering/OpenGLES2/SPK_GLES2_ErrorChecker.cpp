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
#include "Rendering/OpenGLES2/SPK_GLES2_DEF.h"
#include <sstream>

namespace {
#define DEF_ERR(x) case x: return #x;

	const char* getErrorString(GLenum err) {
		switch (err) {
		DEF_ERR(GL_INVALID_ENUM)
		DEF_ERR(GL_INVALID_OPERATION)
		DEF_ERR(GL_INVALID_VALUE)
		DEF_ERR(GL_INVALID_FRAMEBUFFER_OPERATION)
		DEF_ERR(GL_INVALID_INDEX)
		default:
			return "UNKNOWN_ERROR";
		}
	}
}

namespace SPK
{
namespace GLES2
{
	void checkError(const char* fileName, int line) {
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			std::ostringstream str;
			str << fileName << ":" << line << ": " << getErrorString(err) << std::endl;
			SPK_LOG_DEBUG(str.str().c_str());
		}
	}
}
}
