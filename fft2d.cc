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
#include <mpi.h>

#include "Complex.h"
#include "InputImage.h"

using namespace std;

int rc;

void Transform1D(Complex* h, int w, Complex* H, int bin);
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
    int fwd = 1;
    int inv = 0;
    int w = image.GetWidth();
    int height = image.GetHeight();
    
    Complex* H = new Complex[w*height];
    Complex* H_Temp = new Complex[w*height];
    Complex* H_Temp2 = new Complex[w*height];
    Complex* H_Final = new Complex[w*height];
    Complex* H_Temp_inv = new Complex[w*height];
    Complex* H_Temp2_inv = new Complex[w*height];
    Complex* H_Temp3_inv = new Complex[w*height];
    Complex* H_Final_inv = new Complex[w*height];

    int nCPU;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD,&nCPU);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Status status;
    
    
    int nRowCPU = w/nCPU;
    
    Complex* imagePixels = image.GetImageData();
    
    int Displacement = nRowCPU*rank*w;
    

    //1D transform of all rows
    for(int ii=0;ii<nRowCPU;ii++)
    {
        Transform1D(imagePixels+Displacement+(ii*w),w,H+Displacement+(ii*w), fwd);
    }

    if(rank == 0)
    {
      for(int cpuIndex = 1; cpuIndex < nCPU; cpuIndex++)
      {
        MPI_Recv(H + nRowCPU * cpuIndex * w,
                nRowCPU * w * sizeof(Complex),
                MPI_CHAR,
                cpuIndex,
                0,
                MPI_COMM_WORLD,
                &status);
      }
    }

  if(rank != 0)
  {
    MPI_Send(H + Displacement,
             nRowCPU * w * sizeof(Complex),
             MPI_CHAR,
             0,
             0,
             MPI_COMM_WORLD);
  }
  
  if(rank == 0)
  {  //Transpose of the matrix
    for(int jj=0;jj<w;jj++)
    {
        for(int kk=0;kk<height;kk++)
        {
        	H_Temp[kk+height*jj] = H[jj+w*kk];	
        }
    }
  }

    int Displacement_Transpose = nRowCPU*height*rank;

  if(rank!=0)
  {
    MPI_Recv(H_Temp + Displacement_Transpose,
             nRowCPU * height * sizeof(Complex),
             MPI_CHAR,
             0,
             0,
             MPI_COMM_WORLD,
             &status);
  }


  if(rank == 0)
  {
    for(int cpuIndex = 1; cpuIndex < nCPU; cpuIndex++)
    {
      MPI_Send(H_Temp + nRowCPU * cpuIndex * height,
                nRowCPU * height * sizeof(Complex),
                MPI_CHAR,
                cpuIndex,
                0,
                MPI_COMM_WORLD);
    }
  }
  
    //Transform 1D data
    for(int ll=0;ll<nRowCPU;ll++)
    {
        Transform1D(H_Temp+Displacement_Transpose+(ll*height),height,H_Temp2+Displacement_Transpose+(ll*height),fwd);
    }

  if(rank == 0)
  {
    for(int cpuIndex = 1; cpuIndex < nCPU; cpuIndex++)
    {
      MPI_Recv(H_Temp2 + nRowCPU * cpuIndex * height,
                nRowCPU * height * sizeof(Complex),
                MPI_CHAR,
                cpuIndex,
                0,
                MPI_COMM_WORLD,
                &status);
    }

  }

  if(rank != 0)
  {
    MPI_Send(H_Temp2 + Displacement,
             nRowCPU * height * sizeof(Complex),
             MPI_CHAR,
             0,
             0,
             MPI_COMM_WORLD);
  }
  if(rank == 0)
  {
    //Transpose of the matrix
    for(int jj=0;jj<w;jj++)
    {
        for(int kk=0;kk<height;kk++)
        {
                  H_Final[jj+w*kk] = H_Temp2[kk+height*jj];
        }
    }
  }
    if(rank == 0)
    {
        image.SaveImageData("MyAfter2d.txt", H_Final, w, height);
    }


//
//
//
//
//
//
//
//
//
//
//

    
    
    
    nRowCPU = w/nCPU;
    Displacement = nRowCPU*rank*w;
    Displacement_Transpose = nRowCPU*height*rank;
    
    if(rank!=0)
    {
        MPI_Recv(H_Final + Displacement,
                 nRowCPU * height * sizeof(Complex),
                 MPI_CHAR,
                 0,
                 0,
                 MPI_COMM_WORLD,
                 &status);
    }
    
    
    if(rank == 0)
    {
        for(int cpuIndex = 1; cpuIndex < nCPU; cpuIndex++)
        {
            MPI_Send(H_Final + nRowCPU * cpuIndex * height,
                     nRowCPU * height * sizeof(Complex),
                     MPI_CHAR,
                     cpuIndex,
                     0,
                     MPI_COMM_WORLD);
        }
    }


    for(int ii=0;ii<nRowCPU;ii++)
    {
        Transform1D(H_Final+Displacement+(ii*w),w,H_Temp_inv+Displacement+(ii*w), inv);
    }
    
    if(rank == 0)
    {
        for(int cpuIndex = 1; cpuIndex < nCPU; cpuIndex++)
        {
            MPI_Recv(H_Temp_inv + nRowCPU * cpuIndex * w,
                     nRowCPU * w * sizeof(Complex),
                     MPI_CHAR,
                     cpuIndex,
                     0,
                     MPI_COMM_WORLD,
                     &status);
        }
    }
    
    if(rank != 0)
    {
        MPI_Send(H_Temp_inv + Displacement,
                 nRowCPU * w * sizeof(Complex),
                 MPI_CHAR,
                 0,
                 0,
                 MPI_COMM_WORLD);
    }
        
    if(rank == 0)
    {  //Transpose of the matrix
        for(int jj=0;jj<w;jj++)
        {
            for(int kk=0;kk<height;kk++)
            {
                H_Temp2_inv[kk+height*jj] = H_Temp_inv[jj+w*kk];
            }
        }
    }
    

        
    if(rank!=0)
    {
        MPI_Recv(H_Temp2_inv + Displacement_Transpose,
                 nRowCPU * height * sizeof(Complex),
                 MPI_CHAR,
                 0,
                 0,
                 MPI_COMM_WORLD,
                 &status);
    }
    
    
    if(rank == 0)
    {
        for(int cpuIndex = 1; cpuIndex < nCPU; cpuIndex++)
        {
            MPI_Send(H_Temp2_inv + nRowCPU * cpuIndex * height,
                     nRowCPU * height * sizeof(Complex),
                     MPI_CHAR,
                     cpuIndex,
                     0,
                     MPI_COMM_WORLD);
        }
    }
    
    //Transform 1D data
    for(int ll=0;ll<nRowCPU;ll++)
    {
        Transform1D(H_Temp2_inv+Displacement_Transpose+(ll*height),height,H_Temp3_inv+Displacement_Transpose+(ll*height),inv);
    }
    
    if(rank == 0)
    {
        for(int cpuIndex = 1; cpuIndex < nCPU; cpuIndex++)
        {
            MPI_Recv(H_Temp3_inv + nRowCPU * cpuIndex * height,
                     nRowCPU * height * sizeof(Complex),
                     MPI_CHAR,
                     cpuIndex,
                     0,
                     MPI_COMM_WORLD,
                     &status);
        }
        
    }
    
    if(rank != 0)
    {
        MPI_Send(H_Temp3_inv + Displacement,
                 nRowCPU * height * sizeof(Complex),
                 MPI_CHAR,
                 0,
                 0,
                 MPI_COMM_WORLD);
    }

    
    if(rank == 0)
    {
        //Transpose of the matrix
        for(int jj=0;jj<w;jj++)
        {
            for(int kk=0;kk<height;kk++)
            {
                H_Final_inv[jj+w*kk] = H_Temp3_inv[kk+height*jj];
            }
        }
    }
    if(rank == 0)
    {
        image.SaveImageData("MyAfterInverse.txt", H_Final_inv, w, height);
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
//    if(rank ==0)
//    {
//    for(int ii=0;ii<w;ii++)
//    {
//     	Transform1D(H_Final+(ii*w),w,H_Temp_inv+(ii*w), inv);
//    }
//    image.SaveImageData("WithoutMpi_BeforeFirst_Transpose.txt", H_Temp_inv, w, height);
//
//    //First Transpose
//    for(int jj=0;jj<w;jj++)
//    {
//     	for(int kk=0;kk<height;kk++)
//        {
//                H_Temp2_inv[kk+height*jj] = H_Temp_inv[jj+w*kk];
//        }
//    }
//    image.SaveImageData("WithoutMpi_AfterFirst_Transpose.txt", H_Temp2_inv, height, w);
//
//    //Second 1d Inverse DFT
//    for(int ll=0;ll<w;ll++)
//    {
//     	Transform1D(H_Temp2_inv+(ll*height),height,H_Temp3_inv+(ll*height),inv);
//    }
//    image.SaveImageData("WithoutMpi_BeforeSecond_Transpose.txt", H_Temp3_inv, height, w);
//
//    for(int jj=0;jj<w;jj++)
//    {
//        for(int kk=0;kk<height;kk++)
//        {
//                  H_Final_inv[jj+w*kk] = H_Temp3_inv[kk+height*jj];
//        }
//    }
//
//    image.SaveImageData("MyAfter2d_INV.txt", H_Final_inv, w, height);
//    }
//
}

void Transform1D(Complex* h, int w, Complex* H, int bin)
{
  // Implement a simple 1-d DFT using the double summation equation
  // given in the assignment handout.  h is the time-domain input
  // data, w is the width (N), and H is the output array.
	
	for(int n=0; n<w; n++)
	{
		Complex W_DFT(0,0);
		for(int k=0;k<w;k++)
		{
			double real = cos(2*M_PI*n*k/w);
      			double imag = sin(2*M_PI*n*k/w);
			if( bin == 0)
			{
				real = real/w;
				imag = imag/w;
				Complex W_DFT(real,-imag);
				H[n] = H[n]+W_DFT*h[k];
				continue;
			}
      			Complex W_DFT(real, imag);
			H[n] = H[n]+ W_DFT*h[k];
		}
	}
}

int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  // MPI initialization here
  rc = MPI_Init(&argc,&argv);
  if (rc != MPI_SUCCESS)
  {
    printf ("Error starting MPI program. Terminating.\n");
    MPI_Abort(MPI_COMM_WORLD, rc);
  }
  Transform2D(fn.c_str()); // Perform the transform.
  // Finalize MPI here
  MPI_Finalize();

}  
  

  
