
/*  target ： 使用此段代码可以实现用C/C++对png格式为图片进行读取并创建纹理
    param ：   需要读取的图片的路径和文件名
    return： 返回创建好的textureId
    include: #include<png.h>
    lib:    libpng
*/


 GLuint CreateTextureFromPng(const char *filename) {

    // Open the PNG file
    FILE* inputFile = fopen(filename, "rb");
    if (inputFile == 0) {
        perror(filename);
        return NULL;
    }

    // Read the file header and validate that it is a PNG
    static const int kSigSize = 8;
    png_byte header[kSigSize];
    fread(header, 1, kSigSize, inputFile);
    if (png_sig_cmp(header, 0, kSigSize)) {
        printf("%s is not a PNG.\n", filename);
        fclose(inputFile);
        return NULL;
    }

    // Set up our control structure
    png_structp pngControl =
        png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngControl) {
        printf("png_create_read_struct failed.\n");
        fclose(inputFile);
        return NULL;
    }

    // Set up our image info structure
    png_infop pngInfo = png_create_info_struct(pngControl);
    if (!pngInfo) {
        printf("error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&pngControl, NULL, NULL);
        fclose(inputFile);
        return NULL;
    }

    // Install an error handler
    if (setjmp(png_jmpbuf(pngControl))) {
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
    switch (colorFormat) {
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
                   4);  // glTexImage2d requires rows to be 4-byte aligned

    // Allocate storage for the pixel data
    png_byte* buffer = (png_byte*)malloc(stride * height);
    if (buffer == NULL) {
        printf("error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&pngControl, &pngInfo, NULL);
        fclose(inputFile);
        return NULL;
    }

    // libpng needs an array of pointers into the image data for each row
    png_byte** rowPointers = (png_byte**)malloc(height * sizeof(png_byte*));
    if (rowPointers == NULL) {
        printf("Failed to allocate temporary row pointers\n");
        png_destroy_read_struct(&pngControl, &pngInfo, NULL);
        free(buffer);
        fclose(inputFile);
        return NULL;
    }
    for (unsigned int r = 0; r < height; r++) {
        rowPointers[r] = buffer + r * stride;
    }

    // Read in the actual image bytes
    png_read_image(pngControl, rowPointers);
    png_read_end(pngControl, NULL);

    // Set up the OpenGL texture to contain this image
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    mWidth = width;
    mHeight = height;

    // Send the image data to GL
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, buffer);

    // Initialize the sampling properties (it seems the sample may not work if
    // this isn't done) The user of this texture may very well want to set their
    // own filtering, but we're going to pay the (minor) price of setting this
    // up for them to avoid the dreaded "black image" if they forget.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
    // clean up
    png_destroy_read_struct(&pngControl, &pngInfo, NULL);
    free(buffer);
    free(rowPointers);
    fclose(inputFile);

    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;;
}

