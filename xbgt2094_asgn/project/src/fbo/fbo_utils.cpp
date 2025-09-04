#include "fbo_utils.h"

ColourDepthFBO* FBOUtils::createColourDepthFBO(ColourDepthFrameBufferConfig cfg)
{
	return new ColourDepthFBO(cfg);
}

DepthFBO* FBOUtils::createDepthFBO(DepthFrameBufferConfig cfg)
{
	return new DepthFBO(cfg);
}