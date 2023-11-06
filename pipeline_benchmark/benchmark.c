#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/sysctl.h>
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <pthread.h>
#include <mach/thread_act.h>

/* macro to define the operation (+, *, /) to avoid duplicated code 
   usage example: a = b _OP_ c */
#if !(defined _OP_)
#    define _OP_ +
#endif

/* macro to print the operation */
#if !(defined _OP_STRING_)
#    define _OP_STRING_ "add"
#endif

static long long timestamp();

/* this pins the thread to a core for more accurate measurements */
static void pin_thread(int core);

int main(int argc, char *argv[])
{
    long long ts_start, ts_end;
    long long nseconds;

    /* we do not really care about the results of operations
       but we need to use them to avoid optimizing the loop away */
    double result = 1;

    /* number of loop iterations */
    long long num_iter = 1000000000;

    /* number of operations per iteration */
    long long ops_per_iter = 12; // ENTERED BY AKSHATH

    /* total number of operations */
    long long num_ops = ops_per_iter * num_iter;

    /* time per operation in nanoseconds */
    double time_per_op;

    pin_thread(0);

    double d1 = 1, d2 = 1, d3 = 1, d4 = 1, d5 = 1, d6 = 1, d7 = 1, d8 = 1 , d9 = 1 , d10 = 1, d11 = 1, d12 =1;
    double factor = 0.123;

    /* pipelining scenario */
    ts_start = timestamp();
    for (int i = 0; i < num_iter; i++) {
        d1  = d1  _OP_ factor;
        d2  = d2  _OP_ factor;
        d3  = d3  _OP_ factor;
        d4  = d4  _OP_ factor;
        d5  = d5  _OP_ factor;
        d6  = d6  _OP_ factor;
        d7  = d7  _OP_ factor;
        d8  = d8  _OP_ factor;
        d9  = d9  _OP_ factor;
        d10 = d10 _OP_ factor;
        d11 = d11 _OP_ factor;
        d12 = d12 _OP_ factor;
    }
    ts_end = timestamp();

    result = d1 + d2 + d3 + d4 + d5 + d6;
    result += d7 + d8 + d9 + d10 + d11 + d12;

    nseconds    = (ts_end - ts_start);
    time_per_op = (double)nseconds / num_ops;

    printf("%s,%10d,%s,%f\n", _OP_STRING_, (unsigned)result, "    pipelined", time_per_op);

    /* no pipelining scenario */
    ts_start = timestamp();
    for (int i = 0; i < num_iter; i++) {
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
        d1 = d1 _OP_ factor;
    }
    ts_end = timestamp();

    result = d1;

    nseconds    = (ts_end - ts_start);
    time_per_op = (double)nseconds / num_ops;

    printf("%s,%10d,%s,%f\n", _OP_STRING_, (unsigned)result, "not pipelined", time_per_op);

    return EXIT_SUCCESS;
}

static long long timestamp()
{
    struct timespec ts;
    long long       timestamp;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    timestamp = ts.tv_sec * 1000000000LL + ts.tv_nsec;
    return timestamp;
}

// pin thread on linux
/*
static void pin_thread(int core)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core, &set);
    sched_setaffinity(0, sizeof(cpu_set_t), &set);
}
*/


// pin thread on mac
/*
The linux method above cannot be implemented 1-to-1 on Mac as thread pinning in not implemented in the same way.
I have used the workaround found here: http://www.hybridkernel.com/2015/01/18/binding_threads_to_cores_osx.html

It doesn't make much of a difference here as the process seems to be carried out on one core anyway, but it will be
helpful in the future to pin tasks to separate cores (by assigning different affinity tags to different processes).
*/

#define SYSCTL_CORE_COUNT   "machdep.cpu.core_count"

typedef struct cpu_set {
  uint32_t    count;
} cpu_set_t;

static inline void
CPU_ZERO(cpu_set_t *cs) { cs->count = 0; }

static inline void
CPU_SET(int num, cpu_set_t *cs) { cs->count |= (1 << num); }

static inline int
CPU_ISSET(int num, cpu_set_t *cs) { return (cs->count & (1 << num)); }

int sched_getaffinity(pid_t pid, size_t cpu_size, cpu_set_t *cpu_set)
{
  int32_t core_count = 0;
  size_t  len = sizeof(core_count);
  int ret = sysctlbyname(SYSCTL_CORE_COUNT, &core_count, &len, 0, 0);
  if (ret) {
    printf("error while get core count %d\n", ret);
    return -1;
  }
  cpu_set->count = 0;
  for (int i = 0; i < core_count; i++) {
    cpu_set->count |= (1 << i);
  }

  return 0;
}

int pthread_setaffinity_np(pthread_t thread, size_t cpu_size,
                           cpu_set_t *cpu_set)
{
  thread_port_t mach_thread;
  int core = 0;

  for (core = 0; core < 8 * cpu_size; core++) {
    if (CPU_ISSET(core, cpu_set)) break;
  }
  printf("binding to core %d\n", core);
  thread_affinity_policy_data_t policy = { core };
  mach_thread = pthread_mach_thread_np(thread);
  thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY,
                    (thread_policy_t)&policy, 1);
  return 0;
}


static void pin_thread(int core)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core, &set);
    // sched_setaffinity(0, sizeof(cpu_set_t), &set);
    pthread_setaffinity_np(0, sizeof(cpu_set_t), &set);
}