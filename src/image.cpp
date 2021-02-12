#include "../include/image.h"


image::image(std::string _fname){
	fname=_fname;
	openFrame(fname);
	height_orig = img.rows;
        width_orig = img.cols;
	rescale=15;
	limup=150;
        limdown=650;

}

image::~image(){};

void image::openFrame(std::string str){
	img = imread(str, CV_16UC1);//IMREAD_GRAYSCALE);
//	std::cout <<"------->"<< img.depth() << std::endl;
}

void image::resize_and_frame(){
	if(rescale>1){
		resize(img, img_large, Size(img.cols*rescale,img.rows));
	}else{
		img_large=img;
	}
	height = img_large.rows;
        width = img_large.cols;
	shifted=Mat::zeros(Size(width,height),CV_16UC1);
//        std::cout<<"resized image size: "<<height<<" "<<width<<std::endl;
//	std::cout<<"shifted size      : "<<shifted.rows<<" "<<shifted.cols<<std::endl;
}

void image::set_original_size(){
	resize(shifted,shifted,Size(shifted.cols/rescale,shifted.rows));
}

void image::show(int option){
	namedWindow( "w1", WINDOW_AUTOSIZE );
	if(option==0){
		imshow("original",img);
		imshow("original rescaled",img_large);
		imshow("corrected",shifted);
	}else if(option==1){
		imshow("w1",shifted);
		waitKey(1);
	}
}

void image::correctSlant(){
//	std::cout<<"Correcting slang"<<std::endl;
	float thismin=1e5;
	float pos[height];
	int corr[height];
	for(int i=0;i<height;i++){
		pos[i]=poly[0]+poly[1]*i+poly[2]*pow(i,2);
		if(pos[i]<thismin){
			thismin=pos[i];
		}
	}
	thismin=round(thismin);
	for(int i=0;i<height;i++){
		corr[i]=round((int)pos[i]-thismin);
		for(int j=0;j<width;j++){
			int old_coord=min(width-1,j+corr[i]);
			int old_value=img_large.at<ushort>(i,old_coord);
			shifted.at<short>(i,j,0)=old_value;
		}
	}
//	std::cout<<"Finished slang estimate"<<std::endl;
}


void image::setCounter(int _counter){
	counter=_counter;
}

void image::setSeqLength(int size){
	seqSize=size;
}

void image::setCube(){
	int dims[2] = {shifted.rows,shifted.cols};
        wavMap = Mat(2,dims,CV_16UC1);

}
void image::addColToCubeLayers(){
	for(int j=0;j<shifted.cols;j++){
			wavMap.col(j)=shifted.col(counter);
	}
	cubeLayer.push_back(wavMap);
}

void image::writeWav(){
	for(int i=0;i<counter;i++){
		imwrite("Wav_"+std::to_string(i)+".tif",cubeLayer.at(i));
	}
}

void image::findMinimaAndFit(){
	int counter = 0;
	Nlines=limdown-limup;
	int x[Nlines];
	Point min_loc[Nlines],max_loc[Nlines];
	double min[Nlines],max[Nlines];

	std::cout<<"ok"<<std::endl;
	for(int i=limup;i<limdown;i++){
		x[counter]=i;
		Mat row = img_large.row(i);
		minMaxLoc(row, &min[counter], &max[counter], &min_loc[counter], &max_loc[counter]);
	//	std::cout<<"min position in line "<<i<<" : "<<min_loc[counter].x<<std::endl;
		counter++;
	}

    int i,j,k,n;
    cout.precision(12);                        //set precision
    cout.setf(ios::fixed);
    n=2;                                // n is the degree of Polynomial
    double X[2*n+1];                        //Array that will store the values of sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
    for (i=0;i<2*n+1;i++)
    {
        X[i]=0;
        for (j=0;j<Nlines;j++)
            X[i]=X[i]+pow(x[j],i);        //consecutive positions of the array will store N,sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
    }
    double B[n+1][n+2],a[n+1];            //B is the Normal matrix(augmented) that will store the equations, 'a' is for value of the final coefficients
    for (i=0;i<=n;i++)
        for (j=0;j<=n;j++)
            B[i][j]=X[i+j];            //Build the Normal matrix by storing the corresponding coefficients at the right positions except the last column of the matrix
    double Y[n+1];                    //Array to store the values of sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
    for (i=0;i<n+1;i++)
    {
        Y[i]=0;
        for (j=0;j<Nlines;j++)
        Y[i]=Y[i]+pow(x[j],i)*min_loc[j].x;        //consecutive positions will store sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
    }
    for (i=0;i<=n;i++)
        B[i][n+1]=Y[i];                //load the values of Y as the last column of B(Normal Matrix but augmented)
    n=n+1;                //n is made n+1 because the Gaussian Elimination part below was for n equations, but here n is the degree of polynomial and for n degree we get n+1 equations
    for (i=0;i<n;i++)                    //From now Gaussian Elimination starts(can be ignored) to solve the set of linear equations (Pivotisation)
        for (k=i+1;k<n;k++)
            if (B[i][i]<B[k][i])
                for (j=0;j<=n;j++)
                {
                    double temp=B[i][j];
                    B[i][j]=B[k][j];
                    B[k][j]=temp;
                }

    for (i=0;i<n-1;i++)            //loop to perform the gauss elimination
        for (k=i+1;k<n;k++)
            {
                double t=B[k][i]/B[i][i];
                for (j=0;j<=n;j++)
                    B[k][j]=B[k][j]-t*B[i][j];    //make the elements below the pivot elements equal to zero or elimnate the variables
            }
    for (i=n-1;i>=0;i--)                //back-substitution
    {                        //x is an array whose values correspond to the values of x,y,z..
        a[i]=B[i][n];                //make the variable to be calculated equal to the rhs of the last equation
        for (j=0;j<n;j++)
            if (j!=i)            //then subtract all the lhs values except the coefficient of the variable whose value                                   is being calculated
                a[i]=a[i]-B[i][j]*a[j];
        a[i]=a[i]/B[i][i];            //now finally divide the rhs by the coefficient of the variable to be calculated
    }
    for (i=0;i<n;i++){
	poly[i]=a[i];
        cout<<"x^"<<i<<"="<<a[i]<<endl;            // Print the values of x^0,x^1,x^2,x^3,....
	}
    	cout<<"\n";
}
