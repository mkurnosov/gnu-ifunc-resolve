# GNU IFUNC resolver

This example demonstrates a simple approach to obtain symbol (function name) which is called from the glibc for [GNU IFUNCs](https://sourceware.org/glibc/wiki/GNU_IFUNC): `memcpy`, `strcmp`, `cos`, etc.

> The GNU indirect function support (IFUNC) is a feature of the GNU toolchain that allows a developer to create multiple implementations of a given function and to select amongst them at runtime using a resolver function which is also written by the developer. The resolver function is called by the dynamic loader during early startup to resolve which of the implementations will be used by the application. Once an implementation choice is made it is fixed and may not be changed for the lifetime of the process.

Example for GNU/Linux with glibc 2.28 and Intel Sandy Bridge processor:

 ```
  $ ./gnu_ifunc_resolve
  GNU libc version: 2.28, release: stable
  GNU IFUNC 'memcpy' is resolved to '__memmove_avx_unaligned_erms'
  GNU IFUNC 'strcmp' is resolved to '__strcmp_avx2'
  GNU IFUNC 'malloc' is resolved to '__libc_malloc'
  GNU IFUNC 'strlen' is resolved to '__strlen_avx2'
  GNU IFUNC 'strcmp' is resolved to '__strcmp_avx2'
  GNU IFUNC 'strcpy' is resolved to '__strcpy_sse2_unaligned'
  GNU IFUNC 'cos' is resolved to '__cos_fma'
 ```
  
