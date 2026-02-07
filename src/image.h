#ifndef IMAGE_H_
#define IMAGE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Image manipulation library

///////////////////////////////////////////////////////////////////////////////
// Helper macros
///////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
    #define CLITERAL(type)      type
#else
    #define CLITERAL(type)      (type)
#endif


///////////////////////////////////////////////////////////////////////////////
// Color theory
///////////////////////////////////////////////////////////////////////////////

typedef struct Color8u3 {
    uint8_t r, g, b;
} Color8u3;

typedef struct Color8u4 {
    uint8_t r, g, b, a;
} Color8u4;

typedef Color8u4 Color;

// Default colors (these taken straight from Raylib)
#define LIGHTGRAY  CLITERAL(Color){ 200, 200, 200, 255 }   // Light Gray
#define GRAY       CLITERAL(Color){ 130, 130, 130, 255 }   // Gray
#define DARKGRAY   CLITERAL(Color){ 80, 80, 80, 255 }      // Dark Gray
#define YELLOW     CLITERAL(Color){ 253, 249, 0, 255 }     // Yellow
#define GOLD       CLITERAL(Color){ 255, 203, 0, 255 }     // Gold
#define ORANGE     CLITERAL(Color){ 255, 161, 0, 255 }     // Orange
#define PINK       CLITERAL(Color){ 255, 109, 194, 255 }   // Pink
#define RED        CLITERAL(Color){ 230, 41, 55, 255 }     // Red
#define MAROON     CLITERAL(Color){ 190, 33, 55, 255 }     // Maroon
#define GREEN      CLITERAL(Color){ 0, 228, 48, 255 }      // Green
#define LIME       CLITERAL(Color){ 0, 158, 47, 255 }      // Lime
#define DARKGREEN  CLITERAL(Color){ 0, 117, 44, 255 }      // Dark Green
#define SKYBLUE    CLITERAL(Color){ 102, 191, 255, 255 }   // Sky Blue
#define BLUE       CLITERAL(Color){ 0, 121, 241, 255 }     // Blue
#define DARKBLUE   CLITERAL(Color){ 0, 82, 172, 255 }      // Dark Blue
#define PURPLE     CLITERAL(Color){ 200, 122, 255, 255 }   // Purple
#define VIOLET     CLITERAL(Color){ 135, 60, 190, 255 }    // Violet
#define DARKPURPLE CLITERAL(Color){ 112, 31, 126, 255 }    // Dark Purple
#define BEIGE      CLITERAL(Color){ 211, 176, 131, 255 }   // Beige
#define BROWN      CLITERAL(Color){ 127, 106, 79, 255 }    // Brown
#define DARKBROWN  CLITERAL(Color){ 76, 63, 47, 255 }      // Dark Brown

#define WHITE      CLITERAL(Color){ 255, 255, 255, 255 }   // White
#define BLACK      CLITERAL(Color){ 0, 0, 0, 255 }         // Black
#define BLANK      CLITERAL(Color){ 0, 0, 0, 0 }           // Blank (Transparent)
#define MAGENTA    CLITERAL(Color){ 255, 0, 255, 255 }     // Magenta
#define RAYWHITE   CLITERAL(Color){ 245, 245, 245, 255 }   // Credits



///////////////////////////////////////////////////////////////////////////////
// Images
///////////////////////////////////////////////////////////////////////////////

typedef enum {
    PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1, // 8 bit per pixel (no alpha)
    PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,    // 8*2 bpp (2 channels)
    PIXELFORMAT_UNCOMPRESSED_R5G6B5,        // 16 bpp
    PIXELFORMAT_UNCOMPRESSED_R8G8B8,        // 24 bpp
    PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,      // 16 bpp (1 bit alpha)
    PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,      // 16 bpp (4 bit alpha)
    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,      // 32 bpp (RGBA, 8 bits per component)
    PIXELFORMAT_UNCOMPRESSED_R32,           // 32 bpp (1 channel - float)
    PIXELFORMAT_UNCOMPRESSED_R32G32B32,     // 32*3 bpp (3 channels - float)
    PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,  // 32*4 bpp (4 channels - float)
    PIXELFORMAT_UNCOMPRESSED_R16,           // 16 bpp (1 channel - half float)
    PIXELFORMAT_UNCOMPRESSED_R16G16B16,     // 16*3 bpp (3 channels - half float)
    PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,  // 16*4 bpp (4 channels - half float)
    PIXELFORMAT_COMPRESSED_DXT1_RGB,        // 4 bpp (no alpha)
    PIXELFORMAT_COMPRESSED_DXT1_RGBA,       // 4 bpp (1 bit alpha)
    PIXELFORMAT_COMPRESSED_DXT3_RGBA,       // 8 bpp
    PIXELFORMAT_COMPRESSED_DXT5_RGBA,       // 8 bpp
    PIXELFORMAT_COMPRESSED_ETC1_RGB,        // 4 bpp
    PIXELFORMAT_COMPRESSED_ETC2_RGB,        // 4 bpp
    PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,   // 8 bpp
    PIXELFORMAT_COMPRESSED_PVRT_RGB,        // 4 bpp
    PIXELFORMAT_COMPRESSED_PVRT_RGBA,       // 4 bpp
    PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,   // 8 bpp
    PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA    // 2 bpp
} Pixel_Format;

// Example implementation
typedef struct Image8_t {
    void *data;
    uint32_t width, height;
    uint64_t bytes_per_pixel;
    int mipmaps;
    int format;

#ifdef __cplusplus
    Image8_t();
#endif
} Image8_t;
typedef Image8_t Image8;



// Row-major order is used for matrices whose data is exactly how it's laid down in memory
// Like how the C/C++ definition
// int m[6] =
// {
// 2, 4, 6,
// 1, 7, 5,
// };
// Represents a row-major matrix.
#define mat_index_rm(x, y, width) (y * width + x)

// Column-major matrices are used for graphics libraries like OpenGL.
// The function
// void glUniformMatrix4fv(GLint location, GLsizei count,
//                         GLboolean transpose, const GLfloat *value);
// from OpenGL provides a transpose option for this reason.
// DirectX would use row-major matrices instead.
#define mat_index_cm(x, y, height) (x * height + y)

#define image_index_rm(x, y, width) (mat_index_rm(x, y, width))
#define image_index_cm(x, y, height) (mat_index_cm(x, y, height))

#define image_set_pixel_indexed(data, pixel, index) (data[(index)] = (pixel))


#define image_blank(image) (memset(image.data, 0, image.width*image.height*image.bytes_per_pixel))


void image8_init(Image8_t* self, uint32_t width, uint32_t height);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // IMAGE_H_
