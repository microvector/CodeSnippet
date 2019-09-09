/* 
  环境: OpenGL ES 3.0 随机代码环境下编译
  include:增加 #include <png.h>
  cmake: 增加 png库的引用

  其余和OpenGL ES 3.0 随机代码环境一致


 */
#include <stdlib.h>
#include "esUtil.h"
#include <png.h>
#include <string.h>

typedef struct
{
    // Handle to a program object
    GLuint programObject;

    // Sampler location
    GLint samplerLoc;

    // Texture handle
    GLuint textureId;

} UserData;

/* OpenGL texture info */
struct gl_texture_t
{
    GLsizei width;
    GLsizei height;

    GLenum format;
    GLint internalFormat;
    GLuint id;

    GLubyte *texels;
};

///
//  Generate a png image
//
void GetPNGtextureInfo(int color_type, struct gl_texture_t *texinfo)
{
    switch (color_type)
    {
    case PNG_COLOR_TYPE_GRAY:
        texinfo->format = GL_LUMINANCE;
        texinfo->internalFormat = 1;
        break;

    case PNG_COLOR_TYPE_GRAY_ALPHA:
        texinfo->format = GL_LUMINANCE_ALPHA;
        texinfo->internalFormat = 2;
        break;

    case PNG_COLOR_TYPE_RGB:
        texinfo->format = GL_RGB;
        texinfo->internalFormat = 3;
        break;

    case PNG_COLOR_TYPE_RGB_ALPHA:
        texinfo->format = GL_RGBA;
        texinfo->internalFormat = 4;
        break;

    default:
        /* Badness */
        break;
    }
}

/* Factory to build TexWrapper objects from a given PNG file */
GLuint createTextureFromPng(const char *filename)
{
    // Open the PNG file
    FILE *inputFile = fopen(filename, "rb");
    if (inputFile == 0)
    {
        perror(filename);
        return NULL;
    }

    // Read the file header and validate that it is a PNG
    static const int kSigSize = 8;
    png_byte header[kSigSize];
    fread(header, 1, kSigSize, inputFile);
    if (png_sig_cmp(header, 0, kSigSize))
    {
        printf("%s is not a PNG.\n", filename);
        fclose(inputFile);
        return NULL;
    }

    // Set up our control structure
    png_structp pngControl =
        png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngControl)
    {
        printf("png_create_read_struct failed.\n");
        fclose(inputFile);
        return NULL;
    }

    // Set up our image info structure
    png_infop pngInfo = png_create_info_struct(pngControl);
    if (!pngInfo)
    {
        printf("error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&pngControl, NULL, NULL);
        fclose(inputFile);
        return NULL;
    }

    // Install an error handler
    if (setjmp(png_jmpbuf(pngControl)))
    {
        printf("libpng reported an error\n");
        png_destroy_read_struct(&pngControl, &pngInfo, NULL);
        fclose(inputFile);
        return NULL;
    }

    // Set up the png reader and fetch the remaining bits of the header
    png_init_io(pngControl, inputFile);
    png_set_sig_bytes(pngControl, kSigSize);
    png_read_info(pngControl, pngInfo);

    // Get basic information about the PNG we're reading
    int bitDepth;
    int colorFormat;
    png_uint_32 width;
    png_uint_32 height;
    png_get_IHDR(pngControl, pngInfo, &width, &height, &bitDepth, &colorFormat,
                 NULL, NULL, NULL);

    GLint format;
    switch (colorFormat)
    {
    case PNG_COLOR_TYPE_GRAY:
        format = GL_LUMINANCE;
        break;

    case PNG_COLOR_TYPE_GRAY_ALPHA:
        format = GL_LUMINANCE_ALPHA;
        break;

    case PNG_COLOR_TYPE_RGB:
        format = GL_RGB;
        break;

    case PNG_COLOR_TYPE_RGB_ALPHA:
        format = GL_RGBA;
        break;
    default:
        printf("%s: Unknown libpng color format %d.\n", filename,
               colorFormat);
        return NULL;
    }

    // Refresh the values in the png info struct in case any transformation
    // shave been applied.
    png_read_update_info(pngControl, pngInfo);
    int stride = png_get_rowbytes(pngControl, pngInfo);
    stride += 3 - ((stride - 1) %
                   4); // glTexImage2d requires rows to be 4-byte aligned

    // Allocate storage for the pixel data
    png_byte *buffer = (png_byte *)malloc(stride * height);
    if (buffer == NULL)
    {
        printf("error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&pngControl, &pngInfo, NULL);
        fclose(inputFile);
        return NULL;
    }

    // libpng needs an array of pointers into the image data for each row
    png_byte **rowPointers = (png_byte **)malloc(height * sizeof(png_byte *));
    if (rowPointers == NULL)
    {
        printf("Failed to allocate temporary row pointers\n");
        png_destroy_read_struct(&pngControl, &pngInfo, NULL);
        free(buffer);
        fclose(inputFile);
        return NULL;
    }
    for (unsigned int r = 0; r < height; r++)
    {
        rowPointers[r] = buffer + r * stride;
    }

    // Read in the actual image bytes
    png_read_image(pngControl, rowPointers);
    png_read_end(pngControl, NULL);

    // Set up the OpenGL texture to contain this image
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Send the image data to GL
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, buffer);

    // Initialize the sampling properties (it seems the sample may not work if
    // this isn't done) The user of this texture may very well want to set their
    // own filtering, but we're going to pay the (minor) price of setting this
    // up for them to avoid the dreaded "black image" if they forget.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // clean up
    png_destroy_read_struct(&pngControl, &pngInfo, NULL);
    free(buffer);
    free(rowPointers);
    fclose(inputFile);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Return the textureId
    return textureId;
}

///
// Initialize the shader and program object
//
int Init(ESContext *esContext)
{
    UserData *userData = esContext->userData;
    char vShaderStr[] =
        "#version 300 es                            \n"
        "layout(location = 0) in vec4 a_position;   \n"
        "layout(location = 1) in vec2 a_texCoord;   \n"
        "out vec2 v_texCoord;                       \n"
        "void main()                                \n"
        "{                                          \n"
        "   gl_Position = a_position;               \n"
        "   v_texCoord = a_texCoord;                \n"
        "}                                          \n";

    char fShaderStr[] =
        "#version 300 es                                     \n"
        "precision mediump float;                            \n"
        "in vec2 v_texCoord;                                 \n"
        "layout(location = 0) out vec4 outColor;             \n"
        "uniform sampler2D s_texture;                        \n"
        "void main()                                         \n"
        "{                                                   \n"
        "  outColor = texture( s_texture, v_texCoord );      \n"
        "}                                                   \n";

    // Load the shaders and get a linked program object
    userData->programObject = esLoadProgram(vShaderStr, fShaderStr);

    // Get the sampler location
    userData->samplerLoc = glGetUniformLocation(userData->programObject, "s_texture");

    // Load the texture
    userData->textureId = createTextureFromPng("/home/ding/Downloads/SourceCode/opengles3-book-master/wallpaper_1.png");

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(ESContext *esContext)
{
    UserData *userData = esContext->userData;
    // GLfloat vVertices[] = {
    //     -0.5f, 0.5f, 0.0f,  // Position 0
    //     0.0f, 0.0f,         // TexCoord 0
    //     -0.5f, -0.5f, 0.0f, // Position 1
    //     0.0f, 1.0f,         // TexCoord 1
    //     0.5f, -0.5f, 0.0f,  // Position 2
    //     1.0f, 1.0f,         // TexCoord 2
    //     0.5f, 0.5f, 0.0f,   // Position 3
    //     1.0f, 0.0f          // TexCoord 3
    // };
    GLfloat vVertices[] = {
        -0.5f, 0.5f, 0.0f,  // Position 0
        -0.5f, -0.5f, 0.0f, // Position 1
        0.5f, -0.5f, 0.0f,  // Position 2
        0.5f, 0.5f, 0.0f,   // Position 3
    };

    GLfloat tVertices[] = {
        0.0f, 0.0f, // TexCoord 0
        0.0f, 1.0f, // TexCoord 1
        1.0f, 1.0f, // TexCoord 2
        1.0f, 0.0f  // TexCoord 3
    };

    GLushort indices[] = {0, 1, 2, 0, 2, 3};
    GLuint textureId;

    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->programObject);

    // Load the vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices);
    // Load the texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT,
                          GL_FALSE, 2 * sizeof(GLfloat), tVertices);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->textureId);

    // Set the sampler texture unit to 0
    glUniform1i(userData->samplerLoc, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

///
// Cleanup
//
void ShutDown(ESContext *esContext)
{
    UserData *userData = esContext->userData;

    // Delete texture object
    glDeleteTextures(1, &userData->textureId);

    // Delete program object
    glDeleteProgram(userData->programObject);
}

int esMain(ESContext *esContext)
{
    esContext->userData = malloc(sizeof(UserData));

    esCreateWindow(esContext, "Simple Texture 2D", 1080, 720, ES_WINDOW_RGB);

    if (!Init(esContext))
    {
        return GL_FALSE;
    }

    esRegisterDrawFunc(esContext, Draw);
    esRegisterShutdownFunc(esContext, ShutDown);

    return GL_TRUE;
}
