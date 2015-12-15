/** @file HYP.hpp
  * @brief functions declarations that manipulates an image for you hold your past.
  */

#ifndef _HYP_HPP_
#define _HYP_HPP_

// std includes
#include <sstream>
#include <algorithm>

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// DEBUG
#include <debug.hpp>

// thanks to prof. Bogdan T. Nassu - nassubt-ufpr@yahoo.com.br
#include <line2d.h>

/** @namespace Color
  * Types of color, for indexing in cv::Mat
  */
namespace Color{
	typedef enum{ B, G, R } BGR;
	typedef enum{ H, S, V } HSV;
};

//! Simple class that represents a quadrilateral.
class Quadrilateral {
protected:
	// always four, right? :)
	#define QUADRILATERAL_SIZE 4
	cv::Point point[QUADRILATERAL_SIZE];
public:
	cv::Point& operator[]( size_t index ) {
		return point[index];
	}

	float area() {
		return abs	(
					( point[0].x*point[1].y - point[0].y*point[1].x +
					  point[1].x*point[2].y - point[1].y*point[2].x +
					  point[2].x*point[3].y - point[2].y*point[3].x +
					  point[3].x*point[0].y - point[3].y*point[0].x
					)/2.f
				);
	}

	size_t size(){
		return QUADRILATERAL_SIZE;
	}
};

/*---------------------------------
  PROTÓTIPOS
  ---------------------------------*/
cv::Mat	best_green ( const cv::Mat& frame ); //HYP
void 	get_good_quadrilaterals (cv::Mat& img, std::vector<Quadrilateral>& quadrilateral);
void 	draw_point ( cv::Mat& img, std::vector<Quadrilateral>& vec ); //HYP
void 	draw_point ( cv::Mat& img, std::vector<cv::Point>& vec, cv::Scalar s = cv::Scalar(255,0,255)); //HYP
void 	sort_point ( std::vector<Quadrilateral>& vec, std::vector<cv::Point>& cent ); //HYP
void 	mask ( const cv::Mat& src, cv::Mat &dst, const cv::Mat& mask );	//HYP

#endif //_HYP_HPP_
