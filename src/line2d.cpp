/*============================================================================*/
/* 2D LINES AND LINE SEGMENTS, REPRESENTED BY POLAR EQUATIONS.                */
/*----------------------------------------------------------------------------*/
/* Author: Bogdan T. Nassu - nassubt-ufpr@yahoo.com.br                        */
/*                                                                            */
/* 2015/04/07                                                                 */
/*============================================================================*/

#include "line2d.h"

using namespace cv;

/*============================================================================*/
/* Line2D CLASS                                                               */
/*============================================================================*/
/** Default constructor. Creates an invalid line. */

Line2d::Line2d ()
{
	angle = -1;
}

/*----------------------------------------------------------------------------*/
/** Constructor. Creates a line that passes through two points.
 *
 * Parameters: Point2f pt1: a point.
 *            Point2f pt2: another point. */

Line2d::Line2d (Point2f pt1, Point2f pt2)
{
	angle = atan2 (pt2.y - pt1.y, pt2.x - pt1.x);
	while (angle < 0)
		angle += LINE_2PI;

	// theta is perpendicular to the line.
	theta = angleSum (angle, LINE_PI_2);
	si = sin (theta);
	co = cos (theta);

	r = pt1.x*co + pt1.y*si;
}

/*----------------------------------------------------------------------------*/
/** Constructor. Creates a line based on a point where it passes and its angle.
 *
 * Parameters: Point2f pt: a point where the line passes.
 *            float angle: angle between the line and the x axis, in radians. */

Line2d::Line2d (Point2f pt, float angle)
{
	// Make sure the angle is positive and in the interval [0,2PI).
	while (angle >= LINE_2PI)
		angle -= LINE_2PI; // Faster and more precise than working with divisions, unless the number is too large.
	while (angle < 0)
		angle += LINE_2PI;
	this->angle = angle;

	// theta is perpendicular to the line.
	theta = angleSum (angle, LINE_PI/2);
	si = sin (theta);
	co = cos (theta);

	r = pt.x*co + pt.y*si;
}

/*----------------------------------------------------------------------------*/
/** Constructor. Creates a line with the given theta and r.
 *
 * Parameters: float theta: line theta.
 *             float r: line r. */

Line2d::Line2d (float theta, float r)
{
	// Find the angle.
	angle = theta - LINE_PI_2;
	while (angle >= LINE_2PI)
		angle -= LINE_2PI; // Faster and more precise than working with divisions, unless the number is too large.
	while (angle < 0)
		angle += LINE_2PI;

	// Adjust theta.
	this->theta = angleSum (angle, LINE_PI_2);
	si = sin (this->theta);
	co = cos (this->theta);

	this->r = r;
}

/*----------------------------------------------------------------------------*/
/** Returns the sum of two angles, in the interval [0,2PI).
 *
 * Parameters: float a1: one angle.
 *             float a2: other angle.
 *
 * Return value: a1+a2, in the interval [0,2PI). */

float Line2d::angleSum (float a1, float a2)
{
	float sum = a1+a2;
	while (sum < 0)
		sum += (float) LINE_2PI;
	while (sum > LINE_PI*2)
		sum -= (float) LINE_2PI;
	
	return (sum);
}

/*----------------------------------------------------------------------------*/
/** Returns the smallest difference between two angles.
 *
 * Parameters: float a1: one angle. Must be in the interval [0, 2PI).
 *             float a2: another angle. Must be in the interval [0, 2PI).
 *
 * Return Value: the difference between the angles. */

float Line2d::smallestAngleDiff (float a1, float a2)
{
	while (a1 > LINE_PI)
		a1 -= LINE_PI;

	while (a2 > LINE_PI)
		a2 -= LINE_PI;

	float diff = fabs (a1-a2);
	if (diff > LINE_PI_2)
		diff = LINE_PI - diff;
	return (diff);
}

/*----------------------------------------------------------------------------*/
/** Finds the point where two lines cross.
 *
 * Parameters: Line l2: the other line.
 *
 * Return Value: the crossing point. */

Point2f Line2d::crossingPoint (Line2d l2)
{
	if (parallel (l2)) // The lines are parallel!
		throw (std::runtime_error ("Trying to find the crossing point of two parallel lines!"));
	
	Point2f pt;
	pt.x = (l2.r*si - r*l2.si) / (l2.co*si - co*l2.si);
	pt.y = (r - pt.x*co)/si;
	return (pt);
}

/*============================================================================*/
/* LineSegment2D CLASS                                                        */
/*============================================================================*/
/** Default constructor. Creates an invalid line segment. */

LineSegment2d::LineSegment2d (): Line2d ()
{
	pt1 = Point2f (0,0);
	pt2 = Point2f (0,0);
	length = 0;
}

/*----------------------------------------------------------------------------*/
/** Constructor. Creates a line segment that passes through two points.
 *
 * Parameters: Point2f pt1: a point.
 *            Point2f pt2: another point. */

LineSegment2d::LineSegment2d (Point2f pt1, Point2f pt2): Line2d (pt1, pt2)
{
	this->pt1 = pt1;
	this->pt2 = pt2;
	length = sqrtf ((pt1.x-pt2.x)*(pt1.x-pt2.x) + (pt1.y-pt2.y)*(pt1.y-pt2.y));
}

/*----------------------------------------------------------------------------*/
/** Returns the shortest distance between the line segment and a point.
 *
 * Parameters: Point2f pt: a point.
 *
 * Return value: shortest distance between the line segment and pt. */

float LineSegment2d::shortestDistanceTo (Point2f pt)
{
	Point2f a (pt.x - pt1.x, pt.y - pt1.y); // Vector pt to pt1.
	Point2f b (pt.x - pt2.x, pt.y - pt2.y); // Vector pt to pt2.
	Point2f c (pt2.x - pt1.x, pt2.y - pt1.y); // Vector pt2 to pt1.

	float dot = (a.x*c.x + a.y*c.y) / (length*length);
	if (dot <= 0) // The point is beyond pt1.
		return (sqrtf (a.x*a.x + a.y*a.y));
	else if (dot >= 1.0f) // The point is beyond pt2.
		return (sqrtf (b.x*b.x + b.y*b.y));

	Point2f proj (pt1.x + dot*c.x, pt1.y + dot*c.y);
	return (sqrtf ((pt.x - proj.x)*(pt.x - proj.x) + (pt.y - proj.y)*(pt.y - proj.y)));
}

/*============================================================================*/
