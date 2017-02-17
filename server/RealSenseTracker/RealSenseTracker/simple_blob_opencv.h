

//int main(){
//	Mat src;
//	src = imread("D:\\data\\ir-images\\ir.jpg", 1);
//	//src = imread("D:\\data\\ir-images\\ir2.jpg", 1);
//	//src = imread("D:\\data\\ir-images\\ir3.jpg", 1);
//	//src = imread("D:\\data\\ir-images\\ir4.jpg", 1);
//	char* source_window = "Source";
//	namedWindow(source_window, CV_WINDOW_AUTOSIZE);
//	imshow(source_window, src);
//	vector<vector<Point> > contours;
//	vector<Vec4i> hierarchy;
//	blur(src, src, Size(3, 3));
//	cvtColor(src, src, CV_BGR2GRAY);
//	int dilation_size = 5;
//	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2*dilation_size, 2*dilation_size), Point(dilation_size, dilation_size));
//	dilate(src, src, element);
//	threshold(src, src, 125, 255, 3);
//	Mat im_with_keypoints;
//	vector<KeyPoint> keypoints;
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
//	detector->detect(src, keypoints);
//	for (int i = 0; i<keypoints.size(); i++){
//		float X = keypoints[i].pt.x;
//		float Y = keypoints[i].pt.y;
//		cout << X << ", " << Y << endl;
//	}
//	drawKeypoints(src, keypoints, im_with_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
//	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
//	imshow("Contours", im_with_keypoints);
//	waitKey(0);
//	return(0);
//}