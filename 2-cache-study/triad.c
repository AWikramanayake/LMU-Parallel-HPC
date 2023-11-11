#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NITERS 100000

int run_triad(size_t n, FILE** writeFile)
{
    double *a = malloc(n * sizeof(double));
    double *b = malloc(n * sizeof(double));
    double *c = malloc(n * sizeof(double));
    double *d = malloc(n * sizeof(double));

    struct timespec start, end;

    for (size_t i = 0; i < n; i++) {
        b[i] = i;
        c[i] = i;
        d[i] = i;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    for(size_t it = 0; it < NITERS; it++) {
    // vector triad kernel:
        for(size_t i = 0; i < n; i++) {
            a[i] = b[i] + c[i] * d[i];
        }
        if(a[n] < 0) return 1; // prevent DCE
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    long long seconds = end.tv_sec - start.tv_sec;
    long long nanos   = end.tv_nsec - start.tv_nsec + (seconds * 1000000000LL);

    double mflops = 2.0 * NITERS * n / (nanos * 1.0e-9) / 1.0e6;

    printf("%zu,%f,%f\n", n, nanos / 1000000000.0, mflops);
    fprintf(*writeFile,"%zu, %f, %f \n", n, nanos / 1000000000.0, mflops);

    free(a);
    free(b);
    free(c);
    free(d);

    return 0;
}

int main()
{
    size_t n, time, mflops;
    size_t xpoints[] = {2, 3, 4, 5, 8, 10, 14, 20, 28, 40, 56, 79, 100, 141, 200, 282, 398, 562, 794, 1000, 1413, 1995, 2818,
                3981, 5623, 7943, 10000, 14125, 19953, 28184, 39811, 56234, 79433};

                //, 100000, 141254, 199526, 281838, 398107, 562341, 794328, 1000000};

    FILE *fpt;
    fpt = fopen("triad_results.csv", "w+");
    fprintf(fpt,"array size,time,mflops\n");
    for (int i = 0; i < sizeof(xpoints)/sizeof(size_t); i++)
    {
        run_triad(xpoints[i]*128, &fpt);
    }
    fclose(fpt);

    return 0;
}