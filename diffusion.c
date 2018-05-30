#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#include <string.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#define TIME_STEP 2
#define TRUE 1


/* global variable*/
float **data[TIME_STEP];
int NX=8192,NY=8192; 
/* global nx,ny. can set it from commandline*/

int LNY; /* local ny*/
int start, end; /* global index */
int rank, size; /* rank_index, rank_size */
int NT=40; /* number of time steps default. can set it from commandline*/

float *receive[2]; 
/* receive data. receive[0] for rank-1 proc. receive[1] for rank+1 proc.*/

float *send[2];
/* send data.  send[0] for rank-1 proc. send[0] for rank+1 proc.*/

float **result[TIME_STEP];
/* NX*NY matrix for log result. After calculation,gahter local data here*/

void print_matrix(char* header, float** matrix, int ny, int nx){
  if(rank) return;
  int i, j;
  printf("%s\n", header);
  for(i = 0; i < ny; i++){
    for(j = 0; j < nx ;j++){
      printf("%.3f ", matrix[i][j]);
    }
    printf("\n");
  }
  printf("\n");
  fflush(0);
}

float** matrix_malloc(int nx, int ny){
  /** must free memory outer */
  int i;
  float** ret = malloc(sizeof(float*)*ny);
  for( i = 0; i < ny; i++){
      ret[i] = (float *)malloc(sizeof(float)*nx);
  }
  return ret;
}


void data_malloc(int nx, int ny, float*** data){
  int i;
  for(i = 0; i < TIME_STEP; i++){
    data[i] = matrix_malloc(nx, ny);
  }
}

void set_receive(int nx, int start, int end , int bottom){
  receive[0] = NULL;
  receive[1] = NULL;
  if(start && LNY){
    /* not (top or flat) */
    receive[0] = (float *)malloc(sizeof(float)*nx);
  }
  if(end < bottom){
    receive[1] = (float *)malloc(sizeof(float)*nx);
  }
  /* receive[x] == NULL means  not communicating with other proc */
}

int divide(int rank,int nprocs, int nrows, int *start, int *end){
  int length = (nrows - 1)/nprocs+1; /* need: nrows >= nprocs */
  *start = length * rank;
  *end = length * (rank+1); /* end is start of rank+1 */
  if(*start >= nrows) *start = nrows;
  if(*end >= nrows) *end = nrows;
  return length;
}

void gather_data(){
  int i,j,r;
  for(r = 0; r < size; r++){
    for(i = 0; i < TIME_STEP; i++){
      int lstart, lend;
      int lny = divide(r, size, NY, &lstart, &lend);/* local ny on rank'r'*/
      int count = 0;
      MPI_Request req[lny];
      MPI_Status stat[lny];
      if(rank == r){
        /* rank'r' send data to rank0  non-blocking way*/
        for(j = lstart; j < lend; j++){
            MPI_Request request;
            int lindex = j - lstart;
            MPI_Isend(data[i][lindex], NX, MPI_FLOAT, 0, j, MPI_COMM_WORLD, &request);
        }
      }

      if(rank == 0){
        /* rank'0' receive data from rank'r'  non-blocking way*/
        for(j = lstart; j < lend; j++){
            MPI_Irecv(result[i][j], NX, MPI_FLOAT, r, j, MPI_COMM_WORLD, &req[count++]);
            MPI_Waitall(count, req, stat);
        }
      }
    }
  }
}

void write_data(char* filename,float **data,int nx,int ny,char *mode){
  int i,j;
  FILE *fp;
  char *dir=dirname(strdup(filename));
  struct stat st;

  if (stat(dir,&st) != 0 &&
      mkdir(dir,0777) != 0){
    perror("mkdir error\n");
    exit(1);
  }

  if((fp = fopen(filename,mode))==NULL){
    perror("cannot write data\n");
    exit(1);
  }
  for(i=0;i<ny;i++){
    for(j=0;j<nx;j++){
      fprintf(fp,"%4.3f ",data[i][j]);
    }
    fprintf(fp,"\n");
  }
  fprintf(fp,"\n");
  fclose(fp);
}

/* in microseconds (us) */
double get_elapsed_time(struct timeval *begin, struct timeval *end)
{
    return (end->tv_sec - begin->tv_sec) * 1000000
            + (end->tv_usec - begin->tv_usec);
}

void init()
{
  int x, y;
  int cx = NX/2, cy = 0; /* center of ink */
  int rad = (NX+NY)/8; /* radius of ink */

  for(y = start; y < end; y++) {
    for(x = 0; x < NX; x++) {
      float v = 0.0;
      if (((x-cx)*(x-cx)+(y-cy)*(y-cy)) < rad*rad) {
	v = 1.0;
      }
      data[0][y - start][x] = v;
      data[1][y - start][x] = v;
    }
  }
  return;
}

void local_calc(int t){
  /* calculate only using local memory */
  int x, y;
  int from = t%2;
  int to = (t+1)%2;
  for (y = 1; y < LNY-1; y++) {
    for (x = 1; x < NX-1; x++) {
data[to][y][x] = 0.2 * (data[from][y][x]
      + data[from][y][x-1]
      + data[from][y][x+1]
      + data[from][y-1][x]
      + data[from][y+1][x]);
    }
  }
}

void global_calc(int t){
  /* calculate row'0' and row'LNY-1' using received data*/
  int from = t%2;
  int to = (t+1)%2;
  int x,y,i;
  int index_array[2] = {0, LNY-1};
  float* up;
  float* down;
  for(i = 0; i < 2; i++){
    y = index_array[i];
    if(receive[0] == NULL && y == 0 ||
        receive[1] == NULL && y == LNY - 1){
      /** global row'0' and row'NY' is constant */
      continue;
    }

    if(y == 0){
      up = receive[0]; /* use receive_data from rank'r-1' */
      down = data[from][y+1]; /* use local data */
    }
    if(y == LNY-1){
      up = data[from][y-1];  /* use receive_data from rank'r+1' */
      down = receive[1];
    }
    for (x = 1; x < NX - 1; x++){
      data[to][y][x] = 0.2 * (up[x]+
            + data[from][y][x-1]
            + data[from][y][x+1]
            + data[from][y][x]
            + down[x]);
    }
  }
}

void go_step(int step){
  MPI_Request req[4];
	MPI_Status stat[4];
  int count = 0;
  int from = step % 2;
  if(receive[0] != NULL){
    send[0] = data[from][0];
    MPI_Isend(send[0], NX, MPI_FLOAT, rank-1, step, MPI_COMM_WORLD, &req[count++]);
    MPI_Irecv(receive[0], NX, MPI_FLOAT, rank-1, step, MPI_COMM_WORLD, &req[count++]);
  }
  if(receive[1] != NULL){
    send[1] = data[from][LNY-1];
    MPI_Isend(send[1], NX, MPI_FLOAT, rank+1, step, MPI_COMM_WORLD, &req[count++]);
    MPI_Irecv(receive[1], NX, MPI_FLOAT, rank+1, step, MPI_COMM_WORLD, &req[count++]);
  }
  local_calc(step);
  MPI_Waitall(count, req, stat);
  global_calc(step);
}

int  main(int argc, char *argv[])
{
  struct timeval t1, t2;
  int log=TRUE-TRUE; /* log data for answer check or not */
  int opt;
  int i;

  while((opt=getopt(argc,argv,"::lx:y:t:")) != -1){
    switch (opt) {
      case 'l':
        log=TRUE;
        break;
      case 'x':
        NX=atoi(optarg);
        break;
      case 'y':
        NY=atoi(optarg);
        break;
      case 't':
        NT=atoi(optarg);
        break;
      case ':':
        perror("need option argment\n");
        exit(1);
    }
  }

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  divide(rank, size, NY, &start, &end);
  LNY = end - start;

  data_malloc(NX,LNY,data);
  set_receive(NX, start, end, NY);
  init();
  MPI_Barrier(MPI_COMM_WORLD);
  /* start */
  gettimeofday(&t1, NULL);
  for(i = 0; i < NT; i++){
    go_step(i);
    MPI_Barrier(MPI_COMM_WORLD);
  }
  /* end */
  gettimeofday(&t2, NULL);

  if(rank == 0){
      double us;
      double gflops;
      int op_per_point = 5; 
      us = get_elapsed_time(&t1, &t2);
      gflops = ((double)NX*NY*NT*op_per_point)/us/1000.0;
      printf("Size,Name,NX,NY,Elapsed time(sec),Speed(GFlops)\n");
      printf("%d, %s, %d, %d, %.3lf , %.3lf\n", size,argv[0],NX,NY,us/1000000.0,gflops);
      fflush(0);
  }

  if(log){
    char filename[100];
    int i;
    sprintf(filename,"%s%s_%d_%d_%d_%ds","output/",basename(strdup(argv[0])),NT,NX,NY,size);
    for(i = 0; i < TIME_STEP; i++){
       data_malloc(NX,NY,result);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    gather_data();
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0){
      write_data(filename,result[0],NX,NY,"w");
      write_data(filename,result[1],NX,NY,"a");
      print_matrix("result:",result[0], NY, NX);
      print_matrix("result:",result[1], NY, NX);
      printf("log suceed\n");
    }
  }
  MPI_Finalize();
  return 0;
}

