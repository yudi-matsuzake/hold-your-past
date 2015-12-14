/*============================================================================*/
/* 2D LINES AND LINE SEGMENTS, REPRESENTED BY POLAR EQUATIONS.                */
/*----------------------------------------------------------------------------*/
/* Author: Bogdan T. Nassu - nassubt-ufpr@yahoo.com.br                        */
/*                                                                            */
/* 2015/04/07                                                                 */
/*============================================================================*/

#ifndef __LINE2D_H
#define __LINE2D_H

/*============================================================================*/

#include "cxcore.h"

#define LINE_PI_4 0.78539816339744830961566084581988f
#define LINE_PI_2 1.5707963267948966192313216916398f
#define LINE_PI 3.1415926535897932384626433832795f
#define LINE_2PI 6.283185307179586476925286766559f

/*============================================================================*/

class Line2d
{
public:
	Line2d ();
	Line2d (cv::Point2f pt1, cv::Point2f pt2);
	Line2d (cv::Point2f pt, float angle);
	Line2d (float theta, float r);

	static float angleSum (float a1, float a2);
	static float angleDiff (float a1, float a2) {return (angleSum (a1, -a2));}
	static float smallestAngleDiff (float a1, float a2);

	inline bool isValid () {return (angle >= 0);}
	inline float getY (float x) {return ((r-x*co)/si);} // Will throw a floating point exception if the line is vertical.
	inline float getX (float y) {return ((r-y*si)/co);} // Will throw a floating point exception if the line is horizontal.
	inline cv::Point2f pointWithX (float x) {return (cv::Point2f (x, getY (x)));} // Will throw a floating point exception if the line is vertical.
	inline cv::Point2f pointWithY (float y) {return (cv::Point2f (getX (y), y));} // Will throw a floating point exception if the line is horizontal.
	inline bool isMostlyVertical () {return ((angle > LINE_PI_4 && angle < LINE_PI_2 + LINE_PI_4) || (angle > LINE_PI + LINE_PI_4 && angle < LINE_2PI - LINE_PI_4));}
	inline bool parallel (Line2d l2) {return (angle == l2.angle);}
	cv::Point2f crossingPoint (Line2d l2);

public:
	// This is a very simple class, so we keep access to internal parameters public. However, you should NOT set these values directly!
	float theta; // r = x*cos(theta) + y*sin(theta).
	float r; // r = x*cos(theta) + y*sin(theta).
	float si; // sin(theta).
	float co; // cos(theta).
	float angle; // Angle, in radians, in the interval [0,2PI). A negative angle indicates the line is not valid.
};

/*============================================================================*/

class LineSegment2d: public Line2d
{
public:
	LineSegment2d ();
	LineSegment2d (cv::Point2f pt1, cv::Point2f pt2);

	float shortestDistanceTo (cv::Point2f pt);

public:
	cv::Point2f pt1; // A line segment is defined by 2 points.
	cv::Point2f pt2; // A line segment is defined by 2 points.
	float length; // Precomputed length.
};

/*============================================================================*/
#endif // __LINE2D_H
