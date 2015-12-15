/** @file hold-yout-past.cpp
  * @brief the file that has the main function.
  */

#include <HYP.hpp>

#define WINDOW_TITLE		"Frame original"
#define WINDOW_TITLE_PROCESSED	"Frame processado"
#define ESC_KEY 27

#define HGREEN 60
#define MIN_RECT_AREA 2500

#define DEFAULT_OUTPUT "_out"

using namespace std;
using namespace cv;


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
	Mat greens_proc = greens.clone();
	Mat frame_proc = frame.clone();

	//get quadrilaterals
	vector<Quadrilateral> vec;

	get_good_quadrilaterals(greens_proc, vec);

	draw_point(frame_proc, vec);

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
