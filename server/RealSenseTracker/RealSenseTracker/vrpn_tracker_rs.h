#ifndef VRPN_TRACKER_RS_H
#define VRPN_TRACKER_RS_H

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <vector>

#include <vrpn_Text.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>
#include <vrpn_Connection.h>

#include <pxcsensemanager.h>
#include <pxcmetadata.h>
#include <pxcprojection.h>

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\opencv.hpp>

#include "render_window.h"

using namespace cv;
using namespace std;

/////////////////////// TRACKER /////////////////////////////
class myTracker : public vrpn_Tracker
{
public:
	myTracker(vrpn_Connection *c = 0);
	virtual ~myTracker() {
		terminate_window();
		projection->Release();
		pSenseManager->Close();
		pSession->Release();
	};

	virtual void mainloop();

protected:
	struct timeval _timestamp;
	PXCSession *pSession;
	PXCSenseManager *pSenseManager;
	PXCProjection *projection;
	GLFWwindow* first_window;
	GLFWwindow* second_window;
	GLFWwindow* depth_window;
	Ptr<SimpleBlobDetector> detector;
};

//struct PointVertex
//{
//	float fX;
//	float fY;
//	float fZ;
//	float fNX;
//	float fNY;
//	float fNZ;
//	float fU;
//	float fV;
//};

myTracker::myTracker(vrpn_Connection *c /*= 0 */) :
vrpn_Tracker("Tracker0", c)
{
	pSession = PXCSession::CreateInstance();
	pSenseManager = pSession->CreateSenseManager();
	pSenseManager->EnableStream(PXCCapture::STREAM_TYPE_IR, 640, 480, 30);
	pSenseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 640, 480, 30);
	pSenseManager->Init();
	//
	first_window = setup_window();
	second_window = setup_window();
	depth_window = setup_window();
	//
	SimpleBlobDetector::Params params;
	params.minDistBetweenBlobs = 5.0f;
	params.filterByInertia = false;
	params.filterByConvexity = false;
	params.filterByColor = false;
	params.filterByCircularity = false;
	params.filterByArea = true;
	params.minArea = 20.0f;
	params.maxArea = 500.0f;
	detector = SimpleBlobDetector::create(params);
	//
	PXCCapture::Device *device = pSenseManager->QueryCaptureManager()->QueryDevice();
	projection = device->CreateProjection();
}

void
myTracker::mainloop()
{
	vrpn_gettimeofday(&_timestamp, NULL);
	float c_x = 0, c_y = 0, c_z = 0;
	float d_quat_0, d_quat_1, d_quat_2, d_quat_3;
	if (pSenseManager->AcquireFrame(true) >= PXC_STATUS_NO_ERROR)
	{
		PXCCapture::Sample *sample;
		sample = pSenseManager->QuerySample();
		PXCImage *ir_image = sample->ir;
		PXCImage::ImageInfo imgInfo = ir_image->QueryInfo();
		PXCImage::ImageData data;
		ir_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &data);
		render_frame(data.planes[0], imgInfo.width, imgInfo.height, first_window);
		IplImage* irimg = cvCreateImageHeader(cvSize(imgInfo.width, imgInfo.height), 8, 4);
		cvSetData(irimg, data.planes[0], imgInfo.width * 4 * sizeof(pxcBYTE));
		Mat src = cvarrToMat(irimg);
		ir_image->ReleaseAccess(&data);
		//OpenCV tracking and finding the blobs
		blur(src, src, Size(10, 10));
		int dilation_size = 3;
		Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * dilation_size, 2 * dilation_size), Point(dilation_size, dilation_size));
		dilate(src, src, element);
		threshold(src, src, 95, 255, 3);
		Mat gray_src;
		cvtColor(src, gray_src, CV_BGRA2GRAY);
		vector<KeyPoint> keypoints;
		detector->detect(gray_src, keypoints);
		vector<PXCPoint3DF32> wcords;
		if (keypoints.size() == 3){
			float sum_x, sum_y;
			sum_x = 0;
			sum_y = 0;
			for (int i = 0; i < keypoints.size(); i++){
				float X = keypoints[i].pt.x;
				float Y = keypoints[i].pt.y;
				sum_x += X;
				sum_y += Y;
			}
			c_x = sum_x / 3;
			c_y = sum_y / 3;
			c_z = 0.0;
			d_quat_0 = 0.0;
			d_quat_1 = 0.0;
			d_quat_2 = 0.0;
			d_quat_3 = 0.0;
			PXCPoint3DF32 center_point;
			center_point.x = c_x;
			center_point.y = c_y;
			center_point.z = 0;
			wcords.push_back(center_point);
		}
		//Render the blobs
		PXCImage::ImageInfo iinfo;
		iinfo.width = imgInfo.width;
		iinfo.height = imgInfo.height;
		iinfo.format = PXCImage::PIXEL_FORMAT_RGB32;
		PXCImage::ImageData blured_data;
		blured_data.format = PXCImage::PIXEL_FORMAT_RGB32;
		blured_data.pitches[0] = data.pitches[0];
		blured_data.planes[0] = src.data;
		PXCImage *blured_image = pSession->CreateImage(&iinfo, &blured_data);
		blured_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &blured_data);
		render_frame(blured_data.planes[0], iinfo.width, iinfo.height, second_window);
		blured_image->ReleaseAccess(&blured_data);
		//Create a depth image mapped to the colored image
		PXCImage *mapped_depth_image = projection->CreateDepthImageMappedToColor(sample->depth, blured_image);
		//
		PXCImage::ImageInfo depth_info = mapped_depth_image->QueryInfo();
		PXCImage::ImageData depth_data;
		mapped_depth_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &depth_data);
		render_frame(depth_data.planes[0], depth_info.width, depth_info.height, depth_window);
		mapped_depth_image->ReleaseAccess(&depth_data);
		//Depth Image
		PXCImage *depth_image = mapped_depth_image;
		PXCImage::ImageInfo mapped_depth_info = depth_image->QueryInfo();
		PXCImage::ImageData mapped_depth_data;
		depth_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &mapped_depth_data);
		//----------------------------------------------------------------------------------
		//for (int y = 0; y < (int)mapped_depth_info.height; y++){
		//	for (int x = 0; x < (int)mapped_depth_info.width; x++){
		//		if (x == (int)c_x && y == (int)c_y){
		//			int d = y* (mapped_depth_data.pitches[0] / sizeof(short)) + x;
		//			short d_value = mapped_depth_data.planes[0][d];
		//			cout << "Found blob pixels: " << x << ", " << y << " Depth: " << d_value << endl;
		//		}
		//	}
		//}
		//----------------------------------------------------------------------

		short invalids[1];
		invalids[0] = pSenseManager->QueryCaptureManager()->QueryDevice()->QueryDepthConfidenceThreshold();
		invalids[1] = pSenseManager->QueryCaptureManager()->QueryDevice()->QueryDepthLowConfidenceValue();
		short g_depthdata[640][480];
		memset(g_depthdata, 0, sizeof(g_depthdata));
		short *dpixels = (short*)mapped_depth_data.planes[0];
		int dpitch = mapped_depth_data.pitches[0] / sizeof(short);
		for (int y = 0; y < (int)mapped_depth_info.height; y++)
		{
			for (int x = 0; x < (int)mapped_depth_info.width; x++)
			{
				short d = dpixels[y*dpitch + x];
				if (d == invalids[0] || d == invalids[1]) continue;
				g_depthdata[x][y] = d;
			}
		}

		//map camera to depth

		if (c_x != 0 && c_y != 0 && c_z != 0){
			int wsize = wcords.size();
			PXCPoint3DF32 invP3D = { -1.f, -1.f, 0.f };
			vector<PXCPoint3DF32> dcordsAbs(wsize, invP3D);
			vector<PXCPoint3DF32> wcords3D(wsize, invP3D);
			vector<PXCPointF32> pixels;
			PXCPointF32 invP = { -1.f, -1.f };
			pixels.resize(wsize, invP);

			for (int i = 0; i < wsize; i++) {
				if (wcords[i].x < 0) continue;
				dcordsAbs[i].x = wcords[i].x * mapped_depth_info.width, dcordsAbs[i].y = wcords[i].y * mapped_depth_info.height;
				dcordsAbs[i].z = 0;
			}

			projection->ProjectDepthToCamera(wsize, &dcordsAbs[0], &wcords3D[0]);
			projection->ProjectCameraToDepth(wsize, &wcords3D[0], &pixels[0]);
			//
			//for (std::vector<PXCPointF32>::iterator it = pixels.begin(); it != pixels.end(); ++it){
			//	cout << it->x << ", " << it->y << endl;
			//}

			//for (int i = 0; i < wsize; i++) {
			//	int x_value = pixels[i].x;
			//	int y_value = pixels[i].y;
			//	wcords[0].z = g_depthdata[x_value][y_value];
			//	cout << wcords[0].z << endl;
			//}

			for (int y = 0; y < (int)mapped_depth_info.height; y++)
			{
				for (int x = 0; x < (int)mapped_depth_info.width; x++)
					{
						PXCPointF32 pixel = pixels[0];
						int x_value = pixel.x;
						int y_value = pixel.y;
						wcords[0].z = g_depthdata[x_value][y_value];

					}
			}

			c_x = wcords[0].x;
			c_y = wcords[0].y;
			c_z = wcords[0].z;
		}

		//IplImage* depthimg = cvCreateImageHeader(cvSize(mapped_depth_info.width, mapped_depth_info.height), 16, 1);
		//cvSetData(depthimg, mapped_depth_data.planes[0], mapped_depth_info.width * sizeof(pxcBYTE));
		//
		//Mat depth_src = cvarrToMat(depthimg);

		

		cout << "X: " << c_x << " , Y: " << c_y << " , Z: " << c_z << endl;
		//----------------------------------------------------------------------------

		//float *depthValues = 0;
		//memcpy(depthValues, mapped_depth_data.planes[0], mapped_depth_info.height * mapped_depth_info.width);



		//vector<PXCPointF32> dcords;
		//int wsize = wcords.size();
		//PXCPoint3DF32 invP3D = { -1.f, -1.f, 0.f };
		//vector<PXCPoint3DF32> dcordsAbs(wsize, invP3D);
		//vector<PXCPoint3DF32> wcords3D(wsize, invP3D);
		//PXCPointF32 invP = { -1.f, -1.f };
		//dcords.resize(wsize, invP);
		//float dwidth = (float)mapped_depth_info.width, dheight = (float)mapped_depth_info.height;
		//float *dpixels = (float*)mapped_depth_data.planes[0];
		//int dpitch = mapped_depth_data.pitches[0] / sizeof(float);
		//for (int i = 0; i < wsize; i++) {
		//	if (wcords[i].x < 0) continue;
		//	dcordsAbs[i].x = wcords[i].x * dwidth, dcordsAbs[i].y = wcords[i].y * dheight;
		//	//dcordsAbs[i].z = (float)(((pxcI16*)(mapped_depth_data.planes[0] + (int)dcordsAbs[i].y * mapped_depth_data.pitches[0] ))[(int)dcordsAbs[i].x]);
		//	dcordsAbs[i].z = dpixels[2000];
		//}
		//projection->ProjectDepthToCamera(wsize, &dcordsAbs[0], &wcords3D[0]);
		//projection->ProjectCameraToDepth(wsize, &wcords3D[0], &dcords[0]);
		//float dwidthInv = 1.f / mapped_depth_info.width, dheightInv = 1.f / mapped_depth_info.height;
		//for (int i = 0; i < wsize; i++) {
		//	if (dcordsAbs[i].z <= 0) dcords[i].x = dcords[i].y = -1.f;
		//	else dcords[i].x *= dwidthInv; dcords[i].y *= dheightInv;
		//	cout << "Values: " << dcords[i].x << ", " << dcords[i].y << endl;
		//}

		depth_image->ReleaseAccess(&depth_data);

		//vector<PXCPointF32> dcords;
		//int csize = ccords.size();
		//PXCPointF32 invP = { -1.f, -1.f };
		//vector<PXCPointF32> ccordsAbs(csize, invP);
		//float cwidth = (float)blured_image->QueryInfo().width;
		//float cheight = (float)blured_image->QueryInfo().height;
		//for (int i = 0; i < csize; i++) {
		//	ccordsAbs[i].x = ccords[i].x * cwidth;
		//	ccordsAbs[i].y = ccords[i].y * cheight;
		//	
		//}
		//
		//dcords.resize(csize);
		//projection->MapColorToDepth(mapped_depth_image, csize, &ccordsAbs[0], &dcords[0]);
		//
		//float dwidthInv = 1.f / depth_info.width;
		//float dheightInv = 1.f / depth_info.height;

		//for (int i = 0; i < csize; i++) {
		//	
		//	if (dcords[i].x < 0) continue;
		//	dcords[i].x *= dwidthInv; 
		//	dcords[i].y *= dheightInv;
		//	
		//}
		//float d;
		//for (std::vector<PXCPointF32>::iterator it = dcords.begin(); it != dcords.end(); ++it){
		//	int index = it->y *depth_data.pitches[0] / sizeof(double) + it->x;
		//	d = depth_data.planes[0][index];
		//	
		//}
		//c_z = d;
		//depth_image->ReleaseAccess(&depth_data);
		//cout << "Position: " << c_x << ", " << c_y << ", " << c_z << endl;
		//cout << "Orientation: " << d_quat_0 << ", " << d_quat_1 << ", " << d_quat_2 << ", " << d_quat_3 << endl;
		pSenseManager->ReleaseFrame();
	}
	vrpn_Tracker::timestamp = _timestamp;
	static float angle = 0; angle += 0.001f;
	pos[0] = c_x;
	pos[1] = c_y;
	pos[2] = c_z;
	d_quat[0] = 0.0f;
	d_quat[1] = 0.0f;
	d_quat[2] = 0.0f;
	d_quat[3] = 1.0f;
	char msgbuf[1000];
	d_sensor = 0;
	int  len = vrpn_Tracker::encode_to(msgbuf);
	if (d_connection->pack_message(len, _timestamp, position_m_id, d_sender_id, msgbuf,
		vrpn_CONNECTION_LOW_LATENCY))
	{
		fprintf(stderr, "can't write message: tossing\n");
	}
	server_mainloop();
}
#endif