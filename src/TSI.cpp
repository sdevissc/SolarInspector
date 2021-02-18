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
	fclose(fp);


	chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	fs::path rootpath(rootdir);
	std::vector<fs::path> pv = get_all(rootpath, ".tif");
	sort(pv.begin(),pv.end());
	int counter=0;
	image img;
	img.setSeqLength(pv.size());
	float progress = 0.0;

	cout<<"Find brightest image, expected to be used to calculate the slant correction"<<endl;
	// At the same time, calculate the transversalium correction
	for(std::vector<fs::path>::iterator it = pv.begin(); it != pv.end(); ++it) {
		displayProgress(progress);
		progress += 1.0/pv.size();


                fs::path cPath=*it;
                string str=rootdir+"/"+ cPath.filename().string();
                img.openFrame(str);
		img.setCounter(counter);
                img.findBrightestImage();
                counter++;
        }
	cout<<endl;
	cout<<"Found out that the brightest image is "<<img.refcounter<<endl;

	cout<<"Calculate the slant correction"<<endl;
	fs::path brightPath=pv.at(img.refcounter);
	img.openFrame(rootdir + "/"+ brightPath.filename().string());
	img.setCounter(counter);
	img.resize_and_frame();
        img.findMinimaAndFit();
        img.correctSlant();

	img.setFlat();
	counter=0;
	progress=0;
	cout<<"Calculate transversalium correction"<<endl;
	for(std::vector<fs::path>::iterator it = pv.begin(); it != pv.end(); ++it) {
		displayProgress(progress);
                progress += 1.0/pv.size();

		fs::path cPath=*it;
                string str=rootdir+"/"+ cPath.filename().string();
                img.openFrame(str);
                img.setCounter(counter);
                img.calculateTransversaliumFlat();
	}
	img.writeFlat();
	cout<<endl;
	// Now run on the whole series and apply the corrections
	counter=0;
	progress=0;
	cout<<"Run on the whole series and apply the corrections"<<endl;
	for(std::vector<fs::path>::iterator it = pv.begin(); it != pv.end(); ++it) {
		displayProgress(progress);
		progress += 1.0/pv.size();

		fs::path cPath=*it;
		string str=rootdir+"/"+ cPath.filename().string();
		img.openFrame(str);
		img.setCounter(counter);
		img.correctFlat();
		img.resize_and_frame();
        	img.correctSlant();
		img.set_original_size();
		img.addFrame();
		counter++;
	}

	// Extract the wav maps, one per spectrum column.
	img.stackAndWrite();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        cout<<std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()<<" ms"<<endl;


	waitKey(0);
   return 0;
}
