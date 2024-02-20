#ifndef _ELF_LOADER_H_
#define _ELF_LOADER_H_

#include <unistd.h>
#include <elf.h>

#if INTPTR_MAX == INT32_MAX
	typedef Elf32_Ehdr Elf_Ehdr;
	typedef Elf32_Phdr Elf_Phdr;
	typedef Elf32_Sym Elf_Sym;
	typedef Elf32_Shdr Elf_Shdr;
	typedef Elf32_Rel Elf_Rel;
	typedef Elf32_Word Elf_Word;
	#define ELF_R_SYM(x) ELF32_R_SYM(x)
	#define ELF_R_TYPE(x) ELF32_R_TYPE(x)
#elif INTPTR_MAX == INT64_MAX
	typedef Elf64_Ehdr Elf_Ehdr;
	typedef Elf64_Phdr Elf_Phdr;
	typedef Elf64_Sym Elf_Sym;
	typedef Elf64_Shdr Elf_Shdr;
	typedef Elf64_Rel Elf_Rel;
	typedef Elf64_Word Elf_Word;
	#define ELF_R_SYM(x) ELF64_R_SYM(x)
	#define ELF_R_TYPE(x) ELF64_R_TYPE(x)
#else
	#error("Type error")
#endif

#define STACK_SIZE (8*1024*1024)
#define STACK_STORAGE_SIZE 0x5000
#define STACK_STRING_SIZE 0x5000

#define ROUND_UP(v, s) ((v + s - 1) & -s)
#define ROUND_DOWN(v, s) (v & -s)

// Not all archs have this one defined
#ifndef AT_RANDOM
	#define AT_RANDOM 25
#endif

/*!
* \brief Map the ELF into memory.
*/
void elf_load(char *elf_start, void *stack, int stack_size, size_t *base_addr, size_t *entry);

/*!
* \brief Map the ELF into memory and run it with the provided arguments.
*/
void elf_run(void *buf, char **argv, char **env);

#endif // _ELF_LOADER_H_