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

#if !defined( __IPP_IWPP_IMAGE_OP__ )
#define __IPP_IWPP_IMAGE_OP__

#include "iw/iw_image_op.h"
#include "iw++/iw_image.hpp"

namespace ipp
{

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiCopy family
///////////////////////////////////////////////////////////////////////////// */

// Copies image data to destination image with different number of channels.
// If number of channels is the same, then calls iwiCopy function.
// C API descriptions has more details.
// Throws:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiCopyMixed(
    const IwiImage *pSrcImage,              // [in]     Pointer to the source image
    IwiImage       *pDstImage,              // [in,out] Pointer to the destination image
    int             channelsShift,          // [in]     Number of channels to shift source or destination. E.g.: C4+channelsShift->C3 or C3->C4+channelsShift
    IwiRoi         *pRoi          = NULL    // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiCopyMixed(pSrcImage, pDstImage, channelsShift, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Copies image data to destination image with masking.
// If mask is NULL, then calls iwiCopy function.
// For masked operation, the function writes pixel values in the destination buffer only if the spatially corresponding
// mask array value is non-zero.
// C API descriptions has more details.
// Throws:
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiCopyMask(
    const IwiImage *pSrcImage,          // [in]     Pointer to the source image
    IwiImage       *pDstImage,          // [in,out] Pointer to the destination image
    const IwiImage *pMaskImage = NULL,  // [in]     Pointer to mask image. Mask must be 8-bit, 1 channel image
    IwiRoi         *pRoi       = NULL   // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiCopyMask(pSrcImage, pDstImage, pMaskImage, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Copies selected channel from one multi-channel image to another.
// C API descriptions has more details.
// Throws:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiCopyChannel(
    const IwiImage *pSrcImage,         // [in]     Pointer to the source image
    int             srcChannel,        // [in]     Source channel to copy from
    IwiImage       *pDstImage,         // [in,out] Pointer to the destination image
    int             dstChannel,        // [in]     Destination channel to copy to
    IwiRoi         *pRoi        = NULL // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiCopyChannel(pSrcImage, srcChannel, pDstImage, dstChannel, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Splits multi-channel image into array of single channel images.
// C API descriptions has more details.
// Throws:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiCopySplit(
    const IwiImage *pSrcImage,          // [in]     Pointer to the source image
    IwiImage* const pDstImage[],        // [in,out] Array of pointers to destination images
    IwiRoi         *pRoi        = NULL  // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiCopySplit(pSrcImage, (::IwiImage**)pDstImage, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Merges array of single channel images into one multi-channel image.
// C API descriptions has more details.
// Throws:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiCopyMerge(
    const IwiImage* const pSrcImage[],          // [in]     Array of pointers to source images
    IwiImage             *pDstImage,            // [in,out] Pointer to the destination image
    IwiRoi               *pRoi        = NULL    // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiCopyMerge((const ::IwiImage**)pSrcImage, pDstImage, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Copies image data to destination image and constructs border of specified size.
// Destination image must have enough memory for a border according to inMemSize member.
// C API descriptions has more details.
// Throws:
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsSizeErr                       1) size fields values are illegal
//                                          2) border.borderTop or border.borderLeft are greater than corresponding inMemSize values of destination image
//                                          3) dst_width  + dst_inMemSize.borderRight  < min_width  + border.borderRight
//                                          4) dst_height + dst_inMemSize.borderBottom < min_height + border.borderBottom
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiCopyMakeBorder(
    const IwiImage *pSrcImage,          // [in]     Pointer to the source image
    IwiImage       *pDstImage,          // [in,out] Pointer to the destination image actual data start
    IwiBorderSize   borderSize,         // [in]     Size of border to reconstruct
    IwiBorderType   border,             // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderWrap
    IwiRoi         *pRoi        = NULL  // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiCopyMakeBorder(pSrcImage, pDstImage, borderSize, border, border, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiSet family
///////////////////////////////////////////////////////////////////////////// */

// Sets each channel of the image to the specified values with masking.
// If mask is NULL, then calls iwiSet function.
// For masked operation, the function writes pixel values in the destination buffer only if the spatially corresponding
// mask array value is non-zero.
// C API descriptions has more details.
// Throws:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiSetMask(
    IwValue         value,              // [in]     Values to set to
    IwiImage       *pDstImage,          // [in,out] Pointer to the destination image
    const IwiImage *pMaskImage = NULL,  // [in]     Pointer to mask image. Mask must be 8-bit, 1 channel image
    IwiRoi         *pRoi       = NULL   // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    if(value.IsUniform())
    {
        IppStatus ippStatus = ::iwiSetUniformMask(value, pDstImage, pMaskImage, pRoi);
        OWN_ERROR_CHECK(ippStatus)
        return ippStatus;
    }
    else
    {
        IppStatus ippStatus = ::iwiSetMask(value, pDstImage, pMaskImage, pRoi);
        OWN_ERROR_CHECK(ippStatus)
        return ippStatus;
    }
}

// Sets selected channel of the multi-channel image to the specified value.
// C API descriptions has more details.
// Throws:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiSetChannel(
    double          value,            // [in]     Value to set to
    IwiImage       *pDstImage,        // [in,out] Pointer to the destination image
    int             channelNum,       // [in]     Number of channel to be set
    IwiRoi         *pRoi       = NULL // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiSetChannel(value, pDstImage, channelNum, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiSwapChannels
///////////////////////////////////////////////////////////////////////////// */

// Swaps image channels according to the destination order parameter.
// C API descriptions has more details.
// Throws:
//      ippStsChannelOrderErr               destination order is out of the range
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiSwapChannels(
    const IwiImage *pSrcImage,          // [in]     Pointer to the source image
    IwiImage       *pDstImage,          // [in,out] Pointer to the destination image
    const int      *pDstOrder,          // [in]     Pointer to the destination image channels order: dst[channel] = src[dstOrder[channel]]
    double          value      = 0,     // [in]     Value to fill the destination channel if number of destination channels is bigger than number of source channels
    IwiRoi         *pRoi       = NULL   // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiSwapChannels(pSrcImage, pDstImage, pDstOrder, value, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiScale
///////////////////////////////////////////////////////////////////////////// */

// Converts image from one data type to another with specified scaling and shifting
// DST = saturate(SRC*mulVal + addVal)
// C API descriptions has more details.
// Throws:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer. If data types are different
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiScale(
    const IwiImage     *pSrcImage,                  // [in]     Pointer to the source image
    IwiImage           *pDstImage,                  // [in,out] Pointer to the destination image
    Ipp64f              mulVal,                     // [in]     Multiplier
    Ipp64f              addVal,                     // [in]     Addend
    IppHintAlgorithm    mode    = ippAlgHintNone,   // [in]     Accuracy mode
    IwiRoi             *pRoi    = NULL              // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiScale(pSrcImage, pDstImage, mulVal, addVal, mode, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Returns multiplication and addend values for iwiScale function to perform accurate data range scaling between two data types
// Data range for float values is considered to be from 0 to 1
// Throws:
//      ippStsDataTypeErr                   data type is illegal
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiScale_GetScaleVals(
    IppDataType srcType,    // [in]     Source data type
    IppDataType dstType,    // [in]     Destination data type
    Ipp64f     &mulVal,     // [out]    Pointer to multiplier
    Ipp64f     &addVal      // [out]    Pointer to addend
)
{
    IppStatus ippStatus = ::iwiScale_GetScaleVals(srcType, dstType, &mulVal, &addVal);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

};

#endif
