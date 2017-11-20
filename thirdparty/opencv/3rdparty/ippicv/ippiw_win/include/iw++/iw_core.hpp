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

#if !defined( __IPP_IWPP_CORE__ )
#define __IPP_IWPP_CORE__

#include "iw/iw_core.h"


// IW++ interface configuration switches
#define IW_ENABLE_EXCEPTIONS 1 // IW++ can return all errors by exceptions or by classic return codes
#define IW_ENABLE_AUTO_INIT  0 // Automatically initialize global library objects


#if IW_ENABLE_EXCEPTIONS == 0
#define OWN_ERROR_THROW(IPP_STATUS)      return (IPP_STATUS);
#define OWN_ERROR_THROW_ONLY(IPP_STATUS) (void)(IPP_STATUS);
#else
#define OWN_ERROR_THROW(IPP_STATUS)      throw IwException(IPP_STATUS);
#define OWN_ERROR_THROW_ONLY(IPP_STATUS) OWN_ERROR_THROW(IPP_STATUS);
#endif
#define OWN_ERROR_CHECK_THROW_ONLY(IPP_STATUS)\
{\
    if((IPP_STATUS) < 0)\
    {\
        OWN_ERROR_THROW_ONLY(IPP_STATUS)\
    }\
}
#define OWN_ERROR_CHECK(IPP_STATUS)\
{\
    if((IPP_STATUS) < 0)\
    {\
        OWN_ERROR_THROW(IPP_STATUS)\
    }\
}

// Common IW++ API declaration macro
#define IW_DECL_CPP(RET_TYPE) IW_INLINE RET_TYPE __STDCALL

namespace ipp
{

/* /////////////////////////////////////////////////////////////////////////////
//                   OwnAutoInit - automatic global initializer
///////////////////////////////////////////////////////////////////////////// */
#if IW_ENABLE_AUTO_INIT
class OwnAutoInit
{
public:
    OwnAutoInit()
    {
        ::iwInit();
    }
    ~OwnAutoInit()
    {
        ::iwRelease();
    }
};
static OwnAutoInit __auto_init__;
#endif

/* /////////////////////////////////////////////////////////////////////////////
//                   Base IW++ definitions
///////////////////////////////////////////////////////////////////////////// */

#if IW_ENABLE_EXCEPTIONS
// Stores an error code value for an exception thrown by the function
class IwException
{
public:
    // Constructor with status assignment
    IwException(
        IppStatus status    // IppStatus value
    )
    {
        m_status = status;
    }

    // Default destructor
    ~IwException() {}

    // IwException to IppStatus cast operator
    inline operator IppStatus() const { return m_status;}

    IppStatus m_status;   // Stored IppStatus value
};
#endif


// Stores values for an array for array type casting
class IwValue
{
public:
    // Default constructor. Sets array to zero
    IwValue()
    {
        m_uniform = true;
        m_val[0] = m_val[1] = m_val[2] = m_val[3] = 0;
    }

    // Uniform template-based constructor. Sets channels to one value
    template<typename T>
    IwValue(T valUniform)
    {
        m_uniform = true;
        SetValue<T>(valUniform);
    }

    // Per-channel template-based constructor. Sets channels to individual values
    template<typename T>
    IwValue(T valC1, T valC2, T valC3, T valC4 = 0)
    {
        m_uniform = false;
        SetValue<T>(valC1, valC2, valC3, valC4);
    }

    // Buffer template-based constructor. Sets values from a buffer of specific type
    template<typename T>
    IwValue(T *pBuffer, unsigned int channels)
    {
        SetValue(pBuffer, channels);
    }

    // Buffer parameter-based constructor. Sets values from a buffer of specific type
    IwValue(void *pBuffer, IppDataType type, unsigned int channels)
    {
        SetValue(pBuffer, type, channels);
    }

    // Uniform template setter. Sets channels to one value
    template<typename T>
    void SetValue(T valUniform)
    {
        m_uniform = true;
        m_val[0] = m_val[1] = m_val[2] = m_val[3] = (Ipp64f)valUniform;
    }

    // Per-channel template setter. Sets channels to individual values
    template<typename T>
    void SetValue(T valC1, T valC2, T valC3, T valC4 = 0)
    {
        m_uniform = false;
        m_val[0] = (Ipp64f)valC1, m_val[1] = (Ipp64f)valC2, m_val[2] = (Ipp64f)valC3, m_val[3] = (Ipp64f)valC4;
    }

    // Buffer template-based setter. Sets values from a buffer of specific type
    template<typename T>
    void SetValue(T *pBuffer, unsigned int channels)
    {
        if(channels == 1)
        {
            m_uniform = true;
            m_val[0] = m_val[1] = m_val[2] = m_val[3] = (Ipp64f)(pBuffer[0]);
        }
        else
        {
            m_uniform = false;
            if(channels > 4)
                channels = 4;

            for(unsigned int i = 0; i < channels; i++)
                m_val[i] = (Ipp64f)(pBuffer[i]);
        }
    }

    // Buffer parameter-based setter. Sets values from a buffer of specific type
    void SetValue(void *pBuffer, IppDataType type, unsigned int channels)
    {
        if(!channels)
            OWN_ERROR_THROW_ONLY(ippStsDataTypeErr);

        switch(type)
        {
            case ipp8u:
                SetValue((Ipp8u*)pBuffer, channels);  break;
            case ipp8s:
                SetValue((Ipp8s*)pBuffer, channels);  break;
            case ipp16u:
                SetValue((Ipp16u*)pBuffer, channels); break;
            case ipp16s:
                SetValue((Ipp16s*)pBuffer, channels); break;
            case ipp32u:
                SetValue((Ipp32u*)pBuffer, channels); break;
            case ipp32s:
                SetValue((Ipp32s*)pBuffer, channels); break;
            case ipp32f:
                SetValue((Ipp32f*)pBuffer, channels); break;
            case ipp64u:
                SetValue((Ipp64u*)pBuffer, channels); break;
            case ipp64s:
                SetValue((Ipp64s*)pBuffer, channels); break;
            case ipp64f:
                SetValue((Ipp64f*)pBuffer, channels); break;
            default:
                OWN_ERROR_THROW_ONLY(ippStsDataTypeErr);
        }
    }

    // Returns uniform flag. True means that all values were initialized by uniform constructor or setter
    bool IsUniform() const { return m_uniform; }

    // IwValue to Ipp64f cast operator
    inline operator       Ipp64f  () const { return ((Ipp64f*)m_val)[0];}

    // IwValue to Ipp64f* cast operator
    inline operator       Ipp64f* () const { return (Ipp64f*)m_val;}

    // IwValue to const Ipp64f* cast operator
    inline operator const Ipp64f* () const { return (const Ipp64f*)m_val;}

private:
    bool   m_uniform; // Uniform flag. True means that all values were initialized by uniform constructor or setter
    Ipp64f m_val[4];  // reserve 8-bit per 4 channels
};

// Convert IppDataType to actual length in bytes
// Returns:
//      Size of IppDataType in bytes
IW_DECL_CPP(int) iwTypeToLen(
    IppDataType type    // Data type
)
{
    return ::iwTypeToLen(type);
}

// Returns 1 if data type is of float type and 0 otherwise
// Returns:
//      Absolute value
IW_DECL_CPP(int) iwTypeIsFloat(
    IppDataType type    // Data type
)
{
    return ::iwTypeIsFloat(type);
}

// Returns minimum possible value for specified data type
// Returns:
//      Minimum value
IW_DECL_CPP(double) iwTypeGetMin(
    IppDataType type    // Data type for min value
)
{
    return ::iwTypeGetMin(type);
}

// Returns maximum possible value for specified data type
// Returns:
//      Maximum value
IW_DECL_CPP(double) iwTypeGetMax(
    IppDataType type    // Data type for max value
)
{
    return ::iwTypeGetMax(type);
}

// Returns values range for specified data type
// Returns:
//      Range value
IW_DECL_CPP(double) iwTypeGetRange(
    IppDataType type    // Data type for range value
)
{
    return ::iwTypeGetRange(type);
}

// Cast double value to input type with rounding and saturation
// Returns:
//      Rounded and saturated value
IW_DECL_CPP(double) iwValueCastToType(
    double      val,    // Input value
    IppDataType dstType // Data type for saturation range
)
{
    return ::iwValueCastToType(val, dstType);
}

// Converts relative value in range of [0,1] to the absolute value according to specified type
// Returns:
//      Absolute value
IW_DECL_CPP(double) iwValueRelToAbs(
    double      val,    // Relative value. From 0 to 1
    IppDataType type    // Data type for the absolute range
)
{
    return ::iwValueRelToAbs(val, type);
}

/* /////////////////////////////////////////////////////////////////////////////
//                   IW library-scope objects initialization
///////////////////////////////////////////////////////////////////////////// */

// Initializes global storage objects.
// Use of this function is optional. It allocates global TLS objects and Buffer Pool to optimize memory usage for
// repeating IW calls on same data size by prevention of repeated allocations/deallocations of memory for a thread.
IW_DECL_CPP(void) iwInit()
{
    ::iwInit();
}

// Releases unused Buffer Pool memory.
// Call this function after pipeline execution to decrease memory consumption if your application uses the iwInit() function
IW_DECL_CPP(void) iwCleanup()
{
    ::iwCleanup();
}

// Releases global structures for TLS and Buffer Pool.
// This function must be called at the end of library use in the user application only if iwInit() function was used.
IW_DECL_CPP(void) iwRelease()
{
    ::iwRelease();
}

// This class sets Intel IPP optimizations for the current region and restores previous optimizations at the region end
class IwSetCpuFeaturesRegion
{
public:
    IwSetCpuFeaturesRegion()
    {
        m_stored = ippGetEnabledCpuFeatures();
    }
    IwSetCpuFeaturesRegion(Ipp64u featuresMask)
    {
        m_stored = ippGetEnabledCpuFeatures();
        Set(featuresMask);
    }
    IppStatus Set(Ipp64u featuresMask)
    {
        return ippSetCpuFeatures(featuresMask);
    }

    ~IwSetCpuFeaturesRegion()
    {
        ippSetCpuFeatures(m_stored);
    }

private:
    Ipp64u m_stored;
};

/* /////////////////////////////////////////////////////////////////////////////
//                   IW with Threading Layer control
///////////////////////////////////////////////////////////////////////////// */

// This function sets number of threads for IW functions with parallel execution support
IW_DECL_CPP(void) iwSetThreadsNum(
    int threads     // Number of threads to use
)
{
    ::iwSetThreadsNum(threads);
}

// This function returns number of threads used by IW functions with parallel execution support
// Returns:
//      Number of threads or 0 if compiled without internal threading support
IW_DECL_CPP(int)  iwGetThreadsNum()
{
    return ::iwGetThreadsNum();
}

// This function returns initial number of threads used by IW functions with parallel execution support
// Returns:
//      Default number of threads or 0 if compiled without internal threading support
IW_DECL_CPP(int)  iwGetThreadsNumDefault()
{
    return ::iwGetThreadsNumDefault();
}


/* /////////////////////////////////////////////////////////////////////////////
//                   IwAutoBuffer - auto buffer object
///////////////////////////////////////////////////////////////////////////// */
template<typename T>
class IwAutoBuffer
{
public:
    IwAutoBuffer()                                        { m_pBuffer = NULL; m_size = 0; }
    IwAutoBuffer(size_t size, size_t elSize = sizeof(T))  { m_pBuffer = NULL; Alloc(size, elSize); }
    ~IwAutoBuffer()                                       { Release(); }
    inline T* Alloc(size_t size, size_t elSize = sizeof(T))
    {
        Release();
        m_size = size;
        m_pBuffer = (T*)ippMalloc_L((size*elSize));
        return m_pBuffer;
    }
    inline void Release()             { if(m_pBuffer) ippFree(m_pBuffer); m_pBuffer = NULL; m_size = 0; }
    inline size_t GetSize()           { return m_size; }
    inline operator T* ()             { return (T*)m_pBuffer;}
    inline operator const T* () const { return (const T*)m_pBuffer;}
private:
    // Disable copy operations
    IwAutoBuffer(IwAutoBuffer &) {};
    IwAutoBuffer& operator =(const IwAutoBuffer &) {return *this;};

    size_t m_size;
    T*     m_pBuffer;
};


/* /////////////////////////////////////////////////////////////////////////////
//                   IwTls - TLS data storage interface
///////////////////////////////////////////////////////////////////////////// */

// Template-based TLS abstraction layer class.
// This is an extension of C IwTls structure with automatic objects destruction
template<class TYPE>
class IwTls: private ::IwTls
{
public:
    // Default constructor
    IwTls()
    {
        IppStatus status = ::iwTls_Init(this, (IwTlsDestructor)(IwTls::TypeDestructor));
        OWN_ERROR_CHECK_THROW_ONLY(status);
    }

    // Default destructor
    ~IwTls()
    {
        ::iwTls_Release(this);
        // Non-throwing member
    }

    // Allocates object for current thread and returns pointer to it
    TYPE* Create()
    {
        TYPE *pData = new TYPE;
        if(!pData)
            return NULL;
        IppStatus status = ::iwTls_Set(this, pData);
        if(status < 0)
        {
            delete pData;
            OWN_ERROR_CHECK_THROW_ONLY(status);
            return NULL;
        }
        return pData;
    }

    // Releases object for current thread
    void Release()
    {
        IppStatus status = ::iwTls_Set(this, NULL);
        if(status < 0)
            OWN_ERROR_CHECK_THROW_ONLY(status);
    }

    // Returns pointer to object for current thread
    TYPE* Get()
    {
        return (TYPE*)::iwTls_Get(this);
    }

private:
    // Object destructor
    static void __STDCALL TypeDestructor(void *pData)
    {
        if(pData)
            delete ((TYPE*)pData);
    }
};

};

#endif
