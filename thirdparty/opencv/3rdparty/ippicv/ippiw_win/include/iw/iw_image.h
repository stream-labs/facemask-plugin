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

#if !defined( __IPP_IW_IMAGE__ )
#define __IPP_IW_IMAGE__

#include "iw/iw_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* /////////////////////////////////////////////////////////////////////////////
//                   Image IW definitions
///////////////////////////////////////////////////////////////////////////// */

// Convert IppiMaskSize enumerator to actual IppiSizeL size
// Returns:
//      Width and height of IppiMaskSize in pixels
IW_DECL(IppiSizeL) iwiMaskToSize(
    IppiMaskSize mask    // Kernel or mask size enumerator
);

// Convert kernel or mask size to border size
// Returns:
//      Border required for a filter with specified kernel size
static IW_INLINE IppiBorderSize iwiSizeToBorderSize(
    IppiSizeL kernelSize   // Size of kernel as from iwiMaskToSize() or arbitrary
)
{
    IppiBorderSize bordSize;
    bordSize.borderLeft = bordSize.borderRight  = (Ipp32u)(kernelSize.width/2);
    bordSize.borderTop  = bordSize.borderBottom = (Ipp32u)(kernelSize.height/2);
    return bordSize;
}

// Converts symmetric kernel or mask length to border size
// Returns:
//      Border required for a filter with specified kernel length
static IW_INLINE IppiBorderSize iwiLenToBorderSize(
    IppSizeL kernelSize   // Length of symmetric kernel
)
{
    IppiBorderSize bordSize;
    bordSize.borderLeft = bordSize.borderRight  = (Ipp32u)(kernelSize/2);
    bordSize.borderTop  = bordSize.borderBottom = (Ipp32u)(kernelSize/2);
    return bordSize;
}

// Shift pointer to specific pixel coordinates
// Returns:
//      Shifted pointer
static IW_INLINE void* iwiShiftPtr(
    const void *pPtr,       // Original pointer
    IppSizeL    step,       // Image step
    int         typeSize,   // Size of image type as from iwTypeToLen()
    int         channels,   // Number of channels in image
    IppSizeL    x,          // x shift, as columns
    IppSizeL    y           // y shift, as rows
)
{
    return (((Ipp8u*)pPtr) + step*y + typeSize*channels*x);
}

/* /////////////////////////////////////////////////////////////////////////////
//                   IwiImage - Image structure
///////////////////////////////////////////////////////////////////////////// */

// IwiImage is a base structure for IW image processing functions to store input and output data.
typedef struct _IwiImage
{
    void           *m_pBuffer;      // Pointer to image buffer. This variable must be NULL for any external buffer
    void           *m_ptr;          // Pointer to start of actual image data
    IppSizeL        m_step;         // Distance, in bytes, between the starting points of consecutive lines in the source image memory
    IppiSizeL       m_size;         // Image size, in pixels
    IppDataType     m_dataType;     // Image pixel type
    int             m_typeSize;     // Size of image pixel type in bytes
    int             m_channels;     // Number of image channels
    IppiBorderSize  m_inMemSize;    // Memory border size around image data
} IwiImage;

// Resets image structure values
// Returns:
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiImage_Init(
    IwiImage *pImage
);

// Initializes image structure with external buffer
// Returns:
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiImage_InitExternal(
    IwiImage               *pImage,         // Pointer to IwiImage structure
    IppiSizeL               size,           // Image size, in pixels, without border
    IppDataType             dataType,       // Image pixel type
    int                     channels,       // Number of image channels
    IppiBorderSize const   *pInMemBorder,   // Size of border around image or NULL if there is no border
    void                   *pBuffer,        // Pointer to the external buffer image buffer
    IppSizeL                step            // Distance, in bytes, between the starting points of consecutive lines in the external buffer
);

// Initializes image structure and allocates image data
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiImage_Alloc(
    IwiImage               *pImage,         // Pointer to IwiImage structure
    IppiSizeL               size,           // Image size, in pixels, without border
    IppDataType             dataType,       // Image pixel type
    int                     channels,       // Number of image channels
    IppiBorderSize const   *pInMemBorder    // Size of border around image or NULL if there is no border
);

// Releases image data if it was allocated by iwiImage_Alloc
// Returns:
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiImage_Release(
    IwiImage   *pImage      // Pointer to IwiImage structure
);

// Returns pointer to specified pixel position in image buffer
// Returns:
//      Pointer to the image data
IW_DECL(void*)     iwiImage_GetPtr(
    const IwiImage *pImage, // Pointer to IwiImage structure
    IppSizeL        x,      // x shift, as columns
    IppSizeL        y       // y shift, as rows
);

// Add border size to current inMem image border, making image size smaller. Resulted image cannot be smaller than 1x1 pixels
// Returns:
//      ippStsSizeErr                       ROI size is illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiImage_BorderAdd(
    IwiImage       *pImage,     // Pointer to IwiImage structure
    IppiBorderSize  borderSize  // Size of border
);

// Subtracts border size from current inMem image border, making image size bigger. Resulted border cannot be lesser than 0
// Returns:
//      ippStsOutOfRangeErr                 ROI is out of image
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiImage_BorderSub(
    IwiImage       *pImage,     // Pointer to IwiImage structure
    IppiBorderSize  borderSize  // Size of border
);

// Set border size to current inMem image border, adjusting image size. Resulted image cannot be smaller than 1x1 pixels
// Returns:
//      ippStsSizeErr                       ROI size is illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiImage_BorderSet(
    IwiImage       *pImage,     // Pointer to IwiImage structure
    IppiBorderSize  borderSize  // Size of border
);

// Applies ROI to the current image by adjusting size and starting point of the image.
// Can be applied recursively.
// Returns:
//      ippStsOutOfRangeErr                 ROI is out of image
//      ippStsSizeErr                       ROI size is illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiImage_RoiSet(
    IwiImage       *pImage, // Pointer to IwiImage structure
    IppiRectL       roi     // ROI rectangle of the required sub-image
);

// Returns sub-image with size and starting point of the specified ROI.
// Can be applied recursively.
// Returns:
//      IwiImage structure of sub-image
IW_DECL(IwiImage) iwiImage_GetRoiImage(
    const IwiImage *pImage, // Pointer to IwiImage structure
    IppiRectL       roi     // ROI rectangle of the required sub-image
);

/* /////////////////////////////////////////////////////////////////////////////
//                   IW Tiling
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//                   Manual tiling control
///////////////////////////////////////////////////////////////////////////// */

// Returns border with proper ippBorderInMem flags for current tile position, image size and border size
// Returns:
//      ippBorderInMem flags
IW_DECL(IppiBorderType) iwiRoi_GetTileBorder(
    IppiBorderType  border,         // Border type
    IppiBorderSize  borderSize,     // Border size
    IppiSizeL       srcImageSize,   // Source image size
    IppiRectL       roi             // Tile position and size
);

// Returns minimal acceptable tile size for the current border size and type
// Returns:
//      Minimal tile size
IW_DECL(IppiSizeL) iwiRoi_GetMinTileSize(
    IppiBorderType  border,     // Border type
    IppiBorderSize  borderSize  // Border size
);

// Function corrects ROI position and size to prevent overlapping between filtering function border and image border in
// case of border reconstruction. If image already has a right or a bottom border in memory and border type flags
// ippBorderInMemRight or ippBorderInMemBottom were specified accordingly then no correction is required.
//
// Overlapping example:
//                      image border
//                      /
// |-------------------|
// | image {  [      ]~|~}     ~ - pixels of the tile border which overlaps the image border.
// |       {  [      ]~|~}         One pixel of the tile border is inside the image, the other is outside
// |       {  [ tile ]~|~}
// |       {  [      ]~|~}
// |-------------------|  \
//                        tile border (2px)
//
// Assumption 1: processing of some pixels can be delayed. If your program expects EXACT same result as specified
// tile parameters demand then you should not use this function.
// Assumption 2: tile size for a function is not less than the maximum border size (use function iwiRoi_GetMinTileSize)
//
// To prevent borders overlapping this function changes the tile according to following logic (keeping assumptions in mind):
// 1. If the "right" tile border overlaps "right" image border, then function decreases tile size to move
//    whole border inside the image.
//
// Corrected overlapping:
//                       image border
//                       /
// |--------------------|
// | image {  [     ]  }|
// |       {  [     ]  }|
// |       {  [ tile]  }|
// |       {  [     ]  }|
// |--------------------\
//                       tile border
//
// 2. Now we need to compensate right adjacent tile. So if the "left" tile border is located in the overlapping zone of
//    the "right" image boundary, then the function assumes that the previous step was taken and changes tile position and
//    size to process all remaining input
//
// Before compensation:                     After compensation (now missing pixels are inside tile ROI):
//                      image border                              image border
//                      /                                         /
// |--------------------|                   |--------------------|
// | image        { ~[ ]|  }                | image       {  [~ ]|  }
// |              { ~[ ]|  }                |             {  [~ ]|  }
// |              { ~[ ]|  }                |             {  [~ ]|  }
// |              { ~[ ]|  }                |             {  [~ ]|  }
// |---------------\----|                   |--------------\-----|
//                 tile border                             tile border
//  ~ - missing pixels after step 1
//
// Returns:
//      1 if correction was performed and 0 otherwise
IW_DECL(int) iwiRoi_CorrectBordersOverlap(
    IppiBorderType  border,         // [in]     Border type
    IppiBorderSize  borderSize,     // [in]     Border size
    IppiSizeL       srcImageSize,   // [in]     Source image size
    IppiRectL      *pRoi            // [in,out] Tile position and size to be checked and corrected
);

/* /////////////////////////////////////////////////////////////////////////////
//                   IwiRoi tiling structure
///////////////////////////////////////////////////////////////////////////// */
typedef enum _OwnRoiInitType
{
    ownRoiInitNone   = 0,
    ownRoiInitSimple = 0xA1A2A3,
    ownRoiInitPipe   = 0xB1B2B3
} OwnRoiInitType;

// Function pointer type for return of src ROI by dst ROI
// Returns:
//      0 if operation failed, any other if successful
typedef int (__STDCALL *IwiRoiRectTransformFunctionPtr)(
    IppiRectL  dstRoi,  // [in]     Destination ROI for transform operation
    IppiRectL *pSrcRoi, // [in,out] Output source ROI for transform operation
    void*      pParams  // [in]     Parameters of transform operation
);

// Tile geometric transform structure
// This structure contains function pointers and parameters which are necessary for tile geometric transformations inside the pipeline
typedef struct _IwiRoiTransform
{
    IwiRoiRectTransformFunctionPtr  getSrcRoiFun;   // Pointer to IwiRoiRectTransformFunctionPtr function which returns source ROI for the current destination one
    void                           *pParams;        // Pointer to user parameters for transform functions
    IppiSizeL                       srcImageSize;   // Image size before transformation

} IwiRoiTransform;

// Main structure for semi-automatic ROI operations
// This structure provides main context for tiling across IW API
// Mainly it contains values for complex pipelines tiling
typedef struct _IwiRoi
{
    IppiRectL         m_srcRoi;            // Absolute ROI for source image
    IppiRectL         m_dstRoi;            // Absolute ROI for destination image

    IppiPointL        m_srcBufferOffset;   // Source buffer relative offset
    IppiPointL        m_dstBufferOffset;   // Destination buffer relative offset

    IppiSizeL         m_srcBufferSize;     // Actual source buffer size
    IppiSizeL         m_dstBufferSize;     // Actual destination buffer size

    IppiSizeL         m_srcImageSize;      // Full source image size
    IppiSizeL         m_dstImageSize;      // Full destination image size

    IppiSizeL         m_maxTileSize;       // Maximum tile size
    IppiBorderType    m_borderType;        // Type of source image border
    IppiBorderSize    m_borderSize;        // Border required for the current ROI operation
    IppiBorderSize    m_borderSizeAcc;     // Accumulated border size for current and parent operations
    IwiRoiTransform   m_transformStruct;   // Transformation proxy functions and data structure

    int               m_initialized;       // Internal initialization states

    struct _IwiRoi   *m_pChild;            // Next ROI in the pipeline
    struct _IwiRoi   *m_pParent;           // Previous ROI in the pipeline

} IwiRoi;

/* /////////////////////////////////////////////////////////////////////////////
//                   IwiRoi based basic tiling
///////////////////////////////////////////////////////////////////////////// */

// Basic tiling initializer for IwiRoi structure.
// Use this method to set up single function tiling or tiling for pipelines with border-less functions.
// For functions which operate with different sizes for source and destination images use destination size as a base
// for tile parameters.
// Returns:
//      Valid IwiRoi structure for simple tiling
IW_DECL(IwiRoi) iwiRoi_SetTile(
    IppiRectL   tileRoi     // [in] Tile offset and size
);

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

// Pipeline tiling root node initializer for IwiRoi structure.
// This initializer should be used first and for IwiRoi structure of the final operation.
// Returns:
//      ippStsBadArgErr                     incorrect arg/param of the function
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiRoiPipeline_Init(
    IwiRoi          *pRoi,              // [in] Pointer to IwiRoi structure
    IppiSizeL        tileSizeMax,       // [in] Maximum tile size for intermediate buffers size calculation
    IppiSizeL        dstImageSize,      // [in] Destination image size for current operation
    IppiBorderType  *pBorderType,       // [in] Border type for the current operation. NULL if operation doesn't have a border
    IppiBorderSize  *pBorderSize,       // [in] Border size for the current operation. NULL if operation doesn't have a border
    IwiRoiTransform *pTransformStruct   // [in] Initialized transform structure if operation performs geometric transformation. NULL if operation doesn't perform transformation
);

// Pipeline tiling child node initializer for IwiRoi structure.
// This initializer should be called for any operation preceding the last operation in reverse order.
// Returns:
//      ippStsBadArgErr                     incorrect arg/param of the function
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiRoiPipeline_InitChild(
    IwiRoi          *pRoi,              // [in] Pointer to IwiRoi structure
    IwiRoi          *pParent,           // [in] Pointer to IwiRoi structure of previous operation
    IppiBorderType  *pBorderType,       // [in] Border type for the current operation. NULL if operation doesn't have a border
    IppiBorderSize  *pBorderSize,       // [in] Border size for the current operation. NULL if operation doesn't have a border
    IwiRoiTransform *pTransformStruct   // [in] Initialized transform structure if operation performs geometric transformation. NULL if operation doesn't perform transformation
);

// Pipeline tiling intermediate buffer size getter
// Returns:
//      Minimal required size of destination intermediate buffer
IW_DECL(IppiSizeL) iwiRoiPipeline_GetDstBufferSize(
    IwiRoi          *pRoi           // [in] Pointer to IwiRoi structure
);

// Sets current tile rectangle for the pipeline to process
// Returns:
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiRoiPipeline_SetTile(
    IwiRoi          *pRoi,          // [in] Pointer to IwiRoi structure
    IppiRectL        tileRoi        // [in] Tile offset and size
);

// Calculates actual border parameter with InMem flags for the current tile absolute and relative offsets and sizes
// Returns:
//      Border type with actual InMem flags
IW_DECL(IppiBorderType) iwiRoiPipeline_GetTileBorder(
    IwiRoi          *pRoi,          // [in] Pointer to IwiRoi structure
    IppiBorderType   border         // [in] Extrapolation algorithm for out of image pixels
);

// Checks for image and buffer boundaries for the source buffer and limits tile rectangle
// Returns:
//      Adjusted source tile rectangle within buffer boundaries
IW_DECL(IppiRectL) iwiRoiPipeline_GetBoundedSrcRect(
    IwiRoi          *pRoi           // [in] Pointer to IwiRoi structure
);

// Checks for image and buffer boundaries for the destination buffer and limits tile rectangle
// Returns:
//      Adjusted destination tile rectangle within buffer boundaries
IW_DECL(IppiRectL) iwiRoiPipeline_GetBoundedDstRect(
    IwiRoi          *pRoi           // [in] Pointer to IwiRoi structure
);

// Returns minimal acceptable tile size for current pipeline
// Returns:
//      Minimal tile size or (0,0) in case of error
IW_DECL(IppiSizeL) iwiRoiPipeline_GetMinTileSize(
    IwiRoi          *pRoi           // [in] Pointer to IwiRoi structure
);

#ifdef __cplusplus
}
#endif

#endif
