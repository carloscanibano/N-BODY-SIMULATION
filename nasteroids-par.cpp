#include <iostream>
#include <stdio.h>
#include <random>
#include "SpaceObject.h"
#include <fstream>
#include "math_functions.cpp"
using namespace std;
#include <iomanip>
#include <omp.h>
#include <cstring>

double start = omp_get_wtime();

int num_asteroids;
int num_iterations;
int num_planets;
int seed;
SpaceObject * asteroid;
SpaceObject * planet;
vector<SpaceObject> asteroids;
vector<SpaceObject> planets;

int main(int argc, char *argv[]) {
  //Check input conditions
  if(argc == 5){
    num_asteroids = atoi(argv[1]);
    num_iterations = atoi(argv[2]);
    num_planets = atoi(argv[3]);
    seed = atoi(argv[4]);
    if(num_asteroids<0 || num_iterations<0 || num_planets<0 || seed<0){
      cout << "nasteroids-seq: Wrong arguments.\nCorrect use:\nnasteroid-seq num_asteroids num_iterations num_planets seed\n";
      return -1;
    }
  }else{
    cout << "nasteroids-seq: Wrong arguments.\nCorrect use:\nnasteroid-seq num_asteroids num_iterations num_planets seed\n";
    return -1;
  }

  //Random distributions
  default_random_engine re{seed};
  uniform_real_distribution<double> xdist{0.0, std::nextafter(width,
	                                    	std::numeric_limits<double>::max())};
  uniform_real_distribution<double> ydist{0.0, std::nextafter(height,
	                                        std::numeric_limits<double>::max())};
  normal_distribution<double> mdist{mass, sdm};

  vector<double> forceX (num_asteroids, 0);
  vector<double> forceY (num_asteroids, 0);
  vector<vector<double>> each_asteroid_forcesX (num_asteroids, vector<double> (num_asteroids, 0));
  vector<vector<double>> each_asteroid_forcesY (num_asteroids, vector<double> (num_asteroids, 0));
  vector<vector<double>> each_asteroid_planet_forcesX (num_asteroids, vector<double> (num_planets, 0));
  vector<vector<double>> each_asteroid_planet_forcesY (num_asteroids, vector<double> (num_planets, 0));
  vector<double> asteroid_forces (2, 0);
  vector<double> planet_forces (2, 0);
  int l, k;

  //Creating asteroids...
  for(int i = 0; i < num_asteroids; i++){
    asteroid = new SpaceObject(xdist(re), ydist(re), 0.0, 0.0, mdist(re));
    asteroids.push_back(*asteroid);
    delete asteroid;
  }

  //Creating planets...
  for(int i = 0; i < num_planets; i++){
    //Clockwise planet placement...
    if(i % 4 == 0){
      planet = new SpaceObject(0, ydist(re), 0.0, 0.0, mdist(re) * 10);
    } else if(i % 4 == 1) {
      planet = new SpaceObject(xdist(re), 0, 0.0, 0.0, mdist(re) * 10);
    } else if(i % 4 == 2){
      planet = new SpaceObject(width, ydist(re), 0.0, 0.0, mdist(re) * 10);
    } else if(i % 4 == 3){
      planet = new SpaceObject(xdist(re), height, 0.0, 0.0, mdist(re) * 10);
    }
    planets.push_back(*planet);
    delete planet;
  }

  //Generating init_conf_txt
  ofstream init;
  init.open("init_conf.txt");
  //Initial configuration
  init << num_asteroids << " " << num_iterations << " " << num_planets << " " << seed << "\n";
  //Asteroids'data
  for(auto &asteroids: asteroids){
    init << fixed << setprecision(3)  << asteroids.x << " " << asteroids.y << " " << asteroids.m << "\n";
  }
  //Planets'data
  for(auto &planets: planets){
    init << fixed << setprecision(3) << planets.x << " " << planets.y << " " << planets.m << "\n";
  }
  init.close();

  //Number of iterations
    for(int i = 0; i < num_iterations; i++){
      fill(forceX.begin(), forceX.end(), 0);
      fill(forceY.begin(), forceY.end(), 0);
      fill(each_asteroid_forcesX.begin(), each_asteroid_forcesX.end(), vector<double>(num_asteroids, 0));
      fill(each_asteroid_forcesY.begin(), each_asteroid_forcesY.end(), vector<double>(num_asteroids, 0));
      fill(each_asteroid_planet_forcesX.begin(), each_asteroid_planet_forcesX.end(), vector<double>(num_planets, 0));
      fill(each_asteroid_planet_forcesY.begin(), each_asteroid_planet_forcesY.end(), vector<double>(num_planets, 0));

      #pragma omp parallel for
      for(int j = 0; j < num_asteroids; j++){
        asteroids[j].before_vx = asteroids[j].vx;
	      asteroids[j].before_vy = asteroids[j].vy;
	      asteroids[j].before_x = asteroids[j].x;
	      asteroids[j].before_y = asteroids[j].y;
      }

      #pragma omp parallel for private(asteroid_forces, k)  collapse(2)
      for(int j = 0; j < num_asteroids; j++){
        for(k = 0; k < num_asteroids; k++){
    	    if((j != k) && (j < k)){
      	    if(distance_between_elements(asteroids[j], asteroids[k]) > 2){
      	      asteroid_forces = normal_movement(asteroids[j], asteroids[k]);
      	      each_asteroid_forcesX[j][k] += asteroid_forces[0];
      	      each_asteroid_forcesY[j][k] += asteroid_forces[1];
      	      each_asteroid_forcesX[k][j] += (asteroid_forces[0] * -1);
      	      each_asteroid_forcesY[k][j] += (asteroid_forces[1] * -1);
      	    }
    	    }
	      }
      }

      #pragma omp parallel for private(k) collapse(2)
      for(int j = 0; j < num_asteroids; j++){
        for(k = 0; k < num_asteroids; k++){
	        forceX[j] += each_asteroid_forcesX[j][k];
	        forceY[j] += each_asteroid_forcesY[j][k];
	      }
      }

      //Fixing one asteroid to compare itself to others asteroids/planets
      #pragma omp parallel for private(planet_forces, l) collapse(2)
      for(int j = 0; j < num_asteroids; j++){
        //Planet forces
	       for(l = 0; l < num_planets; l++){
          planet_forces = normal_movement(asteroids[j], planets[l]);
          each_asteroid_planet_forcesX[j][l] += planet_forces[0];
	        each_asteroid_planet_forcesY[j][l] += planet_forces[1];
	       }
      }

      #pragma omp parallel for private(l) collapse(2)
      for(int j = 0; j < num_asteroids; j++){
        for(l = 0; l < num_planets; l++){
	        forceX[j] += each_asteroid_planet_forcesX[j][l];
	        forceY[j] += each_asteroid_planet_forcesY[j][l];
	      }
      }


      //Update coordinates and speeds
      #pragma omp parallel for
      for(int m = 0; m < num_asteroids; m++){
        change_element_position(&asteroids[m], forceX[m], forceY[m]);
      }

      //Possible collisions with edges
      #pragma omp parallel for
      for(int z = 0; z < num_asteroids; z++){
        edge_handling(&asteroids[z]);
      }

      //Possible collisions
      for(int n = 0; n < num_asteroids; n++){
        for(int o = 0; o < num_asteroids; o++){
	        if((n != o) && (n < o)){
	          if((distance_between_elements(asteroids[n], asteroids[o]) <= 2) && (i > 0)){
	            collision_handling(&asteroids[n], &asteroids[o]);
	            asteroids[n].collisions++;
	            asteroids[o].collisions++;
	          }
	        }
	      }
      }

      //If there are collisions position changes
      #pragma omp parallel for
      for(int p = 0; p < num_asteroids; p++){
        if(asteroids[p].collisions != 0){
	        asteroids[p].x = asteroids[p].before_x;
	        asteroids[p].y = asteroids[p].before_y;
	        change_element_position(&asteroids[p], forceX[p], forceY[p]);
	        asteroids[p].collisions = 0;
	      }
      }
    }

    //Generating out.txt
    ofstream out;
    out.open("out.txt");
    //Storing final data
    for(auto &asteroids: asteroids){
      out << fixed << setprecision(3) << asteroids.x << " " << asteroids.y << " " << asteroids.vx << " " << asteroids.vy << " " << asteroids.m << "\n";
    }
    out.close();

    double end = omp_get_wtime();
    printf("Time = %.16g\n", end - start);

    return 0;
}

