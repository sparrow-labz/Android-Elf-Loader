#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>

#include "elf.h"
#include "code/elfload.h"
#include "code/fakestack.h"
#include "icrt/icrt_utils.h"
#include "icrt/icrt_asm.h"
#include "arch.h"

// Default function called upon exit() in the ELF. 
static void _exit_func(int code)
{
   exit(code);
}

static void _get_rand(char *buf, int size)
{
   int fd = open("/dev/urandom", O_RDONLY, 0);

   read(fd, (unsigned char *) buf, size);
   close(fd);
}

static Elf_Shdr *_get_section(char *name, void *elf_start)
{
   int x; 
   Elf_Ehdr *ehdr = NULL;
   Elf_Shdr *shdr;

   ehdr = (Elf_Ehdr *) elf_start;
   shdr = (Elf_Shdr *)(elf_start + ehdr->e_shoff);

   Elf_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
   char *sh_strtab_p = elf_start + sh_strtab->sh_offset;
    
   for(x = 0; x < ehdr->e_shnum; x++)
   {
      if(!strcmp(name, sh_strtab_p + shdr[x].sh_name))
         return &shdr[x];
   }
   return NULL;
}

void elf_load(char *elf_start, void *stack, int stack_size, size_t *base_addr, size_t *entry)
{
   Elf_Ehdr *hdr;
   Elf_Phdr *phdr;

   int x;
   int elf_prot = 0;
   int stack_prot = 0;
   size_t base;

   hdr = (Elf_Ehdr *)elf_start;
   phdr = (Elf_Phdr * )(elf_start + hdr->e_phoff);   

   if(hdr->e_type == ET_DYN)
   {
      // If this is a DYNAMIC ELF (can be loaded anywhere), set a random base address
      base = (size_t)mmap(0, 100 * PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
      munmap((void *)base, 100 * PAGE_SIZE);
   }
   else
      base = 0;

   if(base_addr != NULL)
      *base_addr = -1;

   if(entry != NULL)
      *entry = base + hdr->e_entry;

   for(x = 0; x < hdr->e_phnum; x++)
   {
      // Get flags for the stack
      if(stack != NULL && phdr[x].p_type == PT_GNU_STACK)
      {
         if(phdr[x].p_flags & PF_R)
            stack_prot =  PROT_READ;

         if(phdr[x].p_flags & PF_W)
            stack_prot |= PROT_WRITE;

         if(phdr[x].p_flags & PF_X)
            stack_prot |= PROT_EXEC;

         // Set stack protection
         mprotect((unsigned char *) stack, stack_size, stack_prot);
      }

      if(phdr[x].p_type != PT_LOAD)
         continue;

      if(!phdr[x].p_filesz)
         continue;

      void *map_start = (void *) ROUND_DOWN(phdr[x].p_vaddr, PAGE_SIZE);
      int round_down_size = (void *) phdr[x].p_vaddr - map_start;

      int map_size = ROUND_UP(phdr[x].p_memsz + round_down_size, PAGE_SIZE);

      mmap(base + map_start, map_size, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);
      memcpy((void *) base + phdr[x].p_vaddr, elf_start + phdr[x].p_offset, phdr[x].p_filesz);

      // Zero-out BSS, if it exists
      if(phdr[x].p_memsz > phdr[x].p_filesz)
         memset((void *)(base + phdr[x].p_vaddr + phdr[x].p_filesz), 0, phdr[x].p_memsz - phdr[x].p_filesz);

      // Set proper protection on the area
      if(phdr[x].p_flags & PF_R)
         elf_prot =  PROT_READ;
   
      if(phdr[x].p_flags & PF_W)
         elf_prot |= PROT_WRITE;
      
      if(phdr[x].p_flags & PF_X)
         elf_prot |= PROT_EXEC;

      mprotect((unsigned char *) (base + map_start), map_size, elf_prot);

      // Is this the lowest memory area we saw. That is, is this the ELF base address?
      if(base_addr != NULL && (*base_addr == -1 || *base_addr > (size_t)(base + map_start)))
         *base_addr = (size_t)(base + map_start);
   }
}

void elf_run(void *buf, char **argv, char **env)
{
   int pid;
   int argc = 0;
   int envc = 0;
   unsigned long base_addr;
   unsigned long eop;

   char pids[24];
   char path[256];

   Elf_Ehdr *hdr = (Elf_Ehdr *)buf;
   size_t auxv_len;
   size_t elf_base;
   size_t elf_entry;
   uint8_t fakestack[4096 * 16];
   uint8_t * auxv_buf;
   uint8_t * stackptr = fakestack + sizeof(fakestack);

   // Fill in 16 random bytes for the loader below
   char rand_bytes[16];
   _get_rand(rand_bytes, 16);

   int (*ptr)(int, char **, char**);

   // First, let's count arguments...
   while(argv[argc])
      argc++;

   // ...and envs
   while(env[envc])
      envc++;

   // Allocate some stack space
   void *stack = mmap(0, STACK_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANON, -1, 0);
   
   // Map the ELF in memory
   elf_load(buf, stack, STACK_SIZE, &elf_base, &elf_entry);
   
   // set loaded program name to int pid
   pid = (uintptr_t) argv[1];
   // get pid of loaded program
   pid = getpid();
   printf("> loaded program pid: %d\n", pid);
   // convert int pid to string
   citoa(pid, pids, 10);
   
   // read auxv
   memset(path, 0, sizeof(path));
   // get /proc/pid/auxv path as string
   strlcat(path, "/proc/", sizeof(path));
   strlcat(path, pids, sizeof(path));
   strlcat(path, "/auxv",  sizeof(path));
   printf("> path %s\n,", path);

   if(read_file(path, &auxv_buf, &auxv_len) < 0)
   return;
   printf("> auxv len: %zu\n", auxv_len);
   printf("> mapping '%s' into memory at 0x%lx\n", argv[1], elf_base);

   // map elf and load interp
   if (map_elf(argv[1], elf_base, (unsigned long *) auxv_buf, &eop) < 0)
      printf("> failed to load elf interp\n");
   printf("> returned from load elf interp\n");

   // build a stack for loader entry
   memset(fakestack, 0, sizeof(fakestack));
   stackptr = fake_stack(stackptr, argc, argv, env, (unsigned long *)auxv_buf);

   // all done
   printf("> starting ...\n\n");   
   jump_start(stackptr, (void *)_exit_func, (void *)eop);

   // Only reacahed if loading error
   exit(0);
}