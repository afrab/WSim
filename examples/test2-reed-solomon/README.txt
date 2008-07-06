reed-solomon
============

Reed-Solomon code (15,10) over GF(16) implementation for msp430 MCU.

I use it for my remote control devices as follows:

I have to transmit 40 bits of info over noisy channel. I break these 40 bits
into 10 units 4 bites each. Then I encode it with encode_rs(data,&data[10]);
and send these 15 units of 4 bits each over channel.

On receiving end I pass these data via eras_dec_rs(data, 0, 0);
If during transmission no more than 2 errors occured, I'll get restored
first 10 units of usefull info. If I got more than 2 errors, eras_dec_rs()
will return -1.



With -O2 option it needs (with test code for coding/encoding) about 2K of
ROM and 24 bytes of RAM.

To compile::

    msp430-gcc -O2 nn4.c -DTEST_SOURCE -mmcu=msp430x123 -g

To run under debugger (say with ddd front-end) type::

    ddd --debugger msp430-gdb ./a.out


on gdb prompt type::

    ta si
    lo
    
then set breakpoint at any place and run.

Have fun.

~d


Source
------
Files_

.. _files: .


------

back_

.. _back:       ../readme.html
