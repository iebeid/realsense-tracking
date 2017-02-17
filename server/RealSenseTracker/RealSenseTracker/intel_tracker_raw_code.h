//#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <vector>
//
//#include <opencv2\highgui\highgui.hpp>
//#include <opencv2\imgproc\imgproc.hpp>
//#include <opencv2\opencv.hpp>
//
//#include <pxcsensemanager.h>
//#include <pxcmetadata.h>
//#include <pxcprojection.h>
//
//#include "render_window.h"
//
//#include <vrpn_Connection.h>
//#include <vrpn_Button.h>
//#include <vrpn_Tracker.h>
//#include <vrpn_Analog.h>
//#include <vrpn_Tracker_Fastrak.h>
//
//#include <future>
//
//using namespace cv;
//using namespace std;

//int main(int argc, char **argv){
//	PXCSession *pSession = PXCSession::CreateInstance();
//	PXCSenseManager *pSenseManager = pSession->CreateSenseManager();
//	pSenseManager->EnableStream(PXCCapture::STREAM_TYPE_IR, 640, 480, 30);
//	pSenseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 640, 480, 30);
//	pSenseManager->Init();
//	//
//	GLFWwindow* first_window = setup_window();
//	GLFWwindow* second_window = setup_window();
//	GLFWwindow* depth_window = setup_window();
//	//
//	SimpleBlobDetector::Params params;
//	params.minDistBetweenBlobs = 5.0f;
//	params.filterByInertia = false;
//	params.filterByConvexity = false;
//	params.filterByColor = false;
//	params.filterByCircularity = false;
//	params.filterByArea = true;
//	params.minArea = 20.0f;
//	params.maxArea = 500.0f;
//	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);
//	//
//	PXCCapture::Device *device = pSenseManager->QueryCaptureManager()->QueryDevice();
//	PXCProjection *projection = device->CreateProjection();
//	//
//	while (pSenseManager->AcquireFrame(true) >= PXC_STATUS_NO_ERROR)
//	{
//		PXCCapture::Sample *sample;
//		sample = pSenseManager->QuerySample();
//		PXCImage *ir_image = sample->ir;
//		PXCImage::ImageInfo imgInfo = ir_image->QueryInfo();
//		PXCImage::ImageData data;
//		ir_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &data);
//		render_frame(data.planes[0], imgInfo.width, imgInfo.height, first_window);
//		IplImage* irimg = cvCreateImageHeader(cvSize(imgInfo.width, imgInfo.height), 8, 4);
//		cvSetData(irimg, data.planes[0], imgInfo.width * 4 * sizeof(pxcBYTE));
//		Mat src = cvarrToMat(irimg);
//		ir_image->ReleaseAccess(&data);
//		//OpenCV tracking
//		blur(src, src, Size(10, 10));
//		int dilation_size = 3;
//		Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * dilation_size, 2 * dilation_size), Point(dilation_size, dilation_size));
//		dilate(src, src, element);
//		threshold(src, src, 95, 255, 3);
//		Mat gray_src;
//		cvtColor(src, gray_src, CV_BGRA2GRAY);
//		vector<KeyPoint> keypoints;
//		detector->detect(gray_src, keypoints);
//		float c_x, c_y, c_z;
//		vector<PXCPointF32> ccords;
//		if (keypoints.size() == 3){
//			float sum_x, sum_y;
//			sum_x = 0;
//			sum_y = 0;
//			for (int i = 0; i < keypoints.size(); i++){
//				float X = keypoints[i].pt.x;
//				float Y = keypoints[i].pt.y;
//				sum_x += X;
//				sum_y += Y;
//				cout << X << ", " << Y << endl;
//				PXCPointF32 point;
//				point.x = X;
//				point.y = Y;
//				ccords.push_back(point);
//			}
//			c_x = sum_x / 3;
//			c_y = sum_y / 3;
//			cout << "Center point: " << c_x << ", " << c_y << endl;
//		}
//		//
//		PXCImage::ImageInfo iinfo;
//		iinfo.width = imgInfo.width;
//		iinfo.height = imgInfo.height;
//		iinfo.format = PXCImage::PIXEL_FORMAT_RGB32;
//		PXCImage::ImageData blured_data;
//		blured_data.format = PXCImage::PIXEL_FORMAT_RGB32;
//		blured_data.pitches[0] = data.pitches[0];
//		blured_data.planes[0] = src.data;
//		PXCImage *blured_image = pSession->CreateImage(&iinfo, &blured_data);
//		blured_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &blured_data);
//		render_frame(blured_data.planes[0], iinfo.width, iinfo.height, second_window);
//		blured_image->ReleaseAccess(&blured_data);
//		pSenseManager->ReleaseFrame();
//		//Depth Image
//		PXCImage *depth_image = sample->depth;
//		PXCImage::ImageInfo depth_info = depth_image->QueryInfo();
//		PXCImage::ImageData depth_data;
//		depth_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &depth_data);
//		render_frame(depth_data.planes[0], depth_info.width, depth_info.height, depth_window);
//		depth_image->ReleaseAccess(&depth_data);
//		//Projection
//		//vector<PXCPointF32> wcords;
//		//
//		//int csize = ccords.size();
//		//PXCPointF32 invP = { -1.f, -1.f };
//		//vector<PXCPointF32> ccordsAbs(csize, invP);
//		//float cwidth = (float)blured_image->QueryInfo().width, cheight = (float)blured_image->QueryInfo().height;
//		//for (int i = 0; i < csize; i++) {
//		//	ccordsAbs[i].x = ccords[i].x * cwidth, ccordsAbs[i].y = ccords[i].y * cheight;
//		//}
//		//wcords.resize(csize);
//		//projection->MapColorToDepth(depth_image, csize, &ccordsAbs[0], &wcords[0]);
//		//float dwidthInv = 1.f / depth_image->QueryInfo().width, dheightInv = 1.f / depth_image->QueryInfo().height;
//		//for (int i = 0; i < csize; i++) {
//		//	if (wcords[i].x < 0) continue;
//		//	wcords[i].x *= dwidthInv; wcords[i].y *= dheightInv;
//		//}
//	}
//	terminate_window();
//	pSenseManager->Close();
//	pSession->Release();
//	return(0);
//}