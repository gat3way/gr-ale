/* -*- c++ -*- */
/* 
 * Copyright 2015 Milen Rangelov <gat3way@gmail.com>
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_ALE_DECODE_FF_IMPL_H
#define INCLUDED_ALE_DECODE_FF_IMPL_H

#include <ale/decode_ff.h>

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


#define ASCII_128  0
#define ASCII_64   1
#define ASCII_38   2



namespace gr {
  namespace ale {

    typedef struct
    {
        double real;
        double imag;
    } Complex;


    class decode_ff_impl : public decode_ff
    {
     private:
      // Nothing to declare in this block.
        double  fft_cs_twiddle[FFT_SIZE];
        double  fft_ss_twiddle[FFT_SIZE];
        double  fft_history[FFT_SIZE];
        Complex fft_out[FFT_SIZE];
        double  fft_mag[FFT_SIZE];
        int     fft_history_offset;
        /*
         * sync information
         */
        double mag_sum[NR][FFT_SIZE];
        //double mag_history[NR][FFT_SIZE][SYMBOLS_PER_WORD];
        int    mag_history_offset;
        int    word_sync[NR];

        // worker data
        int started[NR]; /* if other than DATA has arrived */
        int bits[NR][VOTE_ARRAY_LENGTH];
        int input_buffer_pos[NR];
        int word_sync_position[NR];

        // protocol data
        char to[4];
        char from[4];
        char data[4];
        char rep[4];
        char tis[4];
        char tws[4];
        char current[64];
        char current2[64];
        int ber[NR];
        int lastber;
        int bestpos;

        int inew;
        int ito;
        int ifrom;
        int idata;
        int irep;
        int itis;
        int itws;
        int state;
        int state_count;
        int stage;

        int last_symbol[NR];
        int last_sync_position[NR];
        int sample_count;
        int frequency;
        int recording;
        int wavsec;
        char wavdir[1024];
        int wavsamples;
        FILE *wavfile;



        unsigned long golay_encode(unsigned int data);
        unsigned int golay_decode(unsigned long code, int *errors);
        int decode_word (unsigned long word, int nr, int berw);
        unsigned long modem_de_interleave_and_fec(int *input, int *errors);
        void modem_new_symbol(int sym, int nr);
        void modem_init(void);
        void log(char *current, char *current2, int state, int ber);
        void do_modem(float *sample, int length);
        void start_wav();
        void stop_wav();



     public:
      decode_ff_impl(int freq,char *wdir, int wsec);
      ~decode_ff_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace ale
} // namespace gr

#endif /* INCLUDED_ALE_DECODE_FF_IMPL_H */

