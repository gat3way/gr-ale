gr-ale
====

gr-ale is MIL-STD-188-141a (ALE 2G) decoding block for gnuradio


Usage
-----

The block accepts float input from an USB demod sampled at 8000hz and outputs decoded data to stdout.

It is also capable of recording voice traffic after 3-step ALE handshake is detected for a preconfigured period of time.


Known problems
--------------

Incomplete support of ALE protocol features, currently only sounding and call establishment are decoded


Getting help and reporting bugs
-------------------------------

Please mail gat3way@gat3way.eu or via github issues



Installation from source
------------------------

<pre>
$ git clone https://github.com/gat3way/gr-ale.git gr-ale.git
$ cd gr-ale.git
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
</pre>


There is a nice python multi-channel ALE decoder under examples/ directory. On my machine (six-core AMD bulldozer) it can decode ~22 channels before librtlsdr starts complaining about overruns.


Credits and License
-------------------

gr-ale is written by Milen Rangelov (gat3way@gmail.com) and licensed under the GNU General Public License

Significant portions of source code were based on the LinuxALE project (under GNU License):

 * Copyright (C) 2000 - 2001 
 *   Charles Brain (chbrain@dircon.co.uk)
 *   Ilkka Toivanen (pile@aimo.kareltek.fi)

