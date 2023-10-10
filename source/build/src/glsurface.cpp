/*
 * glsurface.cpp
 *  A 32-bit rendering surface that can quickly blit 8-bit paletted buffers implemented in OpenGL.
 *
 * Copyright � 2018, Alex Dawson. All rights reserved.
 */

#include "glsurface.h"
#include "glad/glad.h"

#include "baselayer.h"
#include "build.h"

static int bufferSize;
static void* buffer;
static GLuint bufferTexID;
static vec2_t bufferRes;

static GLuint paletteTexID;

static GLuint quadVertsID = 0;

static GLuint shaderProgramID = 0;
static GLint texSamplerLoc = -1;
static GLint paletteSamplerLoc = -1;
static GLint colorCorrectionLoc = -1;

static GLuint compileShader(GLenum shaderType, const char* const source)
{
    GLuint shaderID = glCreateShader(shaderType);
    if (shaderID == 0)
        return 0;

    const char* const sources[1] = {source};
    glShaderSource(shaderID,
                   1,
                   sources,
                   NULL);
    glCompileShader(shaderID);

    GLint compileStatus;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus)
    {
        GLint logLength;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
        OSD_Printf("Compile Status: %u\n", compileStatus);
        if (logLength > 0)
        {
            char *infoLog = (char*) Xmalloc(logLength);
            glGetShaderInfoLog(shaderID, logLength, &logLength, infoLog);
            OSD_Printf("Log:\n%s\n", infoLog);
            Xfree(infoLog);
        }
    }

    return shaderID;
}

bool glsurface_initialize(vec2_t bufferResolution)
{
    if (buffer)
        glsurface_destroy();

    buildgl_resetStateAccounting();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    bufferRes  = bufferResolution;
    bufferSize = bufferRes.x * bufferRes.y;

    buffer = Xmalloc(bufferSize);

#ifndef USE_GLES2 
    if (!glIsBuffer(quadVertsID))
#endif
        glGenBuffers(1, &quadVertsID);

    buildgl_bindBuffer(GL_ARRAY_BUFFER, quadVertsID);
    const float quadVerts[] =
        {
            -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, //top-left
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, //bottom-left
             1.0f,  1.0f, 0.0f, 1.0f, 0.0f, //top-right
             1.0f, -1.0f, 0.0f, 1.0f, 1.0f  //bottom-right
        };
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    //specify format/arrangement for vertex positions:
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 5, 0);
    //specify format/arrangement for vertex texture coords:
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 5, (const void*) (sizeof(float) * 3));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    buildgl_activeTexture(GL_TEXTURE0);

#ifndef USE_GLES2 
    if (!glIsTexture(bufferTexID))
#endif
        glGenTextures(1, &bufferTexID);

    buildgl_bindTexture(GL_TEXTURE_2D, bufferTexID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, bufferRes.x, bufferRes.y, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);

    glsurface_setPalette(curpalettefaded);

    const char* const VERTEX_SHADER_CODE =
        "#version 110\n\
         \n\
         attribute vec4 i_vertPos;\n\
         attribute vec2 i_texCoord;\n\
         \n\
         varying vec2 v_texCoord;\n\
         \n\
         void main()\n\
         {\n\
             gl_Position = i_vertPos;\n\
             v_texCoord = i_texCoord;\n\
         }\n";
    const char* const FRAGMENT_SHADER_CODE =
        "#version 120\n\
         \n\
         //s_texture points to an indexed color texture\n\
         uniform sampler2D s_texture;\n\
         //s_palette is the palette texture\n\
         uniform sampler2D s_palette;\n\
         \n\
         varying vec2 v_texCoord;\n\
         \n\
         const float c_paletteScale = 255.0/256.0;\n\
         const float c_paletteOffset = 0.5/256.0;\n\
         uniform vec4 u_colorCorrection;\n\
         const vec2 c_vec2_zero_one = vec2(0.0, 1.0);\n\
         const vec4 c_vec4_luma_709 = vec4(0.2126, 0.7152, 0.0722, 0.0);\n\
         \n\
         void main()\n\
         {\n\
             vec4 color = texture2D(s_texture, v_texCoord.xy);\n\
             color.r = c_paletteScale * color.a + c_paletteOffset;\n\
             color.rgb = texture2D(s_palette, color.rg).rgb;\n\
             \n\
             // DEBUG \n\
             //color = texture2D(s_palette, v_texCoord.xy);\n\
             //color = texture2D(s_texture, v_texCoord.xy);\n\
             \n\
             vec4 v_cc = vec4(u_colorCorrection.x - 1.0, 0.5 * -(u_colorCorrection.y - 1.0), -(u_colorCorrection.z - 1.0), 1.0);\n\
             gl_FragColor = mat4(c_vec2_zero_one.yxxx, c_vec2_zero_one.xyxx, c_vec2_zero_one.xxyx, v_cc.xxxw)\n\
                          * mat4(u_colorCorrection.ywww, u_colorCorrection.wyww, u_colorCorrection.wwyw, v_cc.yyyw)\n\
                          * mat4((c_vec4_luma_709.xxxw * v_cc.z) + u_colorCorrection.zwww,\n\
                                 (c_vec4_luma_709.yyyw * v_cc.z) + u_colorCorrection.wzww,\n\
                                 (c_vec4_luma_709.zzzw * v_cc.z) + u_colorCorrection.wwzw,\n\
                                 c_vec2_zero_one.xxxy)\n\
                          * color;\n\
         }\n";

#ifndef USE_GLES2 
    if (!glIsProgram(shaderProgramID))
#endif
    {
        shaderProgramID = glCreateProgram();

        GLuint vertexShaderID   = compileShader(GL_VERTEX_SHADER, VERTEX_SHADER_CODE);
        GLuint fragmentShaderID = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_CODE);
        glAttachShader(shaderProgramID, vertexShaderID);
        glAttachShader(shaderProgramID, fragmentShaderID);
        glBindAttribLocation(shaderProgramID, 0, "i_vertPos");
        glBindAttribLocation(shaderProgramID, 1, "i_texCoord");
        glLinkProgram(shaderProgramID);
        glDetachShader(shaderProgramID, vertexShaderID);
        glDeleteShader(vertexShaderID);
        glDetachShader(shaderProgramID, fragmentShaderID);
        glDeleteShader(fragmentShaderID);
    }
    buildgl_useShaderProgram(shaderProgramID);

    texSamplerLoc = glGetUniformLocation(shaderProgramID, "s_texture");
    paletteSamplerLoc = glGetUniformLocation(shaderProgramID, "s_palette");
    colorCorrectionLoc = glGetUniformLocation(shaderProgramID, "u_colorCorrection");

    glUniform1i(texSamplerLoc, 0);
    glUniform1i(paletteSamplerLoc, 1);
    glUniform4f(colorCorrectionLoc, g_glColorCorrection.x, g_glColorCorrection.y, g_glColorCorrection.z, g_glColorCorrection.w);

    return true;
}

void glsurface_destroy()
{
    if (!buffer)
        return;

    DO_FREE_AND_NULL(buffer);

    glDeleteBuffers(1, &quadVertsID);
    quadVertsID = 0;

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glDeleteTextures(1, &bufferTexID);
    bufferTexID = 0;
    glDeleteTextures(1, &paletteTexID);
    paletteTexID = 0;

    buildgl_useShaderProgram(0);
    glDeleteProgram(shaderProgramID);
    shaderProgramID = 0;
}

void glsurface_setPalette(void* pPalette)
{
    if (!buffer || !pPalette)
        return;

    buildgl_activeTexture(GL_TEXTURE1);

    if (paletteTexID)
    {
        buildgl_bindTexture(GL_TEXTURE_2D, paletteTexID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void*) pPalette);
    }
    else
    {
        glGenTextures(1, &paletteTexID);
        buildgl_bindTexture(GL_TEXTURE_2D, paletteTexID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pPalette);
    }
}

void* glsurface_getBuffer()
{
    return buffer;
}

vec2_t glsurface_getBufferResolution()
{
    return bufferRes;
}

void glsurface_blitBuffer()
{
    if (!buffer)
        return;


    if (colorCorrectionLoc != -1)
        glUniform4f(colorCorrectionLoc, g_glColorCorrection.x, g_glColorCorrection.y, g_glColorCorrection.z, g_glColorCorrection.w);

#ifdef __ANDROID__ // Touch controls clobber GL state
    buildgl_useShaderProgram(shaderProgramID);
    glDisable(GL_BLEND);

    buildgl_activeTexture(GL_TEXTURE0);
    buildgl_activeTexture(GL_TEXTURE1);
    buildgl_bindTexture(GL_TEXTURE_2D, paletteTexID);

    buildgl_bindBuffer(GL_ARRAY_BUFFER, quadVertsID);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 5, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 5, (const void*) (sizeof(float) * 3));

    buildgl_activeTexture(GL_TEXTURE0);
    buildgl_bindTexture(GL_TEXTURE_2D, bufferTexID);
#endif

    buildgl_activeTexture(GL_TEXTURE0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bufferRes.x, bufferRes.y, GL_ALPHA, GL_UNSIGNED_BYTE, (void*) buffer);

    glDrawArrays(GL_TRIANGLE_STRIP,
                 0,
                 4);
}

void glsurface_refresh()
{
    buildgl_activeTexture(GL_TEXTURE0);
    buildgl_bindTexture(GL_TEXTURE_2D, bufferTexID);
}
