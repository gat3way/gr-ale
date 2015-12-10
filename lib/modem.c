/* -*- c -*-
 * 
 * Copyright (C) 2000 - 2001 Charles Brain, Ilkka Toivanen
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * File:
 *   main.c - Main module for ALE decoder
 * 
 * Version:
 *   $Revision: 1.2 $
 * 
 * Date:
 *   $Date: 2001/05/29 18:53:44 $
 * 
 * Author:
 *   Charles Brain
 *   Ilkka Toivanen
 * 
 * History:
 *   $Log: modem.c,v $
 *   Revision 1.2  2001/05/29 18:53:44  pile
 *   Fixed sync problems and cleaned up garbage printing (decode_word).
 *
 *   Revision 1.1.1.1  2001/05/23 20:19:50  pile
 *   Initial version for sourceforge.net
 *
 *   Revision 0.3  2001/05/21 13:51:16  pile
 *   Minor fixes in output.
 *
 *   Revision 0.2  2001/05/20 20:41:10  pile
 *   Fixed \n and DATA output.
 *
 *   Revision 0.1  2001/05/20 19:42:52  pile
 *   Added text logging to file.
 *
 *   Revision 0.0.1.1  2001/05/20 06:10:06  pile
 *   LinuxALE
 *
 */

#include "math.h"
#include "golay.h"
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define NR                          17
#define FFT_SIZE                    64
#define MOD_64                      64
#define SYMBOLS_PER_WORD            49
#define VOTE_BUFFER_LENGTH          48
#define NOT_WORD_SYNC               0
#define WORD_SYNC                   1
#define BITS_PER_SYMBOL             3
#define VOTE_ARRAY_LENGTH           (SYMBOLS_PER_WORD*BITS_PER_SYMBOL)
#define PI                          M_PI
#define BAD_VOTE_THRESHOLD          25
#define SYNC_ERROR_THRESHOLD        1

typedef struct{
  double real;
  double imag;
} Complex;

static const char *preamble_types[] = {
  "[DATA]", "[THRU]", "[TO]", "[TWS]", "[FROM]", "[TIS]", "[CMD]",
  "[REP]"
};

static const char *CMD_types[] = {
  "[Advanced LQA]", "[LQA]", "[DBA]", "[Channels]", "[DTM]", "[Freq]",
  "[Mode selection]", "[Noise report]"
};

#define ASCII_128  0
#define ASCII_64   1
#define ASCII_38   2

char ASCII_Set[128] = {
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2,
         2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
         2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//const int symbol_lookup[22] = {8,8,8,8,8,8,0,8,1,8,3,8,2,8,6,8,7,8,5,8,4,8};
//const int symbol_lookup[22] = {8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4};

const int g_symbol_lookup[NR][32]   = {
                                        {0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
                                        {8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
                                        {8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
                                        {8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8,8,8,8,8,8,8,8},
                                        {8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8,8,8,8,8,8,8},
                                        {8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8,8,8,8,8,8},
                                        {8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8,8,8,8,8},
                                        {8,8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8,8,8,8},
                                        {8,8,8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8,8,8},
                                        {8,8,8,8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8,8},
                                        {8,8,8,8,8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8,8},
                                        {8,8,8,8,8,8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8,8},
                                        {8,8,8,8,8,8,8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8,8},
                                        {8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8,8},
                                        {8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8,8},
                                        {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4,8},
                                        {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,1,1,3,3,2,2,6,6,7,7,5,5,4,4},
                                    };
int *symbol_lookup = g_symbol_lookup[5];



const int vote_lookup[4]    = {0,0,1,1};	

/*
 * FFT information
 */
static double  fft_cs_twiddle[FFT_SIZE];
static double  fft_ss_twiddle[FFT_SIZE];
static double  fft_history[FFT_SIZE];
static Complex fft_out[FFT_SIZE];
static double  fft_mag[FFT_SIZE];
static int     fft_history_offset=0;

/*
 * sync information
 */
static double mag_sum[NR][FFT_SIZE];
static double mag_history[NR][FFT_SIZE][SYMBOLS_PER_WORD];
static int    mag_history_offset[NR];
static int    word_sync[NR];



// worker data
static int started[NR]; /* if other than DATA has arrived */
static int bits[NR][VOTE_ARRAY_LENGTH];
static int input_buffer_pos[NR];
static int word_sync_position[NR];

// protocol data
static char to[4];
static char from[4];
static char data[4];
static char rep[4];
static char tis[4];
static char tws[4];

static int inew;
static int ito;
static int ifrom;
static int idata;
static int irep;
static int itis;
static int itws;



void decodeCMD (unsigned long word) 
{
  
  int firstletter, metacmd;

  firstletter = (word>>14)&0x0F;
  metacmd = (word>>14)&0x70;

  if (metacmd == 6)
    {
      printf (" %s ", CMD_types[firstletter]);
      /* this part is still under development.
      fprintf (stdout, " %s", CMD_types[firstletter]);
      if (log_file) {
	fprintf (log_file, " %s", CMD_types[firstletter]);
      }
      */
    }
  
}

int decode_word (unsigned long word, FILE *log_file, int nr)
{
  unsigned char a, b, c, preamble;
  time_t timestamp;
  struct tm *UTC_time;

  char msg[200];
  char tmpBuffer[200];
  memset(msg, '\0', 200);
  memset(tmpBuffer, '\0', 200);
  
  timestamp = time(NULL);
  UTC_time=gmtime(&timestamp);
  sprintf (msg, "[%02d:%02d:%02d] ", UTC_time->tm_hour,
	   UTC_time->tm_min, UTC_time->tm_sec);

  c = word&0x7F;
  b = (word>>7)&0x7F;
  a = (word>>14)&0x7F;
  preamble = (word>>21)&0x7;

  /* if CMD just print the stuff */
  if (preamble == 6) 
    {
      int firstletter = (word>>14)&0x0F;printf (" %s ", CMD_types[firstletter]);

      sprintf (tmpBuffer, "%s ",preamble_types[preamble]);
      strcat (msg, tmpBuffer);
      sprintf(tmpBuffer, "%c%c%c",a,b,c);
      strcat (msg, tmpBuffer);
      
      printf ("%s\n", msg);
      if (log_file)
	{
	  fprintf (log_file, "%s\n", msg);
	}
    }
  else 
    /* if other than CMD */
    {
      /* check if proper charset i.e. proper ale addresses */
        if((ASCII_Set[a]==ASCII_38)&&(ASCII_Set[b]==ASCII_38)&&(ASCII_Set[c]==ASCII_38)) 
	{

	  sprintf (tmpBuffer, "%s ",preamble_types[preamble]);
	  strcat (msg, tmpBuffer);
	  sprintf(tmpBuffer, "%c%c%c",a,b,c);
	  strcat (msg, tmpBuffer);
	  
	  //printf ("%s\n", msg);
          switch (preamble)
          {
            inew = 1;
            case 0:
                sprintf(data, "%c%c%c",a,b,c);
                idata = 1;
                break;
            case 2:
                sprintf(to, "%c%c%c",a,b,c);
                ito = 1;
                break;
            case 3:
                sprintf(tws, "%c%c%c",a,b,c);
                itws = 1;
                break;
            case 4:
                sprintf(from, "%c%c%c",a,b,c);
                ifrom = 1;
                break;
            case 5:
                sprintf(tis, "%c%c%c",a,b,c);
                itis = 1;
                break;
            case 7:
                sprintf(rep, "%c%c%c",a,b,c);
                irep = 1;
                break;
            default:
                break;
            }


	  if (log_file)
	    {
	      fprintf (log_file, "%s\n", msg);
	    }
	}
        else return -1;
    }
    return 1;
}

unsigned long modem_de_interleave_and_fec(int *input, int *errors)
{
  int i;
  unsigned long worda;
  unsigned long wordb;
  unsigned int error_a, error_b;
  
  worda = wordb = 0;
	
  for(i = 0; i < VOTE_BUFFER_LENGTH; )
    {
      worda = input[i++] ? (worda<<1)+1 : worda<<1 ;
      wordb = input[i++] ? (wordb<<1)+1 : wordb<<1 ; 
    }
  wordb = wordb ^ 0x000FFF;
  /*
    Now do the FEC
  */
  worda = golay_decode(worda,&error_a);
  wordb = golay_decode(wordb,&error_b);
  /*
    Return the number of errors as the word with the most.
  */
  if(error_a > error_b) 
    *errors = error_a;
  else
    *errors = error_b;
  /*
    Format into a single 24 bit word
  */ 
  worda = (worda<<12) + wordb;
  return (worda);
}


/*
  Process a new received symbol
*/
void modem_new_symbol(int sym, FILE *log_file, int nr)
{
  int        		majority_vote_array[VOTE_BUFFER_LENGTH];
  int        		bad_votes, sum, errors, i;
  unsigned long         word=0;
  int k,j;

  /*
    Update the input buffer
  */
  bits[nr][input_buffer_pos[nr]] = (sym&4) ? 1 : 0;
  input_buffer_pos[nr] = (input_buffer_pos[nr]+1)%VOTE_ARRAY_LENGTH;
  bits[nr][input_buffer_pos[nr]] = (sym&2) ? 1 : 0;
  input_buffer_pos[nr] = (input_buffer_pos[nr]+1)%VOTE_ARRAY_LENGTH;
  bits[nr][input_buffer_pos[nr]] = (sym&1) ? 1 : 0;
  input_buffer_pos[nr] = (input_buffer_pos[nr]+1)%VOTE_ARRAY_LENGTH;
  /*
    Do the majority vote, ignore stuff bits
  */
  bad_votes = 0;
  for(i=0; i<VOTE_BUFFER_LENGTH; i++)
    {
      sum  = bits[nr][(i+input_buffer_pos[nr])%VOTE_ARRAY_LENGTH];
      sum += bits[nr][(i+input_buffer_pos[nr]+SYMBOLS_PER_WORD)%VOTE_ARRAY_LENGTH];
      sum += bits[nr][(i+input_buffer_pos[nr]+(2*SYMBOLS_PER_WORD))%VOTE_ARRAY_LENGTH];
      if(( sum == 1 ) || ( sum == 2)) bad_votes++; 
      majority_vote_array[i] = vote_lookup[sum];
    }
  /*    
    printf("Votes %d\n",bad_votes);

    Now do tests to see if it might be time for a word sync
  */
//printf(".");
  if( word_sync[nr] == NOT_WORD_SYNC )
    {
      if( bad_votes <= BAD_VOTE_THRESHOLD )
	{
	  word = modem_de_interleave_and_fec(majority_vote_array,&errors);
	  if(errors <= SYNC_ERROR_THRESHOLD)
	    {
	      int err = decode_word(word, log_file,nr);
	      /* now we have a proper sync; added after 0.0.1 */
	      word_sync[nr] = WORD_SYNC;
	      word_sync_position[nr] = input_buffer_pos[nr]; 
	    }
	    //else printf("%d\n",errors);
	}
        else
        {
        }
    }
  else
    {
      if(abs(input_buffer_pos[nr]-word_sync_position[nr])<1 )
	{
	  /* Signal new word */
	  word = modem_de_interleave_and_fec(majority_vote_array,&errors);
	  decode_word (word, log_file,nr);
	}
      else 
	{
	  /* we don't have a proper sync; added after 0.0.1 */
	  word_sync[nr] = NOT_WORD_SYNC;
//printf("notsync\n");
	}
    }
}
void modem_init(void)
{
  int i,j,k;
  
  /*
    Create the twiddles etc for the FFT
  */
  for(i=0; i<FFT_SIZE; i++)
    {
      fft_cs_twiddle[i] = cos((-2.0*PI*i)/FFT_SIZE);
      fft_ss_twiddle[i] = sin((-2.0*PI*i)/FFT_SIZE);
      fft_history[i]    = 0;
    }
  fft_history_offset = 0;
  
  for (i=0;i<NR;i++) 
  {
    word_sync[i] = NOT_WORD_SYNC;
    for (j=0;j<FFT_SIZE;j++)
    {
        mag_sum[i][j]=0;
        for (k=0;k<SYMBOLS_PER_WORD;k++) mag_history[i][j][k]=0;
    }
    mag_history_offset[i]=0;
    word_sync[i]=0;
    for (k=0;k<VOTE_ARRAY_LENGTH;k++) bits[i][k]=0;
    input_buffer_pos[i]=0;
    word_sync_position[i]=0;
  }
}
/*
  Main Signal processing routine
*/
void modem(unsigned short *sample, int length, FILE *log_file)
{
  int i,j,k,n,max_offset;
  double new_sample;
  double old_sample;
  double temp_real;
  double temp_imag;
  double max_magnitude;
  double max_magnitude2[NR];
  static int last_symbol[NR];
  static int last_diff[NR];
  static int last_sync_position[NR];
  static int sample_count[NR];
  

  for(i=0; i<length; i++)
    {
      if(sample[i]&0x8000)
      {
	/*
	  converting between 2's compliment and floating point maths 
  	  a mystical factor is (1/2^15)
	*/
	new_sample = -(((~sample[i])&0x7FFF)*0.00003051757);
      }
      else
      {
	new_sample = sample[i]*0.00003051757;
      }
      /*
	Do the Sliding FFT
      */
      old_sample = fft_history[fft_history_offset];
      fft_history[fft_history_offset] = new_sample;
      /*
	Update output, only do first half, imag in = 0;
      */
      for(n=0; n<FFT_SIZE/2; n++)
	{
	  temp_real       = fft_out[n].real-old_sample+new_sample;
	  temp_imag       = fft_out[n].imag;
	  fft_out[n].real = (temp_real*fft_cs_twiddle[n]) - (temp_imag*fft_ss_twiddle[n]);
	  fft_out[n].imag = (temp_real*fft_ss_twiddle[n]) + (temp_imag*fft_cs_twiddle[n]);
	  fft_mag[n]      = sqrt((fft_out[n].real*fft_out[n].real)+(fft_out[n].imag*fft_out[n].imag))*10;
	}
      /*
	Save the max output value in a running average over one ALE word
	This is used for bit sync and symbol decoding
      */ 
      max_magnitude = 0;
      /*      for( n = 6; n <= 22; n += 2)*/
      for( n = 1; n <= 27; n++)
	{
	  if (( fft_mag[n] > max_magnitude ))
	    {
	      max_magnitude = fft_mag[n];
	      max_offset    = n;
	    }
	}




      /*
	Update the magnitude history for sync purposes
      */
      for (n=0;n<NR;n++)
      {
          //mag_sum[n][sample_count[n]]-= mag_history[n][sample_count[n]][mag_history_offset[n]];
          mag_sum[n][sample_count[n]] += max_magnitude;
          mag_history[n][sample_count[n]][mag_history_offset[n]] = max_magnitude;
          //printf("%d ",(int)mag_sum[n][sample_count[n]]);
      }

      /*
	Decide what to do 
      */
      for (j=0;j<NR;j++) if(word_sync[j] == NOT_WORD_SYNC)
	{
	  /*
	    Update bit sync position
	  */
	  max_magnitude = 0;
	  for( n=0; n < FFT_SIZE; n++)
	    {
	      if(mag_sum[j][n] > max_magnitude)
		{
		  max_magnitude = mag_sum[j][n];
		  last_sync_position[j] = n;
		}
	    }   
	}



     //for( n=0; n < FFT_SIZE; n++) printf("%f ",mag_sum[0][n]);printf("\n\n");

      for (n=0;n<NR;n++) if (sample_count[n]==last_sync_position[n])
	{
            last_symbol[n] = g_symbol_lookup[n][max_offset]; 
	}


      ito=ifrom=itis=irep=idata=itws=0;

      for (n=0;n<NR;n++) if(sample_count[n] == 0 )
	{
            modem_new_symbol(last_symbol[n], log_file,n);
	}


      if (ito) printf("To: %s\n",to);
      if (itws) printf("TWS: %s\n",tws);
      if (itis) printf("TIS: %s\n",tis);
      if (ifrom) printf("From: %s\n",from);
      if (idata) printf("Data: %s\n",data);
      if (irep) printf("Rep: %s\n",rep);
      

      fft_history_offset = (fft_history_offset+1)%FFT_SIZE;

      for (n=0;n<NR;n++) 
        {
            sample_count[n] = (sample_count[n]+1)%MOD_64; 
            if(sample_count[n] == 0) 
            {
                mag_history_offset[n] = (mag_history_offset[n]+1)%SYMBOLS_PER_WORD;
          if (mag_history_offset[n]==0) for (j=0;j<FFT_SIZE;j++)
            {
                mag_sum[n][j]=0;
            }

            }
        }

    }
}
