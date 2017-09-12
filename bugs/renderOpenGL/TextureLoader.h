/*
 * TextureLoader.h
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#ifndef RENDEROPENGL_TEXTURELOADER_H_
#define RENDEROPENGL_TEXTURELOADER_H_

#include <string>

class TextureLoader {
public:
	static unsigned loadFromPNG(const std::string filename, int* outWidth, int* outHeight);
};

#endif /* RENDEROPENGL_TEXTURELOADER_H_ */
