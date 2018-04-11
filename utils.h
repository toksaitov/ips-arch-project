#ifndef UTILS_H
#define UTILS_H

#if defined __i386 || defined __i386__ || defined _M_IX86
#define x86_32_CPU
#elif defined __amd64 || defined __amd64__ || defined _M_X64 || defined _M_AMD64
#define x86_64_CPU
#endif

#define UTILS_MIN(A,B) (((A)<(B))?(A):(B))
#define UTILS_MAX(A,B) (((A)>(B))?(A):(B))
#define UTILS_CLAMP(X,MIN,MAX) (IPS_MIN(IPS_MAX((X),(MIN)),(MAX)))
#define UTILS_NORMALIZE(X,MIN,MAX) (((X)-(MIN))/((MAX)-(MIN)));
#define UTILS_COUNT_OF(X) ((sizeof(X)/sizeof(0[X])) / ((size_t)(!(sizeof(X) % sizeof(0[X])))))

int utils_get_number_of_cpu_cores(void);

#endif /* UTILS_H */

