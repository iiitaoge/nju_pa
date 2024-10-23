#include <fs.h>
#include <common.h>
#include <ramdisk.h>  // 引入磁盘操作函数
#include <device.h> // 引入设备函数

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

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENT, FD_DISPINFO, FD_FB};

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
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, serial_write},  // 默认函数修改成了串口输出
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, serial_write},  // 同上
  [FD_EVENT] = {},
  [FD_DISPINFO] = {},
  [FD_FB] = {},
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
      // file_table[i].read = NULL;
      // file_table[i].write = NULL;
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
    size_t read_size;

    if (file_table[fd].read == NULL)
    {
        // 计算实际可读取的字节数，防止越界
        if (file_table[fd].open_offset + len < file_table[fd].size)
        {
            read_size = len;
        }
        else
        {
            read_size = file_table[fd].size - file_table[fd].open_offset;
        }

        // 如果已经到达文件末尾
        if (read_size == 0)
        {
            return 0; // EOF
        }

        // 从ramdisk中读取数据
        read_size = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, read_size);
        // printf("%d\n", file_table[fd].disk_offset + file_table[fd].open_offset);
        file_table[fd].open_offset += read_size;
        return read_size;
    }
    else
    {
        // 使用自定义的读函数
        read_size = len;
        if (file_table[fd].size && file_table[fd].open_offset + len > file_table[fd].size)
        {
            read_size = file_table[fd].size - file_table[fd].open_offset;
        }
        read_size = file_table[fd].read(buf, file_table[fd].open_offset, read_size);
        file_table[fd].open_offset += read_size;
        return read_size;
    }
}


size_t fs_write(int fd, const void *buf, size_t len) {

    size_t write_size = 0;
    // printf("调用了write函数\n");
    if (file_table[fd].write == NULL) // 调用ramdisk.write
    {
      // 计算实际可读取的字节数，防止越界
      if (file_table[fd].open_offset + len < file_table[fd].size)
      {
          write_size = len;
      }
      else
      {
          write_size = file_table[fd].size - file_table[fd].open_offset;
      }

      // 如果已经到达文件末尾
      if (write_size == 0)
      {
          return 0; // EOF
      }
      write_size = ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, write_size);
      file_table[fd].open_offset += write_size;
      return write_size;
    }
    else  // stdout 和 stderr 调用
    {
      // 使用自定义的写函数
      write_size = len;
      if (file_table[fd].size && file_table[fd].open_offset + len > file_table[fd].size)
      {
          write_size = file_table[fd].size - file_table[fd].open_offset;
      }
      write_size = file_table[fd].write((void *)buf, file_table[fd].open_offset, write_size);
      file_table[fd].open_offset += write_size;
      return write_size;
    }
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

  // 键盘初始化
  Finfo event_file = 
  {
    .name = "/dev/events",    // 文件名称
    .size = 0,               // 文件大小，单位为字节
    .disk_offset = 0,         // 偏移量
    .open_offset = 0,         // 初始打开偏移量
    .read = events_read,      // 自定义读函数
    .write = invalid_write    // 自定义写函数
  };
  file_table[FD_EVENT] = event_file;

  // /proc/dispinfo 初始化
  Finfo dispinfo_file = 
  {
    .name = "/proc/dispinfo",
    .size = 0,
    .disk_offset = 0,
    .open_offset = 0,
    .read = dispinfo_read,
    .write = invalid_write,
  };
  file_table[FD_DISPINFO] = dispinfo_file;


  // 显存文件初始化
  Finfo fb_file = 
  {
    .name = "/dev/fb",
    .size = 0,
    .disk_offset = 0,
    .open_offset = 0,
    .read = invalid_read,
    .write = fb_write,
  };
  file_table[FD_FB] = fb_file;
  printf("fb size : %d\n", file_table[FD_FB].size);
}
