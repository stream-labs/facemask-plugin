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

#if !defined( __IPP_IWPP_IMAGE__ )
#define __IPP_IWPP_IMAGE__

#include "iw++/iw_core.hpp"
#include "iw/iw_image.h"

namespace ipp
{

/* /////////////////////////////////////////////////////////////////////////////
//                   Image IW++ definitions
///////////////////////////////////////////////////////////////////////////// */

// Stores the width and height of a rectangle. Extends IppiSizeL structure
class IwiSize: public IppiSizeL
{
public:
    // Default constructor. Sets values to zero
    IwiSize()
    {
        Set(0, 0);
    }

    // One value template constructor. Sets size to same value. Useful for simple initialization, e.g.: size = 0
    template<typename T>
    IwiSize(
        T size // Size of square rectangle
    )
    {
        Set((IppSizeL)size, (IppSizeL)size);
    }

    // Constructor. Sets size to specified values
    IwiSize(
        IppSizeL _width, // Width of rectangle
        IppSizeL _height // Height of rectangle
    )
    {
        Set(_width, _height);
    }

    // Constructor from IppiSize structure
    IwiSize(
        IppiSize size // IppiSize structure
    )
    {
        Set(size.width, size.height);
    }

    // Constructor from IppiSizeL structure
    IwiSize(
        IppiSizeL size // IppiSizeL structure
    )
    {
        Set(size.width, size.height);
    }

    // Constructor from IppiRect structure
    IwiSize(
        IppiRect rect // IppiRect structure
    )
    {
        Set(rect.width, rect.height);
    }

    // Constructor from IppiRectL structure
    IwiSize(
        IppiRectL rect // IppiRectL structure
    )
    {
        Set(rect.width, rect.height);
    }

    // Sets size to specified values
    void Set(
        IppSizeL _width, // Width of rectangle
        IppSizeL _height // Height of rectangle
    )
    {
        width  = _width;
        height = _height;
    }

    // IwiSize to IppiSize cast operator
    inline operator IppiSize()  const { IppiSize size = {(int)width, (int)height}; return size; }
};

// Stores the geometric position of a point. Extends IppiPoint structure
class IwiPoint: public IppiPointL
{
public:
    // Default constructor. Sets values to zero
    IwiPoint()
    {
        Set(0, 0);
    }

    // One value template constructor. Sets position to same value. Useful for simple initialization, e.g.: point = 0
    template<typename T>
    IwiPoint(
        T point // Position of point
    )
    {
        Set((IppSizeL)point, (IppSizeL)point);
    }

    // Constructor. Sets position to specified values
    IwiPoint(
        IppSizeL _x, // X coordinate of point
        IppSizeL _y  // Y coordinate of point
    )
    {
        Set(_x, _y);
    }

    // Constructor from IppiPoint structure
    IwiPoint(
        IppiPoint point // IppiPoint structure
    )
    {
        Set(point.x, point.y);
    }

    // Constructor from IppiPointL structure
    IwiPoint(
        IppiPointL point // IppiPointL structure
    )
    {
        Set(point.x, point.y);
    }

    // Constructor from IppiRect structure
    IwiPoint(
        IppiRect  rect // IppiRect structure
    )
    {
        Set(rect.x, rect.y);
    }

    // Constructor from IppiRectL structure
    IwiPoint(
        IppiRectL rect // IppiRectL structure
    )
    {
        Set(rect.x, rect.y);
    }

    // Sets position to specified values
    void Set(
        IppSizeL _x, // X coordinate of point
        IppSizeL _y  // Y coordinate of point
    )
    {
        x = _x;
        y = _y;
    }

    // IwiPoint to IppiPoint cast operator
    inline operator IppiPoint()  const { IppiPoint point = {(int)x, (int)y}; return point; }
};

// Stores the geometric position and size of a rectangle. Extends IppiRect structure
class IwiRect: public IppiRectL
{
public:
    // Default constructor. Sets values to zero
    IwiRect()
    {
        Set(0, 0, 0, 0);
    }

    // One value template constructor. Sets position to zero and size to same value. Useful for simple initialization, e.g.: rect = 0
    template<typename T>
    IwiRect(
        T size // Size of rectangle
    )
    {
        Set(0, 0, (IppSizeL)size, (IppSizeL)size);
    }

    // Constructor. Sets rectangle to specified values
    IwiRect(
        IppSizeL _x,     // X coordinate of rectangle
        IppSizeL _y,     // Y coordinate of rectangle
        IppSizeL _width, // Width of rectangle
        IppSizeL _height // Height of rectangle
    )
    {
        Set(_x, _y, _width, _height);
    }

    // Constructor from IppiSize structure. Sets position to 0 and size to IppiSize value
    IwiRect(
        IppiSize size
    )
    {
        Set(0, 0, size.width, size.height);
    }

    // Constructor from IppiSizeL structure. Sets position to 0 and size to IppiSizeL value
    IwiRect(
        IppiSizeL size
    )
    {
        Set(0, 0, size.width, size.height);
    }

    // Constructor from IwiSize class. Sets position to 0 and size to IwiSize value
    IwiRect(
        IwiSize size
    )
    {
        Set(0, 0, size.width, size.height);
    }

    // Constructor from IppiRect class
    IwiRect(
        IppiRect rect
    )
    {
        Set(rect.x, rect.y, rect.width, rect.height);
    }

    // Constructor from IppiRectL class
    IwiRect(
        IppiRectL rect
    )
    {
        Set(rect.x, rect.y, rect.width, rect.height);
    }

    // Sets rectangle to specified values
    void Set(
        IppSizeL _x,     // X coordinate of rectangle
        IppSizeL _y,     // Y coordinate of rectangle
        IppSizeL _width, // Width of rectangle
        IppSizeL _height // Height of rectangle
    )
    {
        x = _x;
        y = _y;
        width  = _width;
        height = _height;
    }

    // IwiRect to IwiPoint cast operator
    inline operator IwiPoint() const { return IwiPoint(x, y); }

    // IwiRect to IwiSize cast operator
    inline operator IwiSize()  const { return IwiSize(width, height); }

    // IwiRect to IppiPoint cast operator
    inline operator IppiPoint() const { IppiPoint point = {(int)x, (int)y}; return point; }

    // IwiRect to IppiSize cast operator
    inline operator IppiSize()  const { IppiSize  size  = {(int)width, (int)height}; return size; }

    // IwiRect to IppiPointL cast operator
    inline operator IppiPointL() const { IppiPointL point = {x, y}; return point; }

    // IwiRect to IppiSizeL cast operator
    inline operator IppiSizeL()  const { IppiSizeL  size  = {width, height}; return size; }
};

// Stores extrapolation type of border and border value for constant border
class IwiBorderType: public IwValue
{
public:
    // Default constructor
    IwiBorderType()
    {
        m_borderType  = ippBorderRepl;
        m_borderFlags = 0;
    }

    // Default constructor with border type
    IwiBorderType(IppiBorderType borderType)
    {
        m_borderType  = (IppiBorderType)(borderType&0xF);
        m_borderFlags = borderType&0xFFFFFFF0;
    }

    // Constructor for borders combination
    IwiBorderType(int borderType)
    {
        m_borderType  = (IppiBorderType)(borderType&0xF);
        m_borderFlags = borderType&0xFFFFFFF0;
    }

    // Default constructor with border type and value
    IwiBorderType(IppiBorderType borderType, IwValue value):
        IwValue(value)
    {
        m_borderType  = (IppiBorderType)(borderType&0xF);
        m_borderFlags = borderType&0xFFFFFFF0;
    }

    // IwiBorderType to IppiBorderType cast operator
    inline operator IppiBorderType() const { return (IppiBorderType)(m_borderType|m_borderFlags); }

    IppiBorderType m_borderType;
    int            m_borderFlags;
};

// Stores border size data
class IwiBorderSize: public IppiBorderSize
{
public:
    // Default constructor. Sets values to zero
    IwiBorderSize()
    {
        Set(0, 0, 0, 0);
    }

    // One value template constructor. Sets border to same value. Useful for simple initialization, e.g.: border = 0
    template<typename T>
    IwiBorderSize(
        T border // Position of point
    )
    {
        Set((Ipp32u)border, (Ipp32u)border, (Ipp32u)border, (Ipp32u)border);
    }

    // Constructor. Sets border to the specified values
    IwiBorderSize(
        Ipp32u _left,   // Size of border to the left
        Ipp32u _top,    // Size of border to the top
        Ipp32u _right,  // Size of border to the right
        Ipp32u _bottom  // Size of border to the bottom
    )
    {
        Set(_left, _top, _right, _bottom);
    }

    // Constructor from IppiBorderSize structure
    IwiBorderSize(
        IppiBorderSize border // IppiBorderSize structure
    )
    {
        Set(border.borderLeft, border.borderTop, border.borderRight, border.borderBottom);
    }

    // Constructor from the image ROI
    IwiBorderSize(
        IwiSize imageSize,   // Size of the image
        IwiRect imageRoi     // Image ROI
    )
    {
        Set(imageSize, imageRoi);
    }

    // Constructor from the IppiMaskSize
    IwiBorderSize(
        IppiMaskSize mask   // Size of the mask or kernel
    )
    {
        SetByMask(mask);
    }

    // Constructor from the mask or kernel size of filter
    IwiBorderSize(
        IwiSize kernelSize   // Size of the mask or kernel
    )
    {
        SetByMask(kernelSize);
    }

    // Sets border to the specified values
    void Set(
        Ipp32u _left,   // Size of border to the left
        Ipp32u _top,    // Size of border to the top
        Ipp32u _right,  // Size of border to the right
        Ipp32u _bottom  // Size of border to the bottom
    )
    {
        borderLeft      = _left;
        borderTop       = _top;
        borderRight     = _right;
        borderBottom    = _bottom;
    }

    // Sets border from the image ROI
    void Set(
        IwiSize imageSize,   // Size of the image
        IwiRect imageRoi     // Image ROI
    )
    {
        borderLeft      = (Ipp32u)imageRoi.x;
        borderTop       = (Ipp32u)imageRoi.y;
        borderRight     = (Ipp32u)(imageSize.width  - imageRoi.x - imageRoi.width);
        borderBottom    = (Ipp32u)(imageSize.height - imageRoi.y - imageRoi.height);
    }

    // Sets border from the IppiMaskSize
    void SetByMask(
        IppiMaskSize mask   // Size of the mask or kernel
    )
    {
        *this = ::iwiSizeToBorderSize(::iwiMaskToSize(mask));
    }

    // Sets border from the mask or kernel size of filter
    void SetByMask(
        IwiSize kernelSize   // Size of the mask or kernel
    )
    {
        *this = ::iwiSizeToBorderSize(kernelSize);
    }

    // Sets border from the symmetric kernel or mask length
    void SetByMask(
        IppSizeL kernelSize   // Length of the symmetric kernel
    )
    {
        *this = ::iwiLenToBorderSize(kernelSize);
    }
};

// Convert IppiMaskSize enumerator to actual IwiSize size
// Returns:
//      Width and height of IppiMaskSize in pixels
IW_DECL_CPP(IwiSize) iwiMaskToSize(
    IppiMaskSize mask    // Kernel or mask size enumerator
)
{
    return ::iwiMaskToSize(mask);
}

// Convert kernel or mask size to border size
// Returns:
//      Border required for a filter with specified kernel size
IW_DECL_CPP(IwiBorderSize) iwiSizeToBorderSize(
    IwiSize kernelSize   // Size of kernel as from iwiMaskToSize() or arbitrary
)
{
    return ::iwiSizeToBorderSize(kernelSize);
}

// Converts symmetric kernel or mask length to border size
// Returns:
//      Border required for a filter with specified kernel length
IW_DECL_CPP(IwiBorderSize) iwiLenToBorderSize(
    IppSizeL kernelSize   // Length of symmetric kernel
)
{
    return ::iwiLenToBorderSize(kernelSize);
}

/* /////////////////////////////////////////////////////////////////////////////
//                   IwiImage - Image class
///////////////////////////////////////////////////////////////////////////// */

// IwiImage is a base class for IW image processing functions to store input and output data.
class IwiImage: public ::IwiImage
{
public:
    // Default constructor. Sets values to zero
    IwiImage()
    {
        IppStatus status = iwiImage_Init(this);
        OWN_ERROR_CHECK_THROW_ONLY(status);
    }

    // Copy constructor. Initializes image structure with external buffer
    IwiImage(
        const ::IwiImage &image         // Source image
    )
    {
        IppStatus status = iwiImage_Init(this);
        OWN_ERROR_CHECK_THROW_ONLY(status);

        status = Init(image.m_size, image.m_dataType, image.m_channels, image.m_inMemSize, image.m_ptr, image.m_step);
        OWN_ERROR_CHECK_THROW_ONLY(status);
    }

    // Constructor with initialization. Initializes image structure with external buffer
    IwiImage(
        IwiSize         size,                           // Image size, in pixels
        IppDataType     dataType,                       // Image pixel type
        int             channels,                       // Number of image channels
        IwiBorderSize   inMemBorder = IwiBorderSize(),  // Size of border around image or NULL if there is no border
        void           *pBuffer     = NULL,             // Pointer to the external buffer image buffer
        IppSizeL        step        = 0                 // Distance, in bytes, between the starting points of consecutive lines in the external buffer
    )
    {
        IppStatus status = iwiImage_Init(this);
        OWN_ERROR_CHECK_THROW_ONLY(status);

        status = Init(size, dataType, channels, inMemBorder, pBuffer, step);
        OWN_ERROR_CHECK_THROW_ONLY(status);
    }

    // Default destructor
    ~IwiImage()
    {
        Release();
    }

    // Copy operator for C++ object. Initializes image structure with external buffer
    IwiImage& operator=(const IwiImage &image)
    {
        IppStatus status = Init(image.m_size, image.m_dataType, image.m_channels, image.m_inMemSize, image.m_ptr, image.m_step);
        OWN_ERROR_CHECK_THROW_ONLY(status);
        return *this;
    }

    // Copy operator for C object. Initializes image structure with external buffer
    IwiImage& operator=(const ::IwiImage &image)
    {
        IppStatus status = Init(image.m_size, image.m_dataType, image.m_channels, image.m_inMemSize, image.m_ptr, image.m_step);
        OWN_ERROR_CHECK_THROW_ONLY(status);
        return *this;
    }

    // Initializes image structure with external buffer
    // Returns:
    //      ippStsNoErr                         no errors
    IppStatus Init(
        IwiSize         size,                           // Image size, in pixels
        IppDataType     dataType,                       // Image pixel type
        int             channels,                       // Number of image channels
        IwiBorderSize   inMemBorder = IwiBorderSize(),  // Size of border around image or NULL if there is no border
        void           *pBuffer     = NULL,             // Pointer to the external buffer image buffer
        IppSizeL        step        = 0                 // Distance, in bytes, between the starting points of consecutive lines in the external buffer
    )
    {
        IppStatus status = Release();
        OWN_ERROR_CHECK(status);

        status = iwiImage_InitExternal(this, size, dataType, channels, &inMemBorder, pBuffer, step);
        OWN_ERROR_CHECK(status);
        return ippStsNoErr;
    }

    // Initializes image structure and allocates image data
    // Throws:
    //      ippStsDataTypeErr                   data type is illegal
    //      ippStsNumChannelsErr                channels value is illegal
    // Returns:
    //      ippStsNoErr                         no errors
    IppStatus Alloc(
        IwiSize         size,                           // Image size, in pixels
        IppDataType     dataType,                       // Image pixel type
        int             channels,                       // Number of image channels
        IwiBorderSize   inMemBorder = IwiBorderSize()   // Size of border around image or NULL if there is no border
    )
    {
        IppStatus status = iwiImage_Alloc(this, size, dataType, channels, &inMemBorder);
        OWN_ERROR_CHECK(status);
        return ippStsNoErr;
    }

    // Releases image data if it was allocated by IwiImage::Alloc
    // Returns:
    //      ippStsNoErr                         no errors
    IppStatus Release()
    {
        IppStatus status = iwiImage_Release(this);
        OWN_ERROR_CHECK(status);
        return ippStsNoErr;
    }

    // Returns pointer to specified pixel position in image buffer
    // Returns:
    //      Pointer to the image data
    inline void* ptr(
        IppSizeL x = 0,         // x shift, as columns
        IppSizeL y = 0          // y shift, as rows
    )
    {
        return iwiImage_GetPtr(this, x, y);
    }

    // Applies ROI to the current image by adjusting size and starting point of the image.
    // Can be applied recursively.
    // Throws:
    //      ippStsOutOfRangeErr                 ROI is out of image
    //      ippStsSizeErr                       ROI size is illegal
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    IppStatus RoiSet(
        ipp::IwiRect roi             // Roi rectangle of the required sub-image
    )
    {
        IppStatus status = iwiImage_RoiSet(this, roi);
        OWN_ERROR_CHECK(status);
        return ippStsNoErr;
    }

    // Returns sub-image with size and starting point of the specified ROI
    // Returns:
    //      IwiImage object of sub-image
    IwiImage GetRoiImage(
        ipp::IwiRect roi             // Roi rectangle of the required sub-image
    )
    {
        return iwiImage_GetRoiImage(this, roi);
    }

    // Add border size to current inMem image border, making image size smaller. Resulted image cannot be smaller than 1x1 pixels
    // Throws:
    //      ippStsSizeErr                       ROI size is illegal
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    inline IwiImage& operator+=(const IppiBorderSize &right)
    {
        IppStatus status = iwiImage_BorderAdd(this, right);
        OWN_ERROR_CHECK_THROW_ONLY(status);
        return *this;
    }

    // Subtracts border size from current inMem image border, making image size bigger. Resulted border cannot be lesser than 0
    // Throws:
    //      ippStsOutOfRangeErr                 ROI is out of image
    //      ippStsNullPtrErr                    unexpected NULL pointer
    inline IwiImage& operator-=(const IppiBorderSize &right)
    {
        IppStatus status = iwiImage_BorderSub(this, right);
        OWN_ERROR_CHECK_THROW_ONLY(status);
        return *this;
    }

    // Set border size to current inMem image border, adjusting image size. Resulted image cannot be smaller than 1x1 pixels.
    // Throws:
    //      ippStsSizeErr                       ROI size is illegal
    //      ippStsNullPtrErr                    unexpected NULL pointer
    inline IwiImage& operator=(const IppiBorderSize &right)
    {
        IppStatus status = iwiImage_BorderSet(this, right);
        OWN_ERROR_CHECK_THROW_ONLY(status);
        return *this;
    }

    // Compares image structures and returns true if structure parameters are compatible, e.g. copy operation can be performed without reallocation
    bool operator==(const ipp::IwiImage& right)
    {
        if(this->m_dataType == right.m_dataType &&
            this->m_channels == right.m_channels &&
            this->m_size.width == right.m_size.width &&
            this->m_size.height == right.m_size.height)
            return true;
        else
            return false;
    }
    // Reversed structures compare operator
    bool operator!=(const ipp::IwiImage& right)
    {
        return !(*this==right);
    }
};

/* /////////////////////////////////////////////////////////////////////////////
//                   IW Tiling
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//                   Manual tiling control
///////////////////////////////////////////////////////////////////////////// */

// Returns border with proper ippBorderInMem flags for current tile position, image size and border size
// Returns:
//      ippBorderInMem flags
IW_DECL_CPP(IwiBorderType) iwiRoi_GetTileBorder(
    IppiBorderType  border,         // Border type
    IppiBorderSize  borderSize,     // Border size
    IppiSizeL       srcImageSize,   // Source image size
    IppiRectL       roi             // Tile position and size
)
{
    return ::iwiRoi_GetTileBorder(border, borderSize, srcImageSize, roi);
}

// Returns minimal acceptable tile size for the current border size and type
// Returns:
//      Minimal tile size
IW_DECL_CPP(IwiSize) iwiRoi_GetMinTileSize(
    IppiBorderType  border,     // Border type
    IppiBorderSize  borderSize  // Border size
)
{
    return ::iwiRoi_GetMinTileSize(border, borderSize);
}

// Function corrects ROI position and size to prevent overlapping between filtering function border and image border in
// case of border reconstruction. If image already has a right or a bottom border in memory and border type flags
// ippBorderInMemRight or ippBorderInMemBottom were specified accordingly then no correction is required.
//
// C API descriptions has more details.
// Returns:
//      1 if correction was performed and 0 otherwise
IW_DECL_CPP(int) iwiRoi_CorrectBordersOverlap(
    IppiBorderType  border,         // [in]     Border type
    IppiBorderSize  borderSize,     // [in]     Border size
    IppiSizeL       srcImageSize,   // [in]     Source image size
    IppiRectL      *pRoi            // [in,out] Tile position and size to be checked and corrected
)
{
    return ::iwiRoi_CorrectBordersOverlap(border, borderSize, srcImageSize, pRoi);
}

/* /////////////////////////////////////////////////////////////////////////////
//                   IwiRoi based basic tiling
///////////////////////////////////////////////////////////////////////////// */

// This is a wrapper class for the basic IwiRoi tiling API
class IwiRoi: public ::IwiRoi
{
public:
    // Default constructor.
    IwiRoi()
    {
        this->m_initialized = ownRoiInitNone;
    }

    // Constructor with initialization.
    IwiRoi(
        const IppiRectL &tileRoi    // [in] Tile offset and size
    )
    {
        this->m_initialized = ownRoiInitNone;
        SetTile(tileRoi);
    }

    // Basic tiling initializer for IwiRoi structure.
    // Use this method to set up single function tiling or tiling for pipelines with border-less functions.
    // For functions which operate with different sizes for source and destination images use destination size as a base
    // for tile parameters.
    void SetTile(
        const IppiRectL &tileRoi    // [in] Tile offset and size
    )
    {
        *(::IwiRoi*)this = ::iwiRoi_SetTile(tileRoi);
    }

    // Assignment operator from IppiRectL structure.
    IwiRoi& operator=(
        const IppiRectL &tileRoi    // [in] Tile offset and size
    )
    {
        SetTile(tileRoi);
        return *this;
    }
};

/* /////////////////////////////////////////////////////////////////////////////
//                   IwiRoi based pipeline tiling
///////////////////////////////////////////////////////////////////////////// */

// Important notice:
// This tiling API is created for tiling of complex pipelines with functions which use borders.
// Tiling of pipelines instead of isolated functions can increase scalability of threading or performance of
// non-threaded functions by performing all operations inside CPU cache.
//
// This is advanced tiling method, so you better know what you are doing.
// 1. Pipeline tiling operates in reverse order: from destination to source.
//    a. Use tile size based on final destination image size
//    b. Initialize IwiRoi structure with iwiRoiPipeline_Init for the last operation
//    c. Initialize IwiRoi structure for other operations from last to first with iwiRoiPipeline_InitChild
// 2. Derive border size for each operation from its mask size, kernel size or specific border size getter if any
// 3. If you have geometric transform inside pipeline, fill IwiRoiTransform structure for IwiRoi for this transform operation
// 4. In case of threading don't forget to copy initialized IwiRoi structures to local thread or initialize them on
//    per-thread basis. Access to structures is not thread safe.
// 5. Do not exceed maximum tile size specified during initialization. This can lead to buffers overflow

// This is a wrapper class for the pipeline IwiRoi tiling API
class IwiRoiPipeline: public IwiRoi
{
public:
    // Default constructor.
    IwiRoiPipeline()
    {
        this->m_initialized = ownRoiInitNone;
    }

    // Constructor with initialization for the root node.
    // Throws:
    //      ippStsBadArgErr                     incorrect arg/param of the function
    //      ippStsNullPtrErr                    unexpected NULL pointer
    IwiRoiPipeline(
        IwiSize          tileSizeMax,                   // [in] Maximum tile size for intermediate buffers size calculation
        IwiSize          dstImageSize,                  // [in] Destination image size for current operation
        IppiBorderType  *pBorderType        = NULL,     // [in] Border type for the current operation. NULL if operation doesn't have a border
        IppiBorderSize  *pBorderSize        = NULL,     // [in] Border size for the current operation. NULL if operation doesn't have a border
        IwiRoiTransform *pTransformStruct   = NULL      // [in] Initialized transform structure if operation performs geometric transformation. NULL if operation doesn't perform transformation
    )
    {
        this->m_initialized = ownRoiInitNone;
        IppStatus status = Init(tileSizeMax, dstImageSize, pBorderType, pBorderSize, pTransformStruct);
        OWN_ERROR_CHECK_THROW_ONLY(status);
    }

    // Constructor with initialization for the child node.
    // Throws:
    //      ippStsBadArgErr                     incorrect arg/param of the function
    //      ippStsNullPtrErr                    unexpected NULL pointer
    IwiRoiPipeline(
        ::IwiRoi        *pParent,                       // [in] Pointer to IwiRoi structure of previous operation
        IppiBorderType  *pBorderType        = NULL,     // [in] Border type for the current operation. NULL if operation doesn't have a border
        IppiBorderSize  *pBorderSize        = NULL,     // [in] Border size for the current operation. NULL if operation doesn't have a border
        IwiRoiTransform *pTransformStruct   = NULL      // [in] Initialized transform structure if operation performs geometric transformation. NULL if operation doesn't perform transformation
    )
    {
        this->m_initialized = ownRoiInitNone;
        IppStatus status = InitChild(pParent, pBorderType, pBorderSize, pTransformStruct);
        OWN_ERROR_CHECK_THROW_ONLY(status);
    }


    // Pipeline tiling root node initializer for IwiRoi structure.
    // This initializer should be used first and for IwiRoi structure of the final operation.
    // Throws:
    //      ippStsBadArgErr                     incorrect arg/param of the function
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    IppStatus Init(
        IwiSize          tileSizeMax,                   // [in] Maximum tile size for intermediate buffers size calculation
        IwiSize          dstImageSize,                  // [in] Destination image size for current operation
        IppiBorderType  *pBorderType        = NULL,     // [in] Border type for the current operation. NULL if operation doesn't have a border
        IppiBorderSize  *pBorderSize        = NULL,     // [in] Border size for the current operation. NULL if operation doesn't have a border
        IwiRoiTransform *pTransformStruct   = NULL      // [in] Initialized transform structure if operation performs geometric transformation. NULL if operation doesn't perform transformation
    )
    {
        IppStatus status = ::iwiRoiPipeline_Init(this, tileSizeMax, dstImageSize, pBorderType, pBorderSize, pTransformStruct);
        OWN_ERROR_CHECK(status);
        return status;
    }

    // Pipeline tiling child node initializer for IwiRoi structure.
    // This initializer should be called for any operation preceding the last operation in reverse order.
    // Throws:
    //      ippStsBadArgErr                     incorrect arg/param of the function
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    IppStatus InitChild(
        ::IwiRoi        *pParent,                       // [in] Pointer to IwiRoi structure of previous operation
        IppiBorderType  *pBorderType        = NULL,     // [in] Border type for the current operation. NULL if operation doesn't have a border
        IppiBorderSize  *pBorderSize        = NULL,     // [in] Border size for the current operation. NULL if operation doesn't have a border
        IwiRoiTransform *pTransformStruct   = NULL      // [in] Initialized transform structure if operation performs geometric transformation. NULL if operation doesn't perform transformation
    )
    {
        IppStatus status = ::iwiRoiPipeline_InitChild(this, pParent, pBorderType, pBorderSize, pTransformStruct);
        OWN_ERROR_CHECK(status);
        return status;
    }

    // Sets current tile rectangle for the pipeline to process
    // Throws:
    //      ippStsContextMatchErr               internal structure is not initialized or of invalid type
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    IppStatus SetTile(
        IwiRect          tileRoi                // [in] Tile offset and size
    )
    {
        if(!this->m_initialized)
            OWN_ERROR_THROW_ONLY(ippStsContextMatchErr);

        IppStatus status = ::iwiRoiPipeline_SetTile(this, tileRoi);
        OWN_ERROR_CHECK(status);
        return status;
    }

    // Pipeline tiling intermediate buffer size getter
    // Throws:
    //      ippStsContextMatchErr               internal structure is not initialized or of invalid type
    // Returns:
    //      Minimal required size of destination intermediate buffer
    IwiSize GetDstBufferSize()
    {
        if(!this->m_initialized)
            OWN_ERROR_THROW_ONLY(ippStsContextMatchErr);

        return ::iwiRoiPipeline_GetDstBufferSize(this);
    }

    // Calculates actual border parameter with InMem flags for the current tile absolute and relative offsets and sizes
    // Throws:
    //      ippStsContextMatchErr               internal structure is not initialized or of invalid type
    // Returns:
    //      Border type with actual InMem flags
    IwiBorderType GetTileBorder(
        IppiBorderType   border                 // [in] Extrapolation algorithm for out of image pixels
    )
    {
        if(!this->m_initialized)
            OWN_ERROR_THROW_ONLY(ippStsContextMatchErr);

        return ::iwiRoiPipeline_GetTileBorder(this, border);
    }

    // Checks for image and buffer boundaries for the source buffer and limits tile rectangle
    // Throws:
    //      ippStsContextMatchErr               internal structure is not initialized or of invalid type
    // Returns:
    //      Adjusted source tile rectangle within buffer boundaries
    IwiRect GetBoundedSrcRect()
    {
        if(!this->m_initialized)
            OWN_ERROR_THROW_ONLY(ippStsContextMatchErr);

        return ::iwiRoiPipeline_GetBoundedSrcRect(this);
    }

    // Checks for image and buffer boundaries for the destination buffer and limits tile rectangle
    // Throws:
    //      ippStsContextMatchErr               internal structure is not initialized or of invalid type
    // Returns:
    //      Adjusted destination tile rectangle within buffer boundaries
    IwiRect GetBoundedDstRect()
    {
        if(!this->m_initialized)
            OWN_ERROR_THROW_ONLY(ippStsContextMatchErr);

        return ::iwiRoiPipeline_GetBoundedDstRect(this);
    }

    // Returns minimal acceptable tile size for current pipeline
    // Throws:
    //      ippStsContextMatchErr               internal structure is not initialized or of invalid type
    //      ippStsErr                           tile calculation error
    // Returns:
    //      Minimal tile size
    IwiSize GetMinTileSize()
    {
        if(!this->m_initialized)
            OWN_ERROR_THROW_ONLY(ippStsContextMatchErr);

        IwiSize size = ::iwiRoiPipeline_GetMinTileSize(this);
        if(!size.width || !size.height)
            OWN_ERROR_THROW_ONLY(ippStsErr);
        return size;
    }
};

}

#endif
