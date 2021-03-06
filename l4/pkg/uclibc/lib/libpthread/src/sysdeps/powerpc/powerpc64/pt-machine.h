/* Machine-dependent pthreads configuration and inline functions.
   powerpc version.
   Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* These routines are from Appendix G of the 'PowerPC 601 RISC Microprocessor
   User's Manual', by IBM and Motorola.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

extern long int testandset (int *spinlock);
extern int __compare_and_swap (long int *p, long int oldval, long int newval);
extern int __compare_and_swap32 (int *p, int oldval, int newval);

/* For multiprocessor systems, we want to ensure all memory accesses
   are completed before we reset a lock.  On other systems, we still
   need to make sure that the compiler has flushed everything to memory.  */
#define MEMORY_BARRIER() __asm__ __volatile__ ("lwsync" : : : "memory")
#define READ_MEMORY_BARRIER() __asm__ __volatile__ ("lwsync" : : : "memory")
#define WRITE_MEMORY_BARRIER() __asm__ __volatile__ ("eieio" : : : "memory")

/* We want the OS to assign stack addresses.  */
#define FLOATING_STACKS 1

/* Maximum size of the stack if the rlimit is unlimited.  */
#define ARCH_STACK_MAX_SIZE     16*1024*1024

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("r1");

/* Register r13 (tp) is reserved by the ABI as "thread pointer". */
struct _pthread_descr_struct;
register struct _pthread_descr_struct *__thread_self __asm__("r13");

#ifdef NOT_FOR_L4
/* Return the thread descriptor for the current thread.  */
#define THREAD_SELF  __thread_self

/* Initialize the thread-unique value.  */
#define INIT_THREAD_SELF(descr, nr)  (__thread_self = (descr))

/* Access to data in the thread descriptor is easy.  */
#define THREAD_GETMEM(descr, member) \
  ((void) (descr), THREAD_SELF->member)
#define THREAD_GETMEM_NC(descr, member) \
  ((void) (descr), THREAD_SELF->member)
#define THREAD_SETMEM(descr, member, value) \
  ((void) (descr), THREAD_SELF->member = (value))
#define THREAD_SETMEM_NC(descr, member, value) \
  ((void) (descr), THREAD_SELF->member = (value))
#endif /* L4 */

/* Compare-and-swap for semaphores. */
/* note that test-and-set(x) is the same as !compare-and-swap(x, 0, 1) */

#define HAS_COMPARE_AND_SWAP
#define HAS_COMPARE_AND_SWAP_WITH_RELEASE_SEMANTICS

PT_EI int
__compare_and_swap (long int *p, long int oldval, long int newval)
{
  long int ret;

  __asm__ __volatile__ (
	   "0:    ldarx %0,0,%1 ;"
	   "      xor. %0,%3,%0;"
	   "      bne 1f;"
	   "      stdcx. %2,0,%1;"
	   "      bne- 0b;"
	   "1:    "
	: "=&r"(ret)
	: "r"(p), "r"(newval), "r"(oldval)
	: "cr0", "memory");
  /* This version of __compare_and_swap is to be used when acquiring
     a lock, so we don't need to worry about whether other memory
     operations have completed, but we do need to be sure that any loads
     after this point really occur after we have acquired the lock.  */
  __asm__ __volatile__ ("isync" : : : "memory");
  return (int)(ret == 0);
}

PT_EI int
__compare_and_swap_with_release_semantics (long int *p,
					   long int oldval, long int newval)
{
  long int ret;

  MEMORY_BARRIER ();
  __asm__ __volatile__ (
	   "0:    ldarx %0,0,%1 ;"
	   "      xor. %0,%3,%0;"
	   "      bne 1f;"
	   "      stdcx. %2,0,%1;"
	   "      bne- 0b;"
	   "1:    "
	: "=&r"(ret)
	: "r"(p), "r"(newval), "r"(oldval)
	: "cr0", "memory");
  return (int)(ret == 0);
}

PT_EI int
__compare_and_swap32 (int *p, int oldval, int newval)
{
  int ret;

  __asm__ __volatile__ (
	   "0:    lwarx %0,0,%1 ;"
	   "      xor. %0,%3,%0;"
	   "      bne 1f;"
	   "      stwcx. %2,0,%1;"
	   "      bne- 0b;"
	   "1:    "
	: "=&r"(ret)
	: "r"(p), "r"(newval), "r"(oldval)
	: "cr0", "memory");
  /* This version of __compare_and_swap is to be used when acquiring
     a lock, so we don't need to worry about whether other memory
     operations have completed, but we do need to be sure that any loads
     after this point really occur after we have acquired the lock.  */
  __asm__ __volatile__ ("isync" : : : "memory");
  return (int)(ret == 0);
}

PT_EI int
__compare_and_swap32_with_release_semantics (long int *p,
					   long int oldval, long int newval)
{
  long int ret;

  MEMORY_BARRIER ();
  __asm__ __volatile__ (
	   "0:    lwarx %0,0,%1 ;"
	   "      xor. %0,%3,%0;"
	   "      bne 1f;"
	   "      stwcx. %2,0,%1;"
	   "      bne- 0b;"
	   "1:    "
	: "=&r"(ret)
	: "r"(p), "r"(newval), "r"(oldval)
	: "cr0", "memory");
  return (int)(ret == 0);
}

PT_EI long int
testandset (int *p)
{
  long int ret, val = 1;

  MEMORY_BARRIER ();
  __asm__ __volatile__ (
	   "0:    lwarx %0,0,%1 ;"
	   "      cmpwi  0,%0,0;"
	   "      bne 1f;"
	   "      stwcx. %2,0,%1;"
	   "      bne- 0b;"
	   "1:    "
	: "=&r"(ret)
	: "r"(p), "r" (val)
	: "cr0", "memory");
  MEMORY_BARRIER ();
  return ret != 0;
}

#endif /* pt-machine.h */
