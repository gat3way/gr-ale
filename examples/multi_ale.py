#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Top Block
# Generated: Tue Dec  8 14:12:15 2015
##################################################

from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import ale
import osmosdr
import math



def median(lst):
    maxel = 0
    maxind = 0
    for a in range(2,30):
        summ=0
        for b in lst:
            if abs((a*1000000)-b)<1000000:
                summ+=1
        if summ>maxel:
            maxel = a
            maxind = a
    return maxind*1000000


class top_block(gr.top_block):

    def __init__(self):

        self.offset = offset = 5000000
        self.base = base = options.offset

        if options.file:
            fl = open(options.file,"r")
            line = fl.readline()
            freqs = []
            names = []
            while line:
                line_s = line.split(" ")[0]
                try:
                    freqs.append(int(line_s))
                    names.append(line[len(line_s):].strip(" ").strip("\n"))
                except Exception:
                    print "???",line_s
                line = fl.readline()

            med = median(freqs)
            self.offset = med
            self.cfreq = []
            i=0
            for el in freqs:
                if abs(el-self.offset)<1000000:
                    self.cfreq.append(el - self.offset)
                    print "Frequency:", el,"["+names[i]+"]" 
                i+=1

        else:
            self.offset = int(math.floor(options.freq / 1000000)) * 1000000
            self.cfreq = [options.freq - self.offset]



        gr.top_block.__init__(self, "Top Block")


        ##################################################
        # Variables
        ##################################################
        self.xlate_taps = xlate_taps = firdes.complex_band_pass(1, 2000000, 100, 48000, 2000)
        self.freq_xlating_fir_filter_xxx = []
        self.blocks_float_to_short = []
        self.band_pass_filter = []
        self.ale_decode_ff = []
        self.blocks_complex_to_real = []


        a = 0
        for cf in self.cfreq:

            self.freq_xlating_fir_filter_xxx.append(filter.freq_xlating_fir_filter_ccc(2000000/48000, (xlate_taps), cf, 2000000))
            self.blocks_complex_to_real.append(blocks.complex_to_real(1))
            self.band_pass_filter.append(filter.fir_filter_ccf(6, firdes.band_pass(
                	10, 48000, 100, 4000, 200, firdes.WIN_HAMMING, 6.76)))
            self.ale_decode_ff.append(ale.decode_ff(self.offset+cf,options.wavdir,options.wavsec))
            a+=1





        ##################################################
        # Blocks
        ##################################################
        self.osmosdr_source_0 = osmosdr.source( args="numchan=" + str(1) + " " + "rtl="+str(options.device) )
        self.osmosdr_source_0.set_sample_rate(2000000)
        self.osmosdr_source_0.set_center_freq(self.base + self.offset, 0)
        self.osmosdr_source_0.set_freq_corr(options.ppm, 0)
        self.osmosdr_source_0.set_dc_offset_mode(0, 0)
        self.osmosdr_source_0.set_iq_balance_mode(0, 0)
        self.osmosdr_source_0.set_gain_mode(False, 0)
        self.osmosdr_source_0.set_gain(options.gain, 0)
        self.osmosdr_source_0.set_if_gain(options.gain, 0)
        self.osmosdr_source_0.set_bb_gain(options.gain, 0)
        self.osmosdr_source_0.set_antenna("", 0)
        self.osmosdr_source_0.set_bandwidth(0, 0)


        ##################################################
        # Connections
        ##################################################
        a = 0

        for cf in self.cfreq:
            self.connect((self.osmosdr_source_0, 0), (self.freq_xlating_fir_filter_xxx[a], 0))
            self.connect((self.freq_xlating_fir_filter_xxx[a], 0), (self.band_pass_filter[a], 0))
            self.connect((self.blocks_complex_to_real[a], 0), (self.ale_decode_ff[a], 0))
            self.connect((self.band_pass_filter[a], 0), (self.blocks_complex_to_real[a], 0))
            a+=1



    def get_xlate_taps(self):
        return self.xlate_taps

    def set_xlate_taps(self, xlate_taps):
        self.xlate_taps = xlate_taps
        self.freq_xlating_fir_filter_xxx_0.set_taps((self.xlate_taps))

    def get_offset_cfreq(self):
        return self.offset_cfreq

    def set_offset_cfreq(self, offset_cfreq):
        self.offset_cfreq = offset_cfreq
        self.freq_xlating_fir_filter_xxx_0.set_center_freq(self.offset_cfreq)

    def get_offset(self):
        return self.offset

    def set_offset(self, offset):
        self.offset = offset
        self.osmosdr_source_0.set_center_freq(self.base+self.offset, 0)

    def get_base(self):
        return self.base

    def set_base(self, base):
        self.base = base
        self.osmosdr_source_0.set_center_freq(self.base+self.offset, 0)



if __name__ == '__main__':
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    parser.add_option("-d", "--device", type="string", default="0", help="RTL device")
    parser.add_option("-o", "--offset", type="int", default="200000000",help="Upconverter LO frequency (default is 200000000)")
    parser.add_option("-p", "--ppm", type="eng_float", default="0",help="PPM error")
    parser.add_option("-g", "--gain", type="eng_float", default=10,help="Receiver gain")
    parser.add_option("-f", "--freq", type="int", default=10,help="Frequency")
    parser.add_option("-F", "--file", type="str", default="",help="Frequencies list file")
    parser.add_option("-w", "--wavdir", type="str", default="",help="Directory to save wav files to")
    parser.add_option("-s", "--wavsec", type="int", default=0,help="Record <wavsec> seconds of communication")

    (options, args) = parser.parse_args()



    tb = top_block()
    tb.start()
    tb.wait()
