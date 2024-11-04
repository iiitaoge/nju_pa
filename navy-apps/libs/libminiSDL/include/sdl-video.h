#ifndef __SDL_VIDEO_H__
#define __SDL_VIDEO_H__

#define SDL_HWSURFACE 0x1
#define SDL_PHYSPAL 0x2
#define SDL_LOGPAL 0x4
#define SDL_SWSURFACE  0x8
#define SDL_PREALLOC  0x10
#define SDL_FULLSCREEN 0x20
#define SDL_RESIZABLE  0x40

#define DEFAULT_RMASK 0x00ff0000
#define DEFAULT_GMASK 0x0000ff00
#define DEFAULT_BMASK 0x000000ff
#define DEFAULT_AMASK 0xff000000

typedef struct {
    int16_t x, y;      // 矩形左上角坐标
    uint16_t w, h;     // 矩形的宽度和高度
} SDL_Rect;

typedef union {
  struct {
    uint8_t r, g, b, a;
  };
  uint32_t val;
} SDL_Color;

typedef struct {
	int ncolors;
	SDL_Color *colors;
} SDL_Palette;

// 使 SDL 适应不同的像素格式和像素深度
typedef struct {
	SDL_Palette *palette;
	uint8_t BitsPerPixel;
	uint8_t BytesPerPixel;
	uint8_t Rloss, Gloss, Bloss, Aloss;
	uint8_t Rshift, Gshift, Bshift, Ashift;
	uint32_t Rmask, Gmask, Bmask, Amask;
} SDL_PixelFormat;

/*
假设一个图像的宽度为 w = 100 像素，并且每个像素为 4 字节（比如 32 位颜色深度），那么：
pitch 通常是 100 * 4 = 400 字节，表示一行数据的实际字节数。
*/
typedef struct {
    uint32_t flags;          // 表面标志
    SDL_PixelFormat *format; // 像素格式（颜色格式）
    int w, h;                // 表面的宽度和高度
    uint16_t pitch;          // 每行像素字节数
    uint8_t *pixels;         // 像素数据 (8位 or 32位)
} SDL_Surface;

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
    uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags);
void SDL_FreeSurface(SDL_Surface *s);
void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color);
void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h);
void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors);
SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags);
uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
int SDL_LockSurface(SDL_Surface *s);
void SDL_UnlockSurface(SDL_Surface *s);

#endif
