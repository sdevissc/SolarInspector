#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp> 
#include <boost/regex.hpp>


#include "../include/image.h"
#include "../include/rapidjson/reader.h"
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/filereadstream.h"
#include "../include/rapidjson/error/en.h"
#include "../utilities/utils.h"
#include <string>
#include <chrono>
#include <regex>
#include <cstdio>
using namespace cv;
using namespace std;
namespace fs = boost::filesystem; 
using namespace rapidjson;

int main(int argc, char *argv[]){
	FILE* fp = fopen(argv[1], "rb");
	char readBuffer[65536];
	FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	Document d;
	d.ParseStream(is);
	assert(d.IsObject());

	std::string rootdir = d["rootdir"].GetString();
	std::string refframe  = d["refframe"].GetString();
//	assert(d["showfigures"].IsNumber());
//	assert(d["showfigures"].IsInt());
//	int showfigures = d["showfigures"].GetInt();
	cout<<rootdir<<endl;
//	cout<<refframe<<endl;
//	cout<<showfigures<<endl;
	fclose(fp);

	chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	image img(rootdir + "SunSeqTest_1000.tif");//refframe);
	img.resize_and_frame();
	img.findMinimaAndFit();
	img.correctSlant();
	fs::path rootpath(rootdir);
	std::vector<fs::path> pv = get_all(rootpath, ".tif");
	sort(pv.begin(),pv.end());
	for(std::vector<fs::path>::iterator it = pv.begin(); it != pv.end(); ++it) {
		fs::path cPath=*it;
		string str=rootdir+"/"+ cPath.filename().string();
		img.openFrame(str);
		img.resize_and_frame();
        	img.correctSlant();
		img.set_original_size();
		imwrite("corrected_"+cPath.filename().string(),img.img);
//		img.show(showfigures);
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        cout<<std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()<<" ms"<<endl;
	

	waitKey(0);
   return 0;
}
