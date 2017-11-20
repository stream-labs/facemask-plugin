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

#if !defined( __IPP_IWPP_IMAGE_COLOR__ )
#define __IPP_IWPP_IMAGE_COLOR__

#include "iw/iw_image_color.h"
#include "iw++/iw_image.hpp"

namespace ipp
{

// Converts IwiColorFmt to number of channels
// Returns:
//      If color format is of interleaved type then returns number of channels.
//      If color format is of planar type then returns number channels for the specified plane.
//      If color format is undefined or number of planes is incorrect then returns 0.
IW_DECL_CPP(int) iwiColorToChannels(
    IwiColorFmt color,          // Color format
    int         planeNum = 0    // Number of plane for planar format, from 0. Ignored for interleaved formats
)
{
    return ::iwiColorToChannels(color, planeNum);
}

// Converts IwiColorFmt to number of planes required to store the image
// Returns:
//      Number of planes in the color format
//      If color format is undefined then returns 0
IW_DECL_CPP(int) iwiColorToPlanes(
    IwiColorFmt color    // Color format
)
{
    return ::iwiColorToPlanes(color);
}

// Calculates image size required to store particular plane for a color format
// Returns:
//      If color format is of interleaved type then returns origSize.
//      If color format is of planar type then returns size for the specified plane.
//      If color format is undefined or number of planes is incorrect then returns {0,0}.
IW_DECL_CPP(IwiSize) iwiColorGetPlaneSize(
    IwiColorFmt color,    // Color format
    IwiSize     origSize, // Full plane image size
    int         planeNum  // Number of plane for planar format, from 0. Ignored for interleaved formats
)
{
    return ::iwiColorGetPlaneSize(color, origSize, planeNum);
}

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiColorConvert family
///////////////////////////////////////////////////////////////////////////// */

// Color conversion aggregator. Performs conversions between every supported color types
// C API descriptions has more details.
// Throws:
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiColorConvert(
    const IwiImage* const  &pSrcImage,                  // [in]     Reference to an array of pointers to source images
    IwiColorFmt             srcFormat,                  // [in]     Source image color format
    IwiImage* const        &pDstImage,                  // [in,out] Reference to an array of pointers to destination images
    IwiColorFmt             dstFormat,                  // [in]     Destination image color format
    Ipp64f                  alphaVal = IwValueMax,      // [in]     Value to set for alpha channel in case of X->XA conversion
    IwiRoi                 *pRoi     = NULL             // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiColorConvert((const ::IwiImage*const*)&pSrcImage, srcFormat, (::IwiImage*const*)&pDstImage, dstFormat, alphaVal, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

};

#endif
