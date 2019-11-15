#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

bool compareContourAreasDescending ( vector<cv::Point> contour1, vector<Point> contour2) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i < j );
}

Mat houghFunction1(Mat img){
	
	//Declaration of variables
	Mat gray, edges, thresh, kernel, closed;
	vector<vector<Point>> contours; //Finding contours
	vector<Vec4i> lines;

	//Convert image to grayscale
	cvtColor(img, gray, COLOR_BGR2GRAY);

	//Fix lighting(equalizing)
	Ptr<CLAHE> clahe = createCLAHE(2, Size(8,8));
	clahe->apply(gray, gray);

	//Get edges of image
	Canny(gray, edges, 75, 150);
	
	//HoughLines to detect barcode
    HoughLinesP(edges, lines, 1, CV_PI/180, 10, 50, 250);

	//Draw on barcode
    for( size_t i = 0; i < lines.size(); i++ ){
        line( gray, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), CV_8UC1, 2, 8);
    }
	
	//Thresholding
	threshold(gray, thresh, 0, 255, THRESH_BINARY_INV);

	//Construct a closing kernel and apply it to the thresholded image
	kernel = getStructuringElement(MORPH_RECT, Size(21, 7));
	morphologyEx(thresh,  closed, MORPH_CLOSE, kernel);

	//Perform a series of erosions and dilations
	erode(closed, closed, kernel, Point(-1, -1), 4);
	dilate(closed, closed, kernel, Point(-1, -1), 4);

	//Find the contours in the thresholded image, then sort the contours by their area, keeping only the largest one
	findContours(closed, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	sort(contours.begin(), contours.end(), compareContourAreasDescending);


	////////////////////////////////THIS PART IS JUST FOR DRAWING////////////////////////////////
	vector<RotatedRect> minRect( contours.size() );
  	for( int i = 0; i < contours.size(); i++ ){ 
		minRect[i] = minAreaRect(Mat(contours[i]));   
    }

	Mat imgClone = img.clone();
	for(int i = contours.size()-1; i< contours.size(); i++){  //iterate through each contour.
	   //rotated rectangle
       Point2f rect_points[4]; 
	   minRect[i].points(rect_points);
       for( int j = 0; j < 4; j++ ){
          line(imgClone, rect_points[j], rect_points[(j+1)%4], (0,255,255), 2, 8);
	   }
    }
	///////////////////////////////////////////////////////////////////////////////////////////


	//Crop Image according to the largest area of contour
	Mat imageCropped = img(boundingRect(contours[contours.size()-1])).clone();
	
	/* IF YOU WANT TO SEE AREA WHERE BARCODE IS
	imshow("General barcode placement", imgClone);
	waitKey(0);
	*/

	return imageCropped;
}

Mat houghFunction2(Mat imageCropped){

	Mat gray, edges, closed, kernel, thresh;
	Mat imgClone = imageCropped.clone();	
	vector<Vec4i> lines;								//vector of points frim Hough
	vector<vector<Point>> contours; 					//Finding contours

	//To grayscale
	cvtColor(imageCropped, gray, COLOR_BGR2GRAY);

	//Detect edges
	Canny(gray, edges, 75, 150);

	//HoughLines to detect barcode
    HoughLinesP( edges, lines, 1, CV_PI/180, 10, 0, 1);

	//Draw on barcode
    for( size_t i = 0; i < lines.size(); i++ ){
        line( gray, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), CV_8UC1, 2, 8);
    }	

	//Thresholding
	threshold(gray, thresh, 0, 255, THRESH_BINARY_INV);

	//Construct a closing kernel and apply it to the thresholded image
	kernel = getStructuringElement(MORPH_RECT, Size(21, 7));
	morphologyEx(thresh,  closed, MORPH_CLOSE, kernel);

	//Perform a series of erosions and dilations
	erode(closed, closed, kernel, Point(-1, -1), 4);
	dilate(closed, closed, kernel, Point(-1, -1), 4);

	findContours(closed, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	sort(contours.begin(), contours.end(), compareContourAreasDescending);


	////////////////////////////////THIS PART IS JUST FOR DRAWING////////////////////////////////
	vector<RotatedRect> minRect(contours.size());
  	for( int i = 0; i < contours.size(); i++ ){ 
		minRect[i] = minAreaRect(Mat(contours[i]));   
    }
	
	for(int i = contours.size()-1; i< contours.size(); i++) //iterate through each contour.
    {
	   //rotated rectangle
       Point2f rect_points[4]; 
	   minRect[i].points(rect_points);
       for( int j = 0; j < 4; j++ ){
          line(imgClone, rect_points[j], rect_points[(j+1)%4], (0,255,255), 1, 8);
	   }
    }
	///////////////////////////////////////////////////////////////////////////////////////////


	//Crop Image according to the largest area of contour
	imageCropped = imageCropped(boundingRect(contours[contours.size()-1]));
	
	/*//WHERE BARCODE IS EXTRACTED
	imshow("Barcode placement", imgClone);
	waitKey(0);
	*/
	
	return imageCropped;
}



int main(int argc, char** argv){

	//Loading of image and declaration of variables
	stringstream imagetoread;
	imagetoread << "./Master_image_2019/" << argv[1] << ".jpg";
	Mat img = imread(imagetoread.str());

	//If image is too big
	if(img.cols > 1000 || img.rows>1000){
		resize(img, img, Size(600, 450));
	}

	Mat imageCropped = houghFunction1(img);
	Mat ROI = houghFunction2(imageCropped);

	//imshow("Original", img);
	imshow("Barcode", ROI);
	waitKey(0);

}