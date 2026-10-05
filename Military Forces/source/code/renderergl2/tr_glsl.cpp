/*
===========================================================================
Copyright (C) 2006-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

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
// tr_glsl.c
#include "tr_local.h"

#include "tr_dsa.h"

extern const char *fallbackShader_bokeh_vp;
extern const char *fallbackShader_bokeh_fp;
extern const char *fallbackShader_calclevels4x_vp;
extern const char *fallbackShader_calclevels4x_fp;
extern const char *fallbackShader_depthblur_vp;
extern const char *fallbackShader_depthblur_fp;
extern const char *fallbackShader_dlight_vp;
extern const char *fallbackShader_dlight_fp;
extern const char *fallbackShader_down4x_vp;
extern const char *fallbackShader_down4x_fp;
extern const char *fallbackShader_fogpass_vp;
extern const char *fallbackShader_fogpass_fp;
extern const char *fallbackShader_generic_vp;
extern const char *fallbackShader_generic_fp;
extern const char *fallbackShader_lightall_vp;
extern const char *fallbackShader_lightall_fp;
extern const char *fallbackShader_pshadow_vp;
extern const char *fallbackShader_pshadow_fp;
extern const char *fallbackShader_shadowfill_vp;
extern const char *fallbackShader_shadowfill_fp;
extern const char *fallbackShader_shadowmask_vp;
extern const char *fallbackShader_shadowmask_fp;
extern const char *fallbackShader_ssao_vp;
extern const char *fallbackShader_ssao_fp;
extern const char *fallbackShader_texturecolor_vp;
extern const char *fallbackShader_texturecolor_fp;
extern const char *fallbackShader_tonemap_vp;
extern const char *fallbackShader_tonemap_fp;
extern const char* fallbackShader_greyscale_vp;
extern const char* fallbackShader_greyscale_fp;

struct uniformInfo_s
{
	char *name;
	int type;
};
using uniformInfo_t = uniformInfo_s;

// These must be in the same order as in uniform_t in tr_local.h.
static uniformInfo_t uniformsInfo[] =
{
	{ "u_DiffuseMap",  GLSL_INT },
	{ "u_LightMap",    GLSL_INT },
	{ "u_NormalMap",   GLSL_INT },
	{ "u_DeluxeMap",   GLSL_INT },
	{ "u_SpecularMap", GLSL_INT },

	{ "u_TextureMap", GLSL_INT },
	{ "u_LevelsMap",  GLSL_INT },
	{ "u_CubeMap",    GLSL_INT },

	{ "u_ScreenImageMap", GLSL_INT },
	{ "u_ScreenDepthMap", GLSL_INT },

	{ "u_ShadowMap",  GLSL_INT },
	{ "u_ShadowMap2", GLSL_INT },
	{ "u_ShadowMap3", GLSL_INT },
	{ "u_ShadowMap4", GLSL_INT },

	{ "u_ShadowMvp",  GLSL_MAT16 },
	{ "u_ShadowMvp2", GLSL_MAT16 },
	{ "u_ShadowMvp3", GLSL_MAT16 },
	{ "u_ShadowMvp4", GLSL_MAT16 },

	{ "u_EnableTextures", GLSL_VEC4 },

	{ "u_DiffuseTexMatrix0",  GLSL_VEC4 },
	{ "u_DiffuseTexMatrix1",  GLSL_VEC4 },
	{ "u_DiffuseTexMatrix2",  GLSL_VEC4 },
	{ "u_DiffuseTexMatrix3",  GLSL_VEC4 },
	{ "u_DiffuseTexMatrix4",  GLSL_VEC4 },
	{ "u_DiffuseTexMatrix5",  GLSL_VEC4 },
	{ "u_DiffuseTexMatrix6",  GLSL_VEC4 },
	{ "u_DiffuseTexMatrix7",  GLSL_VEC4 },

	{ "u_TCGen0",        GLSL_INT },
	{ "u_TCGen0Vector0", GLSL_VEC3 },
	{ "u_TCGen0Vector1", GLSL_VEC3 },

	{ "u_DeformGen",    GLSL_INT },
	{ "u_DeformParams", GLSL_FLOAT5 },

	{ "u_ColorGen",  GLSL_INT },
	{ "u_AlphaGen",  GLSL_INT },
	{ "u_Color",     GLSL_VEC4 },
	{ "u_BaseColor", GLSL_VEC4 },
	{ "u_VertColor", GLSL_VEC4 },

	{ "u_DlightInfo",    GLSL_VEC4 },
	{ "u_LightForward",  GLSL_VEC3 },
	{ "u_LightUp",       GLSL_VEC3 },
	{ "u_LightRight",    GLSL_VEC3 },
	{ "u_LightOrigin",   GLSL_VEC4 },
	{ "u_ModelLightDir", GLSL_VEC3 },
	{ "u_LightRadius",   GLSL_FLOAT },
	{ "u_AmbientLight",  GLSL_VEC3 },
	{ "u_DirectedLight", GLSL_VEC3 },

	{ "u_PortalRange", GLSL_FLOAT },

	{ "u_FogDistance",  GLSL_VEC4 },
	{ "u_FogDepth",     GLSL_VEC4 },
	{ "u_FogEyeT",      GLSL_FLOAT },
	{ "u_FogColorMask", GLSL_VEC4 },

	{ "u_ModelMatrix",               GLSL_MAT16 },
	{ "u_ModelViewProjectionMatrix", GLSL_MAT16 },

	{ "u_Time",          GLSL_FLOAT },
	{ "u_VertexLerp" ,   GLSL_FLOAT },
	{ "u_NormalScale",   GLSL_VEC4 },
	{ "u_SpecularScale", GLSL_VEC4 },

	{ "u_ViewInfo",        GLSL_VEC4 },
	{ "u_ViewOrigin",      GLSL_VEC3 },
	{ "u_LocalViewOrigin", GLSL_VEC3 },
	{ "u_ViewForward",     GLSL_VEC3 },
	{ "u_ViewLeft",        GLSL_VEC3 },
	{ "u_ViewUp",          GLSL_VEC3 },

	{ "u_InvTexRes",           GLSL_VEC2 },
	{ "u_AutoExposureMinMax",  GLSL_VEC2 },
	{ "u_ToneMinAvgMaxLinear", GLSL_VEC3 },

	{ "u_PrimaryLightOrigin",  GLSL_VEC4  },
	{ "u_PrimaryLightColor",   GLSL_VEC3  },
	{ "u_PrimaryLightAmbient", GLSL_VEC3  },
	{ "u_PrimaryLightRadius",  GLSL_FLOAT },

	{ "u_CubeMapInfo", GLSL_VEC4 },

	{ "u_AlphaTest", GLSL_INT },

	{ "u_BoneMatrix", GLSL_MAT16_BONEMATRIX },
	{ "u_Greyscale", GLSL_FLOAT }
};

enum glslPrintLog_t
{
	GLSL_PRINTLOG_PROGRAM_INFO,
	GLSL_PRINTLOG_SHADER_INFO,
	GLSL_PRINTLOG_SHADER_SOURCE
};

static void GLSL_PrintLog(GLuint programOrShader, glslPrintLog_t type, bool developerOnly)
{
	char           *msg;
	static char     msgPart[1024];
	int             maxLength = 0;
	int             i;
	int             printLevel = developerOnly ? PRINT_DEVELOPER : PRINT_ALL;

	switch (type)
	{
		case GLSL_PRINTLOG_PROGRAM_INFO:
			ri.Printf(printLevel, "Program info log:\n");
			qglGetProgramiv(programOrShader, GL_INFO_LOG_LENGTH, &maxLength);
			break;

		case GLSL_PRINTLOG_SHADER_INFO:
			ri.Printf(printLevel, "Shader info log:\n");
			qglGetShaderiv(programOrShader, GL_INFO_LOG_LENGTH, &maxLength);
			break;

		case GLSL_PRINTLOG_SHADER_SOURCE:
			ri.Printf(printLevel, "Shader source:\n");
			qglGetShaderiv(programOrShader, GL_SHADER_SOURCE_LENGTH, &maxLength);
			break;
	}

	if (maxLength <= 0)
	{
		ri.Printf(printLevel, "None.\n");
		return;
	}

	if (maxLength < 1023)
		msg = msgPart;
	else
		msg = static_cast<char *>(ri.Malloc(maxLength));

	switch (type)
	{
		case GLSL_PRINTLOG_PROGRAM_INFO:
			qglGetProgramInfoLog(programOrShader, maxLength, &maxLength, msg);
			break;

		case GLSL_PRINTLOG_SHADER_INFO:
			qglGetShaderInfoLog(programOrShader, maxLength, &maxLength, msg);
			break;

		case GLSL_PRINTLOG_SHADER_SOURCE:
			qglGetShaderSource(programOrShader, maxLength, &maxLength, msg);
			break;
	}

	if (maxLength < 1023)
	{
		msgPart[maxLength + 1] = '\0';

		ri.Printf(printLevel, "%s\n", msgPart);
	}
	else
	{
		for(i = 0; i < maxLength; i += 1023)
		{
			Q_strncpyz(msgPart, msg + i, sizeof(msgPart));

			ri.Printf(printLevel, "%s", msgPart);
		}

		ri.Printf(printLevel, "\n");

		ri.Free(msg);
	}

}

static void GLSL_GetShaderHeader( GLenum shaderType, const GLchar *extra, char *dest, int size )
{
	float fbufWidthScale, fbufHeightScale;

	dest[0] = '\0';

	// HACK: abuse the GLSL preprocessor to turn GLSL 1.20 shaders into 1.30 ones
	if(glRefConfig.glslMajorVersion > 1 || (glRefConfig.glslMajorVersion == 1 && glRefConfig.glslMinorVersion >= 30))
	{
		if (qglesMajorVersion >= 3 && glRefConfig.glslMajorVersion >= 3)
			Q_strcat(dest, size, "#version 300 es\n");
		else if (glRefConfig.glslMajorVersion > 1 || (glRefConfig.glslMajorVersion == 1 && glRefConfig.glslMinorVersion >= 50))
			Q_strcat(dest, size, "#version 150\n");
		else
			Q_strcat(dest, size, "#version 130\n");

		// `extra' may contain #extension which must be directly after #version
		if (extra)
		{
			Q_strcat(dest, size, extra);
		}

		if (qglesMajorVersion >= 2)
		{
			Q_strcat(dest, size, "precision mediump float;\n");
			Q_strcat(dest, size, "precision mediump sampler2DShadow;\n");
		}

		if(shaderType == GL_VERTEX_SHADER)
		{
			Q_strcat(dest, size, "#define attribute in\n");
			Q_strcat(dest, size, "#define varying out\n");
		}
		else
		{
			Q_strcat(dest, size, "#define varying in\n");

			Q_strcat(dest, size, "out vec4 out_Color;\n");
			Q_strcat(dest, size, "#define gl_FragColor out_Color\n");
			Q_strcat(dest, size, "#define texture2D texture\n");
			Q_strcat(dest, size, "#define textureCubeLod textureLod\n");
			Q_strcat(dest, size, "#define shadow2D texture\n");
		}
	}
	else
	{
		if (qglesMajorVersion >= 2)
		{
			Q_strcat(dest, size, "#version 100\n");

			if (extra)
			{
				Q_strcat(dest, size, extra);
			}

			Q_strcat(dest, size, "precision mediump float;\n");

			if (glRefConfig.shadowSamplers)
			{
				Q_strcat(dest, size, "precision mediump sampler2DShadow;\n");
				Q_strcat(dest, size, "#define shadow2D(a,b) shadow2DEXT(a,b)\n");
			}
		}
		else
		{
			Q_strcat(dest, size, "#version 120\n");

			if (extra)
			{
				Q_strcat(dest, size, extra);
			}

			Q_strcat(dest, size, "#define shadow2D(a,b) shadow2D(a,b).r\n");
		}
	}

	// HACK: add some macros to avoid extra uniforms and save speed and code maintenance
	//Q_strcat(dest, size,
	//		 va("#ifndef r_SpecularExponent\n#define r_SpecularExponent %f\n#endif\n", r_specularExponent->value));
	//Q_strcat(dest, size,
	//		 va("#ifndef r_SpecularScale\n#define r_SpecularScale %f\n#endif\n", r_specularScale->value));
	//Q_strcat(dest, size,
	//       va("#ifndef r_NormalScale\n#define r_NormalScale %f\n#endif\n", r_normalScale->value));


	Q_strcat(dest, size, "#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n");

	//Q_strcat(dest, size, va("#ifndef MAX_SHADOWMAPS\n#define MAX_SHADOWMAPS %i\n#endif\n", MAX_SHADOWMAPS));

	Q_strcat(dest, size,
					 va("#ifndef deformGen_t\n"
						"#define deformGen_t\n"
						"#define DGEN_WAVE_SIN %i\n"
						"#define DGEN_WAVE_SQUARE %i\n"
						"#define DGEN_WAVE_TRIANGLE %i\n"
						"#define DGEN_WAVE_SAWTOOTH %i\n"
						"#define DGEN_WAVE_INVERSE_SAWTOOTH %i\n"
						"#define DGEN_BULGE %i\n"
						"#define DGEN_MOVE %i\n"
						"#endif\n",
						DGEN_WAVE_SIN,
						DGEN_WAVE_SQUARE,
						DGEN_WAVE_TRIANGLE,
						DGEN_WAVE_SAWTOOTH,
						DGEN_WAVE_INVERSE_SAWTOOTH,
						DGEN_BULGE,
						DGEN_MOVE));

	Q_strcat(dest, size,
					 va("#ifndef tcGen_t\n"
						"#define tcGen_t\n"
						"#define TCGEN_LIGHTMAP %i\n"
						"#define TCGEN_TEXTURE %i\n"
						"#define TCGEN_ENVIRONMENT_MAPPED %i\n"
						"#define TCGEN_FOG %i\n"
						"#define TCGEN_VECTOR %i\n"
						"#endif\n",
						TCGEN_LIGHTMAP,
						TCGEN_TEXTURE,
						TCGEN_ENVIRONMENT_MAPPED,
						TCGEN_FOG,
						TCGEN_VECTOR));

	Q_strcat(dest, size,
					 va("#ifndef colorGen_t\n"
						"#define colorGen_t\n"
						"#define CGEN_LIGHTING_DIFFUSE %i\n"
						"#endif\n",
						CGEN_LIGHTING_DIFFUSE));

	Q_strcat(dest, size,
							 va("#ifndef alphaGen_t\n"
								"#define alphaGen_t\n"
								"#define AGEN_LIGHTING_SPECULAR %i\n"
								"#define AGEN_PORTAL %i\n"
								"#endif\n",
								AGEN_LIGHTING_SPECULAR,
								AGEN_PORTAL));

	fbufWidthScale = 1.0f / (static_cast<float>(glConfig.vidWidth));
	fbufHeightScale = 1.0f / (static_cast<float>(glConfig.vidHeight));
	Q_strcat(dest, size,
			 va("#ifndef r_FBufScale\n#define r_FBufScale vec2(%f, %f)\n#endif\n", fbufWidthScale, fbufHeightScale));

	if (r_pbr->asInt())
		Q_strcat(dest, size, "#define USE_PBR\n");

	if (r_cubeMapping->asInt())
	{
		int cubeMipSize = r_cubemapSize->asInt();
		int numRoughnessMips = 0;

		while (cubeMipSize)
		{
			cubeMipSize >>= 1;
			numRoughnessMips++;
		}
		numRoughnessMips = std::max(1, numRoughnessMips - 2);
		Q_strcat(dest, size, va("#define ROUGHNESS_MIPS float(%d)\n", numRoughnessMips));
	}

	// OK we added a lot of stuff but if we do something bad in the GLSL shaders then we want the proper line
	// so we have to reset the line counting
	Q_strcat(dest, size, "#line 0\n");
}

static int GLSL_CompileGPUShader(GLuint program, GLuint *prevShader, const GLchar *buffer, int size, GLenum shaderType)
{
	GLint           compiled;
	GLuint          shader;

	shader = qglCreateShader(shaderType);

	qglShaderSource(shader, 1, &buffer, &size);

	// compile shader
	qglCompileShader(shader);

	// check if shader compiled
	qglGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if(!compiled)
	{
		GLSL_PrintLog(shader, GLSL_PRINTLOG_SHADER_SOURCE, false);
		GLSL_PrintLog(shader, GLSL_PRINTLOG_SHADER_INFO, false);
		ri.Error(ERR_DROP, "Couldn't compile shader");
		return 0;
	}

	if (*prevShader)
	{
		qglDetachShader(program, *prevShader);
		qglDeleteShader(*prevShader);
	}

	// attach shader to program
	qglAttachShader(program, shader);

	*prevShader = shader;

	return 1;
}

static int GLSL_LoadGPUShaderText(const char *name, const char *fallback,
	GLenum shaderType, char *dest, int destSize)
{
	char            filename[MAX_QPATH];
	GLchar      *buffer = nullptr;
	const GLchar *shaderText = nullptr;
	int             size;
	int             result;

	if(shaderType == GL_VERTEX_SHADER)
	{
		Com_sprintf(filename, sizeof(filename), "glsl/%s_vp.glsl", name);
	}
	else
	{
		Com_sprintf(filename, sizeof(filename), "glsl/%s_fp.glsl", name);
	}

	if ( r_externalGLSL->asInt() ) {
		size = ri.FS_ReadFile(filename, reinterpret_cast<void **>(&buffer));
	} else {
		size = 0;
		buffer = nullptr;
	}

	if(!buffer)
	{
		if (fallback)
		{
			ri.Printf(PRINT_DEVELOPER, "...loading built-in '%s'\n", filename);
			shaderText = fallback;
			size = strlen(shaderText);
		}
		else
		{
			ri.Printf(PRINT_DEVELOPER, "couldn't load '%s'\n", filename);
			return 0;
		}
	}
	else
	{
		ri.Printf(PRINT_DEVELOPER, "...loading '%s'\n", filename);
		shaderText = buffer;
	}

	if (size > destSize)
	{
		result = 0;
	}
	else
	{
		Q_strncpyz(dest, shaderText, size + 1);
		result = 1;
	}

	if (buffer)
	{
		ri.FS_FreeFile(buffer);
	}
	
	return result;
}

static void GLSL_LinkProgram(GLuint program)
{
	GLint           linked;

	qglLinkProgram(program);

	qglGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(!linked)
	{
		GLSL_PrintLog(program, GLSL_PRINTLOG_PROGRAM_INFO, false);
		ri.Error(ERR_DROP, "shaders failed to link");
	}
}

static void GLSL_ShowProgramUniforms(GLuint program)
{
	int             i, count, size;
	GLenum			type;
	char            uniformName[1000];

	// query the number of active uniforms
	qglGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);

	// Loop over each of the active uniforms, and set their value
	for(i = 0; i < count; i++)
	{
		qglGetActiveUniform(program, i, sizeof(uniformName), nullptr, &size, &type, uniformName);

		ri.Printf(PRINT_DEVELOPER, "active uniform: '%s'\n", uniformName);
	}
}

int ShaderProgram::initGPUShader2( const char *name, int attribs, const char *vpCode, const char *fpCode )
{
	ri.Printf(PRINT_DEVELOPER, "------- GPU shader -------\n");

	if(strlen(name) >= MAX_QPATH)
	{
		ri.Error(ERR_DROP, "GLSL_InitGPUShader2: \"%s\" is too long", name);
	}

	Q_strncpyz(this->name, name, sizeof(this->name));

	this->program = qglCreateProgram();
	this->attribs = attribs;

	if (!(GLSL_CompileGPUShader(this->program, &this->vertexShader, vpCode, strlen(vpCode), GL_VERTEX_SHADER)))
	{
		ri.Printf(PRINT_ALL, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_VERTEX_SHADER\n", name);
		qglDeleteProgram(this->program);
		return 0;
	}

	if(fpCode)
	{
		if(!(GLSL_CompileGPUShader(this->program, &this->fragmentShader, fpCode, strlen(fpCode), GL_FRAGMENT_SHADER)))
		{
			ri.Printf(PRINT_ALL, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_FRAGMENT_SHADER\n", name);
			qglDeleteProgram(this->program);
			return 0;
		}
	}

	if(attribs & ATTR_POSITION)
		qglBindAttribLocation(this->program, ATTR_INDEX_POSITION, "attr_Position");

	if(attribs & ATTR_TEXCOORD)
		qglBindAttribLocation(this->program, ATTR_INDEX_TEXCOORD, "attr_TexCoord0");

	if(attribs & ATTR_LIGHTCOORD)
		qglBindAttribLocation(this->program, ATTR_INDEX_LIGHTCOORD, "attr_TexCoord1");

//  if(attribs & ATTR_TEXCOORD2)
//      qglBindAttribLocation(this->program, ATTR_INDEX_TEXCOORD2, "attr_TexCoord2");

//  if(attribs & ATTR_TEXCOORD3)
//      qglBindAttribLocation(this->program, ATTR_INDEX_TEXCOORD3, "attr_TexCoord3");

	if(attribs & ATTR_TANGENT)
		qglBindAttribLocation(this->program, ATTR_INDEX_TANGENT, "attr_Tangent");

	if(attribs & ATTR_NORMAL)
		qglBindAttribLocation(this->program, ATTR_INDEX_NORMAL, "attr_Normal");

	if(attribs & ATTR_COLOR)
		qglBindAttribLocation(this->program, ATTR_INDEX_COLOR, "attr_Color");

	if(attribs & ATTR_PAINTCOLOR)
		qglBindAttribLocation(this->program, ATTR_INDEX_PAINTCOLOR, "attr_PaintColor");

	if(attribs & ATTR_LIGHTDIRECTION)
		qglBindAttribLocation(this->program, ATTR_INDEX_LIGHTDIRECTION, "attr_LightDirection");

	if(attribs & ATTR_BONE_INDEXES)
		qglBindAttribLocation(this->program, ATTR_INDEX_BONE_INDEXES, "attr_BoneIndexes");

	if(attribs & ATTR_BONE_WEIGHTS)
		qglBindAttribLocation(this->program, ATTR_INDEX_BONE_WEIGHTS, "attr_BoneWeights");

	if(attribs & ATTR_POSITION2)
		qglBindAttribLocation(this->program, ATTR_INDEX_POSITION2, "attr_Position2");

	if(attribs & ATTR_NORMAL2)
		qglBindAttribLocation(this->program, ATTR_INDEX_NORMAL2, "attr_Normal2");

	if(attribs & ATTR_TANGENT2)
		qglBindAttribLocation(this->program, ATTR_INDEX_TANGENT2, "attr_Tangent2");

	GLSL_LinkProgram(this->program);

	return 1;
}

int ShaderProgram::initGPUShader( const char *name, int attribs, bool fragmentShader, const GLchar *extra, bool addHeader, const char *fallback_vp, const char *fallback_fp )
{
	char vpCode[32000];
	char fpCode[32000];
	char *postHeader;
	int size;
	int result;

	size = sizeof(vpCode);
	if (addHeader)
	{
		GLSL_GetShaderHeader(GL_VERTEX_SHADER, extra, vpCode, size);
		postHeader = &vpCode[strlen(vpCode)];
		size -= strlen(vpCode);
	}
	else
	{
		postHeader = &vpCode[0];
	}

	if (!GLSL_LoadGPUShaderText(name, fallback_vp, GL_VERTEX_SHADER, postHeader, size))
	{
		return 0;
	}

	if (fragmentShader)
	{
		size = sizeof(fpCode);
		if (addHeader)
		{
			GLSL_GetShaderHeader(GL_FRAGMENT_SHADER, extra, fpCode, size);
			postHeader = &fpCode[strlen(fpCode)];
			size -= strlen(fpCode);
		}
		else
		{
			postHeader = &fpCode[0];
		}

		if (!GLSL_LoadGPUShaderText(name, fallback_fp, GL_FRAGMENT_SHADER, postHeader, size))
		{
			return 0;
		}
	}

	result = this->initGPUShader2( name, attribs, vpCode, fragmentShader ? fpCode : nullptr );

	return result;
}

void ShaderProgram::initUniforms()
{
	int i, size;

	GLint *uniforms = this->uniforms;

	size = 0;
	for (i = 0; i < UNIFORM_COUNT; i++)
	{
		uniforms[i] = qglGetUniformLocation(this->program, uniformsInfo[i].name);

		if (uniforms[i] == -1)
			continue;
		 
		this->uniformBufferOffsets[i] = size;

		switch(uniformsInfo[i].type)
		{
			case GLSL_INT:
				size += sizeof(GLint);
				break;
			case GLSL_FLOAT:
				size += sizeof(GLfloat);
				break;
			case GLSL_FLOAT5:
				size += sizeof(vec_t) * 5;
				break;
			case GLSL_VEC2:
				size += sizeof(vec_t) * 2;
				break;
			case GLSL_VEC3:
				size += sizeof(vec_t) * 3;
				break;
			case GLSL_VEC4:
				size += sizeof(vec_t) * 4;
				break;
			case GLSL_MAT16:
				size += sizeof(vec_t) * 16;
				break;
			case GLSL_MAT16_BONEMATRIX:
				size += sizeof(vec_t) * 16 * glRefConfig.glslMaxAnimatedBones;
				break;
			default:
				break;
		}
	}

	this->uniformBuffer = static_cast<char *>(ri.Malloc(size));
}

void ShaderProgram::finishGPUShader()
{
	GLSL_ShowProgramUniforms(this->program);
	GL_CheckErrors();
}

void ShaderProgram::setUniformInt( int uniformNum, GLint value )
{
	GLint *uniforms = this->uniforms;
	GLint *compare = reinterpret_cast<GLint *>(this->uniformBuffer + this->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_INT)
	{
		ri.Printf( PRINT_WARNING, "GLSL_SetUniformInt: wrong type for uniform %i in this %s\n", uniformNum, this->name);
		return;
	}

	if (value == *compare)
	{
		return;
	}

	*compare = value;

	qglProgramUniform1iEXT(this->program, uniforms[uniformNum], value);
}

void ShaderProgram::setUniformFloat( int uniformNum, GLfloat value )
{
	GLint *uniforms = this->uniforms;
	GLfloat *compare = reinterpret_cast<GLfloat *>(this->uniformBuffer + this->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_FLOAT)
	{
		ri.Printf( PRINT_WARNING, "GLSL_SetUniformFloat: wrong type for uniform %i in this %s\n", uniformNum, this->name);
		return;
	}

	if (value == *compare)
	{
		return;
	}

	*compare = value;
	
	qglProgramUniform1fEXT(this->program, uniforms[uniformNum], value);
}

void ShaderProgram::setUniformVec2( int uniformNum, const vec2_t v )
{
	GLint *uniforms = this->uniforms;
	vec_t *compare = reinterpret_cast<float *>(this->uniformBuffer + this->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_VEC2)
	{
		ri.Printf( PRINT_WARNING, "GLSL_SetUniformVec2: wrong type for uniform %i in this %s\n", uniformNum, this->name);
		return;
	}

	if (v[0] == compare[0] && v[1] == compare[1])
	{
		return;
	}

	compare[0] = v[0];
	compare[1] = v[1];

	qglProgramUniform2fEXT(this->program, uniforms[uniformNum], v[0], v[1]);
}

void ShaderProgram::setUniformVec3( int uniformNum, const vec3_t v )
{
	GLint *uniforms = this->uniforms;
	vec_t *compare = reinterpret_cast<float *>(this->uniformBuffer + this->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_VEC3)
	{
		ri.Printf( PRINT_WARNING, "GLSL_SetUniformVec3: wrong type for uniform %i in this %s\n", uniformNum, this->name);
		return;
	}

	if (VectorCompare(v, compare))
	{
		return;
	}

	VectorCopy(v, compare);

	qglProgramUniform3fEXT(this->program, uniforms[uniformNum], v[0], v[1], v[2]);
}

void ShaderProgram::setUniformVec4( int uniformNum, const vec4_t v )
{
	GLint *uniforms = this->uniforms;
	vec_t *compare = reinterpret_cast<float *>(this->uniformBuffer + this->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_VEC4)
	{
		ri.Printf( PRINT_WARNING, "GLSL_SetUniformVec4: wrong type for uniform %i in this %s\n", uniformNum, this->name);
		return;
	}

	if (VectorCompare4(v, compare))
	{
		return;
	}

	VectorCopy4(v, compare);

	qglProgramUniform4fEXT(this->program, uniforms[uniformNum], v[0], v[1], v[2], v[3]);
}

void ShaderProgram::setUniformFloat5( int uniformNum, const vec5_t v )
{
	GLint *uniforms = this->uniforms;
	vec_t *compare = reinterpret_cast<float *>(this->uniformBuffer + this->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_FLOAT5)
	{
		ri.Printf( PRINT_WARNING, "GLSL_SetUniformFloat5: wrong type for uniform %i in this %s\n", uniformNum, this->name);
		return;
	}

	if (VectorCompare5(v, compare))
	{
		return;
	}

	VectorCopy5(v, compare);

	qglProgramUniform1fvEXT(this->program, uniforms[uniformNum], 5, v);
}

void ShaderProgram::setUniformMat4( int uniformNum, const mat4_t matrix )
{
	GLint *uniforms = this->uniforms;
	vec_t *compare = reinterpret_cast<float *>(this->uniformBuffer + this->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_MAT16)
	{
		ri.Printf( PRINT_WARNING, "GLSL_SetUniformMat4: wrong type for uniform %i in this %s\n", uniformNum, this->name);
		return;
	}

	if (Mat4Compare(matrix, compare))
	{
		return;
	}

	Mat4Copy(matrix, compare);

	qglProgramUniformMatrix4fvEXT(this->program, uniforms[uniformNum], 1, GL_FALSE, matrix);
}

void ShaderProgram::setUniformMat4BoneMatrix( int uniformNum, /*const*/ mat4_t *matrix, int numMatricies )
{
	GLint *uniforms = this->uniforms;
	vec_t *compare = reinterpret_cast<float *>(this->uniformBuffer + this->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1) {
		return;
	}

	if (uniformsInfo[uniformNum].type != GLSL_MAT16_BONEMATRIX)
	{
		ri.Printf( PRINT_WARNING, "GLSL_SetUniformMat4BoneMatrix: wrong type for uniform %i in this %s\n", uniformNum, this->name);
		return;
	}

	if (numMatricies > glRefConfig.glslMaxAnimatedBones)
	{
		ri.Printf( PRINT_WARNING, "GLSL_SetUniformMat4BoneMatrix: too many matricies (%d/%d) for uniform %i in this %s\n",
				numMatricies, glRefConfig.glslMaxAnimatedBones, uniformNum, this->name);
		return;
	}

	if (!memcmp(matrix, compare, numMatricies * sizeof(mat4_t)))
	{
		return;
	}

	Com_Memcpy(compare, matrix, numMatricies * sizeof(mat4_t));

	qglProgramUniformMatrix4fvEXT(this->program, uniforms[uniformNum], numMatricies, GL_FALSE, &matrix[0][0]);
}

void ShaderProgram::deleteGPUShader()
{
	if(this->program)
	{
		if (this->vertexShader)
		{
			qglDetachShader(this->program, this->vertexShader);
			qglDeleteShader(this->vertexShader);
		}

		if (this->fragmentShader)
		{
			qglDetachShader(this->program, this->fragmentShader);
			qglDeleteShader(this->fragmentShader);
		}

		qglDeleteProgram(this->program);

		if (this->uniformBuffer)
		{
			ri.Free(this->uniformBuffer);
		}

		Com_Memset(this, 0, sizeof(*this));
	}
}

void GLSL_InitGPUShaders()
{
	int             startTime, endTime;
	int i;
	char extradefines[1024];
	int attribs;
	int numGenShaders = 0, numLightShaders = 0, numEtcShaders = 0;

	ri.Printf(PRINT_ALL, "------- GLSL_InitGPUShaders -------\n");

	R_IssuePendingRenderCommands();

	startTime = ri.Milliseconds();

	// OpenGL ES may not have enough attributes to fit ones used for vertex animation
	if ( glRefConfig.maxVertexAttribs > ATTR_INDEX_NORMAL2 ) {
		ri.Printf(PRINT_ALL, "Using GPU vertex animation\n");
		glRefConfig.gpuVertexAnimation = true;
	} else {
		ri.Printf(PRINT_ALL, "Using CPU vertex animation\n");
		glRefConfig.gpuVertexAnimation = false;
	}

	for (i = 0; i < GENERICDEF_COUNT; i++)
	{	
		if ((i & GENERICDEF_USE_VERTEX_ANIMATION) && (i & GENERICDEF_USE_BONE_ANIMATION))
			continue;

		if ((i & GENERICDEF_USE_BONE_ANIMATION) && !glRefConfig.glslMaxAnimatedBones)
			continue;

		attribs = ATTR_POSITION | ATTR_TEXCOORD | ATTR_LIGHTCOORD | ATTR_NORMAL | ATTR_COLOR;
		extradefines[0] = '\0';

		if (i & GENERICDEF_USE_DEFORM_VERTEXES)
			Q_strcat(extradefines, 1024, "#define USE_DEFORM_VERTEXES\n");

		if (i & GENERICDEF_USE_TCGEN_AND_TCMOD)
		{
			Q_strcat(extradefines, 1024, "#define USE_TCGEN\n");
			Q_strcat(extradefines, 1024, "#define USE_TCMOD\n");
		}

		if (i & GENERICDEF_USE_VERTEX_ANIMATION)
		{
			if (!glRefConfig.gpuVertexAnimation)
				continue;

			Q_strcat(extradefines, 1024, "#define USE_VERTEX_ANIMATION\n");
			attribs |= ATTR_POSITION2 | ATTR_NORMAL2;
		}
		else if (i & GENERICDEF_USE_BONE_ANIMATION)
		{
			Q_strcat(extradefines, 1024, va("#define USE_BONE_ANIMATION\n#define MAX_GLSL_BONES %d\n", glRefConfig.glslMaxAnimatedBones));
			attribs |= ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS;
		}

		if (i & GENERICDEF_USE_FOG)
			Q_strcat(extradefines, 1024, "#define USE_FOG\n");

		if (i & GENERICDEF_USE_RGBAGEN)
			Q_strcat(extradefines, 1024, "#define USE_RGBAGEN\n");

		if (!tr.genericShader[i].initGPUShader( "generic", attribs, true, extradefines, true, fallbackShader_generic_vp, fallbackShader_generic_fp ))
		{
			ri.Error(ERR_FATAL, "Could not load generic shader!");
		}

		tr.genericShader[i].initUniforms();

		tr.genericShader[i].setUniformInt( UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
		tr.genericShader[i].setUniformInt( UNIFORM_LIGHTMAP, TB_LIGHTMAP );

		tr.genericShader[i].finishGPUShader();

		numGenShaders++;
	}


	attribs = ATTR_POSITION | ATTR_TEXCOORD;

	if (!tr.textureColorShader.initGPUShader( "texturecolor", attribs, true, extradefines, true, fallbackShader_texturecolor_vp, fallbackShader_texturecolor_fp ))
	{
		ri.Error(ERR_FATAL, "Could not load texturecolor shader!");
	}
	
	tr.textureColorShader.initUniforms();

	tr.textureColorShader.setUniformInt( UNIFORM_TEXTUREMAP, TB_DIFFUSEMAP );

	tr.textureColorShader.finishGPUShader();

	numEtcShaders++;

	for (i = 0; i < FOGDEF_COUNT; i++)
	{
		if ((i & FOGDEF_USE_VERTEX_ANIMATION) && (i & FOGDEF_USE_BONE_ANIMATION))
			continue;

		if ((i & FOGDEF_USE_VERTEX_ANIMATION) && !glRefConfig.gpuVertexAnimation)
			continue;

		if ((i & FOGDEF_USE_BONE_ANIMATION) && !glRefConfig.glslMaxAnimatedBones)
			continue;

		attribs = ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD;
		extradefines[0] = '\0';

		if (i & FOGDEF_USE_DEFORM_VERTEXES)
			Q_strcat(extradefines, 1024, "#define USE_DEFORM_VERTEXES\n");

		if (i & FOGDEF_USE_VERTEX_ANIMATION)
		{
			Q_strcat(extradefines, 1024, "#define USE_VERTEX_ANIMATION\n");
			attribs |= ATTR_POSITION2 | ATTR_NORMAL2;
		}
		else if (i & FOGDEF_USE_BONE_ANIMATION)
		{
			Q_strcat(extradefines, 1024, va("#define USE_BONE_ANIMATION\n#define MAX_GLSL_BONES %d\n", glRefConfig.glslMaxAnimatedBones));
			attribs |= ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS;
		}

		if (!tr.fogShader[i].initGPUShader( "fogpass", attribs, true, extradefines, true, fallbackShader_fogpass_vp, fallbackShader_fogpass_fp ))
		{
			ri.Error(ERR_FATAL, "Could not load fogpass shader!");
		}

		tr.fogShader[i].initUniforms();
		tr.fogShader[i].finishGPUShader();

		numEtcShaders++;
	}


	for (i = 0; i < DLIGHTDEF_COUNT; i++)
	{
		attribs = ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD;
		extradefines[0] = '\0';

		if (i & DLIGHTDEF_USE_DEFORM_VERTEXES)
		{
			Q_strcat(extradefines, 1024, "#define USE_DEFORM_VERTEXES\n");
		}

		if (!tr.dlightShader[i].initGPUShader( "dlight", attribs, true, extradefines, true, fallbackShader_dlight_vp, fallbackShader_dlight_fp ))
		{
			ri.Error(ERR_FATAL, "Could not load dlight shader!");
		}

		tr.dlightShader[i].initUniforms();
		
		tr.dlightShader[i].setUniformInt( UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );

		tr.dlightShader[i].finishGPUShader();

		numEtcShaders++;
	}


	for (i = 0; i < LIGHTDEF_COUNT; i++)
	{
		int lightType = i & LIGHTDEF_LIGHTTYPE_MASK;
		bool fastLight = !(r_normalMapping->asInt() || r_specularMapping->asInt());

		// skip impossible combos
		if ((i & LIGHTDEF_USE_PARALLAXMAP) && !r_parallaxMapping->asInt())
			continue;

		if ((i & LIGHTDEF_USE_SHADOWMAP) && (!lightType || !r_sunlightMode->asInt()))
			continue;

		if ((i & LIGHTDEF_ENTITY_VERTEX_ANIMATION) && (i & LIGHTDEF_ENTITY_BONE_ANIMATION))
			continue;

		if ((i & LIGHTDEF_ENTITY_BONE_ANIMATION) && !glRefConfig.glslMaxAnimatedBones)
			continue;

		attribs = ATTR_POSITION | ATTR_TEXCOORD | ATTR_COLOR | ATTR_NORMAL;

		extradefines[0] = '\0';

		if (r_dlightMode->asInt() >= 2)
			Q_strcat(extradefines, 1024, "#define USE_SHADOWMAP\n");

		if (glRefConfig.swizzleNormalmap)
			Q_strcat(extradefines, 1024, "#define SWIZZLE_NORMALMAP\n");

		if (lightType)
		{
			Q_strcat(extradefines, 1024, "#define USE_LIGHT\n");

			if (fastLight)
				Q_strcat(extradefines, 1024, "#define USE_FAST_LIGHT\n");

			switch (lightType)
			{
				case LIGHTDEF_USE_LIGHTMAP:
					Q_strcat(extradefines, 1024, "#define USE_LIGHTMAP\n");
					if (r_deluxeMapping->asInt() && !fastLight)
						Q_strcat(extradefines, 1024, "#define USE_DELUXEMAP\n");
					attribs |= ATTR_LIGHTCOORD | ATTR_LIGHTDIRECTION;
					break;
				case LIGHTDEF_USE_LIGHT_VECTOR:
					Q_strcat(extradefines, 1024, "#define USE_LIGHT_VECTOR\n");
					break;
				case LIGHTDEF_USE_LIGHT_VERTEX:
					Q_strcat(extradefines, 1024, "#define USE_LIGHT_VERTEX\n");
					attribs |= ATTR_LIGHTDIRECTION;
					break;
				default:
					break;
			}

			if (r_normalMapping->asInt())
			{
				Q_strcat(extradefines, 1024, "#define USE_NORMALMAP\n");

				attribs |= ATTR_TANGENT;

				if ((i & LIGHTDEF_USE_PARALLAXMAP) && !(i & LIGHTDEF_ENTITY_VERTEX_ANIMATION) && !(i & LIGHTDEF_ENTITY_BONE_ANIMATION) && r_parallaxMapping->asInt())
				{
					Q_strcat(extradefines, 1024, "#define USE_PARALLAXMAP\n");
					if (r_parallaxMapping->asInt() > 1)
						Q_strcat(extradefines, 1024, "#define USE_RELIEFMAP\n");

					if (r_parallaxMapShadows->asInt())
						Q_strcat(extradefines, 1024, "#define USE_PARALLAXMAP_SHADOWS\n");

					Q_strcat(extradefines, 1024, va("#define r_parallaxMapOffset %f\n", r_parallaxMapOffset->asFloat()));
				}
			}

			if (r_specularMapping->asInt())
				Q_strcat(extradefines, 1024, "#define USE_SPECULARMAP\n");

			if (r_cubeMapping->asInt())
			{
				Q_strcat(extradefines, 1024, "#define USE_CUBEMAP\n");
				if (r_cubeMapping->asInt() == 2)
					Q_strcat(extradefines, 1024, "#define USE_BOX_CUBEMAP_PARALLAX\n");
			}
			else if (r_deluxeSpecular->asFloat() > 0.000001f)
			{
				Q_strcat(extradefines, 1024, va("#define r_deluxeSpecular %f\n", r_deluxeSpecular->asFloat()));
			}

			switch (r_glossType->asInt())
			{
				case 0:
				default:
					Q_strcat(extradefines, 1024, "#define GLOSS_IS_GLOSS\n");
					break;
				case 1:
					Q_strcat(extradefines, 1024, "#define GLOSS_IS_SMOOTHNESS\n");
					break;
				case 2:
					Q_strcat(extradefines, 1024, "#define GLOSS_IS_ROUGHNESS\n");
					break;
				case 3:
					Q_strcat(extradefines, 1024, "#define GLOSS_IS_SHININESS\n");
					break;
			}
		}

		if (i & LIGHTDEF_USE_SHADOWMAP)
		{
			Q_strcat(extradefines, 1024, "#define USE_SHADOWMAP\n");

			if (r_sunlightMode->asInt() == 1)
				Q_strcat(extradefines, 1024, "#define SHADOWMAP_MODULATE\n");
			else if (r_sunlightMode->asInt() == 2)
				Q_strcat(extradefines, 1024, "#define USE_PRIMARY_LIGHT\n");
		}

		if (i & LIGHTDEF_USE_TCGEN_AND_TCMOD)
		{
			Q_strcat(extradefines, 1024, "#define USE_TCGEN\n");
			Q_strcat(extradefines, 1024, "#define USE_TCMOD\n");
		}

		if (i & LIGHTDEF_ENTITY_VERTEX_ANIMATION)
		{
			Q_strcat(extradefines, 1024, "#define USE_MODELMATRIX\n");

			if (glRefConfig.gpuVertexAnimation)
			{
				Q_strcat(extradefines, 1024, "#define USE_VERTEX_ANIMATION\n");
				attribs |= ATTR_POSITION2 | ATTR_NORMAL2;

				if (r_normalMapping->asInt())
				{
					attribs |= ATTR_TANGENT2;
				}
			}
		}
		else if (i & LIGHTDEF_ENTITY_BONE_ANIMATION)
		{
			Q_strcat(extradefines, 1024, "#define USE_MODELMATRIX\n");
			Q_strcat(extradefines, 1024, va("#define USE_BONE_ANIMATION\n#define MAX_GLSL_BONES %d\n", glRefConfig.glslMaxAnimatedBones));
			attribs |= ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS;
		}

		if (!tr.lightallShader[i].initGPUShader( "lightall", attribs, true, extradefines, true, fallbackShader_lightall_vp, fallbackShader_lightall_fp ))
		{
			ri.Error(ERR_FATAL, "Could not load lightall shader!");
		}

		tr.lightallShader[i].initUniforms();

		tr.lightallShader[i].setUniformInt( UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
		tr.lightallShader[i].setUniformInt( UNIFORM_LIGHTMAP, TB_LIGHTMAP );
		tr.lightallShader[i].setUniformInt( UNIFORM_NORMALMAP, TB_NORMALMAP );
		tr.lightallShader[i].setUniformInt( UNIFORM_DELUXEMAP, TB_DELUXEMAP );
		tr.lightallShader[i].setUniformInt( UNIFORM_SPECULARMAP, TB_SPECULARMAP );
		tr.lightallShader[i].setUniformInt( UNIFORM_SHADOWMAP, TB_SHADOWMAP );
		tr.lightallShader[i].setUniformInt( UNIFORM_CUBEMAP, TB_CUBEMAP );

		tr.lightallShader[i].finishGPUShader();

		numLightShaders++;
	}

	for (i = 0; i < SHADOWMAPDEF_COUNT; i++)
	{
		if ((i & SHADOWMAPDEF_USE_VERTEX_ANIMATION) && (i & SHADOWMAPDEF_USE_BONE_ANIMATION))
			continue;

		if ((i & SHADOWMAPDEF_USE_VERTEX_ANIMATION) && !glRefConfig.gpuVertexAnimation)
			continue;

		if ((i & SHADOWMAPDEF_USE_BONE_ANIMATION) && !glRefConfig.glslMaxAnimatedBones)
			continue;

		attribs = ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD;

		extradefines[0] = '\0';

		if (i & SHADOWMAPDEF_USE_VERTEX_ANIMATION)
		{
			Q_strcat(extradefines, 1024, "#define USE_VERTEX_ANIMATION\n");
			attribs |= ATTR_POSITION2 | ATTR_NORMAL2;
		}

		if (i & SHADOWMAPDEF_USE_BONE_ANIMATION)
		{
			Q_strcat(extradefines, 1024, va("#define USE_BONE_ANIMATION\n#define MAX_GLSL_BONES %d\n", glRefConfig.glslMaxAnimatedBones));
			attribs |= ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS;
		}

		if (!tr.shadowmapShader[i].initGPUShader( "shadowfill", attribs, true, extradefines, true, fallbackShader_shadowfill_vp, fallbackShader_shadowfill_fp ))
		{
			ri.Error(ERR_FATAL, "Could not load shadowfill shader!");
		}

		tr.shadowmapShader[i].initUniforms();
		tr.shadowmapShader[i].finishGPUShader();

		numEtcShaders++;
	}

	attribs = ATTR_POSITION | ATTR_NORMAL;
	extradefines[0] = '\0';

	Q_strcat(extradefines, 1024, "#define USE_PCF\n#define USE_DISCARD\n");

	if (!tr.pshadowShader.initGPUShader( "pshadow", attribs, true, extradefines, true, fallbackShader_pshadow_vp, fallbackShader_pshadow_fp ))
	{
		ri.Error(ERR_FATAL, "Could not load pshadow shader!");
	}
	
	tr.pshadowShader.initUniforms();

	tr.pshadowShader.setUniformInt( UNIFORM_SHADOWMAP, TB_DIFFUSEMAP );

	tr.pshadowShader.finishGPUShader();

	numEtcShaders++;


	attribs = ATTR_POSITION | ATTR_TEXCOORD;
	extradefines[0] = '\0';

	if (!tr.down4xShader.initGPUShader( "down4x", attribs, true, extradefines, true, fallbackShader_down4x_vp, fallbackShader_down4x_fp ))
	{
		ri.Error(ERR_FATAL, "Could not load down4x shader!");
	}
	
	tr.down4xShader.initUniforms();

	tr.down4xShader.setUniformInt( UNIFORM_TEXTUREMAP, TB_DIFFUSEMAP );

	tr.down4xShader.finishGPUShader();

	numEtcShaders++;


	attribs = ATTR_POSITION | ATTR_TEXCOORD;
	extradefines[0] = '\0';

	if (!tr.bokehShader.initGPUShader( "bokeh", attribs, true, extradefines, true, fallbackShader_bokeh_vp, fallbackShader_bokeh_fp ))
	{
		ri.Error(ERR_FATAL, "Could not load bokeh shader!");
	}

	tr.bokehShader.initUniforms();

	tr.bokehShader.setUniformInt( UNIFORM_TEXTUREMAP, TB_DIFFUSEMAP );

	tr.bokehShader.finishGPUShader();

	numEtcShaders++;


	attribs = ATTR_POSITION | ATTR_TEXCOORD;
	extradefines[0] = '\0';

	if (!tr.tonemapShader.initGPUShader( "tonemap", attribs, true, extradefines, true, fallbackShader_tonemap_vp, fallbackShader_tonemap_fp ))
	{
		ri.Error(ERR_FATAL, "Could not load tonemap shader!");
	}

	tr.tonemapShader.initUniforms();

	tr.tonemapShader.setUniformInt( UNIFORM_TEXTUREMAP, TB_COLORMAP );
	tr.tonemapShader.setUniformInt( UNIFORM_LEVELSMAP, TB_LEVELSMAP );

	tr.tonemapShader.finishGPUShader();

	numEtcShaders++;


	for (i = 0; i < 2; i++)
	{
		attribs = ATTR_POSITION | ATTR_TEXCOORD;
		extradefines[0] = '\0';

		if (!i)
			Q_strcat(extradefines, 1024, "#define FIRST_PASS\n");

		if (!tr.calclevels4xShader[i].initGPUShader( "calclevels4x", attribs, true, extradefines, true, fallbackShader_calclevels4x_vp, fallbackShader_calclevels4x_fp ))
		{
			ri.Error(ERR_FATAL, "Could not load calclevels4x shader!");
		}

		tr.calclevels4xShader[i].initUniforms();

		tr.calclevels4xShader[i].setUniformInt( UNIFORM_TEXTUREMAP, TB_DIFFUSEMAP );

		tr.calclevels4xShader[i].finishGPUShader();

		numEtcShaders++;		
	}


	// GLSL 1.10+ or GL_EXT_shadow_samplers extension are required for sampler2DShadow type
	if (glRefConfig.glslMajorVersion > 1 || (glRefConfig.glslMajorVersion == 1 && glRefConfig.glslMinorVersion >= 10)
	    || glRefConfig.shadowSamplers)
	{
		attribs = ATTR_POSITION | ATTR_TEXCOORD;
		extradefines[0] = '\0';

		if (qglesMajorVersion < 3 && glRefConfig.shadowSamplers)
		{
			Q_strcat(extradefines, 1024, "#extension GL_EXT_shadow_samplers : enable\n");
		}

		if (r_shadowFilter->asInt() >= 1)
			Q_strcat(extradefines, 1024, "#define USE_SHADOW_FILTER\n");

		if (r_shadowFilter->asInt() >= 2)
			Q_strcat(extradefines, 1024, "#define USE_SHADOW_FILTER2\n");

		if (r_shadowCascadeZFar->asInt() != 0)
			Q_strcat(extradefines, 1024, "#define USE_SHADOW_CASCADE\n");

		Q_strcat(extradefines, 1024, va("#define r_shadowMapSize %f\n", r_shadowMapSize->asFloat()));
		Q_strcat(extradefines, 1024, va("#define r_shadowCascadeZFar %f\n", r_shadowCascadeZFar->asFloat()));

		if (!tr.shadowmaskShader.initGPUShader( "shadowmask", attribs, true, extradefines, true, fallbackShader_shadowmask_vp, fallbackShader_shadowmask_fp ))
		{
			ri.Error(ERR_FATAL, "Could not load shadowmask shader!");
		}
	
		tr.shadowmaskShader.initUniforms();

		tr.shadowmaskShader.setUniformInt( UNIFORM_SCREENDEPTHMAP, TB_COLORMAP );
		tr.shadowmaskShader.setUniformInt( UNIFORM_SHADOWMAP, TB_SHADOWMAP );
		tr.shadowmaskShader.setUniformInt( UNIFORM_SHADOWMAP2, TB_SHADOWMAP2 );
		tr.shadowmaskShader.setUniformInt( UNIFORM_SHADOWMAP3, TB_SHADOWMAP3 );
		tr.shadowmaskShader.setUniformInt( UNIFORM_SHADOWMAP4, TB_SHADOWMAP4 );

		tr.shadowmaskShader.finishGPUShader();

		numEtcShaders++;
	}


	if (!tr.greyscaleShader.initGPUShader( "greyscale", attribs, true, extradefines, true, fallbackShader_greyscale_vp, fallbackShader_greyscale_fp ))
	{
		ri.Error(ERR_FATAL, "Unable to load greyscale shader");
	}

	tr.greyscaleShader.initUniforms();
	tr.greyscaleShader.setUniformInt( UNIFORM_TEXTUREMAP, TB_DIFFUSEMAP );
	tr.greyscaleShader.finishGPUShader();

	numEtcShaders++;

	// GLSL 1.10+ or GL_OES_standard_derivatives extension are required for dFdx() and dFdy() GLSL functions
	if (glRefConfig.glslMajorVersion > 1 || (glRefConfig.glslMajorVersion == 1 && glRefConfig.glslMinorVersion >= 10)
	    || glRefConfig.standardDerivatives)
	{
		attribs = ATTR_POSITION | ATTR_TEXCOORD;
		extradefines[0] = '\0';

		if (qglesMajorVersion < 3 && glRefConfig.standardDerivatives)
		{
			Q_strcat(extradefines, 1024, "#extension GL_OES_standard_derivatives : enable\n");
		}

		if (!tr.ssaoShader.initGPUShader( "ssao", attribs, true, extradefines, true, fallbackShader_ssao_vp, fallbackShader_ssao_fp ))
		{
			ri.Error(ERR_FATAL, "Could not load ssao shader!");
		}

		tr.ssaoShader.initUniforms();

		tr.ssaoShader.setUniformInt( UNIFORM_SCREENDEPTHMAP, TB_COLORMAP );

		tr.ssaoShader.finishGPUShader();

		numEtcShaders++;


		for (i = 0; i < 4; i++)
		{
			attribs = ATTR_POSITION | ATTR_TEXCOORD;
			extradefines[0] = '\0';

			if (qglesMajorVersion < 3 && glRefConfig.standardDerivatives)
			{
				Q_strcat(extradefines, 1024, "#extension GL_OES_standard_derivatives : enable\n");
			}

			if (i & 1)
				Q_strcat(extradefines, 1024, "#define USE_VERTICAL_BLUR\n");
			else
				Q_strcat(extradefines, 1024, "#define USE_HORIZONTAL_BLUR\n");

			if (!(i & 2))
				Q_strcat(extradefines, 1024, "#define USE_DEPTH\n");


			if (!tr.depthBlurShader[i].initGPUShader( "depthBlur", attribs, true, extradefines, true, fallbackShader_depthblur_vp, fallbackShader_depthblur_fp ))
			{
				ri.Error(ERR_FATAL, "Could not load depthBlur shader!");
			}
		
			tr.depthBlurShader[i].initUniforms();

			tr.depthBlurShader[i].setUniformInt( UNIFORM_SCREENIMAGEMAP, TB_COLORMAP );
			tr.depthBlurShader[i].setUniformInt( UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );

			tr.depthBlurShader[i].finishGPUShader();

			numEtcShaders++;
		}
	}

#if 0
	attribs = ATTR_POSITION | ATTR_TEXCOORD;
	extradefines[0] = '\0';

	if (!tr.testcubeShader.initGPUShader( "testcube", attribs, true, extradefines, true, NULL, NULL ))
	{
		ri.Error(ERR_FATAL, "Could not load testcube shader!");
	}

	tr.testcubeShader.initUniforms();

	tr.testcubeShader.setUniformInt( UNIFORM_TEXTUREMAP, TB_COLORMAP );

	tr.testcubeShader.finishGPUShader();

	numEtcShaders++;
#endif


	endTime = ri.Milliseconds();

	ri.Printf(PRINT_ALL, "loaded %i GLSL shaders (%i gen %i light %i etc) in %5.2f seconds\n", 
		numGenShaders + numLightShaders + numEtcShaders, numGenShaders, numLightShaders, 
		numEtcShaders, (endTime - startTime) / 1000.0);
}

void GLSL_ShutdownGPUShaders()
{
	int i;

	ri.Printf(PRINT_ALL, "------- GLSL_ShutdownGPUShaders -------\n");

	for (i = 0; i < ATTR_INDEX_COUNT && i < glRefConfig.maxVertexAttribs; i++)
		qglDisableVertexAttribArray(i);

	GL_BindNullProgram();

	for ( i = 0; i < GENERICDEF_COUNT; i++)
		tr.genericShader[i].deleteGPUShader();

	tr.textureColorShader.deleteGPUShader();

	for ( i = 0; i < FOGDEF_COUNT; i++)
		tr.fogShader[i].deleteGPUShader();

	for ( i = 0; i < DLIGHTDEF_COUNT; i++)
		tr.dlightShader[i].deleteGPUShader();

	for ( i = 0; i < LIGHTDEF_COUNT; i++)
		tr.lightallShader[i].deleteGPUShader();

	for ( i = 0; i < SHADOWMAPDEF_COUNT; i++)
		tr.shadowmapShader[i].deleteGPUShader();

	tr.pshadowShader.deleteGPUShader();
	tr.down4xShader.deleteGPUShader();
	tr.bokehShader.deleteGPUShader();
	tr.tonemapShader.deleteGPUShader();

	for ( i = 0; i < 2; i++)
		tr.calclevels4xShader[i].deleteGPUShader();

	tr.shadowmaskShader.deleteGPUShader();
	tr.ssaoShader.deleteGPUShader();

	for ( i = 0; i < 4; i++)
		tr.depthBlurShader[i].deleteGPUShader();
}


void ShaderProgram::bindProgram()
{
	GLuint programObject = this ? this->program : 0;
	const char *name = this ? this->name : "NULL";

	if(r_logFile->asInt())
	{
		// don't just call LogComment, or we will get a call to va() every frame!
		GLimp_LogComment(va("--- (%s)->bindProgram() ---\n", name));
	}

	if (GL_UseProgram(programObject))
		backEnd.pc.c_glslShaderBinds++;
}


ShaderProgram *GLSL_GetGenericShaderProgram(int stage)
{
	shaderStage_t *pStage = tess.xstages[stage];
	int shaderAttribs = 0;

	if (tess.fogNum && pStage->adjustColorsForFog)
	{
		shaderAttribs |= GENERICDEF_USE_FOG;
	}

	switch (pStage->rgbGen)
	{
		case CGEN_LIGHTING_DIFFUSE:
			shaderAttribs |= GENERICDEF_USE_RGBAGEN;
			break;
		default:
			break;
	}

	switch (pStage->alphaGen)
	{
		case AGEN_LIGHTING_SPECULAR:
		case AGEN_PORTAL:
			shaderAttribs |= GENERICDEF_USE_RGBAGEN;
			break;
		default:
			break;
	}

	if (pStage->bundle[0].tcGen != TCGEN_TEXTURE)
	{
		shaderAttribs |= GENERICDEF_USE_TCGEN_AND_TCMOD;
	}

	if (tess.shader->numDeforms && !ShaderRequiresCPUDeforms(tess.shader))
	{
		shaderAttribs |= GENERICDEF_USE_DEFORM_VERTEXES;
	}

	if (glState.vertexAnimation)
	{
		shaderAttribs |= GENERICDEF_USE_VERTEX_ANIMATION;
	}
	else if (glState.boneAnimation)
	{
		shaderAttribs |= GENERICDEF_USE_BONE_ANIMATION;
	}

	if (pStage->bundle[0].numTexMods)
	{
		shaderAttribs |= GENERICDEF_USE_TCGEN_AND_TCMOD;
	}

	return &tr.genericShader[shaderAttribs];
}
