//
// Created by liyinbin on 2019/12/11.
//

#ifndef ABEL_BASE_PROFILE_BASE_H_
#define ABEL_BASE_PROFILE_BASE_H_

#include <abel/base/profile/compiler.h>
#include <abel/base/profile/compiler_traits.h>
#include <abel/base/profile/platform.h>
#include <stddef.h>  // For size_t.
#include <string.h>  // For memcpy.

// ------------------------------------------------------------------------
// The C++ standard defines size_t as a built-in type. Some compilers are
// not standards-compliant in this respect, so we need an additional include.
// The case is similar with wchar_t under C++.

#if defined(ABEL_COMPILER_GNUC) || defined(ABEL_COMPILER_MSVC) || defined(ABEL_WCHAR_T_NON_NATIVE) || defined(ABEL_PLATFORM_SONY)
    #if defined(ABEL_COMPILER_MSVC)
        #pragma warning(push, 0)
        #pragma warning(disable: 4265 4365 4836 4574)
    #endif
    #include <stddef.h>
    #if defined(ABEL_COMPILER_MSVC)
        #pragma warning(pop)
    #endif
#endif

// ------------------------------------------------------------------------
// Include stddef.h on Apple's clang compiler to ensure the ptrdiff_t type
// is defined.
#if defined(ABEL_COMPILER_CLANG) && defined(ABEL_PLATFORM_APPLE)
    #include <stddef.h>
#endif

// ------------------------------------------------------------------------
// Include assert.h on C11 supported compilers so we may allow static_assert usage
// http://en.cppreference.com/w/c/error/static_assert
// C11 standard(ISO / IEC 9899:2011) :
// 7.2/3 Diagnostics <assert.h>(p : 186)
#if !defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201100L
    #include <assert.h>
#endif


// ------------------------------------------------------------------------
// By default, GCC defines NULL as ((void*)0), which is the
// C definition. This causes all sort of problems for C++ code, so it is
// worked around by undefining NULL.

#if defined(NULL)
    #undef NULL
#endif


// ------------------------------------------------------------------------
// Define the NULL pointer. This is normally defined in <stddef.h>, but we
// don't want to force a global dependency on that header, so the definition
// is duplicated here.

#if defined(__cplusplus)
    #define NULL 0
#else
    #define NULL ((void*)0)
#endif


// ------------------------------------------------------------------------
// C98/99 Standard typedefs. From the ANSI ISO/IEC 9899 standards document
// Most recent versions of the gcc-compiler come with these defined in
// inttypes.h or stddef.h. Determining if they are predefined can be
// tricky, so we expect some problems on non-standard compilers

//#if (defined(_INTTYPES_H) || defined(_INTTYPES_H_)) && !defined(PRId64)
//    #error "<inttypes.h> was #included before eabase.h, but without __STDC_FORMAT_MACROS #defined. You must #include eabase.h or an equivalent before #including C99 headers, or you must define __STDC_FORMAT_MACRO before #including system headrs."
//#endif

// ------------------------------------------------------------------------
// We need to test this after we potentially include stddef.h, otherwise we
// would have put this into the compilertraits header.
#if !defined(ABEL_COMPILER_HAS_INTTYPES) && (!defined(_MSC_VER) || (_MSC_VER > 1500)) && (defined(ABEL_COMPILER_IS_C99) || defined(INT8_MIN) || defined(ABEL_COMPILER_HAS_C99_TYPES) || defined(_SN_STDINT_H))
    #define ABEL_COMPILER_HAS_INTTYPES
#endif

#ifdef ABEL_COMPILER_HAS_INTTYPES // If the compiler supports inttypes...
// ------------------------------------------------------------------------
// Include the stdint header to define and derive the required types.
// Additionally include inttypes.h as many compilers, including variations
// of GCC define things in inttypes.h that the C99 standard says goes
// in stdint.h.
//
// The C99 standard specifies that inttypes.h only define printf/scanf
// format macros if __STDC_FORMAT_MACROS is defined before #including
// inttypes.h. For consistency, we do that here.
    #ifndef __STDC_FORMAT_MACROS
        #define __STDC_FORMAT_MACROS
    #endif
// The GCC PSP compiler defines standard int types (e.g. uint32_t) but not PRId8, etc.
// MSVC added support for inttypes.h header in VS2013.
    #if !defined(ABEL_COMPILER_MSVC) || (defined(ABEL_COMPILER_MSVC) && ABEL_COMPILER_VERSION >= 1800)
        #include <inttypes.h> // PRId8, SCNd8, etc.
    #endif
    #if defined(_MSC_VER)
        #pragma warning(push, 0)
    #endif
    #include <stdint.h>   // int32_t, INT64_C, UINT8_MAX, etc.
    #include <math.h>     // float_t, double_t, etc.
    #include <float.h>    // FLT_EVAL_METHOD.
    #if defined(_MSC_VER)
        #pragma warning(pop)
    #endif

    #if !defined(FLT_EVAL_METHOD) && (defined(__FLT_EVAL_METHOD__) || defined(_FEVAL)) // GCC 3.x defines __FLT_EVAL_METHOD__ instead of the C99 standard FLT_EVAL_METHOD.
        #ifdef __FLT_EVAL_METHOD__
            #define FLT_EVAL_METHOD __FLT_EVAL_METHOD__
        #else
            #define FLT_EVAL_METHOD _FEVAL
        #endif
    #endif

// MinGW GCC (up to at least v4.3.0-20080502) mistakenly neglects to define float_t and double_t.
// This appears to be an acknowledged bug as of March 2008 and is scheduled to be fixed.
// Similarly, Android uses a mix of custom standard library headers which prior to SDK API level 21
// don't define float_t and double_t.
    #if defined(__MINGW32__) || (defined(ABEL_PLATFORM_ANDROID) && !(defined(ABEL_ANDROID_SDK_LEVEL) && ABEL_ANDROID_SDK_LEVEL >= 21))
        #if defined(__FLT_EVAL_METHOD__)
            #if(__FLT_EVAL_METHOD__== 0)
typedef float float_t;
typedef double double_t;
            #elif(__FLT_EVAL_METHOD__ == 1)
typedef double float_t;
typedef double double_t;
            #elif(__FLT_EVAL_METHOD__ == 2)
typedef long double float_t;
typedef long double double_t;
            #endif
        #else
typedef float  float_t;
typedef double double_t;
        #endif
    #endif

// The CodeSourcery definitions of PRIxPTR and SCNxPTR are broken for 32 bit systems.
    #if defined(__SIZEOF_SIZE_T__) && (__SIZEOF_SIZE_T__ == 4) && (defined(__have_long64) || defined(__have_longlong64))
        #undef  PRIdPTR
        #define PRIdPTR "d"
        #undef  PRIiPTR
        #define PRIiPTR "i"
        #undef  PRIoPTR
        #define PRIoPTR "o"
        #undef  PRIuPTR
        #define PRIuPTR "u"
        #undef  PRIxPTR
        #define PRIxPTR "x"
        #undef  PRIXPTR
        #define PRIXPTR "X"

        #undef  SCNdPTR
        #define SCNdPTR "d"
        #undef  SCNiPTR
        #define SCNiPTR "i"
        #undef  SCNoPTR
        #define SCNoPTR "o"
        #undef  SCNuPTR
        #define SCNuPTR "u"
        #undef  SCNxPTR
        #define SCNxPTR "x"
    #endif
#else // else we must implement types ourselves.

    #if !defined(__BIT_TYPES_DEFINED__) && !defined(__int8_t_defined)
typedef signed char int8_t;             //< 8 bit signed integer
    #endif
    #if !defined( __int8_t_defined )
typedef signed short int16_t;            //< 16 bit signed integer
typedef signed int
    int32_t;            //< 32 bit signed integer. This works for both 32 bit and 64 bit platforms, as we assume the LP64 is followed.
        #define __int8_t_defined
    #endif
typedef unsigned char uint8_t;            //< 8 bit unsigned integer
typedef unsigned short uint16_t;            //< 16 bit unsigned integer
    #if !defined( __uint32_t_defined )
typedef unsigned int
    uint32_t;            //< 32 bit unsigned integer. This works for both 32 bit and 64 bit platforms, as we assume the LP64 is followed.
        #define __uint32_t_defined
    #endif

// According to the C98/99 standard, FLT_EVAL_METHOD defines control the
// width used for floating point _t types.
    #if defined(_MSC_VER) && _MSC_VER >= 1800
// MSVC's math.h provides float_t, double_t under this condition.
    #elif defined(FLT_EVAL_METHOD)
        #if (FLT_EVAL_METHOD == 0)
typedef float           float_t;
typedef double          double_t;
        #elif (FLT_EVAL_METHOD == 1)
typedef double          float_t;
typedef double          double_t;
        #elif (FLT_EVAL_METHOD == 2)
typedef long double     float_t;
typedef long double     double_t;
        #endif
    #endif

    #if   defined(ABEL_COMPILER_MSVC)
typedef signed __int64      int64_t;
typedef unsigned __int64    uint64_t;

    #else
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
    #endif
#endif


// ------------------------------------------------------------------------
// macros for declaring constants in a portable way.
//
// e.g. int64_t  x =  INT64_C(1234567812345678);
// e.g. int64_t  x =  INT64_C(0x1111111122222222);
// e.g. uint64_t x = UINT64_C(0x1111111122222222);
//
// Microsoft VC++'s definitions of INT8_C/UINT8_C/INT16_C/UINT16_C are like so:
//    #define INT8_C(x)     (x)
//    #define INT16_C(x)    (x)
//    #define UINT8_C(x)    (x)
//    #define UINT16_C(x)   (x)
// To consider: undefine Microsoft's and use the casting versions below.
// ------------------------------------------------------------------------

#ifndef INT8_C_DEFINED // If the user hasn't already defined these...
    #define INT8_C_DEFINED

    #ifndef INT8_C
        #define   INT8_C(x)    int8_t(x)   // For the majority of compilers and platforms, long is 32 bits and long long is 64 bits.
    #endif
    #ifndef UINT8_C
        #define  UINT8_C(x)   uint8_t(x)
    #endif
    #ifndef INT16_C
        #define  INT16_C(x)   int16_t(x)
    #endif
    #ifndef UINT16_C
        #define UINT16_C(x)  uint16_t(x)     // Possibly we should make this be uint16_t(x##u). Let's see how compilers react before changing this.
    #endif
    #ifndef INT32_C
        #define  INT32_C(x)  x##L
    #endif
    #ifndef UINT32_C
        #define UINT32_C(x)  x##UL
    #endif
    #ifndef INT64_C
        #define  INT64_C(x)  x##LL         // The way to deal with this is to compare ULONG_MAX to 0xffffffff and if not equal, then remove the L.
    #endif
    #ifndef UINT64_C
        #define UINT64_C(x)  x##ULL        // We need to follow a similar approach for LL.
    #endif
    #ifndef UINTMAX_C
        #define UINTMAX_C(x) UINT64_C(x)
    #endif
#endif

// ------------------------------------------------------------------------
// type sizes
#ifndef INT8_MAX_DEFINED // If the user hasn't already defined these...
    #define INT8_MAX_DEFINED

// The value must be 2^(n-1)-1
    #ifndef INT8_MAX
        #define INT8_MAX                127
    #endif
    #ifndef INT16_MAX
        #define INT16_MAX               32767
    #endif
    #ifndef INT32_MAX
        #define INT32_MAX               2147483647
    #endif
    #ifndef INT64_MAX
        #define INT64_MAX               INT64_C(9223372036854775807)
    #endif
    #ifndef INTMAX_MAX
        #define INTMAX_MAX               INT64_MAX
    #endif
    #ifndef INTPTR_MAX
        #if ABEL_PLATFORM_PTR_SIZE == 4
            #define INTPTR_MAX          INT32_MAX
        #else
            #define INTPTR_MAX          INT64_MAX
        #endif
    #endif

// The value must be either -2^(n-1) or 1-2(n-1).
    #ifndef INT8_MIN
        #define INT8_MIN                -128
    #endif
    #ifndef INT16_MIN
        #define INT16_MIN               -32768
    #endif
    #ifndef INT32_MIN
        #define INT32_MIN               (-INT32_MAX - 1)  // -2147483648
    #endif
    #ifndef INT64_MIN
        #define INT64_MIN               (-INT64_MAX - 1)  // -9223372036854775808
    #endif
    #ifndef INTMAX_MIN
        #define INTMAX_MIN               INT64_MIN
    #endif
    #ifndef INTPTR_MIN
        #if ABEL_PLATFORM_PTR_SIZE == 4
            #define INTPTR_MIN          INT32_MIN
        #else
            #define INTPTR_MIN          INT64_MIN
        #endif
    #endif

// The value must be 2^n-1
    #ifndef UINT8_MAX
        #define UINT8_MAX               0xffU                        // 255
    #endif
    #ifndef UINT16_MAX
        #define UINT16_MAX              0xffffU                      // 65535
    #endif
    #ifndef UINT32_MAX
        #define UINT32_MAX              UINT32_C(0xffffffff)         // 4294967295
    #endif
    #ifndef UINT64_MAX
        #define UINT64_MAX              UINT64_C(0xffffffffffffffff) // 18446744073709551615
    #endif
    #ifndef UINTMAX_MAX
        #define UINTMAX_MAX              UINT64_MAX
    #endif
    #ifndef UINTPTR_MAX
        #if ABEL_PLATFORM_PTR_SIZE == 4
            #define UINTPTR_MAX          UINT32_MAX
        #else
            #define UINTPTR_MAX          UINT64_MAX
        #endif
    #endif
#endif

#ifndef FLT_EVAL_METHOD
    #define FLT_EVAL_METHOD 0
typedef float float_t;
typedef double double_t;
#endif

#if defined(ABEL_COMPILER_HAS_INTTYPES) && (!defined(ABEL_COMPILER_MSVC) || (defined(ABEL_COMPILER_MSVC) && ABEL_COMPILER_VERSION >= 1800))
    #define ABEL_COMPILER_HAS_C99_FORMAT_MACROS
#endif

#ifndef ABEL_COMPILER_HAS_C99_FORMAT_MACROS
// ------------------------------------------------------------------------
// sized printf and scanf format specifiers
// See the C99 standard, section 7.8.1 -- Macros for format specifiers.
//
// The C99 standard specifies that inttypes.h only define printf/scanf
// format macros if __STDC_FORMAT_MACROS is defined before #including
// inttypes.h. For consistency, we define both __STDC_FORMAT_MACROS and
// the printf format specifiers here. We also skip the "least/most"
// variations of these specifiers, as we've decided to do so with
// basic types.
//
// For 64 bit systems, we assume the LP64 standard is followed
// (as opposed to ILP64, etc.) For 32 bit systems, we assume the
// ILP32 standard is followed. See:
//    http://www.opengroup.org/public/tech/aspen/lp64_wp.htm
// for information about this. Thus, on both 32 and 64 bit platforms,
// %l refers to 32 bit data while %ll refers to 64 bit data.

    #ifndef __STDC_FORMAT_MACROS
        #define __STDC_FORMAT_MACROS
    #endif

    #if defined(ABEL_COMPILER_MSVC) || defined(ABEL_COMPILER_BORLAND) // VC++ 7.1+ understands long long as a data type but doesn't accept %ll as a printf specifier.
        #define ABEL_PRI_64_LENGTH_SPECIFIER "I64"
        #define ABEL_SCN_64_LENGTH_SPECIFIER "I64"
    #else
        #define ABEL_PRI_64_LENGTH_SPECIFIER "ll"
        #define ABEL_SCN_64_LENGTH_SPECIFIER "ll"
    #endif // It turns out that some platforms use %q to represent a 64 bit value, but these are not relevant to us at this time.

// Printf format specifiers
    #if defined(ABEL_COMPILER_IS_C99) || defined(ABEL_COMPILER_GNUC)
        #define PRId8     "hhd"
        #define PRIi8     "hhi"
        #define PRIo8     "hho"
        #define PRIu8     "hhu"
        #define PRIx8     "hhx"
        #define PRIX8     "hhX"
    #else // VC++, Borland, etc. which have no way to specify 8 bit values other than %c.
        #define PRId8     "c"  // This may not work properly but it at least will not crash. Try using 16 bit versions instead.
        #define PRIi8     "c"  //  "
        #define PRIo8     "o"  //  "
        #define PRIu8     "u"  //  "
        #define PRIx8     "x"  //  "
        #define PRIX8     "X"  //  "
    #endif

    #define PRId16        "hd"
    #define PRIi16        "hi"
    #define PRIo16        "ho"
    #define PRIu16        "hu"
    #define PRIx16        "hx"
    #define PRIX16        "hX"

    #define PRId32        "d" // This works for both 32 bit and 64 bit systems, as we assume LP64 conventions.
    #define PRIi32        "i"
    #define PRIo32        "o"
    #define PRIu32        "u"
    #define PRIx32        "x"
    #define PRIX32        "X"

    #define PRId64        ABEL_PRI_64_LENGTH_SPECIFIER "d"
    #define PRIi64        ABEL_PRI_64_LENGTH_SPECIFIER "i"
    #define PRIo64        ABEL_PRI_64_LENGTH_SPECIFIER "o"
    #define PRIu64        ABEL_PRI_64_LENGTH_SPECIFIER "u"
    #define PRIx64        ABEL_PRI_64_LENGTH_SPECIFIER "x"
    #define PRIX64        ABEL_PRI_64_LENGTH_SPECIFIER "X"

    #if (ABEL_PLATFORM_PTR_SIZE == 4)
        #define PRIdPTR       PRId32 // Usage of pointer values will generate warnings with
        #define PRIiPTR       PRIi32 // some compilers because they are defined in terms of
        #define PRIoPTR       PRIo32 // integers. However, you can't simply use "p" because
        #define PRIuPTR       PRIu32 // 'p' is interpreted in a specific and often different
        #define PRIxPTR       PRIx32 // way by the library.
        #define PRIXPTR       PRIX32
    #elif (ABEL_PLATFORM_PTR_SIZE == 8)
        #define PRIdPTR       PRId64
        #define PRIiPTR       PRIi64
        #define PRIoPTR       PRIo64
        #define PRIuPTR       PRIu64
        #define PRIxPTR       PRIx64
        #define PRIXPTR       PRIX64
    #endif

// Scanf format specifiers
    #if defined(ABEL_COMPILER_IS_C99) || defined(ABEL_COMPILER_GNUC)
        #define SCNd8     "hhd"
        #define SCNi8     "hhi"
        #define SCNo8     "hho"
        #define SCNu8     "hhu"
        #define SCNx8     "hhx"
    #else // VC++, Borland, etc. which have no way to specify 8 bit values other than %c.
        #define SCNd8     "c" // This will not work properly but it at least will not crash. Try using 16 bit versions instead.
        #define SCNi8     "c" //  "
        #define SCNo8     "c" //  "
        #define SCNu8     "c" //  "
        #define SCNx8     "c" //  "
    #endif

    #define SCNd16        "hd"
    #define SCNi16        "hi"
    #define SCNo16        "ho"
    #define SCNu16        "hu"
    #define SCNx16        "hx"

    #define SCNd32        "d" // This works for both 32 bit and 64 bit systems, as we assume LP64 conventions.
    #define SCNi32        "i"
    #define SCNo32        "o"
    #define SCNu32        "u"
    #define SCNx32        "x"

    #define SCNd64        ABEL_SCN_64_LENGTH_SPECIFIER "d"
    #define SCNi64        ABEL_SCN_64_LENGTH_SPECIFIER "i"
    #define SCNo64        ABEL_SCN_64_LENGTH_SPECIFIER "o"
    #define SCNu64        ABEL_SCN_64_LENGTH_SPECIFIER "u"
    #define SCNx64        ABEL_SCN_64_LENGTH_SPECIFIER "x"

    #if defined(ABEL_COMPILER_MSVC) && (ABEL_COMPILER_VERSION >= 1900)
        #define SCNdPTR       PRIdPTR
        #define SCNiPTR       PRIiPTR
        #define SCNoPTR       PRIoPTR
        #define SCNuPTR       PRIuPTR
        #define SCNxPTR       PRIxPTR
    #elif (ABEL_PLATFORM_PTR_SIZE == 4)
        #define SCNdPTR       SCNd32 // Usage of pointer values will generate warnings with
        #define SCNiPTR       SCNi32 // some compilers because they are defined in terms of
        #define SCNoPTR       SCNo32 // integers. However, you can't simply use "p" because
        #define SCNuPTR       SCNu32 // 'p' is interpreted in a specific and often different
        #define SCNxPTR       SCNx32 // way by the library.
    #elif (ABEL_PLATFORM_PTR_SIZE == 8)
        #define SCNdPTR       SCNd64
        #define SCNiPTR       SCNi64
        #define SCNoPTR       SCNo64
        #define SCNuPTR       SCNu64
        #define SCNxPTR       SCNx64
    #endif
#endif


// ------------------------------------------------------------------------
// bool8_t
// The definition of a bool8_t is controversial with some, as it doesn't
// act just like built-in bool. For example, you can assign -100 to it.
//
#ifndef BOOL8_T_DEFINED // If the user hasn't already defined this...
    #define BOOL8_T_DEFINED
    #if defined(ABEL_COMPILER_MSVC) || (defined(ABEL_COMPILER_INTEL) && defined(ABEL_PLATFORM_WINDOWS)) || defined(ABEL_COMPILER_BORLAND)
        #if defined(__cplusplus)
typedef bool bool8_t;
        #else
typedef int8_t bool8_t;
        #endif
    #else // ABEL_COMPILER_GNUC generally uses 4 bytes per bool.
typedef int8_t bool8_t;
    #endif
#endif


// ------------------------------------------------------------------------
// intptr_t / uintptr_t
// Integer type guaranteed to be big enough to hold
// a native pointer ( intptr_t is defined in STDDEF.H )
//
#if !defined(_INTPTR_T_DEFINED) && !defined(_intptr_t_defined) && !defined(ABEL_COMPILER_HAS_C99_TYPES)
    #if (ABEL_PLATFORM_PTR_SIZE == 4)
typedef int32_t            intptr_t;
    #elif (ABEL_PLATFORM_PTR_SIZE == 8)
typedef int64_t            intptr_t;
    #endif

    #define _intptr_t_defined
    #define _INTPTR_T_DEFINED
#endif

#if !defined(_UINTPTR_T_DEFINED) && !defined(_uintptr_t_defined) && !defined(ABEL_COMPILER_HAS_C99_TYPES)
    #if (ABEL_PLATFORM_PTR_SIZE == 4)
typedef uint32_t           uintptr_t;
    #elif (ABEL_PLATFORM_PTR_SIZE == 8)
typedef uint64_t           uintptr_t;
    #endif

    #define _uintptr_t_defined
    #define _UINTPTR_T_DEFINED
#endif

#if !defined(ABEL_COMPILER_HAS_INTTYPES)
    #ifndef INTMAX_T_DEFINED
        #define INTMAX_T_DEFINED

// At this time, all supported compilers have int64_t as the max
// integer type. Some compilers support a 128 bit integer type,
// but in some cases it is not a true int128_t but rather a
// crippled data type. Also, it turns out that Unix 64 bit ABIs
// require that intmax_t be int64_t and nothing larger. So we
// play it safe here and set intmax_t to int64_t, even though
// an int128_t type may exist.

typedef int64_t intmax_t;
typedef uint64_t uintmax_t;
    #endif
#endif


// ------------------------------------------------------------------------
// ssize_t
// signed equivalent to size_t.
// This is defined by GCC (except the QNX implementation of GCC) but not by other compilers.
//
#if !defined(__GNUC__)
// As of this writing, all non-GCC compilers significant to us implement
// uintptr_t the same as size_t. However, this isn't guaranteed to be
// so for all compilers, as size_t may be based on int, long, or long long.
    #if !defined(_SSIZE_T_) && !defined(_SSIZE_T_DEFINED)
        #define _SSIZE_T_
        #define _SSIZE_T_DEFINED

        #if defined(_MSC_VER) && (ABEL_PLATFORM_PTR_SIZE == 8)
typedef __int64 ssize_t;
        #else
typedef long ssize_t;
        #endif
    #endif
#else
    #include <sys/types.h>
#endif


// ------------------------------------------------------------------------
// Character types
//
#if defined(ABEL_COMPILER_MSVC) || defined(ABEL_COMPILER_BORLAND)
    #if defined(ABEL_WCHAR_T_NON_NATIVE)
// In this case, wchar_t is not defined unless we include
// wchar.h or if the compiler makes it built-in.
        #ifdef ABEL_COMPILER_MSVC
            #pragma warning(push, 3)
        #endif
        #include <wchar.h>
        #ifdef ABEL_COMPILER_MSVC
            #pragma warning(pop)
        #endif
    #endif
#endif


// ------------------------------------------------------------------------
// char8_t  -- Guaranteed to be equal to the compiler's char data type.
//             Some compilers implement char8_t as unsigned, though char
//             is usually set to be signed.
//
// char16_t -- This is set to be an unsigned 16 bit value. If the compiler
//             has wchar_t as an unsigned 16 bit value, then char16_t is
//             set to be the same thing as wchar_t in order to allow the
//             user to use char16_t with standard wchar_t functions.
//
// char32_t -- This is set to be an unsigned 32 bit value. If the compiler
//             has wchar_t as an unsigned 32 bit value, then char32_t is
//             set to be the same thing as wchar_t in order to allow the
//             user to use char32_t with standard wchar_t functions.
//
// ABEL_CHAR8_UNIQUE
// ABEL_CHAR16_NATIVE
// ABEL_CHAR32_NATIVE
// ABEL_WCHAR_UNIQUE
//
// VS2010 unilaterally defines char16_t and char32_t in its yvals.h header
// unless _HAS_CHAR16_T_LANGUAGE_SUPPORT or _CHAR16T are defined.
// However, VS2010 does not support the C++0x u"" and U"" string literals,
// which makes its definition of char16_t and char32_t somewhat useless.
// Until VC++ supports string literals, the build system should define
// _CHAR16T and let cxxbase define char16_t and ABEL_CHAR16.
//
// GCC defines char16_t and char32_t in the C compiler in -std=gnu99 mode,
// as __CHAR16_TYPE__ and __CHAR32_TYPE__, and for the C++ compiler
// in -std=c++0x and -std=gnu++0x modes, as char16_t and char32_t too.
//
// The ABEL_WCHAR_UNIQUE symbol is defined to 1 if wchar_t is distinct from
// char8_t, char16_t, and char32_t, and defined to 0 if not. In some cases,
// if the compiler does not support char16_t/char32_t, one of these two types
// is typically a typedef or define of wchar_t. For compilers that support
// the C++11 unicode character types often overloads must be provided to
// support existing code that passes a wide char string to a function that
// takes a unicode string.
//
// The ABEL_CHAR8_UNIQUE symbol is defined to 1 if char8_t is distinct type
// from char in the type system, and defined to 0 if otherwise.

#if !defined(ABEL_CHAR16_NATIVE)
// To do: Change this to be based on ABEL_COMPILER_NO_NEW_CHARACTER_TYPES.
    #if defined(_MSC_VER) && (_MSC_VER >= 1600) && defined(_HAS_CHAR16_T_LANGUAGE_SUPPORT) && _HAS_CHAR16_T_LANGUAGE_SUPPORT // VS2010+
        #define ABEL_CHAR16_NATIVE 1
    #elif defined(ABEL_COMPILER_CLANG) && defined(ABEL_COMPILER_CPP11_ENABLED)
        #if __has_feature(cxx_unicode_literals)
            #define ABEL_CHAR16_NATIVE 1
        #elif (ABEL_COMPILER_VERSION >= 300) && !(defined(ABEL_PLATFORM_IPHONE) || defined(ABEL_PLATFORM_OSX))
            #define ABEL_CHAR16_NATIVE 1
        #elif defined(ABEL_PLATFORM_APPLE)
            #define ABEL_CHAR16_NATIVE 1
        #else
            #define ABEL_CHAR16_NATIVE 0
        #endif
    #elif defined(__EDG_VERSION__) && (__EDG_VERSION__ >= 404) && defined(__CHAR16_TYPE__) && defined(ABEL_COMPILER_CPP11_ENABLED)// EDG 4.4+.
        #define ABEL_CHAR16_NATIVE 1
    #elif defined(ABEL_COMPILER_GNUC) && (ABEL_COMPILER_VERSION >= 4004) && !defined(ABEL_COMPILER_EDG) && (defined(ABEL_COMPILER_CPP11_ENABLED) || defined(__STDC_VERSION__)) // g++ (C++ compiler) 4.4+ with -std=c++0x or gcc (C compiler) 4.4+ with -std=gnu99
        #define ABEL_CHAR16_NATIVE 1
    #else
        #define ABEL_CHAR16_NATIVE 0
    #endif
#endif

#if !defined(ABEL_CHAR32_NATIVE)                    // Microsoft currently ties char32_t language support to char16_t language support. So we use CHAR16_T here.
// To do: Change this to be based on ABEL_COMPILER_NO_NEW_CHARACTER_TYPES.
    #if defined(_MSC_VER) && (_MSC_VER >= 1600) && defined(_HAS_CHAR16_T_LANGUAGE_SUPPORT) && _HAS_CHAR16_T_LANGUAGE_SUPPORT // VS2010+
        #define ABEL_CHAR32_NATIVE 1
    #elif defined(ABEL_COMPILER_CLANG) && defined(ABEL_COMPILER_CPP11_ENABLED)
        #if __has_feature(cxx_unicode_literals)
            #define ABEL_CHAR32_NATIVE 1
        #elif (ABEL_COMPILER_VERSION >= 300) && !(defined(ABEL_PLATFORM_IPHONE) || defined(ABEL_PLATFORM_OSX))
            #define ABEL_CHAR32_NATIVE 1
        #elif defined(ABEL_PLATFORM_APPLE)
            #define ABEL_CHAR32_NATIVE 1
        #else
            #define ABEL_CHAR32_NATIVE 0
        #endif
    #elif defined(__EDG_VERSION__) && (__EDG_VERSION__ >= 404) && defined(__CHAR32_TYPE__) && defined(ABEL_COMPILER_CPP11_ENABLED)// EDG 4.4+.
        #define ABEL_CHAR32_NATIVE 1
    #elif defined(ABEL_COMPILER_GNUC) && (ABEL_COMPILER_VERSION >= 4004) && !defined(ABEL_COMPILER_EDG) && (defined(ABEL_COMPILER_CPP11_ENABLED) || defined(__STDC_VERSION__)) // g++ (C++ compiler) 4.4+ with -std=c++0x or gcc (C compiler) 4.4+ with -std=gnu99
        #define ABEL_CHAR32_NATIVE 1
    #else
        #define ABEL_CHAR32_NATIVE 0
    #endif
#endif

#if ABEL_CHAR16_NATIVE || ABEL_CHAR32_NATIVE
    #define ABEL_WCHAR_UNIQUE 1
#else
    #define ABEL_WCHAR_UNIQUE 0
#endif


// ABEL_CHAR8_UNIQUE
//
// Check for char8_t support in the cpp type system. Moving forward from c++20,
// the char8_t type allows users to overload function for character encoding.
//
// ABEL_CHAR8_UNIQUE is 1 when the type is a unique in the type system and
// can there be used as a valid overload. ABEL_CHAR8_UNIQUE is 0 otherwise.
//
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0482r6.html
//
#ifdef __cpp_char8_t
    #define CHAR8_T_DEFINED
    #define ABEL_CHAR8_UNIQUE 1
#else
    #define ABEL_CHAR8_UNIQUE 0
#endif

#ifndef CHAR8_T_DEFINED // If the user hasn't already defined these...
    #define CHAR8_T_DEFINED
    #if defined(ABEL_PLATFORM_APPLE)
        #define char8_t char    // The Apple debugger is too stupid to realize char8_t is typedef'd to char, so we #define it.
    #else
typedef char char8_t;
    #endif

    #if ABEL_CHAR16_NATIVE
// In C++, char16_t and char32_t are already defined by the compiler.
// In MS C, char16_t and char32_t are already defined by the compiler/standard library.
// In GCC C, __CHAR16_TYPE__ and __CHAR32_TYPE__ are defined instead, and we must define char16_t and char32_t from these.
        #if defined(__GNUC__) && !defined(__GXX_EXPERIMENTAL_CXX0X__) && defined(__CHAR16_TYPE__) // If using GCC and compiling in C...
            typedef __CHAR16_TYPE__ char16_t;
            typedef __CHAR32_TYPE__ char32_t;
        #endif
    #elif (ABEL_WCHAR_SIZE == 2)
        #if (defined(_MSC_VER) && (_MSC_VER >= 1600)) // if VS2010+ or using platforms that use Dinkumware under a compiler that doesn't natively support C++11 char16_t.
            #if !defined(_CHAR16T)
                #define _CHAR16T
            #endif
            #if !defined(_HAS_CHAR16_T_LANGUAGE_SUPPORT) || !_HAS_CHAR16_T_LANGUAGE_SUPPORT
                typedef wchar_t  char16_t;
                typedef uint32_t char32_t;
            #endif
        #else
            typedef wchar_t  char16_t;
            typedef uint32_t char32_t;
        #endif
    #else
        typedef uint16_t char16_t;
        #if defined(__cplusplus)
            typedef wchar_t  char32_t;
        #else
            typedef uint32_t char32_t;
        #endif
    #endif
#endif


// CHAR8_MIN, CHAR8_MAX, etc.
//
#define ABEL_LIMITS_DIGITS_S(T)  ((sizeof(T) * 8) - 1)
#define ABEL_LIMITS_DIGITS_U(T)  ((sizeof(T) * 8))
#define ABEL_LIMITS_DIGITS(T)    ((ABEL_LIMITS_IS_SIGNED(T) ? ABEL_LIMITS_DIGITS_S(T) : ABEL_LIMITS_DIGITS_U(T)))
#define ABEL_LIMITS_IS_SIGNED(T) ((T)(-1) < 0)
#define ABEL_LIMITS_MIN_S(T)     ((T)((T)1 << ABEL_LIMITS_DIGITS_S(T)))
#define ABEL_LIMITS_MIN_U(T)     ((T)0)
#define ABEL_LIMITS_MIN(T)       ((ABEL_LIMITS_IS_SIGNED(T) ? ABEL_LIMITS_MIN_S(T) : ABEL_LIMITS_MIN_U(T)))
#define ABEL_LIMITS_MAX_S(T)     ((T)(((((T)1 << (ABEL_LIMITS_DIGITS(T) - 1)) - 1) << 1) + 1))
#define ABEL_LIMITS_MAX_U(T)     ((T)~(T)0)
#define ABEL_LIMITS_MAX(T)       ((ABEL_LIMITS_IS_SIGNED(T) ? ABEL_LIMITS_MAX_S(T) : ABEL_LIMITS_MAX_U(T)))

#if !defined(CHAR8_MIN)
    #define CHAR8_MIN ABEL_LIMITS_MIN(char8_t)
#endif

#if !defined(CHAR8_MAX)
    #define CHAR8_MAX ABEL_LIMITS_MAX(char8_t)
#endif

#if !defined(CHAR16_MIN)
    #define CHAR16_MIN ABEL_LIMITS_MIN(char16_t)
#endif

#if !defined(CHAR16_MAX)
    #define CHAR16_MAX ABEL_LIMITS_MAX(char16_t)
#endif

#if !defined(CHAR32_MIN)
    #define CHAR32_MIN ABEL_LIMITS_MIN(char32_t)
#endif

#if !defined(CHAR32_MAX)
    #define CHAR32_MAX ABEL_LIMITS_MAX(char32_t)
#endif



// ABEL_CHAR8 / ABEL_CHAR16 / ABEL_CHAR32 / ABEL_WCHAR
//
// Supports usage of portable string constants.
//
// Example usage:
//     const char16_t* str = ABEL_CHAR16("Hello world");
//     const char32_t* str = ABEL_CHAR32("Hello world");
//     const char16_t  c   = ABEL_CHAR16('\x3001');
//     const char32_t  c   = ABEL_CHAR32('\x3001');
//
#ifndef ABEL_CHAR8
    #if ABEL_CHAR8_UNIQUE
        #define ABEL_CHAR8(s) u8 ## s
    #else
        #define ABEL_CHAR8(s) s
    #endif
#endif

#ifndef ABEL_WCHAR
    #define ABEL_WCHAR_(s) L ## s
    #define ABEL_WCHAR(s)  ABEL_WCHAR_(s)
#endif

#ifndef ABEL_CHAR16
    #if ABEL_CHAR16_NATIVE && !defined(_MSC_VER) // Microsoft doesn't support char16_t string literals.
        #define ABEL_CHAR16_(s) u ## s
        #define ABEL_CHAR16(s)  ABEL_CHAR16_(s)
    #elif (ABEL_WCHAR_SIZE == 2)
        #if defined(_MSC_VER) && (_MSC_VER >= 1900) && defined(__cplusplus) // VS2015 supports u"" string literals.
            #define ABEL_CHAR16_(s) u ## s
            #define ABEL_CHAR16(s)  ABEL_CHAR16_(s)
        #else
            #define ABEL_CHAR16_(s) L ## s
            #define ABEL_CHAR16(s)  ABEL_CHAR16_(s)
        #endif
    #else
//#define ABEL_CHAR16(s) // Impossible to implement efficiently.
    #endif
#endif

#ifndef ABEL_CHAR32
    #if ABEL_CHAR32_NATIVE && !defined(_MSC_VER) // Microsoft doesn't support char32_t string literals.
        #define ABEL_CHAR32_(s) U ## s
        #define ABEL_CHAR32(s)  ABEL_CHAR32_(s)
    #elif (ABEL_WCHAR_SIZE == 2)
        #if defined(_MSC_VER) && (_MSC_VER >= 1900) && defined(__cplusplus) // VS2015 supports u"" string literals.
            #define ABEL_CHAR32_(s) U ## s
            #define ABEL_CHAR32(s)  ABEL_CHAR32_(s)
        #else
//#define ABEL_CHAR32(s) // Impossible to implement.
        #endif
    #elif (ABEL_WCHAR_SIZE == 4)
        #define ABEL_CHAR32_(s) L ## s
        #define ABEL_CHAR32(s)  ABEL_CHAR32_(s)
    #else
        #error Unexpected size of wchar_t
    #endif
#endif

// CBText8 / CBText16
//
// Provided for backwards compatibility with older code.
//
#if defined(CBBASE_ENABLE_CBTEXT_MACROS)
    #define CBText8(x)   x
    #define CBChar8(x)   x

    #define CBText16(x)  ABEL_CHAR16(x)
    #define CBChar16(x)  ABEL_CHAR16(x)
#endif

// ------------------------------------------------------------------------
// static_assert
//
// C++11 static_assert (a.k.a. compile-time assert).
//
// Specification:
//     void static_assert(bool const_expression, const char* description);
//
// Example usage:
//     static_assert(sizeof(int) == 4, "int must be 32 bits");
//
#if defined(_MSC_VER) && (_MSC_VER >= 1600) && defined(__cplusplus)
// static_assert is defined by the compiler for both C and C++.
#elif !defined(__cplusplus) && defined(ABEL_PLATFORM_ANDROID)
// AndroidNDK does not support static_assert despite claiming it's a C11 compiler
    #define NEED_CUSTOM_STATIC_ASSERT
#elif defined(__clang__) && defined(__cplusplus)
// We need to separate these checks on a new line, as the pre-processor on other compilers will fail on the _has_feature macros
    #if !(__has_feature(cxx_static_assert) || __has_extension(cxx_static_assert))
        #define NEED_CUSTOM_STATIC_ASSERT
    #endif
#elif defined(__GNUC__) && (defined(__GXX_EXPERIMENTAL_CXX0X__) || (defined(__cplusplus) && (__cplusplus >= 201103L)))
// static_assert is defined by the compiler.
#elif defined(__EDG_VERSION__) && (__EDG_VERSION__ >= 401) && defined(ABEL_COMPILER_CPP11_ENABLED)
// static_assert is defined by the compiler.
#elif !defined(__cplusplus) && defined(__GLIBC__) && defined(__USE_ISOC11)
// static_assert is defined by the compiler.
#elif !defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201100L
// static_assert is defined by the compiler.
#else
    #define NEED_CUSTOM_STATIC_ASSERT
#endif

#ifdef NEED_CUSTOM_STATIC_ASSERT
    #ifdef __GNUC__
// On GCC the 'unused' attribute can be used to indicate a typedef is not actually used
// (such as in the static_assert implementation below).  New versions of GCC generate
// warnings for unused typedefs in function/method scopes.
        #define ABEL_STATIC_ASSERT_UNUSED_ATTRIBUTE        __attribute__((unused))
    #else
        #define ABEL_STATIC_ASSERT_UNUSED_ATTRIBUTE
    #endif
    #define ABEL_STATIC_ASSERT_TOKEN_PASTE(a, b)        a ## b
    #define ABEL_STATIC_ASSERT_CONCATENATE_HELPER(a, b) ABEL_STATIC_ASSERT_TOKEN_PASTE(a,b)

    #if defined(__COUNTER__) // If this extension is available, which allows multiple statements per line...
        #define static_assert(expression, description) typedef char ABEL_STATIC_ASSERT_CONCATENATE_HELPER(compileTimeAssert,__COUNTER__) [((expression) != 0) ? 1 : -1] ABEL_STATIC_ASSERT_UNUSED_ATTRIBUTE
    #else
        #define static_assert(expression, description) typedef char ABEL_STATIC_ASSERT_CONCATENATE_HELPER(compileTimeAssert,__LINE__) [((expression) != 0) ? 1 : -1] ABEL_STATIC_ASSERT_UNUSED_ATTRIBUTE
    #endif

    #undef NEED_CUSTOM_STATIC_ASSERT
#endif


// Defining common SI unit macros.
//
// The mebibyte is a multiple of the unit byte for digital information. Technically a
// megabyte (MB) is a power of ten, while a mebibyte (MiB) is a power of two,
// appropriate for binary machines. Many Linux distributions use the unit, but it is
// not widely acknowledged within the industry or media.
// Reference: https://en.wikipedia.org/wiki/Mebibyte
//
// Examples:
// 	auto size1 = ABEL_KILOBYTE(16);
// 	auto size2 = ABEL_MEGABYTE(128);
// 	auto size3 = ABEL_MEBIBYTE(8);
// 	auto size4 = ABEL_GIBIBYTE(8);

// define byte for completeness
#define ABEL_BYTE(x) (x)

// Decimal SI units
#define ABEL_KILOBYTE(x) (size_t(x) * 1000)
#define ABEL_MEGABYTE(x) (size_t(x) * 1000 * 1000)
#define ABEL_GIGABYTE(x) (size_t(x) * 1000 * 1000 * 1000)
#define ABEL_TERABYTE(x) (size_t(x) * 1000 * 1000 * 1000 * 1000)
#define ABEL_PETABYTE(x) (size_t(x) * 1000 * 1000 * 1000 * 1000 * 1000)
#define ABEL_EXABYTE(x)  (size_t(x) * 1000 * 1000 * 1000 * 1000 * 1000 * 1000)

// Binary SI units
#define ABEL_KIBIBYTE(x) (size_t(x) * 1024)
#define ABEL_MEBIBYTE(x) (size_t(x) * 1024 * 1024)
#define ABEL_GIBIBYTE(x) (size_t(x) * 1024 * 1024 * 1024)
#define ABEL_TEBIBYTE(x) (size_t(x) * 1024 * 1024 * 1024 * 1024)
#define ABEL_PEBIBYTE(x) (size_t(x) * 1024 * 1024 * 1024 * 1024 * 1024)
#define ABEL_EXBIBYTE(x) (size_t(x) * 1024 * 1024 * 1024 * 1024 * 1024 * 1024)

#ifndef ABEL_CONCAT
# define ABEL_CONCAT(a, b) ABEL_CONCAT_HELPER(a, b)
# define ABEL_CONCAT_HELPER(a, b) a##b
#endif

#ifndef ABEL_SYMBOLSTR
# define ABEL_SYMBOLSTR(a) ABEL_SYMBOLSTR_HELPER(a)
# define ABEL_SYMBOLSTR_HELPER(a) #a
#endif

// ptr:     the pointer to the member.
// type:    the type of the container struct this is embedded in.
// member:  the name of the member within the struct.
#ifndef container_of
# define container_of(ptr, type, member) ({                             \
            const decltype( ((type *)0)->member ) *__mptr = (ptr);  \
            (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

namespace qrpc {
template<typename T>
ABEL_FORCE_INLINE void ignore_result (const T &) {
}
} // namespace qrpc



// ABEL_ARRAYSIZE()
//
// Returns the number of elements in an array as a compile-time constant, which
// can be used in defining new arrays. If you use this macro on a pointer by
// mistake, you will get a compile-time error.
#define ABEL_ARRAYSIZE(array) \
  (sizeof(::abel::macros_internal::ArraySizeHelper(array)))

namespace abel {

namespace macros_internal {
// Note: this internal template function declaration is used by ABEL_ARRAYSIZE.
// The function doesn't need a definition, as we only use its type.
template<typename T, size_t N>
auto ArraySizeHelper (const T (&array)[N]) -> char (&)[N];
}  // namespace macros_internal

}  // namespace abel


// kLinkerInitialized
//
// An enum used only as a constructor argument to indicate that a variable has
// static storage duration, and that the constructor should do nothing to its
// state. Use of this macro indicates to the reader that it is legal to
// declare a static instance of the class, provided the constructor is given
// the abel::base_internal::kLinkerInitialized argument.
//
// Normally, it is unsafe to declare a static variable that has a constructor or
// a destructor because invocation order is undefined. However, if the type can
// be zero-initialized (which the loader does for static variables) into a valid
// state and the type's destructor does not affect storage, then a constructor
// for static initialization can be declared.
//
// Example:
//       // Declaration
//       explicit MyClass(abel::base_internal:LinkerInitialized x) {}
//
//       // Invocation
//       static MyClass my_global(abel::base_internal::kLinkerInitialized);
namespace abel {

namespace base_internal {
enum LinkerInitialized {
    kLinkerInitialized = 0,
};
}  // namespace base_internal

}  // namespace abel


#endif //ABEL_BASE_PROFILE_BASE_H_
