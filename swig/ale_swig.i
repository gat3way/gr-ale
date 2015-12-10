/* -*- c++ -*- */

#define ALE_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "ale_swig_doc.i"

%{
#include "ale/decode_ff.h"
%}


%include "ale/decode_ff.h"
GR_SWIG_BLOCK_MAGIC2(ale, decode_ff);
