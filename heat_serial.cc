#include <cmath>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

typedef double **doubleArray;

doubleArray allocateArray(const int nx) {
  doubleArray T = new double *[nx];
  for (int i = 0; i < nx; i++) {
    T[i] = new double[nx];
  }
  return T;
}

int freeArray(doubleArray array, const int nx) {
  for (int i = 0; i < nx; i++) {
    delete[] array[i];
  }
  delete[] array;
  return 0;
}

int initializeTemperature(doubleArray T, const int nx, const double dx) {
  int i, j;
  // initial condition, boundary condition
  for (i = 0; i < nx; i++) {
    for (j = 0; j < nx; j++) {
      if (j == 0) {
        T[i][j] = cos(i * dx) * cos(i * dx);
      } else if (j == nx - 1) {
        T[i][j] = sin(i * dx) * sin(i * dx);
      } else {
        T[i][j] = 0;
      }
    }
  }
  return 0;
}

int updateTemperatureSerial(doubleArray current, doubleArray next,
                            const double k, const int nx, const double dx,
                            const double dt) {
  int i, j;
  double laplaceT = 0;
  // update inner points [0,nx-1], [1,nx-2]
  for (i = 0; i < nx; i++) {
    for (j = 1; j < nx - 1; j++) {
      if (i == 0) {
        // x=0
        laplaceT = (current[nx - 1][j] + current[i + 1][j] + current[i][j - 1] +
                    current[i][j + 1] - 4 * current[i][j]) /
                   (dx * dx);
      } else if (i == nx - 1) {
        // x= pi
        laplaceT = (current[i - 1][j] + current[0][j] + current[i][j - 1] +
                    current[i][j + 1] - 4 * current[i][j]) /
                   (dx * dx);
      } else {
        laplaceT = (current[i - 1][j] + current[i + 1][j] + current[i][j - 1] +
                    current[i][j + 1] - 4 * current[i][j]) /
                   (dx * dx);
      }
      // forward euler
      next[i][j] = current[i][j] + dt * k * laplaceT;
    }
  }
  return 0;
}

double averageTemperature(doubleArray T, const int nx) {
  double aveT;
  double sum = 0;
  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < nx; j++) {
      sum += T[i][j];
    }
  }
  aveT = sum / (nx * nx);
  return aveT;
}

int writeFile(string fileName, doubleArray T, const int nx) {
  int i, j;
  ofstream output(fileName);
  if (output.is_open()) {
    for (j = 0; j < nx; j++) {
      for (i = 0; i < nx; i++) {
        if (i < nx - 1) {
          output << T[i][j] << ", ";
        } else {
          output << T[i][j];
        }
      }
      output << "\n";
    }
    output.close();
  } else {
    cout << "Unable to open file!\n";
  }
  return 0;
}

int main(int argc, char *argv[]) {

  // time start
  clock_t start = clock();

  // input check
  if (argc != 2) {
    printf("USAGE: %s <nx>\n", argv[0]);
    exit(1);
  }
  const int nx = atoi(argv[1]);

  // set parameters for heat equation
  const double k = 1;
  const double PI = 2 * acos(0);
  const double t = 0.5 * PI * PI / k;
  const double dx = PI / nx;
  // for numerical stability, dt < dx^2/(4k)
  const double dt = 0.5 * dx * dx / (4 * k);
  const int nt = (int)t / dt;
  printf("k=%f, x=%f, t=%f, nx=%d, nt=%d, dx=%f, dt=%f\n", k, PI, t, nx, nt, dx,
         dt);

  // allocate memory
  doubleArray next = allocateArray(nx);
  doubleArray current = allocateArray(nx);

  // initialize Temperature
  initializeTemperature(next, nx, dx);
  initializeTemperature(current, nx, dx);

  // solve heat equation
  doubleArray ptrTemp;
  for (int n = 0; n < nt; n++) {
    updateTemperatureSerial(current, next, k, nx, dx, dt);
    // switch old and new Temperature
    ptrTemp = current;
    current = next;
    next = ptrTemp;
  }

  // average Temperature
  double aveTemp = averageTemperature(next, nx);
  printf("Average temperature = %.4f\n", aveTemp);

  // write final temperature to files
  char fileName[50];
  sprintf(fileName, "heat_serial_nx%d.dat", nx);
  writeFile(fileName, next, nx);

  // free memroy
  freeArray(current, nx);
  freeArray(next, nx);

  // time stop
  clock_t stop = clock();
  double elapsed = (double)(stop - start) / CLOCKS_PER_SEC;
  printf("nx = %d, time [seconds] = %.4f\n", nx, elapsed);

  // end the program
  return 0;
}
