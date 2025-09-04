#pragma once
#include "fbo.h"

class FBOUtils
{
public:
	static ColourDepthFBO* createColourDepthFBO(ColourDepthFrameBufferConfig cfg);
	static DepthFBO* createDepthFBO(DepthFrameBufferConfig cfg);
};
