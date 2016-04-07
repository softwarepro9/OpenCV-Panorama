#define _USE_MATH_DEFINES
#include "opencv/cv.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <math.h>

const short CUBE_BACK 	= 0; // back
const short CUBE_FRONT 	= 1; // front
const short CUBE_LEFT 	= 2; // left
const short CUBE_RIGHT 	= 3; // right
const short CUBE_TOP 	= 4; // top
const short CUBE_BOTTOM = 5; // bottom

struct SphereDirection {
	float theta, phi;
};

SphereDirection CubeDir[] = {// theta	, phi
								{00		, 90}	// x plus
							, 	{180	, 90}	// x minus
							, 	{90		, 90}	// y plus
							, 	{-90	, 90}	// y minus
							, 	{0		, 0}	// z plus
							, 	{0		, 180}	// z minus
};

struct FishEyeImage {
	cv::Mat img, unfished;
	float theta, phi;
};

typedef double Mask_value_t;
typedef cv::Mat_<Mask_value_t> Mask;
void addMasked(const cv::Mat& src1, const cv::Mat& src2, const Mask& mask, cv::Mat& dst)
{
  cv::MatConstIterator_<cv::Vec3b> it1 = src1.begin<cv::Vec3b>(), it1_end = src1.end<cv::Vec3b>();
  cv::MatConstIterator_<cv::Vec3b> it2 = src2.begin<cv::Vec3b>();
  cv::MatConstIterator_<Mask_value_t> mask_it = mask.begin();
  cv::MatIterator_<cv::Vec3b> dst_it = dst.begin<cv::Vec3b>();

  for(; it1 != it1_end; ++it1, ++it2, ++mask_it, ++dst_it)
    *dst_it = (*it1) * (1.0-(*mask_it)) + (*it2) * (*mask_it);
}

int main(int argc, char** argv)
{
    std::vector<FishEyeImage> img_fisheye(4);

    img_fisheye[0].img 		= cv::imread("C:\\img\\test1_back.jpg", CV_LOAD_IMAGE_COLOR);
    img_fisheye[0].theta 	= CubeDir[CUBE_BACK].theta;
    img_fisheye[0].phi 		= CubeDir[CUBE_BACK].phi;

    img_fisheye[1].img 		= cv::imread("C:\\img\\test1_left.jpg", CV_LOAD_IMAGE_COLOR);
    img_fisheye[1].theta 	= CubeDir[CUBE_LEFT].theta;
    img_fisheye[1].phi 		= CubeDir[CUBE_LEFT].phi;

    img_fisheye[2].img 		= cv::imread("C:\\img\\test1_front.jpg", CV_LOAD_IMAGE_COLOR);
    img_fisheye[2].theta 	= CubeDir[CUBE_FRONT].theta;
    img_fisheye[2].phi 		= CubeDir[CUBE_FRONT].phi;

    img_fisheye[3].img 		= cv::imread("C:\\img\\test1_right.jpg", CV_LOAD_IMAGE_COLOR);
    img_fisheye[3].theta 	= CubeDir[CUBE_RIGHT].theta;
    img_fisheye[3].phi 		= CubeDir[CUBE_RIGHT].phi;

    const int outputHeight = 1292;
    cv::Mat img_mask = cv::imread("C:\\img\\mask.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    cv::resize(img_mask, img_mask, cv::Size(outputHeight,outputHeight));
    img_mask.convertTo(img_mask, CV_64FC1);
    cv::normalize(img_mask, img_mask, 0, 1, cv::NORM_MINMAX, -1);
    cv::Mat img_final, img_tmpunfish;

    const float lense_field_of_view = 90.0f;
    const float img_rad_correction = 20;
	float FOV = lense_field_of_view*M_PI/180;


    float sph_theta, sph_phi, sph_x, sph_y, sph_z;
	float fish_r, fish_sph_theta, fish_sph_phi;
    float img_fish_x, img_fish_y, img_fish_rad, img_fish_ctr;

    std::vector<FishEyeImage>::iterator itFish;
    int ixImg = 0;
    for(itFish=img_fisheye.begin(); itFish!=img_fisheye.end(); ++itFish) {
        if(!itFish->img.data)
        {
            std::cout <<  "Could not open or find the images" << std::endl ;
            return -1;
        }

        itFish->unfished = cv::Mat::zeros(outputHeight, outputHeight, CV_8UC3);

        img_fish_ctr = (itFish->img.rows/2);
		img_fish_rad = img_fish_ctr - img_rad_correction;

		for(int i=0; i<itFish->unfished.cols; i++) {
			for(int j=0; j<itFish->unfished.rows; j++) {
				sph_theta = 180*i/itFish->unfished.cols;
				sph_phi = 180*j/itFish->unfished.rows;

				fish_sph_theta = (sph_theta - 90)*M_PI/180;
				fish_sph_phi = (90 - sph_phi)*M_PI/180;

				// get sphere coordinates
				sph_x = cos(fish_sph_phi) * sin(fish_sph_theta);
				sph_y = cos(fish_sph_phi) * cos(fish_sph_theta);
				sph_z = sin(fish_sph_phi);

				sph_theta = atan2(sph_z,sph_x);
				sph_phi = atan2(sqrt(sph_x*sph_x+sph_z*sph_z),sph_y);
				fish_r = img_fish_rad * sph_phi / FOV;

				if(fish_r <= img_fish_rad) {
					img_fish_x = img_fish_ctr + fish_r * cos(sph_theta);
					img_fish_y = img_fish_ctr - fish_r * sin(sph_theta);

					cv::Vec3b color = itFish->img.at<cv::Vec3b>(img_fish_y, img_fish_x);
					itFish->unfished.at<cv::Vec3b>(cv::Point(i,j)) = color;

				}
			}
		}

	    if(ixImg==0) {
	    	img_final = itFish->unfished.clone();
	    } else {
	        cv::copyMakeBorder(img_final, img_final, 0, 0, 0, outputHeight/2, cv::BORDER_CONSTANT, cv::Scalar::all(0));
	        cv::copyMakeBorder(itFish->unfished, img_tmpunfish, 0, 0, outputHeight*ixImg/2, 0, cv::BORDER_CONSTANT, cv::Scalar::all(0));
	        cv::copyMakeBorder(img_mask, img_mask, 0, 0, outputHeight/2, 0, cv::BORDER_CONSTANT, cv::Scalar::all(0));
	        addMasked(img_final, img_tmpunfish, img_mask, img_final);
	    }

	    ixImg++;
    }

    imwrite("C:\\img\\stitched.jpg", img_final);

    cv::namedWindow("final", cv::WINDOW_NORMAL);
    imshow("final", img_final);

    cv::waitKey(0);

    return 0;
}
