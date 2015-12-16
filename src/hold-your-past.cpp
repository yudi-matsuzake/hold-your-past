/** @file hold-yout-past.cpp
  * @brief the file that has the main function.
  */

#include <HYP.hpp>

//--MACROS----------------------------------------------------
#define WINDOW_TITLE		"Hold Your Past Input"
#define WINDOW_TITLE_PROCESSED	"Hold Your Past"
#define ESC_KEY 27
#define DEFAULT_OUTPUT "_out"

// DEBUG MACROS
#define DEBUG_SHOW_INPUT		0
#define DEBUG_SHOW_CORNERS		1
#define DEBUG_SHOW_GREEN_BLOB		1
#define DEBUG_WINDOW_TITLE_GREEN_BLOB 	"GREEN BLOBS"
///////////////////////////////////////////////////////////////

using namespace std;
using namespace cv;

/*---------------------------------
  FLAGS AND CONSTRAITS
  ---------------------------------*/
bool WRITE_CURRENT_FRAME = false;
string filename;

//pipeline
void pipeline ( Mat& frame ){
	static Mat last_frame = Mat(frame.size(), frame.type());

	// Find the green
	Mat green_blob = best_green ( frame );

	#if DEBUG_SHOW_GREEN_BLOB
	imshow ( DEBUG_WINDOW_TITLE_GREEN_BLOB, green_blob );
	#endif
	

	// Find ROIS
	vector< Rect > roi;
	find_connected_components ( green_blob, roi );
	extend_and_group_bounding_rects ( roi, green_blob.size() ); 

	// For every ROI
	for ( size_t i=0; i<roi.size(); i++ ){
		Mat frame_roi = Mat(frame, roi[i]);
		Mat blob_roi  = Mat(green_blob, roi[i]);
		Mat contours  = blob_roi.clone();

		// Get good quadrilaterals
		vector<Quadrilateral> quadrilateral;
		get_good_quadrilaterals ( contours,  quadrilateral );

		if( last_frame.data ){
			// For every quadrilateral, replace the image
			for ( size_t i=0; i<quadrilateral.size(); i++ )
				replace_quadrilateral_by_image (
					frame_roi, last_frame, blob_roi, quadrilateral[i] );
		}

		#if DEBUG_SHOW_CORNERS
		draw_point( frame_roi, quadrilateral );
		#endif
		
	}

	last_frame = frame.clone();
}

//process
void process_pipeline( Mat& frame ){
	static int n_output = 0;

	#if DEBUG_SHOW_INPUT
	imshow ( WINDOW_TITLE, frame );
	#endif
	pipeline( frame );
	imshow ( WINDOW_TITLE_PROCESSED, frame );

	if(WRITE_CURRENT_FRAME){
		stringstream s;
		s << filename << DEFAULT_OUTPUT << n_output << ".png";

		imwrite(s.str(), frame);
		WRITE_CURRENT_FRAME = false;
		n_output++;
	}
	
}

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
	DEBUG("Hello world of debugging, I'm Hold Your Past!", 0);

	// Open a file passed by argument or open the webcam
	VideoCapture cap;

	if(argc == 2) {
		filename = argv[1];
		cap = VideoCapture(filename);
	}else
		cap = VideoCapture(0);

	Mat frame;

	// Read first frame
	if( !( cap.isOpened() && cap.read( frame ) && frame.data ) ){
		cerr << "Erro, não pôde abrir a imagem" << endl;
		exit( EXIT_FAILURE );
	}

	// Window create
	// Main window
	namedWindow ( WINDOW_TITLE_PROCESSED,	CV_WINDOW_NORMAL );

	// Input window
	#if DEBUG_SHOW_INPUT
	namedWindow ( WINDOW_TITLE,		CV_WINDOW_NORMAL );
	#endif

	// Green blob window
	#if DEBUG_SHOW_GREEN_BLOB
	namedWindow ( DEBUG_WINDOW_TITLE_GREEN_BLOB, CV_WINDOW_NORMAL );
	#endif

	// Process the first frame
	process_pipeline ( frame );

	// While frames are coming...
	while( cap.isOpened() && cap.read( frame ) && frame.data && key_process() ){
		process_pipeline ( frame );
	}
	
	// Wait exit
	while(key_process());
	cap.release();
	destroyAllWindows();
	DEBUG("Bye world of debugging!", 0);
	return 0;
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

