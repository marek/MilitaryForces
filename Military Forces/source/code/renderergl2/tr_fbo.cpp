/*
===========================================================================
Copyright (C) 2006 Kirk Barnes
Copyright (C) 2006-2008 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_fbo.c
#include "tr_local.h"

#include "tr_dsa.h"

/*
=============
R_CheckFBO
=============
*/
bool Framebuffer::r_CheckFBO() const
{
	GLenum code = qglCheckNamedFramebufferStatusEXT(this->frameBuffer, GL_FRAMEBUFFER);

	if(code == GL_FRAMEBUFFER_COMPLETE)
		return true;

	// an error occurred
	switch (code)
	{
		case GL_FRAMEBUFFER_UNSUPPORTED:
			ri.Printf(PRINT_WARNING, "R_CheckFBO: (%s) Unsupported framebuffer format\n", this->name);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			ri.Printf(PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete attachment\n", this->name);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			ri.Printf(PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete, missing attachment\n", this->name);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			ri.Printf(PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete, missing draw buffer\n", this->name);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			ri.Printf(PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete, missing read buffer\n", this->name);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			ri.Printf(PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete multisample\n", this->name);
			break;

		default:
			ri.Printf(PRINT_WARNING, "R_CheckFBO: (%s) unknown error 0x%X\n", this->name, code);
			break;
	}

	return false;
}

/*
============
FBO_Create
============
*/
Framebuffer          *FBO_Create(const char *name, int width, int height)
{
	Framebuffer          *fbo;

	if(strlen(name) >= MAX_QPATH)
	{
		ri.Error(ERR_DROP, "FBO_Create: \"%s\" is too long", name);
	}

	if(width <= 0 || width > glRefConfig.maxRenderbufferSize)
	{
		ri.Error(ERR_DROP, "FBO_Create: bad width %i", width);
	}

	if(height <= 0 || height > glRefConfig.maxRenderbufferSize)
	{
		ri.Error(ERR_DROP, "FBO_Create: bad height %i", height);
	}

	if(tr.numFBOs == MAX_FBOS)
	{
		ri.Error(ERR_DROP, "FBO_Create: MAX_FBOS hit");
	}

	fbo = tr.fbos[tr.numFBOs] = static_cast<Framebuffer *>(ri.hunkAlloc(sizeof(*fbo), h_low));
	Q_strncpyz(fbo->name, name, sizeof(fbo->name));
	fbo->index = tr.numFBOs++;
	fbo->width = width;
	fbo->height = height;

	qglGenFramebuffers(1, &fbo->frameBuffer);

	return fbo;
}

/*
=================
FBO_CreateBuffer
=================
*/
void Framebuffer::createBuffer( int format, int index, int multisample )
{
	uint32_t *pRenderBuffer;
	GLenum attachment;
	bool absent;

	switch(format)
	{
		case GL_RGB:
		case GL_RGBA:
		case GL_RGB8:
		case GL_RGBA8:
		case GL_RGB16F_ARB:
		case GL_RGBA16F_ARB:
		case GL_RGB32F_ARB:
		case GL_RGBA32F_ARB:
			this->colorFormat = format;
			pRenderBuffer = &this->colorBuffers[index];
			attachment = GL_COLOR_ATTACHMENT0 + index;
			break;

		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_COMPONENT16_ARB:
		case GL_DEPTH_COMPONENT24_ARB:
		case GL_DEPTH_COMPONENT32_ARB:
			this->depthFormat = format;
			pRenderBuffer = &this->depthBuffer;
			attachment = GL_DEPTH_ATTACHMENT;
			break;

		case GL_STENCIL_INDEX:
		case GL_STENCIL_INDEX1:
		case GL_STENCIL_INDEX4:
		case GL_STENCIL_INDEX8:
		case GL_STENCIL_INDEX16:
			this->stencilFormat = format;
			pRenderBuffer = &this->stencilBuffer;
			attachment = GL_STENCIL_ATTACHMENT;
			break;

		case GL_DEPTH_STENCIL:
		case GL_DEPTH24_STENCIL8:
			this->packedDepthStencilFormat = format;
			pRenderBuffer = &this->packedDepthStencilBuffer;
			attachment = 0; // special for stencil and depth
			break;

		default:
			ri.Printf(PRINT_WARNING, "FBO_CreateBuffer: invalid format %d\n", format);
			return;
	}

	absent = *pRenderBuffer == 0;
	if (absent) {
		qglGenRenderbuffers(1, pRenderBuffer);

		// workaround AMD Windows driver requiring bind to create renderbuffer
		GL_BindRenderbuffer(*pRenderBuffer);
	}

	if (multisample && glRefConfig.framebufferMultisample)
		qglNamedRenderbufferStorageMultisampleEXT(*pRenderBuffer, multisample, format, this->width, this->height);
	else
		qglNamedRenderbufferStorageEXT(*pRenderBuffer, format, this->width, this->height);

	if(absent)
	{
		if (attachment == 0)
		{
			qglNamedFramebufferRenderbufferEXT(this->frameBuffer, GL_DEPTH_ATTACHMENT,   GL_RENDERBUFFER, *pRenderBuffer);
			qglNamedFramebufferRenderbufferEXT(this->frameBuffer, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *pRenderBuffer);
		}
		else
		{
			qglNamedFramebufferRenderbufferEXT(this->frameBuffer, attachment, GL_RENDERBUFFER, *pRenderBuffer);
		}
	}
}


/*
=================
FBO_AttachImage
=================
*/
void Framebuffer::attachImage( image_t *image, GLenum attachment, GLuint cubemapside )
{
	GLenum target = GL_TEXTURE_2D;
	int index;

	if (image->flags & IMGFLAG_CUBEMAP)
		target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + cubemapside;

	qglNamedFramebufferTexture2DEXT(this->frameBuffer, attachment, target, image->texnum, 0);
	index = attachment - GL_COLOR_ATTACHMENT0;
	if (index >= 0 && index <= 15)
		this->colorImage[index] = image;
}


/*
============
FBO_Bind
============
*/
void FBO_Bind(Framebuffer * fbo)
{
	if (!glRefConfig.framebufferObject)
	{
		ri.Printf(PRINT_WARNING, "FBO_Bind() called without framebuffers enabled!\n");
		return;
	}

	if (glState.currentFBO == fbo)
		return;
		
	if (r_logFile->integer)
	{
		// don't just call LogComment, or we will get a call to va() every frame!
		GLimp_LogComment(va("--- FBO_Bind( %s ) ---\n", fbo ? fbo->name : "NULL"));
	}

	GL_BindFramebuffer(GL_FRAMEBUFFER, fbo ? fbo->frameBuffer : 0);
	glState.currentFBO = fbo;
}

/*
============
FBO_Init
============
*/
void FBO_Init()
{
	int             i;
	int             hdrFormat, multisample = 0;

	ri.Printf(PRINT_ALL, "------- FBO_Init -------\n");

	if(!glRefConfig.framebufferObject)
		return;

	tr.numFBOs = 0;

	GL_CheckErrors();

	R_IssuePendingRenderCommands();

	hdrFormat = GL_RGBA8;
	if (r_hdr->integer && glRefConfig.textureFloat)
		hdrFormat = GL_RGBA16F_ARB;

	if (glRefConfig.framebufferMultisample)
		qglGetIntegerv(GL_MAX_SAMPLES, &multisample);

	if (r_ext_framebuffer_multisample->integer < multisample)
		multisample = r_ext_framebuffer_multisample->integer;

	if (multisample < 2 || !glRefConfig.framebufferBlit)
		multisample = 0;

	if (multisample != r_ext_framebuffer_multisample->integer)
		ri.Cvar_SetValue("r_ext_framebuffer_multisample", static_cast<float>(multisample));
	
	// only create a render FBO if we need to resolve MSAA or do HDR
	// otherwise just render straight to the screen (tr.renderFbo = NULL)
	if (multisample && glRefConfig.framebufferMultisample)
	{
		tr.renderFbo = FBO_Create("_render", tr.renderDepthImage->width, tr.renderDepthImage->height);
		tr.renderFbo->createBuffer( hdrFormat, 0, multisample );
		tr.renderFbo->createBuffer( GL_DEPTH_COMPONENT24, 0, multisample );
		tr.renderFbo->r_CheckFBO();

		tr.msaaResolveFbo = FBO_Create("_msaaResolve", tr.renderDepthImage->width, tr.renderDepthImage->height);
		tr.msaaResolveFbo->attachImage( tr.renderImage, GL_COLOR_ATTACHMENT0, 0 );
		tr.msaaResolveFbo->attachImage( tr.renderDepthImage, GL_DEPTH_ATTACHMENT, 0 );
		tr.msaaResolveFbo->r_CheckFBO();
	}
	else if (r_hdr->integer)
	{
		tr.renderFbo = FBO_Create("_render", tr.renderDepthImage->width, tr.renderDepthImage->height);
		tr.renderFbo->attachImage( tr.renderImage, GL_COLOR_ATTACHMENT0, 0 );
		tr.renderFbo->attachImage( tr.renderDepthImage, GL_DEPTH_ATTACHMENT, 0 );
		tr.renderFbo->r_CheckFBO();
	}

	// clear render buffer
	// this fixes the corrupt screen bug with r_hdr 1 on older hardware
	if (tr.renderFbo)
	{
		GL_BindFramebuffer(GL_FRAMEBUFFER, tr.renderFbo->frameBuffer);
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	if (tr.screenScratchImage)
	{
		tr.screenScratchFbo = FBO_Create("screenScratch", tr.screenScratchImage->width, tr.screenScratchImage->height);
		tr.screenScratchFbo->attachImage( tr.screenScratchImage, GL_COLOR_ATTACHMENT0, 0 );
		tr.screenScratchFbo->attachImage( tr.renderDepthImage, GL_DEPTH_ATTACHMENT, 0 );
		tr.screenScratchFbo->r_CheckFBO();
	}

	if (tr.sunRaysImage)
	{
		tr.sunRaysFbo = FBO_Create("_sunRays", tr.renderDepthImage->width, tr.renderDepthImage->height);
		tr.sunRaysFbo->attachImage( tr.sunRaysImage, GL_COLOR_ATTACHMENT0, 0 );
		tr.sunRaysFbo->attachImage( tr.renderDepthImage, GL_DEPTH_ATTACHMENT, 0 );
		tr.sunRaysFbo->r_CheckFBO();
	}

	if (MAX_DRAWN_PSHADOWS && tr.pshadowMaps[0])
	{
		for( i = 0; i < MAX_DRAWN_PSHADOWS; i++)
		{
			tr.pshadowFbos[i] = FBO_Create(va("_shadowmap%d", i), tr.pshadowMaps[i]->width, tr.pshadowMaps[i]->height);
			// FIXME: this next line wastes 16mb with 16x512x512 sun shadow maps, skip if OpenGL 4.3+ or ARB_framebuffer_no_attachments
			tr.pshadowFbos[i]->createBuffer( GL_RGBA8, 0, 0 );
			tr.pshadowFbos[i]->attachImage( tr.pshadowMaps[i], GL_DEPTH_ATTACHMENT, 0 );
			tr.pshadowFbos[i]->r_CheckFBO();
		}
	}

	if (tr.sunShadowDepthImage[0])
	{
		for (i = 0; i < 4; i++)
		{
			tr.sunShadowFbo[i] = FBO_Create("_sunshadowmap", tr.sunShadowDepthImage[i]->width, tr.sunShadowDepthImage[i]->height);
			// FIXME: this next line wastes 16mb with 4x1024x1024 sun shadow maps, skip if OpenGL 4.3+ or ARB_framebuffer_no_attachments
			// This at least gets sun shadows working on older GPUs (Intel)
			tr.sunShadowFbo[i]->createBuffer( GL_RGBA8, 0, 0 );
			tr.sunShadowFbo[i]->attachImage( tr.sunShadowDepthImage[i], GL_DEPTH_ATTACHMENT, 0 );
			tr.sunShadowFbo[i]->r_CheckFBO();
		}
	}

	if (tr.screenShadowImage)
	{
		tr.screenShadowFbo = FBO_Create("_screenshadow", tr.screenShadowImage->width, tr.screenShadowImage->height);
		tr.screenShadowFbo->attachImage( tr.screenShadowImage, GL_COLOR_ATTACHMENT0, 0 );
		tr.screenShadowFbo->r_CheckFBO();
	}

	if (tr.textureScratchImage[0])
	{
		for (i = 0; i < 2; i++)
		{
			tr.textureScratchFbo[i] = FBO_Create(va("_texturescratch%d", i), tr.textureScratchImage[i]->width, tr.textureScratchImage[i]->height);
			tr.textureScratchFbo[i]->attachImage( tr.textureScratchImage[i], GL_COLOR_ATTACHMENT0, 0 );
			tr.textureScratchFbo[i]->r_CheckFBO();
		}
	}

	if (tr.calcLevelsImage)
	{
		tr.calcLevelsFbo = FBO_Create("_calclevels", tr.calcLevelsImage->width, tr.calcLevelsImage->height);
		tr.calcLevelsFbo->attachImage( tr.calcLevelsImage, GL_COLOR_ATTACHMENT0, 0 );
		tr.calcLevelsFbo->r_CheckFBO();
	}

	if (tr.targetLevelsImage)
	{
		tr.targetLevelsFbo = FBO_Create("_targetlevels", tr.targetLevelsImage->width, tr.targetLevelsImage->height);
		tr.targetLevelsFbo->attachImage( tr.targetLevelsImage, GL_COLOR_ATTACHMENT0, 0 );
		tr.targetLevelsFbo->r_CheckFBO();
	}

	if (tr.quarterImage[0])
	{
		for (i = 0; i < 2; i++)
		{
			tr.quarterFbo[i] = FBO_Create(va("_quarter%d", i), tr.quarterImage[i]->width, tr.quarterImage[i]->height);
			tr.quarterFbo[i]->attachImage( tr.quarterImage[i], GL_COLOR_ATTACHMENT0, 0 );
			tr.quarterFbo[i]->r_CheckFBO();
		}
	}

	if (tr.hdrDepthImage)
	{
		tr.hdrDepthFbo = FBO_Create("_hdrDepth", tr.hdrDepthImage->width, tr.hdrDepthImage->height);
		tr.hdrDepthFbo->attachImage( tr.hdrDepthImage, GL_COLOR_ATTACHMENT0, 0 );
		tr.hdrDepthFbo->r_CheckFBO();
	}

	if (tr.screenSsaoImage)
	{
		tr.screenSsaoFbo = FBO_Create("_screenssao", tr.screenSsaoImage->width, tr.screenSsaoImage->height);
		tr.screenSsaoFbo->attachImage( tr.screenSsaoImage, GL_COLOR_ATTACHMENT0, 0 );
		tr.screenSsaoFbo->r_CheckFBO();
	}

	if (tr.renderCubeImage)
	{
		tr.renderCubeFbo = FBO_Create("_renderCubeFbo", tr.renderCubeImage->width, tr.renderCubeImage->height);
		tr.renderCubeFbo->attachImage( tr.renderCubeImage, GL_COLOR_ATTACHMENT0, 0 );
		tr.renderCubeFbo->createBuffer( GL_DEPTH_COMPONENT24_ARB, 0, 0 );
		tr.renderCubeFbo->r_CheckFBO();
	}

	GL_CheckErrors();

	GL_BindFramebuffer(GL_FRAMEBUFFER, 0);
	glState.currentFBO = nullptr;
}

/*
============
FBO_Shutdown
============
*/
void FBO_Shutdown()
{
	int             i, j;
	Framebuffer          *fbo;

	ri.Printf(PRINT_ALL, "------- FBO_Shutdown -------\n");

	if(!glRefConfig.framebufferObject)
		return;

	FBO_Bind(nullptr);

	for(i = 0; i < tr.numFBOs; i++)
	{
		fbo = tr.fbos[i];

		for(j = 0; j < glRefConfig.maxColorAttachments; j++)
		{
			if(fbo->colorBuffers[j])
				qglDeleteRenderbuffers(1, &fbo->colorBuffers[j]);
		}

		if(fbo->depthBuffer)
			qglDeleteRenderbuffers(1, &fbo->depthBuffer);

		if(fbo->stencilBuffer)
			qglDeleteRenderbuffers(1, &fbo->stencilBuffer);

		if(fbo->frameBuffer)
			qglDeleteFramebuffers(1, &fbo->frameBuffer);
	}
}

/*
============
R_FBOList_f
============
*/
void R_FBOList_f()
{
	int             i;
	Framebuffer          *fbo;

	if(!glRefConfig.framebufferObject)
	{
		ri.Printf(PRINT_ALL, "GL_EXT_framebuffer_object is not available.\n");
		return;
	}

	ri.Printf(PRINT_ALL, "             size       name\n");
	ri.Printf(PRINT_ALL, "----------------------------------------------------------\n");

	for(i = 0; i < tr.numFBOs; i++)
	{
		fbo = tr.fbos[i];

		ri.Printf(PRINT_ALL, "  %4i: %4i %4i %s\n", i, fbo->width, fbo->height, fbo->name);
	}

	ri.Printf(PRINT_ALL, " %i FBOs\n", tr.numFBOs);
}

void Framebuffer::blitFromTexture( struct image_s *src, vec4_t inSrcTexCorners, vec2_t inSrcTexScale, ivec4_t inDstBox, ShaderProgram *shaderProgram, vec4_t inColor, int blend )
{
	ivec4_t dstBox;
	vec4_t color;
	vec4_t quadVerts[4];
	vec2_t texCoords[4];
	vec2_t invTexRes;
	Framebuffer *oldFbo = glState.currentFBO;
	mat4_t projection;
	int width, height;

	if (!src)
	{
		ri.Printf(PRINT_WARNING, "Tried to blit from a NULL texture!\n");
		return;
	}

	width  = this ? this->width  : glConfig.vidWidth;
	height = this ? this->height : glConfig.vidHeight;

	if (inSrcTexCorners)
	{
		VectorSet2(texCoords[0], inSrcTexCorners[0], inSrcTexCorners[1]);
		VectorSet2(texCoords[1], inSrcTexCorners[2], inSrcTexCorners[1]);
		VectorSet2(texCoords[2], inSrcTexCorners[2], inSrcTexCorners[3]);
		VectorSet2(texCoords[3], inSrcTexCorners[0], inSrcTexCorners[3]);
	}
	else
	{
		VectorSet2(texCoords[0], 0.0f, 1.0f);
		VectorSet2(texCoords[1], 1.0f, 1.0f);
		VectorSet2(texCoords[2], 1.0f, 0.0f);
		VectorSet2(texCoords[3], 0.0f, 0.0f);
	}

	// framebuffers are 0 bottom, Y up.
	if (inDstBox)
	{
		dstBox[0] = inDstBox[0];
		dstBox[1] = height - inDstBox[1] - inDstBox[3];
		dstBox[2] = inDstBox[0] + inDstBox[2];
		dstBox[3] = height - inDstBox[1];
	}
	else
	{
		VectorSet4(dstBox, 0, height, width, 0);
	}

	if (inSrcTexScale)
	{
		VectorCopy2(inSrcTexScale, invTexRes);
	}
	else
	{
		VectorSet2(invTexRes, 1.0f, 1.0f);
	}

	if (inColor)
	{
		VectorCopy4(inColor, color);
	}
	else
	{
		VectorCopy4(colorWhite, color);
	}

	if (!shaderProgram)
	{
		shaderProgram = &tr.textureColorShader;
	}

	FBO_Bind(this);

	qglViewport( 0, 0, width, height );
	qglScissor( 0, 0, width, height );

	Mat4Ortho(0, width, height, 0, 0, 1, projection);

	GL_Cull( CT_TWO_SIDED );

	GL_BindToTMU(src, TB_COLORMAP);

	VectorSet4(quadVerts[0], dstBox[0], dstBox[1], 0.0f, 1.0f);
	VectorSet4(quadVerts[1], dstBox[2], dstBox[1], 0.0f, 1.0f);
	VectorSet4(quadVerts[2], dstBox[2], dstBox[3], 0.0f, 1.0f);
	VectorSet4(quadVerts[3], dstBox[0], dstBox[3], 0.0f, 1.0f);

	invTexRes[0] /= src->width;
	invTexRes[1] /= src->height;

	GL_State( blend );

	shaderProgram->bindProgram();
	
	shaderProgram->setUniformMat4( UNIFORM_MODELVIEWPROJECTIONMATRIX, projection );
	shaderProgram->setUniformVec4( UNIFORM_COLOR, color );
	shaderProgram->setUniformVec2( UNIFORM_INVTEXRES, invTexRes );
	shaderProgram->setUniformVec2( UNIFORM_AUTOEXPOSUREMINMAX, tr.refdef.autoExposureMinMax );
	shaderProgram->setUniformVec3( UNIFORM_TONEMINAVGMAXLINEAR, tr.refdef.toneMinAvgMaxLinear );

	RB_InstantQuad2(quadVerts, texCoords);

	FBO_Bind(oldFbo);
}

void Framebuffer::blit( ivec4_t inSrcBox, vec2_t srcTexScale, Framebuffer *dst, ivec4_t dstBox, ShaderProgram *shaderProgram, vec4_t color, int blend ) const
{
	vec4_t srcTexCorners;

	if (!this)
	{
		ri.Printf(PRINT_WARNING, "Tried to blit from a NULL FBO!\n");
		return;
	}

	if (inSrcBox)
	{
		srcTexCorners[0] =  inSrcBox[0]                / static_cast<float>(this->width);
		srcTexCorners[1] = (inSrcBox[1] + inSrcBox[3]) / static_cast<float>(this->height);
		srcTexCorners[2] = (inSrcBox[0] + inSrcBox[2]) / static_cast<float>(this->width);
		srcTexCorners[3] =  inSrcBox[1]                / static_cast<float>(this->height);
	}
	else
	{
		VectorSet4(srcTexCorners, 0.0f, 0.0f, 1.0f, 1.0f);
	}

	dst->blitFromTexture( this->colorImage[0], srcTexCorners, srcTexScale, dstBox, shaderProgram, color, blend | GLS_DEPTHTEST_DISABLE );
}

void Framebuffer::fastBlit( ivec4_t srcBox, Framebuffer *dst, ivec4_t dstBox, int buffers, int filter ) const
{
	ivec4_t srcBoxFinal, dstBoxFinal;
	GLuint srcFb, dstFb;

	if (!glRefConfig.framebufferBlit)
	{
		this->blit( srcBox, nullptr, dst, dstBox, nullptr, nullptr, 0 );
		return;
	}

	srcFb = this ? this->frameBuffer : 0;
	dstFb = dst ? dst->frameBuffer : 0;

	if (!srcBox)
	{
		int width =  this ? this->width  : glConfig.vidWidth;
		int height = this ? this->height : glConfig.vidHeight;

		VectorSet4(srcBoxFinal, 0, 0, width, height);
	}
	else
	{
		VectorSet4(srcBoxFinal, srcBox[0], srcBox[1], srcBox[0] + srcBox[2], srcBox[1] + srcBox[3]);
	}

	if (!dstBox)
	{
		int width  = dst ? dst->width  : glConfig.vidWidth;
		int height = dst ? dst->height : glConfig.vidHeight;

		qglScissor(0, 0, width, height);

		VectorSet4(dstBoxFinal, 0, 0, width, height);
	}
	else
	{
		ivec4_t scissorBox;

		Vector4Copy(dstBox, scissorBox);

		if (scissorBox[2] < 0)
		{
			scissorBox[0] += scissorBox[2];
			scissorBox[2] = abs(scissorBox[2]);
		}

		if (scissorBox[3] < 0)
		{
			scissorBox[1] += scissorBox[3];
			scissorBox[3] = abs(scissorBox[3]);
		}

		qglScissor(scissorBox[0], scissorBox[1], scissorBox[2], scissorBox[3]);

		VectorSet4(dstBoxFinal, dstBox[0], dstBox[1], dstBox[0] + dstBox[2], dstBox[1] + dstBox[3]);
	}

	GL_BindFramebuffer(GL_READ_FRAMEBUFFER, srcFb);
	GL_BindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFb);
	qglBlitFramebuffer(srcBoxFinal[0], srcBoxFinal[1], srcBoxFinal[2], srcBoxFinal[3],
	                      dstBoxFinal[0], dstBoxFinal[1], dstBoxFinal[2], dstBoxFinal[3],
						  buffers, filter);

	GL_BindFramebuffer(GL_FRAMEBUFFER, 0);
	glState.currentFBO = nullptr;
}
