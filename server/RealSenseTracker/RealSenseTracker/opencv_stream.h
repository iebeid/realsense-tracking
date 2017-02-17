

//Using OpenCV with RealSense stream data
//int main(int argc, char **argv){
//	PXCSession *pSession = PXCSession::CreateInstance();
//	PXCSenseManager *pSenseManager = pSession->CreateSenseManager();
//	pSenseManager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480, 30);
//	pSenseManager->Init();
//	GLFWwindow* first_window = setup_window();
//	GLFWwindow* second_window = setup_window();
//	while (pSenseManager->AcquireFrame(true) >= PXC_STATUS_NO_ERROR)
//	{
//		double begin_frame = getCPUTime();
//		PXCCapture::Sample *sample;
//		sample = pSenseManager->QuerySample();
//		PXCImage *ir_image = sample->color;
//		PXCImage::ImageInfo imgInfo = ir_image->QueryInfo();
//		PXCImage::ImageData data;
//		ir_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &data);
//		render_frame(data.planes[0], imgInfo.width, imgInfo.height, first_window);
//		IplImage* irimg = cvCreateImageHeader(cvSize(imgInfo.width, imgInfo.height), 8, 4);
//		cvSetData(irimg, data.planes[0], imgInfo.width*4*sizeof(pxcBYTE));
//		Mat src = cvarrToMat(irimg);
//		ir_image->ReleaseAccess(&data);
//		//OpenCV function blur applied to Intel RealSense stream
//		blur(src, src, Size(50, 50));
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
//		double end_frame = getCPUTime();
//	}
//	terminate_window();
//	pSenseManager->Close();
//	pSession->Release();
//	return(0);
//}