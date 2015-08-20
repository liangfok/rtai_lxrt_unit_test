#ifndef __TestRTAI_HPP__
#define __TestRTAI_HPP__

// #include <rtai_sched.h>
// #include <rtai_shm.h>
// #include <rtai.h>
// #include <rtai_sem.h>
// #include <rtai_nam2num.h>
// #include <rtai_registry.h>

#include <rtai.h>
#include <rtai_lxrt.h>

namespace TestRTAI {

typedef enum {
    RT_THREAD_UNDEF,
    RT_THREAD_INIT,
    RT_THREAD_RUNNING,
    RT_THREAD_CLEANUP,
    RT_THREAD_ERROR,
    RT_THREAD_DONE
} rt_thread_state_t;

/*!
 * Creates a point cloud from slice information.
 */
class TestRTAI
{
public:
    /*!
     * The default constructor.
     */
    TestRTAI();

    /*!
     * The destructor.
     */
    virtual ~TestRTAI();

    /*!
     * Runs the RTAI test. This starts a real-time loop.
     *
     * \param period The desired period at which to execute the real-time
     * loop in seconds.
     */
    void runTest(double period);

    /*!
     * Stop the real-time loop.
     */
    void stop();

    /*!
     * This is executed by the real-time thread.
     */
    void * rtMethod(void * arg);

private:

    /*!
     * The current state of the real-time thread.
     */
    rt_thread_state_t rtThreadState;

    /*!
     * The period of the real-time servo loop in nanoseconds.
     */
    long long rtPeriod_ns;

    /*!
     * Whether the real-time loop should continue to cycle.
     */
    bool continueRunning;

    /*!
     * The previous time the real-time loop executed. This is used
     * for measuring the latency.
     */
    long long prevTime;

    /*!
     * The ID of the real-time thread. It is used for joining the real-time thread.
     */
    int rtThreadID;

    /*!
     * The non-real-time task.
     */
    RT_TASK * normalTask;
};

} // namespace TestRTAI

#endif // __TestRTAI_HPP__
