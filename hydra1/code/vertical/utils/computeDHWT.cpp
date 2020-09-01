//converts the time series into wavelet coefficients - data is in single file

#include <ctime>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../type_definition.hpp"
#include "utils.hpp"
#include "computeDHWT.hpp"
#include "signs.hpp"
#include "bitArray.hpp"

using namespace std;

int computeDHWT(char* infile, char* inputDir, int count, int size, char* outputDir)
{
  //arguments :  inputfile name, input directory, no. of time series, size (of time  series), outputDir

  double *data;       // array of data values
  double *coeff;
  double *value;
  // int size;
  float* data_;
  // //int size_bp = 512;
  
  double* coeff_buffer = new double [size];
  int coeff_buffer_counter = 0;
  // char *infile = argv[1];
  // int count = atoi(argv[2]);
  // size = atoi(argv[3]); 
     
  data = new double[size];
  coeff = new double[size];
  value = new double[size - 1]; //originally size - 1

  data_ = new float [size];

  string output_dir = outputDir;
  dirExists(outputDir);

  string input_dir = inputDir;
  string inf = infile;
  // char *in;
  // in = &inf[0];
  //ifstream infile1(in);

  string coeff_out = "coeff_" + inf;     
  string outfile_1 = output_dir + "coeff_" + inf;   
  string outfile_2 = output_dir + "sign_" + inf;   
  string outfile_3 = output_dir + "sum_" + inf;

  char* out1;
  out1 = &outfile_1[0];
  ofstream outfile1(out1, ios::binary | ios::ate);     

         
  char* out2;
  out2 = &outfile_2[0];
#if defined (BIT_SIGNS_MATRIX)
  ofstream outfile2(out2, ios::binary | ios::ate); 
#else
  ofstream outfile2(out2);
#endif 
          
  char* out3;
  out3 = &outfile_3[0];
  ofstream outfile3(out3, ios::binary | ios::ate); 

  
  ofstream out_log("coeff_gen_log.log", ios::ate);   
  ifstream infile1;
  string readin = input_dir + infile;
  char* r_in = &readin[0];

  bool isBinary = true; 

  if(isBinary)
    infile1.open(r_in, ios::binary);
  else
    infile1.open(r_in);
     
  if(!infile1)
    out_log << "Cannot open file"<<infile<<endl;
     
  double x; int j,k, index;
  sums_type* sumcoeff;
  int levels = static_cast<int>(log10(size)/log10(2));  
  signs_type* sign = new signs_type[size];
  int signs_dim;


#if defined(BIT_SIGNS_MATRIX)
  signs_dim = size /N_OF_BITS_IN_INT;
#else 
  signs_dim = size;
#endif
  
  int signs_counter = 0;

  signs_type* signs_buffer = new signs_type [signs_dim];
  sumcoeff = (sums_type*) calloc((levels + 1),sizeof (sums_type));//new double[levels+1];
  //sumcoeff = new double [levels+1];
    

     
  k=0;
    
  out_log<<"\n Coefficients are being generated...... \n";
    
  while(k < count)
    { 

      if(isBinary){
      	read_from_binary_file(infile1, data_, size);
	copy(data_, data_ + size, data);
      }
      else
      	read_from_txt_file(infile1, data, size);

      coeff_buffer_counter = 0;
      signs_counter = 0;
      // index=0; 
      // // for(int mn=0;mn<levels+1;mn++)
      // // 	sumcoeff[mn]=0;
         
      // while(infile1 >> x)
      // 	{
      // 	  data[index] = x;         
      // 	  index++;    
      // 	  //cout<<"\n index = "<<index<<"\t data = "<<data[index-1]<<"\t x = "<<x;          
           
      // 	  if(index==size)
      // 	    {   //cout<<"\n "<<size;           
      // 	      break;
      // 	    }
      // 	}
       
      z_normalize(data, size);

      for (j=0;j<size;j+=2)
	{
	  value[j/2] = (data[j]+data[j+1])/2;
	  coeff[j/2] = (data[j]-data[j+1])/2;
	}
         
      int t_size = size/2; // temporary variable holding size 
      int c_counter = size/2;
      int v_counter = size/2;
      int p_counter = 0;
        
      while(t_size > 1) //ORIGINALLY t_size >= 1
	{                 
	  for (int j=0;j<t_size;j+=2)
	    {

	      // if(c_counter+j/2 >= size)
	      // 	cout << "ATTENZIONE OUT OF BOUND c_counter 1" << endl;
	      // else if (p_counter+j+1>=size-1)
	      // 	cout << "ATTENZIONE OUT OF BOUND p_counter 2" << endl;
	      // else if (v_counter+j/2 >=size-1){
	      // 	cout << "ATTENZIONE OUT OF BOUND v_counter 3" << endl;
	      // }
	      
	      coeff[c_counter+j/2] = (value[p_counter+j]-value[p_counter+j+1])/2;
	      value[v_counter+j/2] = (value[p_counter+j]+value[p_counter+j+1])/2;
	    }
	  p_counter = p_counter + t_size;
        
	  t_size = t_size/2;
	  c_counter = c_counter + t_size;
	  v_counter = v_counter + t_size;     
	}
        
      coeff[size-1] = value[size-2];
        
      //ORIGINAL outfile1<<(double)coeff[size-1]<<" ";
      //CHECk
      coeff_buffer[coeff_buffer_counter] = coeff[size-1];
      coeff_buffer_counter++;
      //      outfile1.write(reinterpret_cast<char*> (&coeff[size-1]), sizeof(coeff[size-1]));
      //CHECK if(coeff[size-1]>=0)
      if(coeff[size-1] >= 0){
	sign[size-1] = 1;
	set_sign<signs_type*>(signs_buffer, 0, signs_counter, 0, true);
	signs_counter++;
      }
      else{
	sign[size-1] = 0;
      	set_sign<signs_type*>(signs_buffer, 0, signs_counter, 0, false);
	signs_counter++;
      }
      
      //      outfile2<<sign[size-1]; 
      j=size-2; 
      int counter_1 = 1; //to be multiplied by 2 after every level
      while(j>=0){
	double* temp_array = new double[counter_1];
	for(int jk=0;jk<counter_1;jk++) {
	  temp_array[jk] = coeff[j];
	  j--;
	}
	
	for(int jk=counter_1-1;jk>=0;jk--) {
	  //ORIGINAL outfile1<<(double)temp_array[jk]<<" ";
	  coeff_buffer[coeff_buffer_counter] = temp_array[jk];
	  coeff_buffer_counter++;
	  //	  outfile1.write(reinterpret_cast<char*> (&temp_array[jk]), sizeof (temp_array[jk]));

	  if(temp_array[jk]>0){
	    set_sign<signs_type*>(signs_buffer, 0, 
				  signs_counter, 0, true);
	    signs_counter++;
	    //outfile2<<1;
	  }
	    
	  else {
	    set_sign<signs_type*>(signs_buffer, 0, 
				  signs_counter, 0, false);
	      signs_counter++;
	    //	    outfile2<<0; 
	  }	
	}
	counter_1*=2;
	delete [] temp_array;
      }
               
      sumcoeff[0] = pow(coeff[size-1],2);
      sumcoeff[0] += pow(coeff[size-2],2);
      
      int i=0, counter = 2, pos = size-3;
     
      for(i=1;i<levels;i++)
	{
          sumcoeff[i] = 0;
          int j;
          for(j=0;j<counter;j++)
	    {
	      sumcoeff[i] += pow(coeff[pos-j],2);
	    }
          pos -= counter;
          counter*=2;                  
	} 

      //outfile3<<sumcoeff<<"\n";        
      sums_type totalSUM = 0;
      sums_type totalSUM_no = 0;
      sums_type total_sum [2];
      for(int mn=0;mn<levels;mn++)
        {
	  if(mn==0)
	    totalSUM += sumcoeff[mn]*pow(2.0,levels);
	  else
	    totalSUM += sumcoeff[mn]*pow(2.0,levels-mn);
                         
	  totalSUM_no += sumcoeff[mn];
        }
        
      total_sum[0] = totalSUM;
      total_sum[1] = totalSUM_no;
      //ORIGINAL outfile3<<(double)totalSUM<<" "<<(double)totalSUM_no<<" ";
      //cout<<"totalSUM: " <<(double)totalSUM<< " " << (double)totalSUM_no <<endl;
      outfile3.write(reinterpret_cast<char*> (total_sum) , 2*sizeof(sums_type));
        
      //outfile1<<"\n";
      //outfile2<<"\n";
      //outfile3<<"\n";
      outfile1.write(reinterpret_cast<char*> (coeff_buffer), sizeof(double)*size);
      write_signs<signs_type*>(outfile2, signs_buffer, signs_dim);
      k=k+1;
    }
  infile1.close();
  outfile1.close();
  outfile2.close();
  outfile3.close();
    
  out_log <<"\n generating level files \n";
  GenerateLevelFiles(coeff_out, output_dir, count,size);
    
  out_log <<"\n Preprocessing Done \n";

  out_log.close();

  delete [] signs_buffer;
  delete [] data_;
  delete [] sign;
  free(sumcoeff);

  delete [] data;
  delete [] coeff;
  delete [] value;
  
  delete [] coeff_buffer;

  return 0;      
}

/*int GetSymbol(double number, int size_bp)
{
  bool found = false;    
    
  for(int j=0;j<size_bp-1;j++)
    {
      if(number<breakpoints[j])
	{
	  found  = true;
	  return j;
	}         
    }
  if(found==false)
    return size_bp-1; 
  else 
    return 0;
    }*/

void GenerateLevelFiles(string infile_name, string output_dir, int count, int size)
{

  string coeff_in = output_dir + infile_name;
  char* in;
  in = &coeff_in[0];
  ifstream infile(in, ios::binary); 

  int levels = static_cast<int>(log2(size)); 
     
  char** out;    
  out = new char*[levels+1];

// char** out_tmp;    
//  out_tmp = new char*[levels+1];
  
   
  ofstream *outfile = new ofstream[levels+1];
//ofstream *tmp = new ofstream[levels+1];
  string level = "level_";
  string ext = "_";
     
  //open level files
  for(int j=0;j<=levels-1;j++)
    {
      char ntemp[10];
      //itoa(j,ntemp,10);
      sprintf(ntemp,"%d",j);
      string outfilename = output_dir + level + ntemp + ext + infile_name ;
      //      string outfilename_tmp = output_dir + "tmp_" + level + ntemp;
             
      out[j] = &outfilename[0];
      //out_tmp[j] = &outfilename_tmp[0];
      outfile[j].open(out[j], ios::binary | ios::ate);             
      //tmp[j].open(out_tmp[j], ios::binary);             
    }
     
  //insert into level files
  for(int j=0;j<count;j++)
    {
      int index=0;
      //int x,k=0;
      int k=0;
      double x;
      double y;
      
      while(infile.read(reinterpret_cast<char*> (&x), sizeof(x))){
	  x = abs(x);
	if(index==0){ 
	  infile.read(reinterpret_cast<char*> (&y), sizeof(y));
	  y = abs(y);
	  outfile[index].write(reinterpret_cast<char*> (&x), sizeof (x));
	  outfile[index].write(reinterpret_cast<char*> (&y), sizeof (y));
	  //outfile[index]<<x<<" " << y <<"\n";
	  //tmp[index] << x << " " <<y << " ";
	  index=2;
	}
	else{
	  //now index is power of 2.
	  k = index;
	  int temp = static_cast<int>(log2(k)) ;//+ 1;
	  outfile[temp].write(reinterpret_cast<char*> (&x), sizeof (x));//outfile[temp]<<x;
	  //tmp[temp]<<x << " ";
	  index++;
	  while(index < k*2){
	    infile.read(reinterpret_cast<char*> (&x), sizeof(double));
	    x= abs(x);
	    outfile[temp].write(reinterpret_cast<char*> (&x), sizeof (x));
	    //outfile[temp]<<x<<" ";
	    //tmp[temp] << x << " ";
	    //cout<<"\t temp = "<<temp<<" : "<<index; 
	    index++;                    
	  }
	  //outfile[temp]<<"\n";
	  //tmp[temp] << endl;
	}      
                   
	if(index==size)
	  break;            
      }
      //cout<<"\n "<<j;
      
    }     
     
  infile.close();
  for(int j=0;j<=levels-1;j++)
    outfile[j].close();       
  // for(int j=0;j<=levels-1;j++)
  //   tmp[j].close();       

  delete [] out;
  delete [] outfile;
}
