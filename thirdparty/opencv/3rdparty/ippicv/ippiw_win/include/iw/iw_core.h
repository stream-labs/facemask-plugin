/* 
// Copyright 2016-2017 Intel Corporation All Rights Reserved.
// 
// The source code, information and material ("Material") contained herein is
// owned by Intel Corporation or its suppliers or licensors, and title
// to such Material remains with Intel Corporation or its suppliers or
// licensors. The Material contains proprietary information of Intel
// or its suppliers and licensors. The Material is protected by worldwide
// copyright laws and treaty provisions. No part of the Material may be used,
// copied, reproduced, modified, published, uploaded, posted, transmitted,
// distributed or disclosed in any way without Intel's prior express written
// permission. No license under any patent, copyright or other intellectual
// property rights in the Material is granted to or conferred upon you,
// either expressly, by implication, inducement, estoppel or otherwise.
// Any license under such intellectual property rights must be express and
// approved by Intel in writing.
// 
// Unless otherwise agreed by Intel in writing,
// you may not remove or alter this notice or any other notice embedded in
// Materials by Intel or Intel's suppliers or licensors in any way.
// 
*/

#if !defined( __IPP_IW_CORE__ )
#define __IPP_IW_CORE__

#include "stddef.h" // NULL definition

#ifdef ICV_BASE
#include "ippicv_defs_l.h"
#else
#include "ippdefs_l.h"
#endif

#include "iw_version.h"

// IW build configuration switches
#ifndef IW_ENABLE_THREADING_LAYER   // Check build scripts define
#define IW_ENABLE_THREADING_LAYER 0 // Enables Intel IPP Threading Layer calls inside IW if possible (requires OpenMP support)
                                    // Parallel version of functions will be used if:
                                    // 1. There is a parallel implementation for a particular function (see function description in the header)
                                    // 2. If iwGetThreadsNum() function result is greater than 1 before functions call or spec initialization call
                                    // Note: tiling cannot be used with internal threading. IwiRoi parameter will be ignored if conditions above are true before function call
                                    // To disable threading on run time: call iwSetThreadsNum(1) before a function call
#endif
// IW build data types selector. These switches can remove Intel IPP functions with some data types and channels from build to decrease memory footprint.
// Functions which operates with several types and channels will be enabled if at least one of parameters has enabled type
// Note that some functionality can become completely disabled if some of these defines are switched off
#ifndef IW_ENABLE_DATA_SIZE_8
#define IW_ENABLE_DATA_SIZE_8  1
#endif
#ifndef IW_ENABLE_DATA_SIZE_16
#define IW_ENABLE_DATA_SIZE_16 1
#endif
#ifndef IW_ENABLE_DATA_SIZE_32
#define IW_ENABLE_DATA_SIZE_32 1
#endif
#ifndef IW_ENABLE_DATA_SIZE_64
#define IW_ENABLE_DATA_SIZE_64 1
#endif
#ifndef IW_ENABLE_CHANNELS_1
#define IW_ENABLE_CHANNELS_1 1
#endif
#ifndef IW_ENABLE_CHANNELS_3
#define IW_ENABLE_CHANNELS_3 1
#endif
#ifndef IW_ENABLE_CHANNELS_4
#define IW_ENABLE_CHANNELS_4 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Intel IPP compatibility check
#if IPP_VERSION_COMPLEX < IW_MIN_COMPATIBLE_IPP_COMPLEX
#define IW_MACRO_TOS(A) #A
#define IW_VERSION_ERROR(MAJOR, MINOR, UPDATE) "warning: Unsupported Intel(R) IPP version. Minimal compatible version is " IW_MACRO_TOS(MAJOR)"." IW_MACRO_TOS(MINOR)"." IW_MACRO_TOS(UPDATE)
#ifdef _MSC_VER
#pragma message(IW_VERSION_ERROR(IW_MIN_COMPATIBLE_IPP_MAJOR, IW_MIN_COMPATIBLE_IPP_MINOR, IW_MIN_COMPATIBLE_IPP_UPDATE))
#else
#warning IW_VERSION_ERROR
#endif
#endif

/* /////////////////////////////////////////////////////////////////////////////
//                   Base IW definitions
///////////////////////////////////////////////////////////////////////////// */

// Common IW API declaration macro
#if defined IW_BUILD_DLL
#if defined _WIN32
#define IW_DECL(RET_TYPE) __declspec(dllexport) RET_TYPE __STDCALL
#else
#define IW_DECL(RET_TYPE) RET_TYPE __STDCALL
#endif
#else
#define IW_DECL(RET_TYPE) RET_TYPE __STDCALL
#endif

#ifdef __cplusplus
#define IW_INLINE inline
#else
#define IW_INLINE
#endif

#define IwValueMin (-IPP_MAXABS_64F)
#define IwValueMax (IPP_MAXABS_64F)

// Convert IppDataType to actual length in bytes
// Returns:
//      Size of IppDataType in bytes
IW_DECL(int) iwTypeToLen(
    IppDataType type    // Data type
);

// Returns 1 if data type is of float type and 0 otherwise
// Returns:
//      Absolute value
IW_DECL(int) iwTypeIsFloat(
    IppDataType type    // Data type
);

// Returns minimum possible value for specified data type
// Returns:
//      Minimum value
IW_DECL(double) iwTypeGetMin(
    IppDataType type    // Data type for min value
);

// Returns maximum possible value for specified data type
// Returns:
//      Maximum value
IW_DECL(double) iwTypeGetMax(
    IppDataType type    // Data type for max value
);

// Returns values range for specified data type
// Returns:
//      Range value
IW_DECL(double) iwTypeGetRange(
    IppDataType type    // Data type for range value
);

// Cast double value to input type with rounding and saturation
// Returns:
//      Rounded and saturated value
IW_DECL(double) iwValueCastToType(
    double      val,    // Input value
    IppDataType dstType // Data type for saturation range
);

// Converts relative value in range of [0,1] to the absolute value according to specified type
// Returns:
//      Absolute value
IW_DECL(double) iwValueRelToAbs(
    double      val,    // Relative value. From 0 to 1
    IppDataType type    // Data type for the absolute range
);


/* /////////////////////////////////////////////////////////////////////////////
//                   IW library-scope objects initialization
///////////////////////////////////////////////////////////////////////////// */

// Initializes global storage objects.
// Use of this function is optional. It allocates global TLS objects and Buffer Pool to optimize memory usage for
// repeating IW calls on same data size by prevention of repeated allocations/deallocations of memory for a thread.
IW_DECL(void) iwInit();

// Releases unused Buffer Pool memory.
// Call this function after pipeline execution to decrease memory consumption if your application uses the iwInit() function
IW_DECL(void) iwCleanup();

// Releases global structures for TLS and Buffer Pool.
// This function must be called at the end of library use in the user application only if iwInit() function was used.
IW_DECL(void) iwRelease();


/* /////////////////////////////////////////////////////////////////////////////
//                   IW with Threading Layer control
///////////////////////////////////////////////////////////////////////////// */

// This function sets number of threads for IW functions with parallel execution support
IW_DECL(void) iwSetThreadsNum(
    int threads     // Number of threads to use
);

// This function returns number of threads used by IW functions with parallel execution support
// Returns:
//      Number of threads or 0 if compiled without internal threading support
IW_DECL(int)  iwGetThreadsNum();

// This function returns initial number of threads used by IW functions with parallel execution support
// Returns:
//      Default number of threads or 0 if compiled without internal threading support
IW_DECL(int)  iwGetThreadsNumDefault();


/* /////////////////////////////////////////////////////////////////////////////
//                   IwVector - C Vector
///////////////////////////////////////////////////////////////////////////// */
typedef struct _IwVector
{
    Ipp8u  *m_pBuffer;
    size_t  m_bufferLen;
    size_t  m_elemSize;
    size_t  m_size;
} IwVector;

IW_DECL(void) iwVector_Init(IwVector *pVector, size_t elemSize, size_t reserve);
IW_DECL(void) iwVector_Reserve(IwVector *pVector, size_t reserveSize);
IW_DECL(void) iwVector_Release(IwVector *pVector);
IW_DECL(void) iwVector_Resize(IwVector *pVector, size_t newSize);
IW_DECL(void) iwVector_PushBack(IwVector *pVector, void *pData);
IW_DECL(void) iwVector_PopBack(IwVector *pVector, void *pData);

/* /////////////////////////////////////////////////////////////////////////////
//                   IwTls - TLS data storage interface
///////////////////////////////////////////////////////////////////////////// */
typedef void (__STDCALL *IwTlsDestructor)(void*); // Pointer to destructor function for TLS object

// TLS abstraction layer structure
// This API can help with threading of IW functions by allowing easy platform-independent per-thread data storage and initialization
typedef struct _IwTls
{
    IwTlsDestructor m_desctuctor; // Pointer to destruction function
    size_t          m_idx;        // Internal TLS index
} IwTls;

// This function initializes TLS structure, reserves TLS index and assign destruction function if necessary.
// Destruction function is required to properly deallocate user object for every thread.
// Returns:
//      ippStsErr                           internal TLS error
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwTls_Init(
    IwTls          *pTls,       // Pointer to IwTls structure
    IwTlsDestructor destructor  // Pointer to object destruction function
);

// Writes pointer to object into TLS storage for current thread
// Returns:
//      ippStsErr                           internal TLS error
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwTls_Set(
    IwTls *pTls, // Pointer to IwTls structure
    void  *pData // Pointer to user object
);

// Tries to get pointer to object from TLS storage for current thread.
// If no object has been yet created for current thread it returns NULL.
// Returns:
//      Pointer to stored data for current thread or NULL if no data was stored
IW_DECL(void*)     iwTls_Get(
    IwTls *pTls // Pointer to IwTls structure
);

// Releases TLS object and all data associated with it for all threads.
// Internal object data for different threads can be released here automatically only if IwTlsDestructor pointer was initialized
// Returns:
//      ippStsErr                           internal TLS error
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwTls_Release(
    IwTls *pTls // Pointer to IwTls structure
);

/* /////////////////////////////////////////////////////////////////////////////
//                   IW version info
///////////////////////////////////////////////////////////////////////////// */

// IW version structure
typedef struct _IwVersion
{
    const IppLibraryVersion *m_pIppsVersion;  // Pointer to ipps version structure with version of linked Intel IPP library
    int                      m_bUserBuild;    // Manual build flag. Must be false for prebuilt library and true for custom user build
} IwVersion;

// Writes version information in IwVersion structure
IW_DECL(void) iwGetLibVersion(
    IwVersion *pVersion // Pointer to IwVersion structure
);

// Global IW states debug structure.
// This structure stores info about TLS and Buffer Pool.
typedef struct _IwStateDebugInfo
{
    int    m_tlsInitialized;
    size_t m_tlsDataIndexesMax;
    size_t m_tlsThreadsMax;

    int    m_poolInitialized;
    size_t m_poolMemoryTotal;
    int    m_poolChunks;
    int    m_poolChunksLocked;
    int    m_poolAllocations;
    int    m_poolReleases;
    size_t m_poolEntries;
    size_t m_poolEntrySizes[16];
} IwStateDebugInfo;

// Fills IwStateDebugInfo structure
IW_DECL(void) iwGetDebugInfo(
    IwStateDebugInfo *pInfo     // Pointer to IwStateDebugInfo structure
);

#ifdef __cplusplus
}
#endif

#endif
