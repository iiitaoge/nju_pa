#include <fs.h>
#include <common.h>
#include <ramdisk.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, invalid_write},
#include "files.h"
};

#define NR_FILES sizeof(file_table) / sizeof(file_table[0]) 

static inline void fd_check(int fd)
{
  // printf("fd = %d\n", fd);
  assert(fd >= 3 && fd < NR_FILES);
  return;
}

int fs_open(const char *pathname, int flags, int mode)
{
  for (int i = 0; i < NR_FILES; i++)
  {
    if (strcmp(pathname, file_table[i].name) == 0)
    {
      file_table[i].read = NULL;
      file_table[i].write = NULL;
      return i;
    }
  }
  printf("%s\n", pathname);
  panic("Could't find the file");
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len)
{
    fd_check(fd);
    size_t sz;

    if (file_table[fd].read == NULL)
    {
        // 计算实际可读取的字节数，防止越界
        if (file_table[fd].open_offset + len < file_table[fd].size)
        {
            sz = len;
        }
        else
        {
            sz = file_table[fd].size - file_table[fd].open_offset;
        }

        // 如果已经到达文件末尾
        if (sz == 0)
        {
            return 0; // EOF
        }

        // 从ramdisk中读取数据
        sz = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, sz);
        // printf("%d\n", file_table[fd].disk_offset + file_table[fd].open_offset);
        file_table[fd].open_offset += sz;
        return sz;
    }
    else
    {
        // 使用自定义的读函数
        sz = len;
        if (file_table[fd].size && file_table[fd].open_offset + len > file_table[fd].size)
        {
            sz = file_table[fd].size - file_table[fd].open_offset;
        }
        sz = file_table[fd].read(buf, file_table[fd].open_offset, sz);
        file_table[fd].open_offset += sz;
        return sz;
    }
}


size_t fs_write(int fd, const void *buf, size_t len) {
    if (fd == 0) {
        Log("不允许往 stdin 输出 %s", file_table[fd].name);
        return 0;
    }
 
    if (fd == 1 || fd == 2) // 标准输出流
    {  
        for (size_t i = 0; i < len; ++i)
            putch(*((char *)buf + i));
        return len;
    }
    size_t write_len = len;
    size_t open_offset = file_table[fd].open_offset;
    size_t size = file_table[fd].size;
    size_t disk_offset = file_table[fd].disk_offset;
    if (open_offset > size) return 0;
    if (open_offset + len > size) write_len = size - open_offset;
    ramdisk_write(buf, disk_offset + open_offset, write_len);
    file_table[fd].open_offset += write_len;
    return write_len;
}

size_t fs_lseek(int fd, size_t offset, int whence)
{
  fd_check(fd);
  switch (whence)
 {
  case SEEK_SET:  // 将 open_offset 设置为 offset
    file_table[fd].open_offset = offset;
    break;
  case SEEK_CUR:  // 在原有的 open_offset基础上往后移 offset
    file_table[fd].open_offset += offset;
    break;
  case SEEK_END:  // 将 open_offset 移到文件末端
    file_table[fd].open_offset = file_table[fd].size + offset;
    break;
  default:
    panic("lseek whence error!");
    break;
 }
  return file_table[fd].open_offset;  // 返回现在的偏移量
}

int fs_close(int fd)
{
  return 0;
}
void init_fs() {
  // TODO: initialize the size of /dev/fb
}
