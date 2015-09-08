// Distributed two-dimensional Discrete FFT transform
// Mohit Prabhushankar
// ECE8893 Project 1


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <signal.h>
#include <math.h>
#include <complex>
//#include <mpi.h>

#include "Complex.h"
#include "InputImage.h"

using namespace std;

typedef complex<double> dcomp;


void Transform1D(Complex* h, int w, Complex* H);

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  // 1) Use the InputImage object to read in the Tower.txt file and
  //    find the width/height of the input image.
  // 2) Use MPI to find how many CPUs in total, and which one
  //    this process is
  // 3) Allocate an array of Complex object of sufficient size to
  //    hold the 2d DFT results (size is width * height)
  // 4) Obtain a pointer to the Complex 1d array of input data
  // 5) Do the individual 1D transforms on the rows assigned to your CPU
  // 6) Send the resultant transformed values to the appropriate
  //    other processors for the next phase.
  // 6a) To send and receive columns, you might need a separate
  //     Complex array of the correct size.
  // 7) Receive messages from other processes to collect your columns
  // 8) When all columns received, do the 1D transforms on the columns
  // 9) Send final answers to CPU 0 (unless you are CPU 0)
  //   9a) If you are CPU 0, collect all values from other processors
  //       and print out with SaveImageData().
  InputImage image(inputFN);  // Create the helper object for reading the image
  // Step (1) in the comments is the line above.
  // Your code here, steps 2-9
    
    
	int w = image.GetWidth();
	int height = image.GetHeight();
    
    Complex* H = new Complex[w*height];
    Complex* H_Temp = new Complex[w*height];
    Complex* H_Final = new Complex[w*height];
    
    Complex* imagePixels = image.GetImageData();
    //1D transform of all rows
    for(int ii=0;ii<height;ii++)
    {
        Transform1D(imagePixels+(ii*w),w,H+(ii*w));
    }

    image.SaveImageData("after1d_mohit.txt", H, w, height);

    //Transpose of the matrix
    for(int jj=0;jj<w;jj++)
    {
        for(int kk=0;kk<height;kk++)
        {
            H_Temp[kk,jj] = H[jj,kk];
        }
    }
    //Transform 1D data
    for(int ll=0;ll<w;ll++)
    {
        Transform1D(H_Temp+(ll*height),height,H+(ll*height));
    }
    
    //Transpose of the matrix
    for(int jj=0;jj<w;jj++)
    {
        for(int kk=0;kk<height;kk++)
        {
            H_Final[kk,jj] = H[jj,kk];
        }
    }
    
    image.SaveImageData("after2d_mohit.txt", H, w, height);
}

void Transform1D(Complex* h, int w, Complex* H)
{
  // Implement a simple 1-d DFT using the double summation equation
  // given in the assignment handout.  h is the time-domain input
  // data, w is the width (N), and H is the output array.
    
//    dcomp i;
//    
//    i = -1;
//    i = sqrt(i);
    
	for(int n=0; n<w; n++)
	{
		Complex W_DFT;
		for(int k=0;k<w;k++)
		{
			W_DFT=pow(exp(sqrt(-1)*2*M_PI/w),(n*k));
			H[n] = H[n]+ W_DFT*h[k];
		}
	}
}

int main(int argc, char** argv)
{
    
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  // MPI initialization here
  Transform2D(fn.c_str()); // Perform the transform.
  // Finalize MPI here
}  
  

  
