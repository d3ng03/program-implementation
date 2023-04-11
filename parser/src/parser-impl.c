#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

typedef char byte;
typedef char bool;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef signed long long sint64;

void print_hex(byte* mem, uint32 n) {
	for (int i = 0;i < n;++i) {
		printf("%02x ", mem[i]);

		if (i > 0 && i % 16 == 15) puts("");
	}
	puts("");
}

byte* read_elf(char* filename, uint32* length) {
	int fd;
	byte* elf_file;

	fd = open(filename,O_RDONLY);

	length = lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);

	elf_file = (byte*)malloc(length);
	memset(elf_file,0,length);
	read(fd,elf_file,length);

	close(fd);

	return elf_file;
}

Elf64_Ehdr* read_elf_header(byte* elf_file) {
	Elf64_Ehdr* elf_header = (Elf64_Ehdr*)malloc(sizeof(Elf64_Ehdr));

	memset(elf_header,0,sizeof(Elf64_Ehdr));
	memcpy(elf_header,elf_file,sizeof(Elf64_Ehdr));

	return elf_header;
}

void print_elf_header(Elf64_Ehdr* elf_header) {
	printf("* ELF Header\n");
	printf("Magic:\t");
	for (int i = 0;i < EI_NIDENT;++i) {
		printf("%02x ", elf_header->e_ident[i]);
	}
	puts("");

	printf("Type:\t%d\n", elf_header->e_type);
	printf("Machine:\t%d\n", elf_header->e_machine);
	printf("Version:\t%d\n", elf_header->e_version);
	printf("Entry:\t%lx\n", elf_header->e_entry);
	printf("Program header offset:\t%lx\n", elf_header->e_phoff);
	printf("Section header offset:\t%lx\n", elf_header->e_shoff);
	printf("Flags:\t%d\n", elf_header->e_flags);
	printf("ELF header size:\t%d\n", elf_header->e_ehsize);
	printf("Program header entry size:\t%d\n", elf_header->e_phentsize);
	printf("Number of program headers:\t%d\n", elf_header->e_phnum);
	printf("Section header entry size:\t%d\n", elf_header->e_shentsize);
	printf("Number of section headers:\t%d\n", elf_header->e_shnum);
	printf("Section header string table index:\t%d\n", elf_header->e_shstrndx);
	puts("");
}


Elf64_Shdr** read_section_headers(byte* elf_file, Elf64_Ehdr* elf_header) {
	uint64 offset = elf_header->e_shoff;
	uint32 n_headers = elf_header->e_shnum;
	Elf64_Shdr** section_headers;

	section_headers = (Elf64_Shdr **)malloc(n_headers * sizeof(Elf64_Shdr *));
	for(int i = 0; i < n_headers; i++){
		section_headers[i] = (Elf64_Shdr *)malloc(sizeof(Elf64_Shdr));
		memset(section_headers[i],0,sizeof(Elf64_Shdr));

	}

	for(int i = 0; i < n_headers; i++){
		memcpy((byte *)section_headers[i],elf_file + offset + i*sizeof(Elf64_Shdr),sizeof(Elf64_Shdr));
	}

	return section_headers;	
}

byte* get_section_header_string_table (byte* elf_file, Elf64_Shdr** section_headers, Elf64_Ehdr* elf_header) {
	uint64 size = section_headers[elf_header->e_shstrndx]->sh_size;
	uint64 offset = section_headers[elf_header->e_shstrndx]->sh_offset;
	byte* section_header_string_table = (byte*)malloc(size);
	
	uint32 n_headers = elf_header->e_shnum;
	
	memset(section_header_string_table,0,size);
	memcpy(section_header_string_table,elf_file + offset,size);
	
	return section_header_string_table;
}

void print_section_headers(Elf64_Shdr** section_headers, byte* section_header_string_header, Elf64_Ehdr* elf_header) {
	uint32 n_headers = elf_header->e_shnum;

	printf("* Section headers\n");
	printf("Idx Name\tSize\t\tAddr\t\t\tOffset\t\tType\n");

	for (int i = 0;i < n_headers;++i) {
		if (i == 0) continue;

		uint32 name = section_headers[i]->sh_name;
		uint64 size = section_headers[i]->sh_size;
		uint32 addr = section_headers[i]->sh_addr;
		uint64 offset = section_headers[i]->sh_offset;
		uint32 type = section_headers[i]->sh_type;
		uint32 link = section_headers[i]->sh_link;

		printf("%d %s\t%08llx\t%016x\t%08llx\t", i, section_header_string_header + name, size, addr, offset);

		switch (type) {
		case 1: printf("SHT_PROGBITS\n"); break;
		case 2: 
			printf("SHT_SYMTAB(%x)\n", link); 
			break;
		case 3: printf("SHT_STRTAB\n"); break;
		case 4: printf("SHT_RELA\n"); break;
		case 5: printf("SHT_HASH\n"); break;
		case 6: printf("SHT_DYNAMIC\n"); break;
		case 7: printf("SHT_NOTE\n"); break;
		case 8: printf("SHT_NOBITS\n"); break;
		case 9: printf("SHT_REL\n"); break;
		case 10: printf("SHT_SHLIB\n"); break;
		case 11: printf("SHT_DYNSYM\n"); break;
		default:
			printf("NOT_SPECIFIED\n");
		}
	}

	puts("");
}

Elf64_Sym** get_symbol_table(byte* elf_file, Elf64_Shdr** section_headers, Elf64_Ehdr* elf_header) {
	uint32 idx_symtab_header;
	Elf64_Sym** symbol_table;

	uint64 offset;
	uint64 size;
	uint32 n_symbols;

	for (int i = 0;i < elf_header->e_shnum;++i) {
		if (section_headers[i]->sh_type == SHT_SYMTAB) {
			idx_symtab_header = i;
			break;
		}
	}
	
	offset = section_headers[idx_symtab_header]->sh_offset;
	size = section_headers[idx_symtab_header]->sh_size;
	n_symbols = size / sizeof(Elf64_Sym);
	symbol_table = (Elf64_Sym **)malloc(sizeof(Elf64_Sym *) * n_symbols);

	for(int i = 0;i < n_symbols; i++){
		symbol_table[i] = (Elf64_Sym *)malloc(sizeof(Elf64_Sym));
		memset(symbol_table[i],0,sizeof(Elf64_Sym));
		memcpy((byte *)symbol_table[i], elf_file + offset + i * sizeof(Elf64_Sym), size);
	}

	return symbol_table;
}

byte* get_symbol_string_table (byte* elf_file, Elf64_Shdr** section_headers, Elf64_Ehdr* elf_header) {
	byte* symbol_string_table;
	uint32 link;

	uint32 idx_symtab_header;
	uint64 offset;
	uint64 size;

	for (int i = 0;i < elf_header->e_shnum;++i) {
		if (section_headers[i]->sh_type == SHT_SYMTAB) {
			idx_symtab_header = i;
			break;
		}
	}

	link = section_headers[idx_symtab_header]->sh_link;
	offset = section_headers[link]->sh_offset;
	size = section_headers[link]->sh_size;
	symbol_string_table = (byte *)malloc(size);

	memset(symbol_string_table, 0, size);
	memcpy(symbol_string_table, elf_file + offset, size);

	return symbol_string_table;
}

void print_symbol_table(Elf64_Shdr** section_headers, byte* section_header_string_table, Elf64_Sym** symbol_table, byte* symbol_string_table, Elf64_Ehdr* elf_header) {
	uint32 idx_symtab_header;

	for (int i = 0;i < elf_header->e_shnum;++i) {
		if (section_headers[i]->sh_type == SHT_SYMTAB) {
			idx_symtab_header = i;
			break;
		}
	}

	uint64 offset = section_headers[idx_symtab_header]->sh_offset;
	uint64 size = section_headers[idx_symtab_header]->sh_size;
	uint32 n_symbols = size / sizeof(Elf64_Sym);

	printf("* Symbol table\n");

	for (int i = 0;i < n_symbols;++i) {
		uint32 name = symbol_table[i]->st_name;
		uint64 value = symbol_table[i]->st_value;
		uint16 shndx = symbol_table[i]->st_shndx;
		Elf64_Shdr* section_header = shndx <= elf_header->e_shnum ? section_headers[shndx]: NULL;
		uint32 sh_name = section_header != NULL ? section_header->sh_name: 0;
		byte* section_name = section_header != NULL ? (section_header_string_table + sh_name) : "*ABS*";

		if (!name) continue;

		printf("%d\t%s\t\t%llx\t%llx\t\t%s\n", i, section_name, value, size, symbol_string_table + name);
	}

	puts("");
}

Elf64_Dyn** get_dynamic_section(byte* elf_file, Elf64_Shdr** section_headers, Elf64_Ehdr* elf_header) { 
	Elf64_Dyn** dynamic_section;
	uint32 idx_dynamic;
	
	uint64 offset;
	uint64 size;
	uint32 n_dyn;

	for (int i = 0;i < elf_header->e_shnum;++i) {
		if (section_headers[i]->sh_type == SHT_DYNAMIC) {
			idx_dynamic = i;
			break;
		}
	}
	offset = section_headers[idx_dynamic]->sh_offset;
	size = section_headers[idx_dynamic]->sh_size;
	n_dyn = size / sizeof(Elf64_Dyn);
	dynamic_section = (Elf64_Dyn **)malloc(sizeof(Elf64_Dyn *) * n_dyn);

	for(int i = 0; i < n_dyn; i++){
		dynamic_section[i] = (Elf64_Dyn *)malloc(sizeof(Elf64_Dyn));
		memset(dynamic_section[i],0,sizeof(Elf64_Dyn));
		memcpy((byte *)dynamic_section[i], elf_file + offset + i * sizeof(Elf64_Dyn), sizeof(Elf64_Dyn));
	}

	return dynamic_section;
}

byte* get_dynamic_string_table(byte* elf_file, Elf64_Shdr** section_headers, Elf64_Ehdr* elf_header) {
	byte* dynamic_string_table;
	uint32 link;

	uint32 idx_dynamic;
	uint64 offset;
	uint64 size;

	for(int i = 0; i < elf_header->e_shnum; ++i){
		if (section_headers[i]->sh_type == SHT_DYNAMIC) {
			idx_dynamic = i;
			break;
		}
	}

	link = section_headers[idx_dynamic]->sh_link;
	offset = section_headers[link]->sh_offset;
	size = section_headers[link]->sh_size;
	dynamic_string_table = (byte *)malloc(size);

	memset(dynamic_string_table,0,size);
	memcpy(dynamic_string_table, elf_file + offset, size);

	return dynamic_string_table;
}

void print_dynamic_section(Elf64_Shdr** section_headers, Elf64_Dyn** dynamic_section, byte* dynamic_string, Elf64_Ehdr* elf_header) {
	uint32 idx_dynamic;

	for (int i = 0;i < elf_header->e_shnum;++i) {
		if (section_headers[i]->sh_type == SHT_DYNAMIC) {
			idx_dynamic = i;
			break;
		}
	}

	uint64 offset = section_headers[idx_dynamic]->sh_offset;
	uint64 size = section_headers[idx_dynamic]->sh_size;
	uint64 n_dyn = size / sizeof(Elf64_Dyn);

	printf("* Dynamic section\n");

	for (int i = 0;i < n_dyn;++i) {
		sint64 tag = dynamic_section[i]->d_tag;
		uint64 val_ptr;
		printf("%d\t", i);

		switch(tag) {
		case DT_NEEDED: 
			printf("NEEDED\t"); 
			val_ptr = dynamic_section[i]->d_un.d_val;
			printf("%s", &dynamic_string[val_ptr]);
			break;
		case DT_INIT: printf("INIT"); break;
		case DT_FINI: printf("FINI"); break;
		case DT_INIT_ARRAY: printf("INIT_ARRY"); break;
		case DT_INIT_ARRAYSZ: printf("INIT_ARRAYSZ"); break;
		case DT_FINI_ARRAY: printf("FINI_ARRAY"); break;
		case DT_FINI_ARRAYSZ: printf("FINI_ARRAYSZ"); break;
		case DT_GNU_HASH: printf("GNU_HASH"); break;
		case DT_STRTAB: printf("STRTAB"); break;
		case DT_SYMTAB: printf("SYMTAB"); break;
		case DT_STRSZ: printf("STRSZ"); break;
		case DT_SYMENT: printf("SYMENT"); break;
		case DT_DEBUG: printf("DEBUG"); break;
		case DT_PLTGOT: printf("PLTGOT"); break;
		case DT_PLTRELSZ: printf("PLTRELSZ"); break;
		case DT_PLTREL: printf("PLTREL"); break;
		case DT_JMPREL: printf("JMPREL"); break;
		case DT_RELA: printf("RELA"); break;
		case DT_RELASZ: printf("RELASZ"); break;
		case DT_RELAENT: printf("RELAENT"); break;
		case DT_FLAGS: printf("FLAGS"); break;
		case DT_FLAGS_1: printf("FLAGS_1"); break;
		case DT_VERNEED: printf("VERNEED"); break;
		case DT_VERNEEDNUM: printf("VERNEEDNUM"); break;
		case DT_VERSYM: printf("VERSYM"); break;
		case DT_RELACOUNT: printf("RELACOUNT"); break;
		default: printf("UNKNOWN"); break;
		}

		puts("");
	}

	puts("");
}

Elf64_Phdr** read_program_headers(byte* elf_file, Elf64_Ehdr* elf_header) {
	uint64 offset = elf_header->e_phoff;
	uint16 n_headers = elf_header->e_phnum;
	Elf64_Phdr** program_headers;
	
	program_headers = (Elf64_Phdr **)malloc(sizeof(Elf64_Phdr *) * n_headers);
	for (int i = 0; i < n_headers; i++){
		program_headers[i] = (Elf64_Phdr *)malloc(sizeof(Elf64_Phdr));
		memset(program_headers[i],0,sizeof(Elf64_Phdr));
		memcpy((byte *)program_headers[i],elf_file + offset + i * sizeof(Elf64_Phdr), sizeof(Elf64_Phdr));
	}

	return program_headers;
}

void print_program_headers(Elf64_Phdr** program_headers, Elf64_Ehdr* elf_header) {
	uint16 n_headers = elf_header->e_phnum;

	printf("* Program headers\n");
	for (int i = 0;i < n_headers;++i) {
		bool skip = 0;
		uint32 type = program_headers[i]->p_type;	
		uint32 flags = program_headers[i]->p_flags;
		uint64 offset = program_headers[i]->p_offset;
		uint64 vaddr = program_headers[i]->p_vaddr;
		uint64 paddr = program_headers[i]->p_paddr;
		uint64 filesz = program_headers[i]->p_filesz;
		uint64 memsz = program_headers[i]->p_memsz;
		uint64 align = program_headers[i]->p_align;


		if (type == PT_NULL) continue;

		switch (type) {
		case PT_LOAD: printf("\tLOAD\t"); break;
		case PT_DYNAMIC: printf("\tDYNAMIC\t"); break;
		case PT_INTERP: printf("\tINTERP\t"); break;
		case PT_NOTE: printf("\tNOTE\t"); break;
		case PT_SHLIB: printf("\tSHLIB\t"); break;
		case PT_PHDR: printf("\tPHDR\t"); break;
		default: skip = 1; break;
		}

		if (skip) continue;

		printf("off\t0x%016llx\tvaddr\t0x%016llx\tpaddr\t0x%016llx\talign\t0x%llx\n", offset, vaddr, paddr, align);
		printf("\t\tfilesz\t0x%015llx\tmemsz\t0x%016llx\tflags\t", filesz, memsz);
		switch (flags) {
		case 0: printf("---"); break;
		case 1: printf("--x"); break;
		case 2: printf("-w-"); break;
		case 3: printf("-wx"); break;
		case 4: printf("r--"); break;
		case 5: printf("r-x"); break;
		case 6: printf("rw-"); break;
		case 7: printf("rwx"); break;
		default: printf("UNKNOWN");break;
		}
		puts("");
	}
	puts("");
}

void free_all(byte* elf_file, Elf64_Ehdr* elf_header, Elf64_Shdr** section_headers, byte* section_header_string_table, Elf64_Sym** symbol_table) {
	free(elf_file);
	free(elf_header);
}

int main(int argc, char* argv[]) {
	byte* elf_file = NULL;
	Elf64_Ehdr* elf_header = NULL;
	Elf64_Shdr** section_headers = NULL;
	Elf64_Sym** symbol_table = NULL;
	Elf64_Dyn** dynamic_section = NULL;
	Elf64_Phdr** program_headers = NULL;
	byte* section_header_string_table = NULL;
	byte* symbol_string_table = NULL;
	byte* dynamic_string = NULL;
	uint32 length = 0;

	if (argc < 2) {
		printf("Usage: parser [target elf]\n");
		return 1;
	}

	elf_file = read_elf(argv[1], &length);
	elf_header = read_elf_header(elf_file);
	print_elf_header(elf_header);
	
	section_headers = read_section_headers(elf_file, elf_header);
	section_header_string_table = get_section_header_string_table(elf_file, section_headers, elf_header);
	print_section_headers(section_headers, section_header_string_table, elf_header);

	symbol_table = get_symbol_table(elf_file, section_headers, elf_header);
	symbol_string_table = get_symbol_string_table(elf_file, section_headers, elf_header);
	print_symbol_table(section_headers, section_header_string_table, symbol_table, symbol_string_table, elf_header);

	dynamic_section = get_dynamic_section(elf_file, section_headers, elf_header);
	dynamic_string = get_dynamic_string_table(elf_file, section_headers, elf_header);
	print_dynamic_section(section_headers, dynamic_section, dynamic_string, elf_header);

	program_headers = read_program_headers(elf_file, elf_header);
	print_program_headers(program_headers, elf_header);
	

	free_all(elf_file, elf_header, section_headers, section_header_string_table, symbol_table);

	return 0;
}
