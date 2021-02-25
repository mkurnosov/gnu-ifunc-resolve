/*
 * gnu_ifunc_resolve.c:
 * 
 * This example demonstrates an approach to obtain real symbol (function name) which
 * is called from the glibc for GNU IFUNCs: memcpy, strcmp, cos, etc.
 *
 * Example for GNU/Linux with glibc 2.28 and Intel Sandy Bridge processor:
 *
 * $ ./gnu_ifunc_resolve
 * GNU libc version: 2.28, release: stable
 * GNU IFUNC 'memcpy' is resolved to '__memmove_avx_unaligned_erms'
 * GNU IFUNC 'strcmp' is resolved to '__strcmp_avx2'
 * GNU IFUNC 'malloc' is resolved to '__libc_malloc'
 * GNU IFUNC 'strlen' is resolved to '__strlen_avx2'
 * GNU IFUNC 'strcmp' is resolved to '__strcmp_avx2'
 * GNU IFUNC 'strcpy' is resolved to '__strcpy_sse2_unaligned'
 * GNU IFUNC 'cos' is resolved to '__cos_fma'
 *
 * (C) 2021 Mikhail Kurnosov <mkurnosov@gmail.com>
 */
 
#define _GNU_SOURCE
#include <gnu/libc-version.h>
#include <dlfcn.h>
#include <elf.h>
#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/*
 * gnu_ifunc_resolve: returns real symbol name for the given GNU IFUNC name.
 *                    On error returns NULL.
 */
const char *gnu_ifunc_resolve(const char *ifunc_name)
{
    /* Obtain address of the ifunc */
    void *ifunc_addr = dlsym(RTLD_DEFAULT, ifunc_name);
    if (ifunc_addr == NULL) {
        fprintf(stderr, "Error: can not obtain address of a symbol '%s': %s\n",
                ifunc_name, dlerror());
        return NULL;
    }

    /* Get a shared object info by the address */
    Dl_info dli;
    int rc = dladdr(ifunc_addr, &dli);
    if (rc == 0) {
        fprintf(stderr, "Error: can not match the address %p to a shared object: %s\n",
                ifunc_addr, dlerror());
        return NULL;
    }

    if (dli.dli_sname != NULL) {
        return dli.dli_sname;
    }
    /* Symbol not found, try to find among local symbols of the shared object */

    /* Calculate address of the symbol relative to base address of the shared object */
    ptrdiff_t sym_rel_addr = (ptrdiff_t)ifunc_addr - (ptrdiff_t)dli.dli_fbase;

    if (elf_version(EV_CURRENT) == EV_NONE) {
        fprintf(stderr, "Error: ELF library too old\n");
        return NULL;
    }

    /* Open ELF file (shared object) and find section with symbol table */
    int fd;
    if ((fd = open(dli.dli_fname, O_RDONLY)) < 0) {
        fprintf(stderr, "Error: can not open ELF file '%s': %s\n", dli.dli_fname, strerror(errno));
        return NULL;
    }
    Elf *elf;
    if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
        close(fd);
        fprintf(stderr, "Error: can not open ELF file '%s'\n", dli.dli_fname);
        return NULL;
    }

    /* Iterate through sections and find symbol table */
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        if (gelf_getshdr(scn, &shdr)) {
            if (shdr.sh_type == SHT_SYMTAB) {
                /* Found a symbol table */
                break;
            }
        }
    }

    /* Find symbol by relative address */
    Elf_Data *sec_data = elf_getdata(scn, NULL);
    if (sec_data) {
        int nsyms = shdr.sh_size / shdr.sh_entsize;
        for (int i = 0; i < nsyms; i++) {
            GElf_Sym sym;
            gelf_getsym(sec_data, i, &sym);
            if ((ptrdiff_t)sym.st_value == sym_rel_addr) {
                return elf_strptr(elf, shdr.sh_link, sym.st_name);
            }
        }
    }

    elf_end(elf);
    close(fd);
    return NULL;
}

int main(int argc, char **argv)
{
    printf("GNU libc version: %s, release: %s\n", gnu_get_libc_version(), gnu_get_libc_release());

    const char *ifunc_syms[] = {"memcpy", "strcmp", "malloc", "strlen", "strcmp", "strcpy",
                                "cos", NULL};

    for (int i = 0; ifunc_syms[i]; i++) {
        const char *sym = gnu_ifunc_resolve(ifunc_syms[i]);
        if (sym) {
            printf("GNU IFUNC '%s' is resolved to '%s'\n", ifunc_syms[i], sym);
        } else {
            fprintf(stderr, "Error: can not resolve ifunc '%s'\n", ifunc_syms[i]);
        }
    }
    return 0;
}
