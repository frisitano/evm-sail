/****************************************************************************/
/*     Sail                                                                 */
/*                                                                          */
/*  Sail and the Sail architecture models here, comprising all files and    */
/*  directories except the ASL-derived Sail code in the aarch64 directory,  */
/*  are subject to the BSD two-clause licence below.                        */
/*                                                                          */
/*  The ASL derived parts of the ARMv8.3 specification in                   */
/*  aarch64/no_vector and aarch64/full are copyright ARM Ltd.               */
/*                                                                          */
/*  Copyright (c) 2013-2021                                                 */
/*    Kathyrn Gray                                                          */
/*    Shaked Flur                                                           */
/*    Stephen Kell                                                          */
/*    Gabriel Kerneis                                                       */
/*    Robert Norton-Wright                                                  */
/*    Christopher Pulte                                                     */
/*    Peter Sewell                                                          */
/*    Alasdair Armstrong                                                    */
/*    Brian Campbell                                                        */
/*    Thomas Bauereiss                                                      */
/*    Anthony Fox                                                           */
/*    Jon French                                                            */
/*    Dominic Mulligan                                                      */
/*    Stephen Kell                                                          */
/*    Mark Wassell                                                          */
/*    Alastair Reid (Arm Ltd)                                               */
/*                                                                          */
/*  All rights reserved.                                                    */
/*                                                                          */
/*  This work was partially supported by EPSRC grant EP/K008528/1 <a        */
/*  href="http://www.cl.cam.ac.uk/users/pes20/rems">REMS: Rigorous          */
/*  Engineering for Mainstream Systems</a>, an ARM iCASE award, EPSRC IAA   */
/*  KTF funding, and donations from Arm.  This project has received         */
/*  funding from the European Research Council (ERC) under the European     */
/*  Union’s Horizon 2020 research and innovation programme (grant           */
/*  agreement No 789108, ELVER).                                            */
/*                                                                          */
/*  This software was developed by SRI International and the University of  */
/*  Cambridge Computer Laboratory (Department of Computer Science and       */
/*  Technology) under DARPA/AFRL contracts FA8650-18-C-7809 ("CIFV")        */
/*  and FA8750-10-C-0237 ("CTSRD").                                         */
/*                                                                          */
/*  SPDX-License-Identifier: BSD-2-Clause                                   */
/****************************************************************************/

#ifndef SAIL_H
#define SAIL_H

#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
/* Keep Sail's mathematical integer ABI while replacing GMP with a bounded,
 * inline representation.  The model's bound ledger proves that every runtime
 * value fits in 768 bits; the implementation traps instead of truncating if
 * that contract is ever violated.  The array-of-one shape deliberately
 * matches mpz_t, so ordinary C generated without --Ofixed-int needs no
 * rewrite. */
#define SAIL_INT_LIMBS 12
typedef struct {
  uint8_t negative;
  uint8_t length;
  uint8_t _padding[6];
  uint64_t limbs[SAIL_INT_LIMBS];
} sail_int_struct;
typedef sail_int_struct sail_int[1];
typedef sail_int_struct mpz_t[1];

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void *sail_malloc(size_t size)
{
  return malloc(size);
}

static inline void sail_free(void *ptr)
{
  free(ptr);
}

#define sail_new(type) (type *)(sail_malloc(sizeof(type)))
#define sail_new_array(type, len) (type *)(sail_malloc((len) * sizeof(type)))

/*
 * Called by the RTS to initialise and clear any library state.
 */
void setup_library(void);
void cleanup_library(void);

/*
 * The Sail compiler expects functions to follow a specific naming
 * convention for allocation, deallocation, and (deep)-copying. These
 * macros implement this naming convention.
 */

/* Allocate a new instance of the type. */
#define CREATE(type) create_ ## type
/* Deallocate an instance of the type (it must be valid) and then
   allocate a new one. Equivalent to KILL(type); CREATE(type);
   except it can be more efficient because it is allowed to
   reuse allocations. */
#define RECREATE(type) recreate_ ## type
/* Allocate a new instance of the type and set its value to type2.
   Equivalent to CREATE(type1); CONVERT_OF(type1, type2); but
   it can be more efficient. */
#define CREATE_OF(type1, type2) create_ ## type1 ## _of_ ## type2
/* Equivalent to KILL(type1); CREATE(type1); CONVERT_OF(type1; type2);
   but more efficient. */
#define RECREATE_OF(type1, type2) recreate_ ## type1 ## _of_ ## type2
/* Set type1 = type2. Both must have already been CREATE()ed.
   The types can be different, and a conversion will be performed. */
#define CONVERT_OF(type1, type2) convert_ ## type1 ## _of_ ## type2
/* Like CONVERT_OF() but the types must be the same and it is just
   copied without any conversion. */
#define COPY(type) copy_ ## type
/* Deallocate an instance of the type. It is not safe to call KILL()
   on a value that has already been KILL()ed or has never been CREATE()ed. */
#define KILL(type) kill_ ## type
/* Set a value to the `undefined` Sail value (it can be anything).
   The instance must have been CREATE()ed. */
#define UNDEFINED(type) undefined_ ## type
/* Return true if two instances of the type are semantically equal
   (e.g. two strings are compared by values, not by their addresses). */
#define EQUAL(type) eq_ ## type

#define SAIL_BUILTIN_TYPE_IMPL(type, const_type)\
  void create_ ## type(type *);\
  void recreate_ ## type(type *);\
  void copy_ ## type(type *, const_type);\
  void kill_ ## type(type *);
#define SAIL_BUILTIN_TYPE(type) SAIL_BUILTIN_TYPE_IMPL(type, const type)
/* ***** Sail unit type ***** */

typedef int unit;

#define UNIT 0

unit UNDEFINED(unit)(const unit);
bool EQUAL(unit)(const unit, const unit);

unit skip(const unit);

/* ***** Sail booleans ***** */

/*
 * and_bool and or_bool are special-cased by the compiler to ensure
 * short-circuiting evaluation.
 */
#ifndef __cplusplus
static inline bool not(bool b)
{
     return !b;
}
#endif
bool EQUAL(bool)(const bool, const bool);
bool UNDEFINED(bool)(const unit);

/* ***** Sail strings ***** */

/*
 * Sail strings are just C strings.
 */
typedef char *sail_string;
typedef const char *const_sail_string;

SAIL_BUILTIN_TYPE_IMPL(sail_string, const_sail_string)

void dec_str(sail_string *str, const sail_int n);
void hex_str(sail_string *str, const sail_int n);
void hex_str_upper(sail_string *str, const sail_int n);

void undefined_string(sail_string *str, const unit u);

bool eq_string(const_sail_string, const_sail_string);
bool EQUAL(sail_string)(const_sail_string, const_sail_string);

void concat_str(sail_string *stro, const_sail_string str1, const_sail_string str2);
bool string_startswith(const_sail_string s, const_sail_string prefix);

/* ***** Sail integers ***** */

typedef int64_t mach_int;
typedef uint64_t mach_uint;

bool EQUAL(mach_int)(const mach_int, const mach_int);
bool EQUAL(mach_uint)(const mach_uint, const mach_uint);
mach_uint CONVERT_OF(mach_uint, mach_int)(const mach_int);
mach_int CONVERT_OF(mach_int, mach_uint)(const mach_uint);

void setup_rts(void);
void cleanup_rts(void);

uint64_t sail_int_get_ui(const sail_int op);
/* Portable fixed-limb bridges used by native backend representations.
 * Limbs are little-endian: limbs[0] contains the least-significant 64 bits. */
void sail_int_to_u64_array(uint64_t *, size_t, const sail_int);
void sail_int_from_u64_array(sail_int *, const uint64_t *, size_t);
void sail_int_from_twos_complement_u64_array(sail_int *, const uint64_t *, size_t);
static inline uint64_t mpz_get_ui(const sail_int op) {
  return op->length == 0 ? UINT64_C(0) : op->limbs[0];
}
static inline long mpz_get_si(const sail_int op) {
  if (op->length > 1) abort();
  uint64_t magnitude = mpz_get_ui(op);
  if (!op->negative) {
    if (magnitude > (uint64_t)LONG_MAX) abort();
    return (long)magnitude;
  }
  if (magnitude > (uint64_t)LONG_MAX + UINT64_C(1)) abort();
  if (magnitude == (uint64_t)LONG_MAX + UINT64_C(1)) return LONG_MIN;
  return -(long)magnitude;
}
static inline void mpz_set_ui(sail_int result, unsigned long value) {
  result->negative = 0;
  result->length = value == 0 ? 0 : 1;
  if (value != 0) result->limbs[0] = value;
}
static inline void mpz_set_si(sail_int result, long value) {
  uint64_t magnitude = value < 0
      ? (uint64_t)(-(value + 1)) + UINT64_C(1)
      : (uint64_t)value;
  result->negative = value < 0;
  result->length = magnitude == 0 ? 0 : 1;
  if (magnitude != 0) result->limbs[0] = magnitude;
}
int mpz_set_str(sail_int result, const char *string, int base);

#define SAIL_INT_FUNCTION(fname, rtype, ...) void fname(rtype *, __VA_ARGS__)

SAIL_BUILTIN_TYPE(sail_int)

void CREATE_OF(sail_int, mach_int)(sail_int *, const mach_int);
void RECREATE_OF(sail_int, mach_int)(sail_int *, const mach_int);
mach_int CREATE_OF(mach_int, sail_int)(const sail_int);
void CREATE_OF(sail_int, sail_string)(sail_int *, const_sail_string);
void RECREATE_OF(sail_int, sail_string)(sail_int *, const_sail_string);
void CONVERT_OF(sail_int, sail_string)(sail_int *, const_sail_string);
mach_int CONVERT_OF(mach_int, sail_int)(const sail_int);
void CONVERT_OF(sail_int, mach_int)(sail_int *, const mach_int);
mach_uint CONVERT_OF(mach_uint, sail_int)(const sail_int);
void CONVERT_OF(sail_int, mach_uint)(sail_int *, const mach_uint);

bool eq_int(const sail_int, const sail_int);
bool EQUAL(sail_int)(const sail_int, const sail_int);
bool lt(const sail_int, const sail_int);
bool gt(const sail_int, const sail_int);
bool lteq(const sail_int, const sail_int);
bool gteq(const sail_int, const sail_int);

mach_int shl_mach_int(const mach_int, const mach_int);
mach_int shr_mach_int(const mach_int, const mach_int);
SAIL_INT_FUNCTION(shl_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(shr_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(undefined_int, sail_int, const int);
SAIL_INT_FUNCTION(undefined_nat, sail_int, const unit);
SAIL_INT_FUNCTION(undefined_range, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(add_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(sub_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(sub_nat, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(mult_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(ediv_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(emod_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(tdiv_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(tmod_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(fdiv_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(fmod_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(max_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(min_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(neg_int, sail_int, const sail_int);
SAIL_INT_FUNCTION(abs_int, sail_int, const sail_int);
SAIL_INT_FUNCTION(pow_int, sail_int, const sail_int, const sail_int);
SAIL_INT_FUNCTION(pow2, sail_int, const sail_int);
void make_the_value(sail_int *, const sail_int);
void size_itself_int(sail_int *, const sail_int);

/* ***** Sail bitvectors ***** */

typedef uint64_t fbits;

bool eq_bit(const fbits a, const fbits b);

static inline bool bit_to_bool(const fbits a)
{
  return (bool) a;
}

bool EQUAL(fbits)(const fbits, const fbits);
bool EQUAL(ref_fbits)(const fbits*, const fbits*);

typedef struct {
  uint64_t len;
  uint64_t bits;
} sbits;

/* Native representation selected from a proven 256-bit unsigned bound. Defining it in the
 * runtime makes the same ABI available to injected FFI headers before the
 * generated model header is included. */
#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } sail_u256;
#endif

// For backwards compatibility
typedef uint64_t mach_bits;
sbits CREATE_OF(sbits, fbits)(const fbits op, const uint64_t len, const bool direction);

/* Bitvector conversions */

fbits CONVERT_OF(fbits, sbits)(const sbits, const bool);
sbits CONVERT_OF(sbits, fbits)(const fbits, const uint64_t, const bool);
fbits UNDEFINED(fbits)(const unit);

sbits undefined_sbits(void);

/*
 * Wrapper around >> operator to avoid UB when shift amount is greater
 * than or equal to 64.
 */
fbits safe_rshift(const fbits, const fbits);

fbits fast_zero_extend(const sbits op, const uint64_t n);
fbits fast_sign_extend(const fbits op, const uint64_t n, const uint64_t m);
fbits fast_sign_extend2(const sbits op, const uint64_t m);

fbits update_fbits(const fbits op, const uint64_t n, const fbits bit);

mach_int fast_signed(const fbits, const uint64_t);
mach_int fast_unsigned(const fbits);

sbits append_sf(const sbits, const fbits, const uint64_t);
sbits append_fs(const fbits, const uint64_t, const sbits);
sbits append_ss(const sbits, const sbits);
fbits fast_replicate_bits(const fbits shift, const fbits v, const mach_int times);

fbits fast_update_subrange(const fbits op,
			   const mach_int n,
			   const mach_int m,
			   const fbits slice);

sbits sslice(const fbits op, const mach_int start, const mach_int len);

bool eq_sbits(const sbits op1, const sbits op2);
bool neq_sbits(const sbits op1, const sbits op2);
sbits not_sbits(const sbits op);
sbits xor_sbits(const sbits op1, const sbits op2);
sbits or_sbits(const sbits op1, const sbits op2);
sbits and_sbits(const sbits op1, const sbits op2);
sbits add_sbits(const sbits op1, const sbits op2);
sbits sub_sbits(const sbits op1, const sbits op2);

/* ***** Sail reals ***** */

typedef struct { uint64_t _num, _den; } real;  /* unused by the EVM guest (no GMP) */

SAIL_BUILTIN_TYPE(real)

void CREATE_OF(real, sail_string)(real *rop, const_sail_string op);
void CONVERT_OF(real, sail_string)(real *rop, const_sail_string op);

void UNDEFINED(real)(real *rop, unit u);

void neg_real(real *rop, const real op);

void mult_real(real *rop, const real op1, const real op2);
void sub_real(real *rop, const real op1, const real op2);
void add_real(real *rop, const real op1, const real op2);
void div_real(real *rop, const real op1, const real op2);

void sqrt_real(real *rop, const real op);
void abs_real(real *rop, const real op);

SAIL_INT_FUNCTION(round_up, sail_int, const real op);
SAIL_INT_FUNCTION(round_down, sail_int, const real op);

void to_real(real *rop, const sail_int op);

bool EQUAL(real)(const real op1, const real op2);

bool lt_real(const real op1, const real op2);
bool gt_real(const real op1, const real op2);
bool lteq_real(const real op1, const real op2);
bool gteq_real(const real op1, const real op2);

void real_power(real *rop, const real base, const sail_int exp);

unit print_real(const_sail_string, const real);
unit prerr_real(const_sail_string, const real);

void random_real(real *rop, unit);

/* ***** String utilities ***** */

SAIL_INT_FUNCTION(string_length, sail_int, const_sail_string s);
void string_drop(sail_string *dst, const_sail_string s, sail_int len);
void string_take(sail_string *dst, const_sail_string s, sail_int len);


/* ***** Printing ***** */

void string_of_int(sail_string *str, const sail_int i);
void string_of_fbits(sail_string *str, const fbits op);
void decimal_string_of_fbits(sail_string *str, const fbits op);

/* ***** Mapping support ***** */

bool valid_dec_bits(const sail_int n, const char *dec);
bool valid_hex_bits(const sail_int n, const_sail_string str);

unit print(const_sail_string str);
unit print_endline(const_sail_string str);

unit prerr(const_sail_string str);
unit prerr_endline(const_sail_string str);

unit print_int(const_sail_string str, const sail_int op);
unit fast_print_int(const_sail_string str, const int64_t op);
unit prerr_int(const_sail_string str, const sail_int op);

unit sail_putchar(const sail_int op);

/* ***** Misc ***** */

SAIL_INT_FUNCTION(get_time_ns, sail_int, const unit);

#ifdef __cplusplus
}
#endif

#endif
