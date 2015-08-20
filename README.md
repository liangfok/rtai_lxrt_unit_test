
# Introduction #
This program provides a simple example of how to run a real-time loop using [RTAI's LXRT module](https://www.rtai.org/userfiles/documentation/magma/html/api/whatis_lxrt.html).

It was tested on a computer with Linux kernel 2.6.32.20 and RTAI 3.9.

# Download #

If you have SSH access to github:

    $ git clone git@github.com:liangfok/rtai_lxrt_unit_test.git

If you do not have SSH access to github:

    $ git clone https://github.com/liangfok/rtai_lxrt_unit_test.git

# Build #

Execute:

    $ cd [directory containing this README]
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

# Execute #

Load the RTAI kernel modules:

    $ cd [directory containing this README]
    $ ./scripts/insmods

Ensure the RTAI kernel modules are loaded:

    $ lsmod | grep rtai
    rtai_shm               11475  0
    rtai_sem               25686  0
    rtai_lxrt             116422  2 rtai_shm,rtai_sem
    rtai_hal               85236  3 rtai_shm,rtai_sem,rtai_lxrt

Run the program:

    $ cd [directory containing this README]
    $ cd build
    $ sudo ./TestRTAI [loop period in seconds]

Typically, you will want a loop period of 0.001 (1 ms).

You should see the actual loop period being printed to the screen. Here is an example:

    TestRTAI::rtmethod: Period = 0.999825ms
    TestRTAI::rtmethod: Period = 0.999971ms
    TestRTAI::rtmethod: Period = 1.00002ms
    TestRTAI::rtmethod: Period = 1.00002ms
    TestRTAI::rtmethod: Period = 0.999975ms
    TestRTAI::rtmethod: Period = 1.00001ms
    TestRTAI::rtmethod: Period = 0.999979ms
    TestRTAI::rtmethod: Period = 1.00018ms
    TestRTAI::rtmethod: Period = 0.99983ms
    TestRTAI::rtmethod: Period = 1.00001ms
    TestRTAI::rtmethod: Period = 0.999975ms
    TestRTAI::rtmethod: Period = 1.00002ms
    TestRTAI::rtmethod: Period = 0.999989ms
    TestRTAI::rtmethod: Period = 1ms

Type any key to stop the program.

Finally, unload the RTAI kernel modules:

    $ cd [directory containing this README]
    $ ./scripts/rmmods