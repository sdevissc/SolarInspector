#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <string>
#include<iomanip>
#include<cmath>
using namespace cv;
using namespace std;

class image {       // The class
  public:             // Access specifier
	  int height,width,height_orig,width_orig;
	  float poly[3];
	  int rescale;
	  int wcenter;
	  int limup;
	  int limdown;
	  Mat img,img_large,shifted;
	  int Nlines;
	  int x[];
	  image(std::string);
	  ~image();

	  void openFrame(std::string);
	  void resize_and_frame();
	  void show(int);
	  void findMinimaAndFit();
	  void correctSlant();
	  void set_original_size();

};