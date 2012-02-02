#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>

#include "complex.h"
#include "heap.h"
#include "pngout.h"

struct ThreadArgs
{
    int start;
};


struct Result
{
    int line;
    unsigned char * bytes;
};

void printhelp(void);

static double centerx, centery, rangex, rangey, juliar, juliai, hue;
static int isjulia = 0;
static Heap hp;
static unsigned int tcount, itercount, dimx, dimy, samples, bail, bail2;
/*static FILE * out;*/

static int writeline = 0;
static pthread_mutex_t linelock;




void wheel(double h, double s, double l, unsigned char * rgb)
{
    double c, hp, x, r1, g1, b1, m;
    
    c = (1 - fabs(2 * l -1) ) * s;
    
    hp = h / 60;
    x = c * (1 - fabs( fmod(hp, 2) - 1) );
    switch((int)hp)
    {
        case 0: r1=c,g1=x,b1=0; break;
        case 1: r1=x,g1=c,b1=0; break;
        case 2: r1=0,g1=c,b1=x; break;
        case 3: r1=0,g1=x,b1=c; break;
        case 4: r1=x,g1=0,b1=c; break;
        case 5: r1=c,g1=0,b1=x; break;
        default: r1=0,g1=0,b1=0; break;
    }
    
    m = l - c / 2;
    
    rgb[0] = (unsigned char) (255*(r1 + m));
    rgb[1] = (unsigned char) (255*(g1 + m)); 
    rgb[2] = (unsigned char) (255*(b1 + m));


}

void * runner(void * argv)
{

    int i, j;

    pthread_mutex_lock(&linelock);
    j = writeline++;
    pthread_mutex_unlock(&linelock);
    
    while(j < dimy)
    {
        unsigned char * result;
        struct Result * res;
        PRECISION y, deltax, deltay;
        
        
        if( !(result = malloc(sizeof(unsigned char ) * 3 * dimx) ) )
        {
            fprintf(stderr, "Out of memory in runner\n");
            exit(1);
        }
        if( !(res = malloc(sizeof(struct Result)) ) )
        {
            fprintf(stderr, "Out of memory in runner\n");
            exit(1);
        }
        
        /* only want to do this once */
        y = centery - rangey / 2 + rangey * j / dimy;
        deltax = rangex / dimx / samples;
        deltay = rangey / dimy / samples;
        
        
        for(i = 0; i < dimx; i++)
        {
        
            PRECISION x, magsum, v, delta;
            unsigned int k, m, n, sumk, diverges;
            Complex total, num;
            double color, bright;

            
            x = centerx - rangex / 2 + rangex * i / dimx;
            
            cpl_init(&num, x, y);
            cpl_init(&total, 0, 0);
            
            sumk = 0;
            diverges = 0;
            magsum = 0.0;
            
            for(m = 0; m < samples; m++)
            {
                for(n = 0; n < samples; n++)
                {
                    num.real = x + n * deltax;
                    num.imag = y + m * deltay; 
                    
                    total.real = num.real;
                    total.imag = num.imag;
                    
                    if(isjulia)
                    {
                        num.real = juliar;
                        num.imag = juliai;
                    }
                    
                    for(k = 0; k < itercount; k++)
                    {
                        PRECISION tr, ti;
                        
                        /* This is a the z^2 +c step.  */
                        tr = total.real * total.real - total.imag * total.imag + num.real;
                        ti = total.real * total.imag * 2 + num.imag;
                        total.real = tr; total.imag = ti;
                        
                        /* changing the *  to + for either of these creates bubble shapes */
                        /* we compare the squares of these two values so we dont have to call sqrt
                            unless we really have to */
                        if(total.real*total.real + total.imag*total.imag >= bail2)
                        {
                            magsum += sqrt(total.real*total.real + total.imag*total.imag);
                            diverges += 1;
                            sumk += k;
                            
                            
                            break;
                        }
                    }
                }
            }
            
            /* If we never diverged (IE a black pixel) then just quit here */
            if(diverges == 0)
            {
                result[i*3] = 0;
                result[i*3+1] = 0;
                result[i*3+2] = 0;
                continue;
            }
            
            /* This is a hack to fix antialiasing and smoothing. */
            if(sumk % (samples * samples) )
            {
               
                v = (int)(1.0 * sumk / diverges);
                delta = -1;
            }
            else
            {
                /* Delta is the smoothing var for the gradient like effect */
                delta = log(log(magsum / diverges) / log(bail) ) / log(2);
                
                v = 1.0 * sumk / diverges - delta;
            }

            color = 180.0 + 180.0*sin(log(log(v+3)+3)*2 +5.57);
            color = hue;
            bright = .25 + .25* sin(log(log(v+3)+3)*16);
            
            /* Darken it based on how many subsamples diverged */
            bright *= (1.0*diverges / (samples * samples) );

            wheel( color,1, bright, result + i*3);

        
        }
        
        res->line = j;
        res->bytes = result;
        heap_push(&hp, res);
        
        pthread_mutex_lock(&linelock);
        j = writeline++;
        pthread_mutex_unlock(&linelock);
    
    }

    return NULL;
}

void * runnerold(void * argv)
{
    /*struct ThreadArgs * ta = argv;*/
    int i, j;
    
    /*for(j = ta->start; j < dimy; j+=tcount)*/
    
    pthread_mutex_lock(&linelock);
    j = writeline++;
    pthread_mutex_unlock(&linelock);

    while(j < dimy)
    {
        unsigned char * result;
        struct Result * res;
        
        if( !(result = malloc(sizeof(unsigned char ) * 3 * dimx) ) )
        {
            fprintf(stderr, "Out of memory in runner\n");
            exit(1);
        }
        if( !(res = malloc(sizeof(struct Result)) ) )
        {
            fprintf(stderr, "Out of memory in runner\n");
            exit(1);
        }
        
        for(i = 0; i < dimx; i++)
        {
            PRECISION x, y, deltax, deltay, magsum;
            unsigned int k, m, n, summa, converges;
            Complex total, num, power;
            
            x = centerx - rangex / 2 + rangex * i / dimx;
            y = centery - rangey / 2 + rangey * j / dimy;
            
            
            cpl_init(&num, x, y);
            cpl_init(&total, 0, 0);
            cpl_init(&power, 2, 0);

            deltax = rangex / dimx / samples;
            deltay = rangey / dimy / samples;
            
            summa = 0;
            converges = 0;
            magsum = 0.0;
            /*
            {
                double p;
                p = sqrt( ( x - .25)*( x - .25)+y*y );
                if(x < p - 2*p*p + .25)
                {
                    result[i*3] = 0;
                    result[i*3+1] =0;
                    result[i*3+2] =0;
                    continue;
                }
                if( (x+1)*(x+1) + y*y < .0625)
                {
                    result[i*3] = 0;
                    result[i*3+1] =0;
                    result[i*3+2] =0;
                    continue;
                }
                
            }*/
            
            
            
            for(m = 0; m < samples; m++)
            {
                for(n = 0; n < samples; n++)
                {
                    num.real = x + n * deltax;
                    num.imag = y + m * deltay; 
                    total.real = num.real;
                    total.imag = num.imag;
                    if(isjulia)
                    {
                        num.real = juliar;
                        num.imag = juliai;
                    }
                    
                    
                    for(k = 0; k < itercount; k++)
                    {
                        PRECISION tr, ti;
                        
                        
                        tr = total.real * total.real - total.imag * total.imag + num.real;
                        ti = total.real * total.imag * 2 + num.imag;
                        total.real = tr; total.imag = ti;

                        if( total.real*total.real + total.imag*total.imag >= 256) /* changing the *  to + for either of these creates bubble shapes */
                        {
                            magsum += sqrt(total.real*total.real + total.imag*total.imag);
                            break;
                        }

                    }
                    
                    if( k == itercount)
                    {
                        
                        converges += 1;
                    }

                        summa += k;

                }

                
                
            }

            k = (int)(1.0 * summa / (samples * samples) );
            if(k == itercount)
            {

                result[i*3] = 0;
                result[i*3+1] =0;
                result[i*3+2] =0;
            }
            else
            {
                double color;
                double bright;
                PRECISION v;
                
                /* this is a hack to make AA and smoothing to work. */
                if( summa % (samples * samples) )
                {
                    v = k;
                }   
                else
                {
                    PRECISION delta;
                
                    magsum /= (samples-converges) * (samples-converges);
                    
                    
                    delta = log(log(magsum ) / log(16) ) / log(2);
                    /*
                    if( delta < -.1 || delta > 1.1)
                    {
                        printf("Uh OH: %f\n", delta);
                    }
                    assert(delta >= -.1);
                    assert(delta <= 1.1);*/
                    v = 1.0*summa / (samples * samples) - delta;
                }


                
                
                /*
                pthread_mutex_lock(&linelock);
                if(j == 25 && i < 60)
                printf("Converges: i: %d v: %f k: %d summa %d, norm %f, mag: %f\n", i, v, k, summa, 1.0*summa / (samples * samples),magsum  );
                pthread_mutex_unlock(&linelock);
                */
                color = hue/* + 90 * sin(log(log(v+3)+3)*2 +5.57)*/;
                
                bright = .25 + .25* sin(log(log(v+3)+3)*16);

                /*bright *= 1.0 * 1- (converges / (samples * samples) );*/

                /*bright = 0.5;*/

                wheel( color,1, bright, result + i*3);
            }
        }
        

        res->line = j;
        res->bytes = result;
        heap_push(&hp, res);
        
        pthread_mutex_lock(&linelock);
        j = writeline++;
        pthread_mutex_unlock(&linelock);
        
    }

    
    
    return NULL;
}


int compare(const void * a, const void * b)
{
    const struct Result * aa, * bb;
    aa = a, bb = b;
    if( aa->line > bb->line)
    {
        return 1;
    }
    if( aa->line < bb->line)
    {
        return -1;
    }
    return 0;
}


void * reaper(void * argv)
{
    int currentline = 0;
    struct Result * res;
    struct Result cur;
   
    
    while( currentline < dimy)
    {
        cur.line = currentline;
        cur.bytes = NULL;
        /*res = list_getmin(&ls, compare, &cur);*/
        heap_wait(&hp, &cur);
        
        res = heap_peek(&hp);

        if(res->line == currentline)
        {
            
            /*fwrite(res->bytes, 1, 3 * dimx, out);*/
            
            imagewriteline(res->bytes);
            
            free(res->bytes);
            heap_pop(&hp);
            free(res);
            currentline++;
            if((currentline & 0xF) == 0)
            {
                fprintf(stderr, " %0.2f%% Q->%d        \r",
                    100.0 * currentline / dimy, heap_size(&hp) );
                fflush(stderr);
            }
        }
    }
    
    fprintf(stderr, " 100.00%% Q->%d        \n", heap_size(&hp) );
    
    
    return NULL;
}






void printhelp(void)
{
    printf("Usage: fractal [parameters] <output.png>\n");
    printf("Parameters:\n");
    printf("  --julia <real> <imag>\n");
    printf("  --center <real> <imag>\n");
    printf("  --range <real> <imag>\n");
    printf("  --dim <x> <y>\n");
    printf("  --threads <num>\n");
    printf("  --iter <num>\n");
    printf("  --samples <num>\n");
    printf("  --help\n");
    printf("\n");

    
}


int main(int argc, char ** argv)
{
    pthread_t * threads;
    struct ThreadArgs * ta;
    int i;
    
    char * outfile = "out.png";
    tcount = 4;
    centerx = 0;
    centery = 0;
    rangex = 3.52;
    rangey = 2.2;
    dimx = 1920;
    dimy = 1200;
    itercount = 1024;
    samples = 1;
    bail = 16;
    bail2 = bail * bail;
    hue = 330;
    
    
    for(i = 1; i < argc; i++)
    {
        if( strcmp("--julia", argv[i]) == 0 ) 
        {
            if(i > argc - 3)
            {
                fprintf(stderr, "Invalid number of args for --julia\n");
                exit(EXIT_FAILURE);
            }
            if( sscanf(argv[i+1], "%lf", &juliar) != 1)
            {
                fprintf(stderr, "Invalid real arg for --julia\n");
                exit(EXIT_FAILURE);
            }
            if( sscanf(argv[i+2], "%lf", &juliai) != 1)
            {
                fprintf(stderr, "Invalid imag arg for --julia\n");
                exit(EXIT_FAILURE);
            }
            i += 2;
            isjulia = 1;


        }
        else if( strcmp("--center", argv[i]) == 0)
        {
            if(i > argc - 3)
            {
                fprintf(stderr, "Invalid number of args for --center\n");
                exit(EXIT_FAILURE);
            }
            if( sscanf(argv[i+1], "%lf", &centerx) != 1)
            {
                fprintf(stderr, "Invalid x arg for --center\n");
                exit(EXIT_FAILURE);
            }
            if( sscanf(argv[i+2], "%lf", &centery) != 1)
            {
                fprintf(stderr, "Invalid y arg for --center\n");
                exit(EXIT_FAILURE);
            }
            i += 2;

        }
        else if( strcmp("--range", argv[i]) == 0)
        {
            if(i > argc - 3)
            {
                fprintf(stderr, "Invalid number of args for --range\n");
                exit(EXIT_FAILURE);
            }
            if( sscanf(argv[i+1], "%lf", &rangex) != 1)
            {
                fprintf(stderr, "Invalid x arg for --range\n");
                exit(EXIT_FAILURE);
            }
            if( sscanf(argv[i+2], "%lf", &rangey) != 1)
            {
                fprintf(stderr, "Invalid y arg for --range\n");
                exit(EXIT_FAILURE);
            }
            i += 2;

        }
        else if( strcmp("--dim", argv[i]) == 0)
        {
            if(i > argc - 3)
            {
                fprintf(stderr, "Invalid number of args for --dim\n");
                exit(EXIT_FAILURE);
            }
            if( sscanf(argv[i+1], "%u", &dimx) != 1)
            {
                fprintf(stderr, "Invalid x arg for --dim\n");
                exit(EXIT_FAILURE);
            }
            if( sscanf(argv[i+2], "%u", &dimy) != 1)
            {
                fprintf(stderr, "Invalid y arg for --dim\n");
                exit(EXIT_FAILURE);
            }
            i += 2;
        }
        else if( strcmp("--threads", argv[i]) == 0)
        {
            if( sscanf(argv[i+1], "%u", &tcount) != 1)
            {
                fprintf(stderr, "Invalid thread count \"%s\"\n", argv[i+1]);
                exit(EXIT_FAILURE);
            }
            i += 1;
        }
        else if( strcmp("--iter", argv[i]) == 0)
        {
            if( sscanf(argv[i+1], "%u", &itercount) != 1)
            {
                fprintf(stderr, "Invalid iteration count \"%s\"\n", argv[i+1]);
                exit(EXIT_FAILURE);
            }
            i += 1;
        }
        else if( strcmp("--samples", argv[i]) == 0)
        {
            if( sscanf(argv[i+1], "%u", &samples) != 1)
            {
                fprintf(stderr, "Invalid sample count \"%s\"\n", argv[i+1]);
                exit(EXIT_FAILURE);
            }
            i += 1;
        }
        else if( strcmp("--hue", argv[i]) == 0)
        {
            if( sscanf(argv[i+1], "%lf", &hue) != 1)
            {
                fprintf(stderr, "Invalid hue \"%s\"\n", argv[i+1]);
                exit(EXIT_FAILURE);
            }
            i += 1;
        }
        else if( strcmp("--help", argv[i]) == 0)
        {
            printhelp();
            exit(EXIT_SUCCESS);
        }
        else
        {
            outfile = argv[i];
        }
    }
    
    
    printf("Running with configuration: \n");
    if(isjulia)
    {
        printf("  Generating a Julia fractal from seed (%f, %fj)\n", juliar, juliai);
    }
    else
    {
        printf("  Generating a Mandelbrot fractal\n");
    }
    printf("  Fractal centered at (%f, %fj) with range (%f, %f)\n", centerx, centery,
        rangex, rangey);
    printf("  Iteration count is %u at %ux antialiasing\n", itercount, 
        samples * samples);
    printf("  Outputing PNG Image (%ux%u) to \"%s\"\n", dimx, dimy, outfile);
    printf("  Thread count: %u\n", tcount);

    
    
    heap_init(&hp, compare);
    pthread_mutex_init(&linelock, NULL);

    
    if( !(ta = malloc(sizeof(struct ThreadArgs) * (tcount + 1) ) ) )
    {
        fprintf(stderr, "No memory in main\n");
        exit(1);
    }
    
    if( !(threads = malloc(sizeof(pthread_t) * tcount) ) )
    {
        fprintf(stderr, "No memory in main\n");
        exit(1);
    }

    
    
    imageopen(outfile, dimx, dimy);
    
    for(i = 0; i < tcount; i++)
    {
        ta[i].start = i;
        pthread_create(&threads[i], NULL, runner, &ta[i]);
    }
    
    pthread_create(&threads[tcount], NULL, reaper, NULL);
    
    for(i = 0; i <tcount; i++)
    {
        pthread_join(threads[i], NULL);
    }
    pthread_join(threads[tcount], NULL);
    
    imageclose();
    
    return 0;
}