/*
===========================================================================
Copyright (C) 2011 Andrei Drexler, Richard Allen, James Canete

This file is part of Reaction source code.

Reaction source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Reaction source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Reaction source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef TR_POSTPROCESS_H
#define TR_POSTPROCESS_H

#include "tr_fbo.h"

void RB_ToneMap(Framebuffer *hdrFbo, ivec4_t hdrBox, Framebuffer *ldrFbo, ivec4_t ldrBox, int autoExposure);
void RB_BokehBlur(Framebuffer *src, ivec4_t srcBox, Framebuffer *dst, ivec4_t dstBox, float blur);
void RB_SunRays(Framebuffer *srcFbo, ivec4_t srcBox, Framebuffer *dstFbo, ivec4_t dstBox);
void RB_GaussianBlur(Framebuffer *srcFbo, Framebuffer *dstFbo, float blur);

#endif
