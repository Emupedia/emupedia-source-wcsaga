/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _GROPENGLEXT_H
#define _GROPENGLEXT_H

#include "globalincs/pstypes.h"
#include "graphics/gropengl.h"

//EXTENSIONS!!!!
//be sure to check for this at startup and handle not finding it gracefully

//to add extensions/functions:
//define an index after the last one, either an extension or a function
//increment NUM_OGL_EXTENSIONS if an extension or NUM_OGL_FUNCTIONS if function
//add function macro for Win32
//add function info to GL_Functions struct
//the structure of extensions/functions are located in gropenglextension.cpp

typedef struct ogl_extension
{
	bool required_to_run;
	bool enabled;
	int num_extensions;
	const char* extension_name[3];
	int num_functions;
	const char* function_names[20];
} ogl_extension;

typedef struct ogl_function
{
	const char* function_name;
	ptr_u function_ptr;
} ogl_function;

extern ogl_function GL_Functions[];
extern ogl_extension GL_Extensions[];
extern ogl_function GL_EXT_Special[];


// Extensions
#define OGL_EXT_FOG_COORD					0
#define OGL_ARB_MULTITEXTURE				1
#define OGL_ARB_TEXTURE_ENV_ADD				2
#define OGL_ARB_TEXTURE_COMPRESSION			3
#define OGL_EXT_TEXTURE_COMPRESSION_S3TC	4
#define OGL_EXT_TEXTURE_FILTER_ANISOTROPIC	5
//#define OGL_NV_FOG_DISTANCE				6
//#define OGL_EXT_SECONDARY_COLOR			7
#define OGL_ARB_TEXTURE_ENV_COMBINE			6
#define OGL_EXT_COMPILED_VERTEX_ARRAY		7
//#define OGL_ARB_TRANSPOSE_MATRIX			10
#define OGL_EXT_DRAW_RANGE_ELEMENTS			8
#define OGL_ARB_TEXTURE_MIRRORED_REPEAT		9
#define OGL_ARB_TEXTURE_NON_POWER_OF_TWO	10
#define OGL_ARB_VERTEX_BUFFER_OBJECT		11
#define OGL_ARB_PIXEL_BUFFER_OBJECT			12
//#define OGL_APPLE_CLIENT_STORAGE			13
#define OGL_SGIS_GENERATE_MIPMAP			13
#define OGL_EXT_FRAMEBUFFER_OBJECT			14
#define OGL_ARB_TEXTURE_RECTANGLE			15
#define OGL_EXT_BGRA						16
#define OGL_ARB_TEXTURE_CUBE_MAP			17
#define OGL_EXT_TEXTURE_LOD_BIAS			18
#define OGL_ARB_POINT_SPRITE				19
#define OGL_ARB_SHADING_LANGUAGE_100		20
#define OGL_ARB_SHADER_OBJECTS				21
#define OGL_ARB_VERTEX_SHADER				22
#define OGL_ARB_FRAGMENT_SHADER				23
#define OGL_SM30							24

#define NUM_OGL_EXTENSIONS					25


// Functions
#define OGL_FOG_COORDF						0			// for better looking fog
#define OGL_FOG_COORD_POINTER				1			// used with vertex arrays
#define OGL_MULTI_TEX_COORD_2F				2			// multitex coordinates
#define OGL_ACTIVE_TEXTURE					3			// currenly active multitexture
#define OGL_CLIENT_ACTIVE_TEXTURE			4
#define OGL_COMPRESSED_TEX_IMAGE_2D			5			// 2d compressed texture
#define OGL_COMPRESSED_TEX_SUB_IMAGE_2D		6			// 2d compressed sub texture
#define OGL_GET_COMPRESSED_TEX_IMAGE		7
//#define OGL_SECONDARY_COLOR_3FV			8			// for better looking fog
//#define OGL_SECONDARY_COLOR_3UBV			9			// specular
#define OGL_LOCK_ARRAYS						8			// HTL
#define OGL_UNLOCK_ARRAYS					9			// HTL
//#define OGL_LOAD_TRANSPOSE_MATRIX_F		12			
//#define OGL_MULT_TRANSPOSE_MATRIX_F		13
#define OGL_DRAW_RANGE_ELEMENTS				10
#define OGL_BIND_BUFFER						11
#define OGL_DELETE_BUFFERS					12
#define OGL_GEN_BUFFERS						13
#define OGL_BUFFER_DATA						14
#define OGL_BUFFER_SUB_DATA					15
#define OGL_MAP_BUFFER						16
#define OGL_UNMAP_BUFFER					17
#define OGL_IS_RENDERBUFFER					18
#define OGL_BIND_RENDERBUFFER				19
#define OGL_DELETE_RENDERBUFFERS			20
#define OGL_GEN_RENDERBUFFERS				21
#define OGL_RENDERBUFFER_STORAGE			22
#define OGL_GET_RENDERBUFFER_PARAMETER_IV	23
#define OGL_IS_FRAMEBUFFER					24
#define OGL_BIND_FRAMEBUFFER				25
#define OGL_DELETE_FRAMEBUFFERS				26
#define OGL_GEN_FRAMEBUFFERS				27
#define OGL_CHECK_FRAMEBUFFER_STATUS		28
#define OGL_FRAMEBUFFER_TEXTURE_2D			29
#define OGL_FRAMEBUFFER_RENDERBUFFER		30
#define OGL_GET_FRAMEBUFFER_ATTACHMENT_PARAMETER_IV		31
#define OGL_GENERATE_MIPMAP					32
#define OGL_DELETE_OBJECT					33
#define OGL_CREATE_SHADER_OBJECT			34
#define OGL_SHADER_SOURCE					35
#define OGL_COMPILE_SHADER					36
#define OGL_GET_OBJECT_PARAMETERIV			37
#define OGL_GET_INFO_LOG					38
#define OGL_CREATE_PROGRAM_OBJECT			39
#define OGL_ATTACH_OBJECT					40
#define OGL_LINK_PROGRAM					41
#define OGL_USE_PROGRAM_OBJECT				42
#define OGL_VALIDATE_PROGRAM				43
#define OGL_ENABLE_VERTEX_ATTRIB_ARRAY		44
#define OGL_DISABLE_VERTEX_ATTRIB_ARRAY		45
#define OGL_GET_ATTRIB_LOCATION				46
#define OGL_VERTEX_ATTRIB_POINTER			47
#define OGL_GET_UNIFORM_LOCATION			48
#define OGL_GET_UNIFORMIV					49
#define OGL_UNIFORM1F						50
#define OGL_UNIFORM3F						51
#define OGL_UNIFORM3FV						52
#define OGL_UNIFORM4FV						53
#define OGL_UNIFORM1I						54
#define OGL_UNIFORM_MATRIX4FV				55

#define NUM_OGL_FUNCTIONS					56


// special extensions/functions (OS specific, non-GL stuff)
#define OGL_SPC_WGL_SWAP_INTERVAL		0
#define OGL_SPC_GLX_SWAP_INTERVAL		1

#define NUM_OGL_EXT_SPECIAL				2


#define Is_Extension_Enabled(x)		GL_Extensions[x].enabled

void opengl_extensions_init();


#define GLEXT_CALL(i, x) if (GL_Functions[i].function_ptr) \
							((x)GL_Functions[i].function_ptr)

// the same as GLEXT_CALL() except that it can be used with a cast or in an if statement
// this doesn't do NULL ptr checking so you have to be careful with it!
#define GLEXT_CALL2(i, x) ((x)GL_Functions[i].function_ptr)

#define GLEXT_SPC_CALL(i, x) if (GL_EXT_Special[i].function_ptr) \
							((x)GL_EXT_Special[i].function_ptr)


#ifdef __APPLE__
// special one, since it's a core feature
typedef void (* glDrawRangeElementsProcPtr) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

// OS X doesn't have the PFN* names so we have to use the real OSX function ptrs
#define PFNGLFOGCOORDFEXTPROC					glFogCoordfEXTProcPtr
#define PFNGLFOGCOORDPOINTEREXTPROC				glFogCoordPointerEXTProcPtr
#define PFNGLMULTITEXCOORD2FARBPROC				glMultiTexCoord2fARBProcPtr
#define PFNGLACTIVETEXTUREARBPROC				glActiveTextureARBProcPtr
#define PFNGLCLIENTACTIVETEXTUREARBPROC			glClientActiveTextureARBProcPtr
#define PFNGLCOMPRESSEDTEXIMAGE2DPROC			glCompressedTexImage2DARBProcPtr
#define PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC		glCompressedTexSubImage2DARBProcPtr
#define PFNGLGETCOMPRESSEDTEXIMAGEARBPROC		glGetCompressedTexImageARBProcPtr
//#define PFNGLSECONDARYCOLOR3FVEXTPROC			glSecondaryColor3fvEXTProcPtr
//#define PFNGLSECONDARYCOLOR3UBVEXTPROC		glSecondaryColor3ubvEXTProcPtr
#define PFNGLLOCKARRAYSEXTPROC					glLockArraysEXTProcPtr
#define PFNGLUNLOCKARRAYSEXTPROC				glUnlockArraysEXTProcPtr
//#define PFNGLLOADTRANSPOSEMATRIXFARBPROC		glLoadTransposeMatrixfARBProcPtr
//#define PFNGLMULTTRANSPOSEMATRIXFARBPROC		glMultTransposeMatrixfARBProcPtr
#define PFNGLDRAWRANGEELEMENTSPROC				glDrawRangeElementsProcPtr
#define PFNGLBINDBUFFERARBPROC					glBindBufferARBProcPtr
#define PFNGLDELETEBUFFERSARBPROC				glDeleteBuffersARBProcPtr
#define PFNGLGENBUFFERSARBPROC					glGenBuffersARBProcPtr
#define PFNGLBUFFERDATAARBPROC					glBufferDataARBProcPtr
#define PFNGLBUFFERSUBDATAARBPROC				glBufferSubDataARBProcPtr
#define PFNGLMAPBUFFERARBPROC					glMapBufferARBProcPtr
#define PFNGLUNMAPBUFFERARBPROC					glUnmapBufferARBProcPtr
#define PFNGLISRENDERBUFFEREXTPROC				glIsRenderbufferEXTProcPtr
#define PFNGLBINDRENDERBUFFEREXTPROC			glBindRenderbufferEXTProcPtr
#define PFNGLDELETERENDERBUFFERSEXTPROC			glDeleteRenderbuffersEXTProcPtr
#define PFNGLGENRENDERBUFFERSEXTPROC			glGenRenderbuffersEXTProcPtr
#define PFNGLRENDERBUFFERSTORAGEEXTPROC			glRenderbufferStorageEXTProcPtr
#define PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC	glGetRenderbufferParameterivEXTProcPtr
#define PFNGLISFRAMEBUFFEREXTPROC				glIsFramebufferEXTProcPtr
#define PFNGLBINDFRAMEBUFFEREXTPROC				glBindFramebufferEXTProcPtr
#define PFNGLDELETEFRAMEBUFFERSEXTPROC			glDeleteFramebuffersEXTProcPtr
#define PFNGLGENFRAMEBUFFERSEXTPROC				glGenFramebuffersEXTProcPtr
#define PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC		glCheckFramebufferStatusEXTProcPtr
#define PFNGLFRAMEBUFFERTEXTURE2DEXTPROC		glFramebufferTexture2DEXTProcPtr
#define PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC		glFramebufferRenderbufferEXTProcPtr
#define PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC	glGetFramebufferAttachmentParameterivEXTProcPtr
#define PFNGLGENERATEMIPMAPEXTPROC				glGenerateMipmapEXTProcPtr
#define PFNGLDELETEOBJECTARBPROC				glDeleteObjectARBProcPtr
#define PFNGLCREATESHADEROBJECTARBPROC			glCreateShaderObjectARBProcPtr
#define PFNGLSHADERSOURCEARBPROC				glShaderSourceARBProcPtr
#define PFNGLCOMPILESHADERARBPROC				glCompileShaderARBProcPtr
#define PFNGLGETOBJECTPARAMETERIVARBPROC		glGetObjectParameterivARBProcPtr
#define PFNGLGETINFOLOGARBPROC					glGetInfoLogARBProcPtr
#define PFNGLCREATEPROGRAMOBJECTARBPROC			glCreateProgramObjectARBProcPtr
#define PFNGLATTACHOBJECTARBPROC				glAttachObjectARBProcPtr
#define PFNGLLINKPROGRAMARBPROC					glLinkProgramARBProcPtr
#define PFNGLUSEPROGRAMOBJECTARBPROC			glUseProgramObjectARBProcPtr
#define PFNGLVALIDATEPROGRAMARBPROC				glValidateProgramARBProcPtr
#define PFNGLENABLEVERTEXATTRIBARRAYARBPROC		glEnableVertexAttribArrayARBProcPtr
#define PFNGLDISABLEVERTEXATTRIBARRAYARBPROC	glDisableVertexAttribArrayARBProcPtr
#define PFNGLGETATTRIBLOCATIONARBPROC			glGetAttribLocationARBProcPtr
#define PFNGLVERTEXATTRIBPOINTERARBPROC			glVertexAttribPointerARBProcPtr
#define PFNGLGETUNIFORMLOCATIONARBPROC			glGetUniformLocationARBProcPtr
#define PFNGLGETUNIFORMIVARBPROC				glGetUniformivARBProcPtr
#define PFNGLUNIFORM1FARBPROC					glUniform1fARBProcPtr
#define PFNGLUNIFORM3FARBPROC					glUniform3fARBProcPtr
#define PFNGLUNIFORM3FVARBPROC					glUniform3fvARBProcPtr
#define PFNGLUNIFORM4FVARBPROC					glUnifrom4fvARBProcPtr
#define PFNGLUNIFORM1IARBPROC					glUniform1iARBProcPtr
#define PFNGLUNIFORMMATRIX4FVARBPROC			glUniformMatrix4fvARBProcPtr
#endif	// __APPLE__

#define vglFogCoordfEXT					GLEXT_CALL( OGL_FOG_COORDF, PFNGLFOGCOORDFEXTPROC )
#define vglFogCoordPointerEXT			GLEXT_CALL( OGL_FOG_COORD_POINTER, PFNGLFOGCOORDPOINTEREXTPROC )
#define vglMultiTexCoord2fARB			GLEXT_CALL( OGL_MULTI_TEX_COORD_2F, PFNGLMULTITEXCOORD2FARBPROC )
#define vglActiveTextureARB				GLEXT_CALL( OGL_ACTIVE_TEXTURE, PFNGLACTIVETEXTUREARBPROC )
#define vglClientActiveTextureARB		GLEXT_CALL( OGL_CLIENT_ACTIVE_TEXTURE, PFNGLCLIENTACTIVETEXTUREARBPROC )
#define vglCompressedTexImage2D			GLEXT_CALL( OGL_COMPRESSED_TEX_IMAGE_2D, PFNGLCOMPRESSEDTEXIMAGE2DPROC )
#define vglCompressedTexSubImage2D		GLEXT_CALL( OGL_COMPRESSED_TEX_SUB_IMAGE_2D, PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC )
#define vglGetCompressedTexImageARB		GLEXT_CALL( OGL_GET_COMPRESSED_TEX_IMAGE, PFNGLGETCOMPRESSEDTEXIMAGEARBPROC )
//#define vglSecondaryColor3fvEXT		GLEXT_CALL( OGL_SECONDARY_COLOR_3FV, PFNGLSECONDARYCOLOR3FVEXTPROC )
//#define vglSecondaryColor3ubvEXT		GLEXT_CALL( OGL_SECONDARY_COLOR_3UBV, PFNGLSECONDARYCOLOR3UBVEXTPROC )
#define vglLockArraysEXT				GLEXT_CALL( OGL_LOCK_ARRAYS, PFNGLLOCKARRAYSEXTPROC )
#define vglUnlockArraysEXT				GLEXT_CALL( OGL_UNLOCK_ARRAYS, PFNGLUNLOCKARRAYSEXTPROC )
//#define vglLoadTransposeMatrixfARB	GLEXT_CALL( OGL_LOAD_TRANSPOSE_MATRIX_F, PFNGLLOADTRANSPOSEMATRIXFARBPROC )
//#define vglMultTransposeMatrixfARB	GLEXT_CALL( OGL_MULT_TRANSPOSE_MATRIX_F, PFNGLMULTTRANSPOSEMATRIXFARBPROC )
#define vglDrawRangeElements			GLEXT_CALL( OGL_DRAW_RANGE_ELEMENTS, PFNGLDRAWRANGEELEMENTSPROC )
#define vglBindBufferARB				GLEXT_CALL( OGL_BIND_BUFFER, PFNGLBINDBUFFERARBPROC )
#define vglDeleteBuffersARB				GLEXT_CALL( OGL_DELETE_BUFFERS, PFNGLDELETEBUFFERSARBPROC )
#define vglGenBuffersARB				GLEXT_CALL( OGL_GEN_BUFFERS, PFNGLGENBUFFERSARBPROC )
#define vglBufferDataARB				GLEXT_CALL( OGL_BUFFER_DATA, PFNGLBUFFERDATAARBPROC )
#define vglBufferSubDataARB				GLEXT_CALL( OGL_BUFFER_SUB_DATA, PFNGLBUFFERSUBDATAARBPROC )
#define vglMapBufferARB					GLEXT_CALL2( OGL_MAP_BUFFER, PFNGLMAPBUFFERARBPROC )
#define vglUnmapBufferARB				GLEXT_CALL( OGL_UNMAP_BUFFER, PFNGLUNMAPBUFFERARBPROC )
#define vglIsRenderbufferEXT			GLEXT_CALL2( OGL_IS_RENDERBUFFER, PFNGLISRENDERBUFFEREXTPROC )
#define vglBindRenderbufferEXT			GLEXT_CALL( OGL_BIND_RENDERBUFFER, PFNGLBINDRENDERBUFFEREXTPROC )
#define vglDeleteRenderbuffersEXT		GLEXT_CALL( OGL_DELETE_RENDERBUFFERS, PFNGLDELETERENDERBUFFERSEXTPROC )
#define vglGenRenderbuffersEXT			GLEXT_CALL( OGL_GEN_RENDERBUFFERS, PFNGLGENRENDERBUFFERSEXTPROC )
#define vglRenderbufferStorageEXT		GLEXT_CALL( OGL_RENDERBUFFER_STORAGE, PFNGLRENDERBUFFERSTORAGEEXTPROC )
#define vglGetRenderbufferParameterivEXT	GLEXT_CALL( OGL_GET_RENDERBUFFER_PARAMETER_IV, PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC )
#define vglIsFramebufferEXT				GLEXT_CALL2( OGL_IS_FRAMEBUFFER, PFNGLISFRAMEBUFFEREXTPROC )
#define vglBindFramebufferEXT			GLEXT_CALL( OGL_BIND_FRAMEBUFFER, PFNGLBINDFRAMEBUFFEREXTPROC )
#define vglDeleteFramebuffersEXT		GLEXT_CALL( OGL_DELETE_FRAMEBUFFERS, PFNGLDELETEFRAMEBUFFERSEXTPROC )
#define vglGenFramebuffersEXT			GLEXT_CALL( OGL_GEN_FRAMEBUFFERS, PFNGLGENFRAMEBUFFERSEXTPROC )
#define vglCheckFramebufferStatusEXT	GLEXT_CALL2( OGL_CHECK_FRAMEBUFFER_STATUS, PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC )
#define vglFramebufferTexture2DEXT		GLEXT_CALL( OGL_FRAMEBUFFER_TEXTURE_2D, PFNGLFRAMEBUFFERTEXTURE2DEXTPROC )
#define vglFramebufferRenderbufferEXT	GLEXT_CALL( OGL_FRAMEBUFFER_RENDERBUFFER, PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC )
#define vglGetFramebufferAttachmentParameterivEXT	GLEXT_CALL( OGL_GET_FRAMEBUFFER_ATTACHMENT_PARAMETER_IV, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC )
#define vglGenerateMipmapEXT			GLEXT_CALL( OGL_GENERATE_MIPMAP, PFNGLGENERATEMIPMAPEXTPROC )
#define vglDeleteObjectARB				GLEXT_CALL( OGL_DELETE_OBJECT, PFNGLDELETEOBJECTARBPROC )
#define vglCreateShaderObjectARB		GLEXT_CALL2( OGL_CREATE_SHADER_OBJECT, PFNGLCREATESHADEROBJECTARBPROC )
#define vglShaderSourceARB				GLEXT_CALL( OGL_SHADER_SOURCE, PFNGLSHADERSOURCEARBPROC )
#define vglCompileShaderARB				GLEXT_CALL( OGL_COMPILE_SHADER, PFNGLCOMPILESHADERARBPROC )
#define vglGetObjectParameterivARB		GLEXT_CALL( OGL_GET_OBJECT_PARAMETERIV, PFNGLGETOBJECTPARAMETERIVARBPROC )
#define vglGetInfoLogARB				GLEXT_CALL( OGL_GET_INFO_LOG, PFNGLGETINFOLOGARBPROC )
#define vglCreateProgramObjectARB		GLEXT_CALL2( OGL_CREATE_PROGRAM_OBJECT, PFNGLCREATEPROGRAMOBJECTARBPROC )
#define vglAttachObjectARB				GLEXT_CALL( OGL_ATTACH_OBJECT, PFNGLATTACHOBJECTARBPROC )
#define vglLinkProgramARB				GLEXT_CALL( OGL_LINK_PROGRAM, PFNGLLINKPROGRAMARBPROC )
#define vglUseProgramObjectARB			GLEXT_CALL( OGL_USE_PROGRAM_OBJECT, PFNGLUSEPROGRAMOBJECTARBPROC )
#define vglValidateProgramARB			GLEXT_CALL( OGL_VALIDATE_PROGRAM, PFNGLVALIDATEPROGRAMARBPROC )
#define vglEnableVertexAttribArrayARB	GLEXT_CALL( OGL_ENABLE_VERTEX_ATTRIB_ARRAY, PFNGLENABLEVERTEXATTRIBARRAYARBPROC )
#define vglDisableVertexAttribArrayARB	GLEXT_CALL( OGL_DISABLE_VERTEX_ATTRIB_ARRAY, PFNGLDISABLEVERTEXATTRIBARRAYARBPROC )
#define vglGetAttribLocationARB			GLEXT_CALL2( OGL_GET_ATTRIB_LOCATION, PFNGLGETATTRIBLOCATIONARBPROC )
#define vglVertexAttribPointerARB		GLEXT_CALL( OGL_VERTEX_ATTRIB_POINTER, PFNGLVERTEXATTRIBPOINTERARBPROC )
#define vglGetUniformLocationARB		GLEXT_CALL2( OGL_GET_UNIFORM_LOCATION, PFNGLGETUNIFORMLOCATIONARBPROC )
#define vglGetUniformivARB				GLEXT_CALL( OGL_GET_UNIFORMIV, PFNGLGETUNIFORMIVARBPROC )
#define vglUniform1fARB					GLEXT_CALL( OGL_UNIFORM1F, PFNGLUNIFORM1FARBPROC )
#define vglUniform3fARB					GLEXT_CALL( OGL_UNIFORM3F, PFNGLUNIFORM3FARBPROC )
#define vglUniform3fvARB				GLEXT_CALL( OGL_UNIFORM3FV, PFNGLUNIFORM3FVARBPROC )
#define vglUniform4fvARB				GLEXT_CALL( OGL_UNIFORM3FV, PFNGLUNIFORM4FVARBPROC )
#define vglUniform1iARB					GLEXT_CALL( OGL_UNIFORM1I, PFNGLUNIFORM1IARBPROC )
#define vglUniformMatrix4fvARB			GLEXT_CALL( OGL_UNIFORM_MATRIX4FV, PFNGLUNIFORMMATRIX4FVARBPROC )


// special extensions
#define vwglSwapIntervalEXT			GLEXT_SPC_CALL( OGL_SPC_WGL_SWAP_INTERVAL, PFNWGLSWAPINTERVALEXTPROC )
#define vglXSwapIntervalSGI			GLEXT_SPC_CALL( OGL_SPC_GLX_SWAP_INTERVAL, PFNGLXSWAPINTERVALSGIPROC )

#endif // _GROPENGLEXT_H
