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
			replace_quadrilateral_by_image ( frame_proc, last_frame, greens, vec[i] );
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

