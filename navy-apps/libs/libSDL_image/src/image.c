#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}


// 加载IMG
SDL_Surface* IMG_Load(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Failed to open file");
        return NULL;
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);  // 重新定位到文件开始位置

    // !!! 动态分配内存缓冲区 不要用数组当内存
    char *buf = (char *)malloc(size);
    if (!buf) {
        fclose(fp);
        perror("Failed to allocate memory");
        return NULL;
    }

    // 读取文件到缓冲区
    fread(buf, 1, size, fp);

    // 调用 STBIMG_LoadFromMemory() 将缓冲区数据转为 SDL_Surface
    SDL_Surface *surface = STBIMG_LoadFromMemory(buf, size);

    // 关闭文件并释放缓冲区
    fclose(fp);
    free(buf);

    return surface;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
