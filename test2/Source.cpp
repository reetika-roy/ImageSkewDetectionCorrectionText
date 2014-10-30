#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void deskew(const char* filename, double angle)
{
	cv::Mat img = cv::imread(filename, 0);

	cv::bitwise_not(img, img);

	std::vector<cv::Point> points;
	cv::Mat_<uchar>::iterator it = img.begin<uchar>();
	cv::Mat_<uchar>::iterator end = img.end<uchar>();
	for (; it != end; ++it)
		if (*it)
			points.push_back(it.pos());

	cv::RotatedRect box = cv::minAreaRect(cv::Mat(points));
	cv::Mat rot_mat = cv::getRotationMatrix2D(box.center, angle, 1);
	cv::Mat rotated;
	cv::warpAffine(img, rotated, rot_mat, img.size(), cv::INTER_CUBIC);
	cv::Size box_size = box.size;
	if (box.angle < -45.)
		std::swap(box_size.width, box_size.height);
	cv::Mat cropped;
	cv::getRectSubPix(rotated, box_size, box.center, cropped);
	cv::imshow("Original", img);
	cv::imshow("Rotated", rotated);
	cv::imshow("Cropped", cropped);
	cv::waitKey(0);
}

void compute_skew(const char* filename)
{
	// Load in grayscale.
	cv::Mat src = cv::imread(filename, 0);
	cv::Size size = src.size();
	cv::bitwise_not(src, src);
	std::vector<cv::Vec4i> lines;
	cv::HoughLinesP(src, lines, 1, CV_PI / 180, 100, size.width / 2.f, 20);
	cv::Mat disp_lines(size, CV_8UC1, cv::Scalar(0, 0, 0));
	double angle = 0.;
	unsigned lines_n = lines.size();
	for (unsigned i = 0; i < lines_n; ++i)
	{
		cv::line(disp_lines, cv::Point(lines[i][0], lines[i][1]),
			cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255, 0, 0));
		angle += atan2((double)lines[i][3] - lines[i][1],
			(double)lines[i][2] - lines[i][0]);
	}
	angle /= lines_n; // mean angle, in radians.

	std::cout << "File " << filename << ": " << angle * 180 / CV_PI << std::endl;

	cv::imshow(filename, disp_lines);
	cv::waitKey(0);
	//cv::destroyWindow(filename);

	deskew(filename, (angle * 180 / CV_PI));
}


const char* img_files[] = { "i1.jpg", "i2.jpg", "i3.jpg", "i4.jpg", "i5.jpg" };

int main()
{
	unsigned files_n = sizeof(img_files) / sizeof(const char*);
	for (unsigned i = 0; i < files_n; ++i)
		compute_skew(img_files[i]);
}