#include <opencv2/opencv.hpp>

#include <pxcsensemanager.h>
#include <pxcmetadata.h>
#include <pxcimage.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

// http://stackoverflow.com/questions/32609341/convert-a-pxcimage-into-an-opencv-mat/32609342#32609342

void ConvertPXCImageToOpenCVMat(PXCImage *inImg, Mat *outImg) {
	int cvDataType;
	int cvDataWidth;


	PXCImage::ImageData data;
	inImg->AcquireAccess(PXCImage::ACCESS_READ, &data);
	PXCImage::ImageInfo imgInfo = inImg->QueryInfo();

	switch (data.format) {
		/* STREAM_TYPE_COLOR */
	case PXCImage::PIXEL_FORMAT_YUY2: /* YUY2 image  */
	case PXCImage::PIXEL_FORMAT_NV12: /* NV12 image */
		break;
	case PXCImage::PIXEL_FORMAT_RGB32: /* BGRA layout on a little-endian machine */
		cvDataType = CV_8UC4;
		cvDataWidth = 4;
		break;
	case PXCImage::PIXEL_FORMAT_RGB24: /* BGR layout on a little-endian machine */
		cvDataType = CV_8UC3;
		cvDataWidth = 3;
		break;
	case PXCImage::PIXEL_FORMAT_Y8:  /* 8-Bit Gray Image, or IR 8-bit */
		cvDataType = CV_8U;
		cvDataWidth = 1;
		break;

		/* STREAM_TYPE_DEPTH */
	case PXCImage::PIXEL_FORMAT_DEPTH: /* 16-bit unsigned integer with precision mm. */
	case PXCImage::PIXEL_FORMAT_DEPTH_RAW: /* 16-bit unsigned integer with device specific precision (call device->QueryDepthUnit()) */
		cvDataType = CV_16U;
		cvDataWidth = 2;
		break;
	case PXCImage::PIXEL_FORMAT_DEPTH_F32: /* 32-bit float-point with precision mm. */
		cvDataType = CV_32F;
		cvDataWidth = 4;
		break;

		/* STREAM_TYPE_IR */
	case PXCImage::PIXEL_FORMAT_Y16:          /* 16-Bit Gray Image */
		cvDataType = CV_16U;
		cvDataWidth = 2;
		break;
	case PXCImage::PIXEL_FORMAT_Y8_IR_RELATIVE:    /* Relative IR Image */
		cvDataType = CV_8U;
		cvDataWidth = 1;
		break;
	}

	//// suppose that no other planes
	//if (data.planes[1] != NULL) throw(0); // not implemented
	//// suppose that no sub pixel padding needed
	//if (data.pitches[0] % cvDataWidth != 0) throw(0); // not implemented

	outImg->create(imgInfo.height, data.pitches[0] / cvDataWidth, cvDataType);

	

	//memcpy(outImg->data, data.planes[0], imgInfo.height*imgInfo.width*cvDataWidth*sizeof(pxcBYTE));
	//memcpy(outImg->data, data.planes[0], imgInfo.height*imgInfo.width);
	//memcpy(outImg->data, data.planes[0], 8);

	inImg->ReleaseAccess(&data);
}