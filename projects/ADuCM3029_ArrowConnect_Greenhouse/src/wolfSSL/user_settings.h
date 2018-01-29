
#if defined(_ARIS_)
//    #define WOLFSSL_CMSIS_RTOS
//#define HAVE_AESGCM
#define NO_WOLFSSL_MEMORY

//#define WOLFSSL_SHA384
//#define WOLFSSL_SHA512
//#define HAVE_CURVE25519
//#define HAVE_ED25519   /* with HAVE_SHA512 */
//#define HAVE_POLY1305
//#define HAVE_CHACHA
//#define HAVE_ONE_TIME_AUTH
#define WOLFSSL_MALLOC_CHECK

//#define IGNORE_KEY_EXTENSIONS
#define NO_WOLFSSL_DIR
//#define DEBUG_WOLFSSL

#define WOLFSSL_STATIC_RSA
//#define HAVE_SUPPORTED_CURVES
#define HAVE_TLS_EXTENSIONS

/* Options for Sample program */
//#define WOLFSSL_NO_VERIFYSERVER
//    #define HAVE_TM_TYPE
#ifndef WOLFSSL_NO_VERIFYSERVER
#   define TIME_OVERRIDES
#   define XTIME time
#   define XGMTIME localtime
#endif

// FIXME rm MBED this for the nucleo
#elif defined(__MBED__) && !defined(__semiconductor__)
#define MBED
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_DEV_RANDOM
#define NO_WOLFSSL_DIR
#define NO_SHA512
#define NO_DH
#define NO_DSA
//#define NO_RSA
#define NO_RC4
#define NO_DES
#define NO_DES3
#define NO_RABBIT
//#define NO_AES
#define NO_ECC256
#define HAVE_ECC
//#define NO_ECC_DHE
#define NO_HC128
#define NO_PSK
#define NO_MD2
#define NO_MD4
//#define NO_MD5
#define NO_OLD_TLS
#define NO_PWDBASED
#define NO_SKID
#define NO_WOLFSSL_SERVER
#define NO_SESSION_CACHE // For Small RAM
//#define IGNORE_KEY_EXTENSIONS
#define NO_WOLFSSL_DIR
//    #define DEBUG_WOLFSSL
#define WOLFSSL_STATIC_RSA
#define HAVE_SUPPORTED_CURVES
#define HAVE_TLS_EXTENSIONS
#define SIZEOF_LONG_LONG  8
/* Options for Sample program */
//#define WOLFSSL_NO_VERIFYSERVER
//#define NO_FILESYSTEM
//    #define HAVE_TM_TYPE
#ifndef WOLFSSL_NO_VERIFYSERVER
#define TIME_OVERRIDES
#define XTIME time
#define XGMTIME localtime
#endif

#elif defined(__linux__)
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_DEV_RANDOM
#define WOLF_LINUX_OS
#define NO_WOLFSSL_DIR
#define NO_SHA512
#define NO_DH
#define NO_DSA
//#define NO_RSA
#define NO_RC4
#define NO_DES
#define NO_DES3
#define NO_RABBIT
//#define NO_AES
#define NO_ECC256
#define HAVE_ECC
//#define NO_ECC_DHE
#define NO_HC128
#define NO_PSK
#define NO_MD2
#define NO_MD4
//#define NO_MD5
#define NO_OLD_TLS
#define NO_PWDBASED
#define NO_SKID
#define NO_WOLFSSL_SERVER
//#define NO_SESSION_CACHE // For Small RAM
#define NO_WOLFSSL_DIR
//#define DEBUG_WOLFSSL
#define WOLFSSL_STATIC_RSA
//#define HAVE_SUPPORTED_CURVES
//#define HAVE_TLS_EXTENSIONS
//#define SIZEOF_LONG_LONG  8
//#define USE_CYASSL_MEMORY
#define NO_WOLFSSL_MEMORY
#define WOLFSSL_NO_VERIFYSERVER
#define NO_SESSION_CACHE // For Small RAM
#define NO_CERT
//#define NO_ECC_DHE
//#define HAVE_AESGCM
//#define NO_FILESYSTEM
    #define HAVE_TM_TYPE
#ifndef WOLFSSL_NO_VERIFYSERVER
    #define TIME_OVERRIDES
    #define XTIME time
    #define XGMTIME localtime_r
#endif

#elif defined(__XCC__)
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_DEV_RANDOM
#define NO_WOLFSSL_DIR
#define NO_SHA512
#define NO_DH
#define NO_DSA
#define NO_HC128
#define SINGLE_THREADED
#define NO_STDIO_FILESYSTEM
#define CTYPE_USER
#define XMALLOC_USER
#define SHA256_DIGEST_SIZE 32
#define WOLFSSL_BASE64_ENCODE
//  #define DEBUG_WOLFSSL
#define WOLFSSL_STATIC_RSA
//#define HAVE_SUPPORTED_CURVES
//#define HAVE_TLS_EXTENSIONS
//#define SIZEOF_LONG_LONG  8
/* Options for Sample program */
//#define USE_CYASSL_MEMORY
//#define NO_WOLFSSL_MEMORY
#define WOLFSSL_NO_VERIFYSERVER
#define NO_FILESYSTEM
#define NO_CERT
#define HAVE_TM_TYPE

#ifndef WOLFSSL_NO_VERIFYSERVER
    #define TIME_OVERRIDES
    #define XTIME time
    #define XGMTIME localtime
#endif

#elif defined(__senseability__)
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_DEV_RANDOM
#define NO_WOLFSSL_DIR
#define NO_SHA512
#define NO_DH
#define NO_DSA
#define NO_RSA
#define NO_RC4
#define NO_DES
#define NO_DES3
#define NO_RABBIT
#define NO_AES
#define NO_ECC256
#define NO_ECC_DHE
#define NO_HC128
#define NO_PSK
#define NO_MD2
#define NO_MD4
#define NO_MD5
#define NO_OLD_TLS
#define NO_PWDBASED
#define NO_SKID
//#define NO_CURVE25519
//#define NO_ED25519
#define NO_WOLFSSL_SERVER
//#define NO_CERTS
#define SINGLE_THREADED
#define NO_STDIO_FILESYSTEM
//#define CTYPE_USER
//#define XMALLOC_USER
//#define SHA256_DIGEST_SIZE 32
#define WOLFSSL_BASE64_ENCODE
//#define DEBUG_WOLFSSL
#define WOLFSSL_STATIC_RSA
//#define HAVE_SUPPORTED_CURVES
//#define HAVE_TLS_EXTENSIONS
//#define SIZEOF_LONG_LONG  8
/* Options for Sample program */
#define NO_SESSION_CACHE // For Small RAM
//#define USE_CYASSL_MEMORY
//#define NO_WOLFSSL_MEMORY
#define WOLFSSL_NO_VERIFYSERVER
#define NO_FILESYSTEM
#define NO_CERT
#define HAVE_TM_TYPE

#ifndef WOLFSSL_NO_VERIFYSERVER
    #define TIME_OVERRIDES
    #define XTIME time
    #define XGMTIME localtime
#endif

#elif defined(__stm32l475iot__)
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_DEV_RANDOM
#define NO_WOLFSSL_DIR
#define NO_SHA512
#define NO_DH
#define NO_DSA
//#define NO_RSA
#define NO_RC4
#define NO_DES
#define NO_DES3
#define NO_RABBIT
//#define NO_AES
#define NO_ECC256
#define NO_ECC_DHE
#define NO_HC128
#define NO_PSK
#define NO_MD2
#define NO_MD4
//#define NO_MD5
#define NO_OLD_TLS
#define NO_PWDBASED
#define NO_SKID
//#define NO_CURVE25519
//#define NO_ED25519
#define NO_WOLFSSL_SERVER
//#define NO_CERTS
#define SINGLE_THREADED
#define NO_STDIO_FILESYSTEM
#define WOLFSSL_BASE64_ENCODE
//#define DEBUG_WOLFSSL
#define WOLFSSL_STATIC_RSA
/* Options for Sample program */
#define NO_SESSION_CACHE // For Small RAM
#define WOLFSSL_LOW_MEMORY
#define TFM_TIMING_RESISTANT
#define RSA_LOW_MEM
#define NO_WOLFSSL_MEMORY
#define WOLFSSL_NO_VERIFYSERVER
#define NO_FILESYSTEM
#define NO_CERT
#define HAVE_TM_TYPE
#ifndef WOLFSSL_NO_VERIFYSERVER
    #define TIME_OVERRIDES
    #define XTIME time
    #define XGMTIME localtime
#endif

#elif defined(__semiconductor__)
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_DEV_RANDOM
#define NO_WOLFSSL_DIR
#define NO_SHA512
#define NO_DH
#define NO_DSA
//#define NO_RSA
#define NO_RC4
#define NO_DES
#define NO_DES3
#define NO_RABBIT
//#define NO_AES
#define NO_ECC256
#define HAVE_ECC
//#define NO_ECC_DHE
#define NO_HC128
#define NO_PSK
#define NO_MD2
#define NO_MD4
//#define NO_MD5
#define NO_OLD_TLS
#define NO_PWDBASED
#define NO_SKID
//#define NO_CURVE25519
//#define NO_ED25519

#define WOLFSSL_STATIC_RSA
#define NO_WOLFSSL_SERVER
//#define NO_CERTS
#define SINGLE_THREADED
#define NO_STDIO_FILESYSTEM
//#define CTYPE_USER
//#define XMALLOC_USER
//#define SHA256_DIGEST_SIZE 32
#define WOLFSSL_BASE64_ENCODE
//#define DEBUG_WOLFSSL
//#define HAVE_SUPPORTED_CURVES
//#define HAVE_TLS_EXTENSIONS
//#define SIZEOF_LONG_LONG  8
/* Options for Sample program */
#define NO_SESSION_CACHE // For Small RAM
#define WOLFSSL_LOW_MEMORY
//#define WOLFSSL_SMALL_STACK
#define TFM_TIMING_RESISTANT
#define RSA_LOW_MEM
//#define USE_CYASSL_MEMORY
#define NO_WOLFSSL_MEMORY
//#define STATIC_CHUNKS_ONLY
//#define LARGE_STATIC_BUFFERS
#define WOLFSSL_NO_VERIFYSERVER
#define NO_FILESYSTEM
#define NO_CERT
#define HAVE_TM_TYPE
#ifndef WOLFSSL_NO_VERIFYSERVER
    #define TIME_OVERRIDES
    #define XTIME time
    #define XGMTIME localtime
#endif

#elif defined(__ADUCM3029__)
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_DEV_RANDOM
#define NO_WOLFSSL_DIR
#define NO_SHA512
#define NO_DH
#define NO_DSA
//#define NO_RSA
#define NO_RC4
#define NO_DES
#define NO_DES3
#define NO_RABBIT
//#define NO_AES
#define NO_ECC256
#define NO_ECC_DHE
#define NO_HC128
#define NO_PSK
#define NO_MD2
#define NO_MD4
//#define NO_MD5
#define NO_OLD_TLS
#define NO_PWDBASED
#define NO_SKID
//#define NO_CURVE25519
//#define NO_ED25519
#define NO_WOLFSSL_SERVER
//#define NO_CERTS
#define SINGLE_THREADED
#define NO_STDIO_FILESYSTEM
#define WOLFSSL_BASE64_ENCODE
//#define DEBUG_WOLFSSL
#define WOLFSSL_STATIC_RSA
/* Options for Sample program */
#define NO_SESSION_CACHE // For Small RAM
#define WOLFSSL_LOW_MEMORY
#define TFM_TIMING_RESISTANT
#define RSA_LOW_MEM
#define NO_WOLFSSL_MEMORY
#define WOLFSSL_NO_VERIFYSERVER
#define NO_FILESYSTEM
#define NO_CERT
#define HAVE_TM_TYPE
#ifndef WOLFSSL_NO_VERIFYSERVER
    #define TIME_OVERRIDES
    #define XTIME time
    #define XGMTIME localtime
#endif

#endif
