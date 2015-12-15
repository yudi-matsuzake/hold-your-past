/** @file HYP.cpp
  * @brief  functions implementation that manipulates an image for you hold your past.
  */
//--INCLUDES--------------------------------------------------
#include <HYP.hpp>
#include <iostream>

//--MACROS----------------------------------------------------
#define BGREEN_MIN_SAT 		60 //60
#define BGREEN_HUE 		60 //60	
#define BGREEN_MAX_DISTANCE 	25 //15
#define BGREEN_MIN_BRIGHT 	25 //25
#define BGREEN_THRESHOLD 	200 //200
#define BGREEN_MEDIAN_BLUR_WIN 	9  //9
///////////////////////////////////
#define QUADRILATERAL_AREA_THRESHOLD 100
///////////////////////////////////


//--NAMESPACES------------------------------------------------
using namespace std;
using namespace cv;

/*------------------------------------------------------------
 _____ _   _ _   _  ____ _____ ___ ___  _   _ ____  
|  ___| | | | \ | |/ ___|_   _|_ _/ _ \| \ | / ___| 
| |_  | | | |  \| | |     | |  | | | | |  \| \___  \
|  _| | |_| | |\  | |___  | |  | | |_| | |\  |___) |
|_|    \___/|_| \_|\____| |_| |___\___/|_| \_|____/ 
=====================================================
-------------------------------------------------------------*/
                                                    

//--BEST_GREEN------------------------------------------------

/** @fn Mat best_green ( const Mat& frame )
  *
  * @param 	frame The input frame
  *
  * @return 	The "best" green mask of frame. The best refers 
  *		in the little world of this program. Or in the
  *		HYP namespace, if you prefer.
  */
Mat best_green ( const Mat& frame ){
	// Frame that will be green-processed
	Mat processed_frame ( frame.size(), CV_8UC1 );
	Mat frame_hsv; // frame converted to the HSV color space

	// Convert to the HSV color space!
	cvtColor( frame, frame_hsv, CV_BGR2HSV );

	// Pointers!
	const uchar* ptr_src;
	uchar* ptr_dst;

	for ( int i = 0; i<frame_hsv.rows; i++ ){
		
		// Pointers to the lines!!
		ptr_src = frame_hsv.ptr<uchar>(i);
		ptr_dst = processed_frame.ptr<uchar>(i);

		// For every line...
		for ( int j = 0; j<frame_hsv.cols; j++ ){

			int green_intensity; // Holds the green intensity!

			/*	Test the green color.
			 *
			 *	If the green intensity were less than BGREEN_MIN_SAT or
			 *	distance from the green on Hue channel were greater
			 *	than BGREEN_MAX_DISTANCE or the brightness were not at least
			 *	BGREEN_MIN_BRIGHT: forget about this green. Set zero for this one.
			 *
			 *	If the green pass to the above test, so the color for
			 *	this is 255 less the distance of BGREEN_HUE. Pass the
			 *	result value for the BGREEN_THRESHOLD.
			 */
			if( ptr_src[Color::S] <= BGREEN_MIN_SAT )
				green_intensity = 0;
			else if ( ptr_src[Color::V] <= BGREEN_MIN_BRIGHT) //25
				green_intensity = 0;
			else if ( ptr_src[Color::H] < BGREEN_HUE - BGREEN_MAX_DISTANCE || //15
				  ptr_src[Color::H] > BGREEN_HUE + BGREEN_MAX_DISTANCE )  //15
				green_intensity = 0;
			else{
				green_intensity = 255 - (ptr_src[Color::H] - BGREEN_HUE);
				green_intensity = (green_intensity > BGREEN_THRESHOLD)?255:0;
			}

			*ptr_dst = green_intensity;

			// Go pointers!!
			ptr_src += frame_hsv.channels();
			ptr_dst += processed_frame.channels();
		}
	}

	// Median blur ---------------------------------------------
	Mat buffer_helper;
	medianBlur(processed_frame, buffer_helper, BGREEN_MEDIAN_BLUR_WIN);

	// Dilatation! ---------------------------------------------
	Mat element = getStructuringElement ( MORPH_RECT, Size(2, 2), Point(1, 1) );
	dilate( buffer_helper, processed_frame, element );

	return processed_frame;
}

//--FIND_GOOD_QUADRILATERALS--------------------------------------------------------

//! Simple struct to link one score to point.
/**
  * Will be useful only for sort purpose.
  */
typedef struct cord_score {
	float score;
	Point p;
} cord_score;

// Function that compares two structs for the sort algorithm
bool cord_score_comp ( cord_score c0, cord_score c1 ){
	return c0.score < c1.score;
}

/** @fn 	void RDP_score ( vector <Point>& curve, vector <float>& points, int a=0, int b=0 )
  *
  * @brief 		Scores based on Ramer–Douglas–Peucker algorithm.
  *
  * @param curve 	Vector of points order representing a curve of points.
  * 
  * @param score	Vector of scores based on Ramer–Douglas–Peucker algorithm.
  *
  * @param a		Index of the vector of point, just of recursion purpose.
  * @param b		Index of the vector of point, just of recursion purpose.
  */
void RDP_score ( vector <Point>& curve, vector <float>& score, int a=0, int b=0 ){

	// If were the first iteration...
	if(!b) b = curve.size();

	// creates a line segment
	LineSegment2d line (curve[a], curve[b]);

	// Finds the greatest distance to the curve.
	float 	max_dist  = 0;
	int 	max_index = 0;
	for ( int i=a+1; i<b; i++ ){
		float d = line.shortestDistanceTo( curve[i] );
		
		if( d > max_dist ){
			max_dist = d;
			max_index = i;
		}
	}

	// Save the score
	score[max_index] = max_dist;

	if( b-a <= 2 ) return;

	// Recursive call
	RDP_score ( curve, score, a, max_index );
	RDP_score ( curve, score, max_index, b );
}

/** @fn void approximate_quadrilateral ( vector<Point>& curve, Quadrilateral& q )
  * 
  * @param curve 	Vector of Points sorted that represents a curve of points;
  *
  * @param q		Approximate quadrilateral of that curve;
  *
  */
void approximate_quadrilateral ( vector<Point>& curve, Quadrilateral& q ){
	// We don't care if the curve is little than 4. Because quadrilateral has 4 points.
	if( curve.size() < 4 ) return;

	// holds RPD score
	vector<float> score(curve.size());
	// calculates RPD_score
	RDP_score ( curve, score );

	// link all point to a score
	vector<cord_score> cord_and_score;
	for ( size_t i=0; i<curve.size(); i++ ){
		cord_score cs = { score[i], curve[i] };
		cord_and_score.push_back( cs );
	}

	/* TODO: 	quick4th - A function to find the four
	 *		greater values of the vector	
	 */
	// sort by score
	std::sort ( cord_and_score.rbegin(), cord_and_score.rend(), cord_score_comp );

	// Stores the points
	q[0] = cord_and_score[0].p;
	q[1] = cord_and_score[1].p;
	q[2] = cord_and_score[2].p;
	q[3] = cord_and_score[3].p;
	
}

/** @fn void sort_point_based_on_center ( Quadrilateral& q )
  *
  * @brief I couldn't find a name such self-explanatory than this.
  *
  * @param q Quadrilateral to be sorted.
  */
void sort_point_based_on_center ( Quadrilateral& q ){

	// Get center of mass
	Point2f c (0, 0);

	// The center of the mass is the mean of the points!
	for( size_t i=0; i<q.size(); i++ ){
		c.x += q[i].x;
		c.y += q[i].y;
	}

	c.x /= q.size();
	c.y /= q.size();

	// Sort!
	//  Vectors that will catch top points and bottom points, respectively.
	std::vector<cv::Point> top, bot;

	for (size_t i = 0; i < q.size(); i++){
		if (q[i].y < c.y)
			top.push_back(q[i]);
		else
			bot.push_back(q[i]);
	}

	q[0] = top[0].x > top[1].x ? top[1] : top[0]; // Top left
	q[1] = top[0].x > top[1].x ? top[0] : top[1]; // Top right
	q[3] = bot[0].x > bot[1].x ? bot[1] : bot[0]; // Bottom left
	q[2] = bot[0].x > bot[1].x ? bot[0] : bot[1]; // Bottom right
}

/** @fn void get_good_quadrilaterals (Mat& img, vector<Quadrilateral>& quadrilateral);
  *
  * @param img 		One chanel image for that we will retrieve the curve of points.
  *
  * @param quadrilatera A vector of approximate quadrilateral.
  */
void get_good_quadrilaterals (Mat& img, vector<Quadrilateral>& quadrilateral){
	// Vector of 
	//  Vector of Points that represents one curve of points.
	vector<vector<Point> > contours0;

	// dumb vector for the findContours function
	vector<Vec4i> hierarchy;

	// OpenCV function that returns all curves of points to `countors0`
	findContours(img, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	// For every curve of points...
	for( size_t i=0; i< contours0.size(); i++ ){

		// Wether the curve of points have more than 4 points to form the quadrilateral.
		if ( contours0[i].size() >= 4 ){
			// Holds a quadrilateral
			Quadrilateral q; 

			// get the approximative quadrilateral
			approximate_quadrilateral ( contours0[i], q );
			
			// if the area of quadrilateral is big enouth
			if ( q.area() > QUADRILATERAL_AREA_THRESHOLD ){

				// "sort" the points based on center
				sort_point_based_on_center (q);

				// finally add to the vector
				quadrilateral.push_back ( q );
			}
		}
	}
}
