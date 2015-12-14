/** @file hold-yout-past.cpp
  * @brief the file that has the main function.
  */
#include <iostream>
#include <sstream>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "line2d.h"

#define WINDOW_TITLE		"Frame original"
#define WINDOW_TITLE_PROCESSED	"Frame processado"
#define ESC_KEY 27

#define HGREEN 60
#define MIN_RECT_AREA 2500

#define DEFAULT_OUTPUT "_out"

using namespace std;
using namespace cv;

namespace Color{
	typedef enum{ B, G, R } BGR;
	typedef enum{ H, S, V } HSV;
};

class Quadrilateral {
private:
	Point point[4];
public:
	Point& operator[]( size_t index ) {
		return point[index];
	}

	size_t size(){
		return 4;
	}
};

/*---------------------------------
  PROTÓTIPOS
  ---------------------------------*/
Mat	best_green ( const Mat& frame );
void 	approxQuadrilateral ( vector<Point>& curve, Quadrilateral& q );
void 	get_points ( Mat& binary_frame, Mat& frame, vector<Quadrilateral>& vec );
void 	draw_point ( Mat& img, vector<Quadrilateral>& vec );
void 	draw_point ( Mat& img, vector<Point>& vec, Scalar s = Scalar(255,0,255));
void 	sort_point ( vector<Quadrilateral>& vec, vector<Point>& cent );
void 	mask ( const Mat& src, Mat &dst, const Mat& mask );


/*---------------------------------
  FLAGS E VARIÁVEIS
  ---------------------------------*/
bool WRITE_CURRENT_FRAME = false;
string filename;


//pipeline
Mat pipeline( const Mat& frame ){
	static Mat last_frame(frame.size(), frame.type());
	static Mat quadri_frame(frame.size(), frame.type());
	static bool first = true;

	//acha os verde
	Mat greens = best_green(frame);
	Mat greens_proc;
	//namedWindow("greens sem medianBlur", CV_WINDOW_NORMAL);
	//imshow("greens sem medianBlur" greens);

	medianBlur(greens, greens_proc, 9);

	//dilatation
	Mat element = getStructuringElement ( MORPH_RECT, Size(3, 3), Point(1, 1) );
	Mat processed_frame2;
	dilate( greens_proc, processed_frame2, element );
	//imshow( "dilatado", processed_frame2 );

	greens_proc = processed_frame2;

	greens = greens_proc.clone();
	//namedWindow("greens", CV_WINDOW_NORMAL);
	//imshow("greens", greens);

	//desenha os pontos
	Mat frame_proc = frame.clone();

	//get quadrilaterals
	vector<Quadrilateral> vec;

	get_points(greens_proc, frame_proc, vec);

	vector<Point> cent;

	sort_point(vec, cent);
	draw_point(frame_proc, vec);
	draw_point(frame_proc, cent);

	if(!first){
		for ( size_t i=0; i<vec.size(); i++){
			vector<Point2f> frame_point;
			vector<Point2f> quadrilateral_point;
			frame_point.push_back( Point2f(0, 0) );
			frame_point.push_back( Point2f(frame_proc.cols, 0) );
			frame_point.push_back( Point2f(frame_proc.cols, frame_proc.rows) );
			frame_point.push_back( Point2f(0, frame_proc.rows) );
			
			for(size_t j=0; j<4; j++)
				quadrilateral_point.push_back( vec[i][j] );

			
			Mat transmtx = getPerspectiveTransform( frame_point,  quadrilateral_point);

			warpPerspective( last_frame, quadri_frame, transmtx, frame_proc.size(), INTER_LINEAR, BORDER_REPLICATE);
			mask( quadri_frame, frame_proc, greens );
		}
	}

	last_frame = frame_proc.clone();
	first = false;
	return frame_proc;
}

//process
void process_pipeline( const Mat& frame ){
	static int n_output = 0;
	Mat processed_frame = pipeline( frame );

	if(WRITE_CURRENT_FRAME){
		stringstream s;
		s << filename << DEFAULT_OUTPUT << n_output << ".png";

		imwrite(s.str(), processed_frame);
		WRITE_CURRENT_FRAME = false;
		n_output++;
	}
	
	imshow ( WINDOW_TITLE, frame );
	imshow ( WINDOW_TITLE_PROCESSED, processed_frame );
}

//saiu do programa?
bool key_process(){
	static int wait = 1;
	int k = waitKey(wait);

	switch(k){
		case 'w':
		case 'W':
			WRITE_CURRENT_FRAME = true;
			break;
		case '.':
			wait = 0;
			break;
		case ' ':
			wait = 1;
			break;
		case 'q':
		case 'Q':
		case ESC_KEY:
			return false;
			break;
	}
	return true;
}

int main( int argc, char* argv[] ){
	//Abre arquivo passado por argumento ou abre a webcam caso nenhum argumento foi dado.
	VideoCapture cap;

	if(argc == 2) {
		filename = argv[1];
		cap = VideoCapture(filename);
	}else
		cap = VideoCapture(0);

	Mat frame;

	//Lê o primeiro frame
	if( !( cap.isOpened() && cap.read( frame ) && frame.data ) ){
		cerr << "Erro, não pôde abrir a imagem" << endl;
		exit( EXIT_FAILURE );
	}

	//cria janelas
	namedWindow ( WINDOW_TITLE,		CV_WINDOW_NORMAL );
	namedWindow ( WINDOW_TITLE_PROCESSED,	CV_WINDOW_NORMAL );

	//processa o primeiro frame
	process_pipeline ( frame );

	//enquanto ainda ler frames...
	while( cap.isOpened() && cap.read( frame ) && frame.data && key_process() ){
		process_pipeline ( frame );
	}
	
	while(key_process());
	cap.release();
	destroyAllWindows();
	return 0;
}

int _round ( double n ){
	return (int)(n+0.5);
}

//best green
Mat best_green ( const Mat& frame ){
	Mat processed_frame ( frame.size(), CV_8UC1 );


	//equalize
	Mat frame_hsv;

	/*
	Mat frame_ycrcb;
	Mat frame_bgr;
	vector<Mat> channel;
	
	cvtColor( frame, frame_ycrcb, CV_BGR2YCrCb );
	split( frame_ycrcb, channel );
	equalizeHist( channel[0], channel[0] );
	merge( channel, frame_ycrcb );

	cvtColor( frame_ycrcb, frame_bgr, CV_YCrCb2BGR );
	*/
	cvtColor( frame, frame_hsv, CV_BGR2HSV );

	/*
	namedWindow( "equalized", CV_WINDOW_NORMAL );
	imshow( "equalized", frame_bgr );
	*/

	

	//pega os verde
	const uchar* ptr_src;
	uchar* ptr_dst;

	for ( int i = 0; i<frame_hsv.rows; i++ ){
		
		//pega ponteiros das linhas
		ptr_src = frame_hsv.ptr<uchar>(i);
		ptr_dst = processed_frame.ptr<uchar>(i);
		for ( int j = 0; j<frame_hsv.cols; j++ ){

			int green_intensity;

			if( ptr_src[Color::S] <= 60 )
				green_intensity = 0;
			else if ( ptr_src[Color::V] <= 25)
				green_intensity = 0;
			else if ( ptr_src[Color::H] < HGREEN-15 ||
				  ptr_src[Color::H] > HGREEN+15 )
				green_intensity = 0;
			else{
				green_intensity = 255 - (ptr_src[Color::H] - HGREEN);

				green_intensity = (green_intensity > 200)?255:0;
			}

			//*ptr_dst = (_round((green_intensity+ptr_src[Color::S]+ptr_src[Color::V])) > 200)?255:0;
			*ptr_dst = green_intensity;


			ptr_src += frame_hsv.channels();
			ptr_dst += processed_frame.channels();
		}
	}


	return processed_frame;
}


int episilon_test(int episilon, vector<vector<Point> >& contours, vector<vector<Point> >& contours0){

	contours.resize(contours0.size());

	for( size_t i = 0; i<contours0.size(); i++)
		approxPolyDP(Mat(contours0[i]), contours[i], episilon,  true);
	
	for( size_t i=0; i< contours.size() ; i++ ){
		if(contours[i].size() == 4)
			return 0;
		if(contours[i].size() > 4)
			return 1;
	}

	return -1;
}

void sortCorners(std::vector<cv::Point>& corners, cv::Point2f center)
{
    std::vector<cv::Point2f> top, bot;

    for (size_t i = 0; i < corners.size(); i++)
    {
        if (corners[i].y < center.y)
            top.push_back(corners[i]);
        else
            bot.push_back(corners[i]);
    }

    cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
    cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
    cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
    cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

    corners.clear();
    corners.push_back(tl);
    corners.push_back(tr);
    corners.push_back(br);
    corners.push_back(bl);
}

void sort_point ( vector<Quadrilateral>& vec, vector<Point>& cent){

	for( size_t  i=0; i<vec.size(); i++){
		//pega centro de massa
		Point2f c (0, 0);

		for( size_t j=0; j<vec[i].size(); j++ ){
			c.x += vec[i][j].x;
			c.y += vec[i][j].y;
		}

		c.x /= vec[i].size();
		c.y /= vec[i].size();
		cent.push_back(c);

		//ordena
		vector<Point> v;
		
		v.push_back(vec[i][0]);	
		v.push_back(vec[i][1]);	
		v.push_back(vec[i][2]);	
		v.push_back(vec[i][3]);	
		sortCorners ( v, c );
		
		vec[i][0] = v[0];
		vec[i][1] = v[1];
		vec[i][2] = v[2];
		vec[i][3] = v[3];
	}
}

void draw_point ( Mat& frame, vector<Quadrilateral>& vec ){

	Scalar s[4] = 	{
				Scalar(255, 0, 0),
				Scalar(0, 255, 0),
				Scalar(0, 0, 255),
				Scalar(0, 255, 255)
			};

	for(size_t i=0; i<vec.size(); i++){

		for (size_t j=0; j< 4 ; j++){
			circle(frame, vec[i][j], 4, s[j], 2);
		}
	}
}

void 	draw_point ( Mat& img, vector<Point>& vec, Scalar s ){
	for(size_t i=0; i<vec.size(); i++){
		for (size_t j=0; j< 4 ; j++){
			circle(img, vec[i], 4, s, 2);
		}
	}

}

size_t get_rect_area(Quadrilateral q){
	int min_x = q[0].x, min_y = q[0].y;
	int max_x = q[0].x, max_y = q[0].y;

	for(size_t i=1; i<4; i++){
		if(q[i].x < min_x)
			min_x = q[i].x;
		if(q[i].y < min_y)
			min_y = q[i].y;
		if(q[i].x > max_x)
			max_x = q[i].x;
		if(q[i].y > max_y)
			max_y = q[i].y;
	}

	return (max_x-min_x)*(max_y-min_y);
}

// calculate points
void RDP_points ( vector <Point>& curve, vector <float>& points, int a=0, int b=0 ){

	//se for a primeira iteração...
	if(!b) b = curve.size();

	LineSegment2d line (curve[a], curve[b]);

	float 	max_dist = 0;
	int 	max_index = 0;
	for ( int i=a; i<=b; i++ ){
		if( (i != a) && (i != b) ){
			float d = line.shortestDistanceTo( curve[i] );
			
			if( d > max_dist ){
				max_dist = d;
				max_index = i;
			}
		}
	}

	points[max_index] = max_dist;

	if( b-a <= 3 ) return;

	RDP_points ( curve, points, a, max_index );
	RDP_points ( curve, points, max_index, b );
	
}

typedef struct cord_score {
	float score;
	Point p;
} cord_score;

bool cord_score_comp ( cord_score c0, cord_score c1 ){
	return c0.score < c1.score;
}

void approxQuadrilateral ( vector<Point>& curve, Quadrilateral& q ){
	if( curve.size() < 4 ) return;

	vector<float> score(curve.size());
	RDP_points ( curve, score );

	vector<cord_score> cord_and_score;

	for ( size_t i=0; i<curve.size(); i++ ){
		cord_score cs = { score[i], curve[i] };
		cord_and_score.push_back( cs );
	}

	std::sort ( cord_and_score.rbegin(), cord_and_score.rend(), cord_score_comp );

	q[0] = cord_and_score[0].p;
	q[1] = cord_and_score[1].p;
	q[2] = cord_and_score[2].p;
	q[3] = cord_and_score[3].p;
	
}

void get_points (Mat& img, Mat& frame, vector<Quadrilateral>& quadrilateral){
	vector<vector<Point> > contours0;
	vector<Vec4i> hierarchy;

	findContours(img, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	for( size_t i=0; i< contours0.size(); i++ ){
		Scalar s( (i*10)%255, (i*5)%255, (i*3)%255 );
		//draw_point( frame, contours0[i], s );

		if ( contours0[i].size() >= 4 ){
			Quadrilateral q;

			approxQuadrilateral ( contours0[i], q );
			
			quadrilateral.push_back ( q );
		}
	}
}

void 	mask ( const Mat& src, Mat& dst, const Mat& mask){
	const uchar* ptr_src;
	uchar* ptr_dst;
	const uchar* ptr_mask;

	for(int i=0; i<src.rows; i++){
		ptr_src = src.ptr<uchar>(i);
		ptr_dst = dst.ptr<uchar>(i);
		ptr_mask = mask.ptr<uchar>(i);

		for(int j=0; j<src.cols; j++){
			
			if(*ptr_mask == 255){
				ptr_dst[Color::B] = ptr_src[Color::B];
				ptr_dst[Color::G] = ptr_src[Color::G];
				ptr_dst[Color::R] = ptr_src[Color::R];
			}

			ptr_src += src.channels();
			ptr_dst += dst.channels();
			ptr_mask += mask.channels();
		}
	}
}
