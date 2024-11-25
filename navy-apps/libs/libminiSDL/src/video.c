#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/*
SDL_Surface *src: 源图像的指针。
SDL_Rect *srcrect: 指定源图像的矩形区域，定义要拷贝的区域，若为 NULL 则表示整个源图像。
SDL_Surface *dst: 目标图像的指针，通常是屏幕缓冲区。
SDL_Rect *dstrect: 指定目标图像的矩形区域，表示源图像将拷贝到目标图像的哪个位置。如果为 NULL，则默认放置在左上角。
*/
void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(dst && src);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BytesPerPixel == 1 || dst->format->BytesPerPixel == 4);
  int swidth = src->w;
  int sheight = src->h;
  int sw = (srcrect == NULL ? swidth : srcrect->w);
  int sh = (srcrect == NULL ? sheight : srcrect->h);
  int sstart = (srcrect == NULL ? 0 : srcrect->y * swidth + srcrect->x); // 如果srcrect为NULL，就把sstart设为0
  int dwidth = dst->w;
  int dheight = dst->h;
  // 不需要具体的 x, y x与y只是为了确定起点在哪
  int dstart = (dstrect == NULL ? 0 : dstrect->y * dwidth + dstrect->x); // 如果dstrect为NULL，就把dstart设为0
  for (int i = 0; i < sh; i++)
  {
    if (dst->format->BytesPerPixel == 1) // 像素为 1字节
    {
      memcpy(dst->pixels + dstart + i * dwidth, src->pixels + sstart + i * swidth, sw);
    }
    else  // 像素为 4 字节
    {
      memcpy((uint32_t *)dst->pixels + dstart + i * dwidth, (uint32_t *)src->pixels + sstart + i * swidth, 4 * sw);
    }
  }
}
/*
SDL_Surface *dst: 目标图像的指针，即要填充的表面。
SDL_Rect *dstrect: 指定填充的矩形区域，若为 NULL 则填充整个目标表面。
uint32_t color: 指定填充的颜色，颜色格式通常由目标表面的 SDL_PixelFormat 定义（例如 RGB、RGBA）。
*/
void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color) {
  assert(dst);
  assert(dst->format->BytesPerPixel == 1 || dst->format->BytesPerPixel == 4);
  int width = dst->w;
  int height = dst->h;
  // dstrect为NULL时，把整个dst填充为color
  int w = (dstrect == NULL ? width : (int)dstrect->w);                 // 如果dstrect为NULL，就把w设为width
  int h = (dstrect == NULL ? height : (int)dstrect->h);                // 如果dstrect为NULL，就把h设为height
  int start = (dstrect == NULL ? 0 : dstrect->y * width + dstrect->x); // 如果dstrect为NULL，就把start设为0
  for (int i = 0; i < h; ++i)
  {
    if (dst->format->BytesPerPixel == 1)  // 启用调色板
    {
      SDL_Color c;
      c.a = (color >> 24) & 0xff;
      c.r = (color >> 16) & 0xff;
      c.g = (color >> 8) & 0xff;
      c.b = color & 0xff;
      for (int j = 0; j < w; ++j)
      {
        dst->format->palette->colors[dst->pixels[start + i * width + j]] = c;
      }
    }
    else
    {
      for (int j = 0; j < w; ++j)
      {
        dst->pixels[4 * (start + i * width + j)] = color & 0xff;
        dst->pixels[4 * (start + i * width + j) + 1] = (color >> 8) & 0xff;
        dst->pixels[4 * (start + i * width + j) + 2] = (color >> 16) & 0xff;
        dst->pixels[4 * (start + i * width + j) + 3] = (color >> 24) & 0xff;
      }
    }
  }
}


// 如果 ' x '， ' y '， ' w ' 和 ' h ' 均为 0， SDL_UpdateRect 将更新整个屏幕。
void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h) {
  if (x == 0 && y == 0 && w == 0 && h == 0)
  {
    w = s->w;
    h = s->h;
  }
  uint32_t *pixels = malloc(w * h * sizeof(uint32_t));
  int pos = 0;
  int start = y * s->w + x;
  for (int i = 0; i < h; ++i)
  {
    for (int j = 0; j < w; ++j)
    {
      int index = i * s->w + j;
      if (s->format->BytesPerPixel == 1)  // 启用调色板
      {
        SDL_Color color = s->format->palette->colors[s->pixels[start + index]];
        pixels[pos++] = color.a << 24 | color.r << 16 | color.g << 8 | color.b;
      }
      else
      {
        pixels[pos++] = s->pixels[start + 4 * index + 3] << 24 | s->pixels[start + 4 * index + 2] << 16 | s->pixels[start + 4 * index + 1] << 8 | s->pixels[start + 4 * index];
      }
    }
  }
  NDL_DrawRect(pixels, x, y, w, h);
  free(pixels);
}

// APIs below are already implemented.

static inline int maskToShift(uint32_t mask) {
  switch (mask) {
    case 0x000000ff: return 0;
    case 0x0000ff00: return 8;
    case 0x00ff0000: return 16;
    case 0xff000000: return 24;
    case 0x00000000: return 24; // hack
    default: assert(0);
  }
}

SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
  uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  assert(depth == 8 || depth == 32);
  SDL_Surface *s = malloc(sizeof(SDL_Surface));
  assert(s);
  s->flags = flags;
  s->format = malloc(sizeof(SDL_PixelFormat));
  assert(s->format);
  if (depth == 8) {
    s->format->palette = malloc(sizeof(SDL_Palette));
    assert(s->format->palette);
    s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
    assert(s->format->palette->colors);
    memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
    s->format->palette->ncolors = 256;
  } else {
    s->format->palette = NULL;
    s->format->Rmask = Rmask; s->format->Rshift = maskToShift(Rmask); s->format->Rloss = 0;
    s->format->Gmask = Gmask; s->format->Gshift = maskToShift(Gmask); s->format->Gloss = 0;
    s->format->Bmask = Bmask; s->format->Bshift = maskToShift(Bmask); s->format->Bloss = 0;
    s->format->Amask = Amask; s->format->Ashift = maskToShift(Amask); s->format->Aloss = 0;
  }

  s->format->BitsPerPixel = depth;
  s->format->BytesPerPixel = depth / 8;

  s->w = width;
  s->h = height;
  s->pitch = width * depth / 8;
  assert(s->pitch == width * s->format->BytesPerPixel);

  if (!(flags & SDL_PREALLOC)) {
    s->pixels = malloc(s->pitch * height);
    assert(s->pixels);
  }

  return s;
}

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
      Rmask, Gmask, Bmask, Amask);
  assert(pitch == s->pitch);
  s->pixels = pixels;
  return s;
}

void SDL_FreeSurface(SDL_Surface *s) {
  if (s != NULL) {
    if (s->format != NULL) {
      if (s->format->palette != NULL) {
        if (s->format->palette->colors != NULL) free(s->format->palette->colors);
        free(s->format->palette);
      }
      free(s->format);
    }
    if (s->pixels != NULL && !(s->flags & SDL_PREALLOC)) free(s->pixels);
    free(s);
  }
}

SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags) {
  if (flags & SDL_HWSURFACE) NDL_OpenCanvas(&width, &height);
  return SDL_CreateRGBSurface(flags, width, height, bpp,
      DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
}

void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(src && dst);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BitsPerPixel == 8);

  int x = (srcrect == NULL ? 0 : srcrect->x);
  int y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  assert(dstrect);
  if(w == dstrect->w && h == dstrect->h) {
    /* The source rectangle and the destination rectangle
     * are of the same size. If that is the case, there
     * is no need to stretch, just copy. */
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_BlitSurface(src, &rect, dst, dstrect);
  }
  else {
    assert(0);
  }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
  assert(s);
  assert(s->format);
  assert(s->format->palette);
  assert(firstcolor == 0);

  s->format->palette->ncolors = ncolors;  // 这里设置了 palette
  memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

  if(s->flags & SDL_HWSURFACE) {
    assert(ncolors == 256);
    for (int i = 0; i < ncolors; i ++) {
      uint8_t r = colors[i].r;
      uint8_t g = colors[i].g;
      uint8_t b = colors[i].b;
    }
    SDL_UpdateRect(s, 0, 0, 0, 0);
  }
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len) {
  int i;
  uint8_t (*pdst)[4] = dst;
  uint8_t (*psrc)[4] = src;
  union {
    uint8_t val8[4];
    uint32_t val32;
  } tmp;
  int first = len & ~0xf;
  for (i = 0; i < first; i += 16) {
#define macro(i) \
    tmp.val32 = *((uint32_t *)psrc[i]); \
    *((uint32_t *)pdst[i]) = tmp.val32; \
    pdst[i][0] = tmp.val8[2]; \
    pdst[i][2] = tmp.val8[0];

    macro(i + 0); macro(i + 1); macro(i + 2); macro(i + 3);
    macro(i + 4); macro(i + 5); macro(i + 6); macro(i + 7);
    macro(i + 8); macro(i + 9); macro(i +10); macro(i +11);
    macro(i +12); macro(i +13); macro(i +14); macro(i +15);
  }

  for (; i < len; i ++) {
    macro(i);
  }
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags) {
  assert(src->format->BitsPerPixel == 32);
  assert(src->w * src->format->BytesPerPixel == src->pitch);
  assert(src->format->BitsPerPixel == fmt->BitsPerPixel);

  SDL_Surface* ret = SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

  assert(fmt->Gmask == src->format->Gmask);
  assert(fmt->Amask == 0 || src->format->Amask == 0 || (fmt->Amask == src->format->Amask));
  ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

  return ret;
}

uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  assert(fmt->BytesPerPixel == 4);
  uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
  if (fmt->Amask) p |= (a << fmt->Ashift);
  return p;
}

int SDL_LockSurface(SDL_Surface *s) {
  assert(0);
  return 0;
}

void SDL_UnlockSurface(SDL_Surface *s) {
  assert(0);
}
