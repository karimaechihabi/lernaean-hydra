//
//  dft.c
//  sfa C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//  
//


#include "../config.h"
#include "../globals.h"
#include "../include/dft.h"
#include "math.h"

void fft_from_ts_chunk(struct sfa_trie * trie)
{  
      unsigned long ts_loaded = 0;
      unsigned long ts_length = trie->settings->timeseries_size;
      int transforms_size = trie->settings->fft_size;
      int offset = 0;
      offset = trie->settings->start_offset;

      unsigned int sample_size = trie->settings->sample_size;
	
      ts_type * ts= NULL;
      ts = fftwf_malloc ( sizeof ( ts_type ) * ts_length); 
   
      fftwf_complex *ts_out=NULL; 
      ts_out = fftwf_malloc ( sizeof ( fftwf_complex ) * (transforms_size/2+2) ) ;

      fftwf_plan plan_forward=NULL;	
      //plan_forward = fftwf_plan_dft_r2c_1d (ts_length, ts, ts_out, FFTW_ESTIMATE );

      ts_type * transform = NULL;
      transform =  calloc(transforms_size+2, sizeof(ts_type));

     for (int i = 0; i < sample_size; ++i)
     {
           /* sample raw data is already in memory from sfa_trie_set_bins */
	   ts_type * ts_orig  = (ts_type *) trie->buffer_manager->current_ts_record;
	   for (int k =0; k < ts_length; ++k)
	   {
	     ts[k] = ts_orig[k]; 
	   }

	   plan_forward = fftwf_plan_dft_r2c_1d (ts_length, ts, ts_out, FFTW_ESTIMATE);

           fftwf_execute(plan_forward);
	   fftwf_destroy_plan ( plan_forward);
	   
	   ts_out[0][1] = 0;
	   
	   int j = 0;
	   
	   //since normalizing, ignore first coefficent because it is 0., start k =1
	   for(int k = 1; k< transforms_size/2+2; ++k)
	   {
	     transform[j] = ts_out[k][0];
	     transform[j+1] =  ts_out[k][1];
	     j +=2;
	   }
	     ts_type *cur_coeff_line;
    	     trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;

             for (int j = 0; j < transforms_size; ++j)	       
	     {
		 cur_coeff_line = (ts_type *) trie->buffer_manager->current_dft_record;
		 ts_type value =(ts_type) roundf(transform[j]*100.0)/100.0;      
		 //we will use the dft memory to temporarily store
		 //the transforms, which will be discarded
		 //once the bins are set. The dft memory array will
		 //then store the transforms for the overlapping
		 //subsequences.
		 //trie->buffer_manager->dft_mem_array[j-offset][i] = value;
		 cur_coeff_line[i] = value;      
		 //fprintf(stderr,"order_line[%d][%d] = %g \n", j,i,trie->order_line[j][i]);
		 trie->buffer_manager->current_dft_record += sizeof(ts_type) * sample_size;     
	     }
	     //reset current record to point to the start of the array
	     trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;
             trie->buffer_manager->current_ts_record += sizeof(ts_type) * ts_length;	     
      }
      
      trie->buffer_manager->current_ts_record = trie->buffer_manager->ts_mem_array;  
      trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;
      
      fftwf_free (ts);
      fftwf_free (ts_out);
      free(transform);
}


double real_ephi (double u, double m)
{
  return cos(2*M_PI*u/m);
}

double complex_ephi (double u, double m)
{
  return -sin(2*M_PI*u/m);
}

double complex_mul_real(double r1, double im1, double r2, double im2)
{
    return r1*r2 - im1*im2;
}

double complex_mul_imag(double r1, double im1, double r2, double im2)
{
    return r1*im2 + r2*im1;
}

void fft_from_ts(struct sfa_trie * trie, ts_type *ts,  fftwf_complex  *ts_out, ts_type *transform, fftwf_plan plan_forward)
{  
      unsigned long ts_length = trie->settings->timeseries_size;
      int transforms_size = trie->settings->fft_size;
      int offset = 0;
      offset = trie->settings->start_offset;
      
       fftwf_execute ( plan_forward);
	   
       ts_out[0][1] = 0;

       //check if in the case of whole matching, we need to multiply by norm and -1 
        int j = 0;
	   
       //since normalizing, ignore first coefficent because it is 0., start k =1
	//norm for lower_bounding

       for(int k = 1; k< transforms_size/2+1; ++k)
       {
	     transform[j] = ts_out[k][0];
	     transform[j+1] =  ts_out[k][1];
	     j +=2;
       }

       int sign = 1;
       ts_type norm = trie->settings->norm_factor;
       
       for(int j = 0; j< transforms_size; ++j)
       {
  	     transform[j] *= sign *norm;
	     sign *= -1;
       }

      return;
}

/*

  The current transform is pointed to by dft_mem_array
   
*/

void sfa_from_fft(struct sfa_trie * trie, ts_type * cur_transform, unsigned char * cur_sfa_word)
{  
      unsigned long ts_loaded = 0;
      unsigned long ts_length = trie->settings->timeseries_size;
      int dft_length = trie->settings->fft_size;
      int word_length = trie->settings->fft_size;

      int offset = 0;
      offset = trie->settings->start_offset;
      
     for (int k = 0; k < word_length; ++k)
     {
       unsigned int c;
       for (c = 0; c < trie->settings->num_symbols-1; c++)
       {
           if (cur_transform[k] < trie->bins[k][c])
	   {
             break;
           }
       }
       cur_sfa_word[k] = (unsigned char) (c);
       
     }
          
}


