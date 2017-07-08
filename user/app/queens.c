#include <tinix.h>

long sum = 0, upperlim = 1;

void test(long row, long ld, long rd)
{

    if (row != upperlim)
    {
        long pos = upperlim & ~(row | ld | rd);
        while (pos)
        {
            long p = pos & -pos;

            pos -= p;
            test(row + p, (ld + p) << 1, (rd + p) >> 1);
        }
    } else
        sum++;
}

void tinix_main()
{
    unsigned tm;
    int n;

    printf("Nr of queens:");
    scanf("%d", &n);

    tm = sys_getticks();

    if ((n < 1) || (n > 32))
    {
        printf("Nr must <= 32\n");
        sys_exit();
    }
    upperlim = (upperlim << n) - 1;

    test(0, 0, 0);
    printf("Total %ld solutions, time %d ms.\n", sum, (int) (sys_getticks() - tm));
    sys_exit();
}
