
# Introduction #
This program provides a simple example of how to run a real-time loop using RTAI's LXRT system.

It was tested on a system with Linux kernel 2.6.32.20 and RTAI 3.9.

# Compilation #

Execute:

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

# Execution #

Load the RTAI kernel modules:

    $ ./scripts/insmods

Ensure the RTAI kernel modules are loaded:
    $ lsmod | grep rtai
    rtai_shm               11475  0 
    rtai_sem               25686  0 
    rtai_lxrt             116422  2 rtai_shm,rtai_sem
    rtai_hal               85236  3 rtai_shm,rtai_sem,rtai_lxrt

Run the program:

   $ cd build
   $ sudo ./TestRTAI [loop period in seconds]

Typically, you will want a loop period of 0.001, which is 1 millisecond.

You should see the actual loop period being printed to the screen.

Type any key to stop the program.

Finally, unload the RTAI kernel modules:

    $ ./scripts/rmmods