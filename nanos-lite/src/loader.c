#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

// 声明
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;


static uintptr_t loader(PCB *pcb, const char *filename)
{
  int fd = fs_open(filename, 0, 0);
  if (fd == -1)
  {
    panic("loader: can't open file %s!", filename);
  }
  Elf_Ehdr ehdr;
  fs_read(fd, (void *)&ehdr, sizeof(Elf_Ehdr));
  // printf("entry = %p\n", ehdr.e_entry);
  // 判断魔数是否正确
  assert(*(uint32_t*)ehdr.e_ident == *(uint32_t*)ELFMAG);
  // 判断是否为 riscv32 的elf
  assert(ehdr.e_machine == EM_RISCV);
  // 创建一个存放程序头表的数组
  Elf_Phdr phdr[ehdr.e_phnum];
  fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
  fs_read(fd, phdr, sizeof(Elf_Phdr) * ehdr.e_phnum);
  for (size_t i = 0; i < ehdr.e_phnum; i++)
  {
    if (phdr[i].p_type == PT_LOAD)
    {
      fs_lseek(fd, phdr[i].p_offset, SEEK_SET);
      fs_read(fd, (void *)phdr[i].p_vaddr, phdr[i].p_filesz);
      memset((void *)(phdr[i].p_vaddr + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
    }
  }
  fs_close(fd);
  return ehdr.e_entry;
}

// // 加载 elf
// static uintptr_t loader(PCB *pcb, const char *filename) {
//   Elf_Ehdr ehdr;
//   // 把 build/ramdisk的文件加载作为elf进来

//   // int fd = fs_open(filename, 0, 0);
//   // fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
//   ramdisk_read(&ehdr, 37648, sizeof(Elf_Ehdr));
//   printf("entry = %p\n", ehdr.e_entry);
//   // 检查魔数，防止加载非elf文件
//   assert(*(uint32_t*)ehdr.e_ident == *(uint32_t*)ELFMAG);
//   // 判断是否为 riscv32 的elf
//   assert(ehdr.e_machine == EM_RISCV);
//   // 创建一个存放程序头表的数组
//   Elf_Phdr phdr[ehdr.e_phnum];
//   // 读入程序头表
//   ramdisk_read(phdr, ehdr.e_phoff + 37648, sizeof(Elf_Phdr) * ehdr.e_phnum);

//   for (int i = 0; i < ehdr.e_phnum; i++) 
//   {
//     if (phdr[i].p_type == PT_LOAD) 
//     {
//       ramdisk_read((void*)phdr[i].p_vaddr, 37648 + phdr[i].p_offset, phdr[i].p_memsz);
//       // 把 .bss 段设置为0
//       memset((void*)(37648 + phdr[i].p_vaddr+phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
//     }
//   }
//   return ehdr.e_entry;  // 返回程序入口
  
// }

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

