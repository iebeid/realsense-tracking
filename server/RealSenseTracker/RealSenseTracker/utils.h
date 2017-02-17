#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <pxcsensemanager.h>
#include <pxcmetadata.h>
#include <pxcprojection.h>

using namespace std;

void norm(PXCPoint3DF32 &v)
{
	float len = v.x*v.x + v.y*v.y + v.z*v.z;
	if (len>0) {
		len = sqrt(len);
		v.x = v.x / len; v.y = v.y / len; v.z = v.z / len;
	}
}

PXCPoint3DF32 cross(PXCPoint3DF32 &v0, PXCPoint3DF32 &v1)
{
	PXCPoint3DF32 vec = { v0.y*v1.z - v0.z*v1.y, v0.z*v1.x - v0.x*v1.z, v0.x*v1.y - v0.y*v1.x };
	return vec;
}

float dot(PXCPoint3DF32 &v0, PXCPoint3DF32 &v1)
{
	float r = 0;
	r += v0.x * v1.x;
	r += v0.y * v1.y;
	r += v0.z * v1.z;
	return r;
}

PXCImage* ColorToDepthByQueryUVMap(PXCImage *color, PXCImage *depth, PXCProjection *projection, PXCSession *pSession)
{
	PXCImage *drawDepth = 0;
	PXCImage::ImageInfo drawDepthInfo;
	memset(&drawDepthInfo, 0, sizeof(drawDepthInfo));
	drawDepthInfo.width = 640;
	drawDepthInfo.height = 480;
	drawDepthInfo.format = PXCImage::PIXEL_FORMAT_RGB32;
	drawDepth = 0;
	drawDepth = pSession->CreateImage(&drawDepthInfo);
	vector<PXCPointF32>   uvMap;
	//PXCImage::ImageInfo drawDepthInfo = drawDepth->QueryInfo();
	PXCImage::ImageData drawDepthDat;
	if (PXC_STATUS_NO_ERROR > drawDepth->AcquireAccess(PXCImage::ACCESS_WRITE, drawDepthInfo.format, &drawDepthDat))
		return 0;

	/* Retrieve the color pixels */
	PXCImage::ImageData cdata;
	pxcStatus sts = color->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &cdata);
	if (sts >= PXC_STATUS_NO_ERROR) {
		sts = projection->QueryUVMap(depth, &uvMap[0]);
		if (sts >= PXC_STATUS_NO_ERROR) {
			PXCImage::ImageInfo cinfo = color->QueryInfo();
			PXCImage::ImageInfo dinfo = depth->QueryInfo();
			for (pxcI32 y = 0; y < dinfo.height; y++) {
				pxcI32* pDrawDepthDat = (pxcI32*)(drawDepthDat.planes[0] + y * drawDepthDat.pitches[0]);
				PXCPointF32 *uvTest = &uvMap[y * dinfo.width];
				for (pxcI32 x = 0; x < dinfo.width; x++) {
					//pDrawDepthDat[x] = 0;
					if (uvTest[x].x >= 0 && uvTest[x].x < 1 && uvTest[x].y >= 0 && uvTest[x].y < 1) {
						pxcI32 tmpColor = *(pxcI32*)(cdata.planes[0] + (int)(uvTest[x].y * cinfo.height) * cdata.pitches[0] + 4 * (int)(uvTest[x].x * cinfo.width));
						pDrawDepthDat[x] = tmpColor;
					}
				}
			}
		}
		color->ReleaseAccess(&cdata);
	}
	drawDepth->ReleaseAccess(&drawDepthDat);
	return drawDepth;
}