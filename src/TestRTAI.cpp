// A simple test of RTAI, the Real-Time Application Interface

#include <TestRTAI.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <pthread.h>
#include <limits>

namespace TestRTAI {

#define NON_REALTIME_PRIORITY 1
#define MAX_START_LATENCY_CYCLES 30

/*!
 * A helper method takes a pointer to a TestRTAI object and calls 
 * rtMethod() on it. It is necessary to be compatible with
 * rt_thread_create().
 *
 * \param[in] tester A pointer to the ServoClockDreamer class.
 * \return the return value of the call to TestRTAI::rtMethod().
 */
void * call_rtMethod(void * tester)
{
    std::cout << "call_rtMethod: Method called!" << std::endl;
    TestRTAI * tt = static_cast<TestRTAI*>(tester);
    return tt->rtMethod(NULL);
}

TestRTAI::TestRTAI() :
    rtThreadState(RT_THREAD_UNDEF),
    continueRunning(true),
    prevTime(0),
    rtThreadID(0)
{
}

TestRTAI::~TestRTAI()
{
}

void TestRTAI::runTest(double period)
{
    std::cout << "TestRTAI::runTest: Method called!" << std::endl;

    // Compute the period of the real-time servo loop.
    rtPeriod_ns = period * 1e9;
    long long const rtPeriod_us(rtPeriod_ns / 1000);
    
    // Allow non-root user to use Linux POSIX soft real-time process management and
    // memory lock functions.
    // See: https://www.rtai.org/userfiles/documentation/magma/html/api/group__lxrt.html#ga74
    rt_allow_nonroot_hrt();

    // Change scheduler of this thread to be RTAI
    std::cout << "TestRTAI::runTest: Switching to RTAI scheduler..." << std::endl;

    normalTask = rt_task_init_schmod(
        nam2num("MY_TASK"),    // name
        NON_REALTIME_PRIORITY, // priority
        0,                     // stack_size
        0,                     // max_msg_size
        SCHED_FIFO,            // scheduling policy
        0xF                    // CPUs allowed
    );

    if (!normalTask)
        throw std::runtime_error("rt_task_init_schmod failed for non-RT task");
    
    rtThreadState = RT_THREAD_UNDEF;

    // Spawn the real-time thread. The real-time thread executes call_rtMethod().
    std::cout << "TestRTAI::runTest: Spawning RT thread..." << std::endl;

    rtThreadID = rt_thread_create(
        (void*)call_rtMethod, // method to call
        this,                 // parameters
        50000                 // stack size
    ); 

    // Wait up to MAX_START_LATENCY_CYCLES for real-time thread to begin running
    std::cout << "TestRTAI::runTest: Waiting for RT thread to run..." << std::endl;
    for (int ii = 0; ii < MAX_START_LATENCY_CYCLES; ii++) 
    {
        if (rtThreadState == RT_THREAD_RUNNING || rtThreadState == RT_THREAD_ERROR) 
            break;
        usleep(rtPeriod_us);
    }
    
    if (rtThreadState != RT_THREAD_RUNNING) 
    {
        std::stringstream ss;
        ss << "TestRTAI::runTest: ERROR: Real-time thread not running, state is: ";

        switch (rtThreadState) 
        {
            case RT_THREAD_UNDEF:   ss << "RT_THREAD_UNDEF";   break;
            case RT_THREAD_INIT:    ss << "RT_THREAD_INIT";    break;
            case RT_THREAD_RUNNING: ss << "RT_THREAD_RUNNING"; break;
            case RT_THREAD_CLEANUP: ss << "RT_THREAD_CLEANUP"; break;
            case RT_THREAD_ERROR:   ss << "RT_THREAD_ERROR";   break;
            case RT_THREAD_DONE:    ss << "RT_THREAD_DONE";    break;
            default:                ss << "Invalid state: " << rtThreadState;
        }

        std::cout << ss.str() << std::endl;

        usleep(15 * rtPeriod_us);
        rt_task_delete(normalTask);
        rt_thread_join(rtThreadID);  // blocks until the real-time thread exits.
        throw std::runtime_error("TestRTAI::runTest: RT thread failed to start");
    }  
    
    std::cout << "TestRTAI::runTest: OK - real-time thread started." << std::endl;
}

void TestRTAI::stop()
{
    continueRunning = false;
    rt_thread_join(rtThreadID);  // blocks until the real-time thread exits.
    rt_task_delete(normalTask);
}

void * TestRTAI::rtMethod(void *)
{    
    //////////////////////////////////////////////////
    // Initialize shared memory, RT task, and semaphores.
    
    rtThreadState = RT_THREAD_INIT;
       
    // Switch to use RTAI real-time scheduler
    std::cout << "TestRTAI::rtMethod: Switching to RTAI real-time scheduler..." << std::endl;
    RT_TASK * task = rt_task_init_schmod(
        nam2num("MY_RT_TASK"),    // name
        0,                        // priority
        0,                        // stack size
        0,                        // max_msg_size
        SCHED_FIFO,               // scheduling policy
        0xF                       // CPUs allowed
    );

    if (task == NULL) 
    {
        std::cerr << "TestRTAI::rtMethod: Call to rt_task_init_schmod failed for MY_RT_TASK" << std::endl;
        rtThreadState = RT_THREAD_ERROR;
        return NULL;
    }
    
    rt_allow_nonroot_hrt();
    
    //////////////////////////////////////////////////
    // Start the real time engine...
    
    RTIME tickPeriod = nano2count(rtPeriod_ns);

    std::cout << "TestRTAI::rtMethod: Starting the real-time engine..." << std::endl;

    std::cout << "TestRTAI::rtMethod: Making real-time task periodic, tickPeriod = " << tickPeriod << "..." << std::endl;
    rt_task_make_periodic(task, rt_get_time() + tickPeriod, tickPeriod); 

    std::cout << "TestRTAI::rtMethod: Locking current memory into RAM..." << std::endl;
    mlockall(MCL_CURRENT | MCL_FUTURE);

    std::cout << "TestRTAI::rtMethod: Calling rt_make_hard_read_time()..." << std::endl;
    rt_make_hard_real_time();

    rtThreadState = RT_THREAD_RUNNING;

    //////////////////////////////////////////////////
    // The servo loop.

    bool firstRound = true;

    std::cout << "TestRTAI::rtMethod: starting the servo loop!" << std::endl;
    while (continueRunning) 
    {
        long long const currTime(nano2count(rt_get_cpu_time_ns()));

        if (firstRound)
        {
            std::cout << "TestRTAI::rtmethod: First round" << std::endl;
            firstRound = false;
        }
        else
            std::cout << "TestRTAI::rtmethod: Period = " << (count2nano(currTime - prevTime) / 1e6) << "ms" << std::endl;

        prevTime = currTime;

        rt_task_wait_period();
    }
    
    //////////////////////////////////////////////////
    // Clean up after ourselves.
    
    std::cout << "TestRTAI::rtMethod: Exiting RT thread" << std::endl;
    
    rtThreadState = RT_THREAD_CLEANUP;
    rt_make_soft_real_time();
    rt_task_delete(task);
    return NULL;
}
   
} // namespace TestRTAI

int main (int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stdout,"Usage: %s [cycle period]\n",argv[0]);
        return 1;
    }

    TestRTAI::TestRTAI tester;
    double period = atof(argv[1]);

    if (period > 0)
    {
        try
        {

            tester.runTest(period); // TODO: spawn thread to call this

            std::cout << "Press ENTER to exit..." << std::endl;
            std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );

            tester.stop();
        }
        catch (std::runtime_error & re)
        {
            std::cerr << "Problems running real-time test: " << re.what() << std::endl;
        }

    }
    else
        std::cout << "Invalid period. Must be greater than zero." << std::endl;

    return 0;
}
