/*
   This example is free and distributed under
   Mozilla Public License Version 2.0.
*/

// cls && g++ so-styblinski-tang.cpp -o so-styblinski-tang.exe && so-styblinski-tang.exe

#include <cmath>
#include <string>
#include <vector>
#include <climits>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "openGA.hpp"

const int SPACE_SIZE = 5;
const int POPULATION_SIZE = 31;

struct MySolution {
  std::vector < double > x;

  std::string to_string() const {
    std::ostringstream out;
	
    out << "{";
    for (unsigned long i = 0; i < x.size(); i++) {
      out << (i ? "," : "") << std::setprecision(10) << x[i];
    }
    out << "}";
	
    return out.str();
  }
};

struct MyMiddleCost {
  // This is where the results of simulation is stored but not yet finalized.
  double cost;
};

typedef EA::Genetic < MySolution, MyMiddleCost > GA_Type;
typedef EA::GenerationType < MySolution, MyMiddleCost > Generation_Type;

void init_genes(MySolution & p, const std:: function < double(void) > & rnd01) {
  // Straight twin.
  for (int i = 0; i < SPACE_SIZE; i++) {
    p.x.push_back(INT_MAX * 2.0 * (rnd01() - 0.5));
  }
  // Inverted twin.
  for (int i = 0; i < SPACE_SIZE; i++) {
	  double real = p.x[i];
	  unsigned long long *integer = (unsigned long long *)&(real);
	  (*integer) = ~(*integer);
      p.x.push_back( real );
  }
}

bool eval_solution(const MySolution & p, MyMiddleCost & c) {
  double cost1, cost2;

  // DNA-insired pairs.
  cost1 = cost2 = p.x.size() / 2;
  for (int i = 0, j = p.x.size()/2; i < p.x.size()/2; i++, j++) {
    cost1 += p.x[i] * p.x[i] + sin(2.0 * M_PI * p.x[i]);
    cost2 += p.x[j] * p.x[j] + sin(2.0 * M_PI * p.x[j]);
  }
  
  c.cost = std::min(std::ceil(cost1), std::ceil(cost2));
  
  return true;
}

MySolution mutate(
  const MySolution & base, const std:: function < double(void) > & rnd01, double shrink_scale) {
  MySolution next = base;

  int index = rand() % (next.x.size() / 2);
  unsigned long long *regular = (unsigned long long *)&next.x[ index ];
  unsigned long long *inverted = (unsigned long long *)&next.x[ next.x.size() / 2 + index ];

  //Toggle bit and random postiton.
  unsigned long long mask = 1;
  mask <<= rand() % sizeof(mask);
  (*regular) ^= mask;
  (*inverted) ^= mask;
  
  return next;
}

MySolution crossover(
  const MySolution & A, const MySolution & B, const std:: function < double(void) > & rnd01) {
  MySolution next;
  
  for (unsigned long i = 0; i < A.x.size(); i++) {
    double r = rnd01();
    next.x.push_back(r * A.x[i] + (1.0 - r) * B.x[i]);
  }
  
  return next;
}

double calculate_SO_total_fitness(const GA_Type::thisChromosomeType & X) {
  // Finalize the cost.
  return X.middle_costs.cost;
}

std::ofstream output_file;

void SO_report_generation(
  int generation_number,
  const EA::GenerationType < MySolution, MyMiddleCost > & last_generation, const MySolution & best_genes) {
  std::cout <<
    "Generation [" << generation_number << "], " <<
    "Best=" << last_generation.best_total_cost << ", " <<
    "Average=" << last_generation.average_cost << ", " <<
    "Best genes=(" << best_genes.to_string() << ")" << ", " <<
    "Exe_time=" << last_generation.exe_time <<
    std::endl;

  output_file <<
    generation_number << "\t" <<
    last_generation.average_cost << "\t" <<
    last_generation.best_total_cost << "\t";
	
  for(int i=0; i<best_genes.x.size(); i++) {
    output_file << best_genes.x[i] << "\t";
  }
	
  output_file << std::endl;
}

int main() {
  srand(time(NULL));
	
  output_file.open("result-so-styblinski-tang.txt");
  
  output_file <<
    "step" << "\t" <<
    "cost_avg" << "\t" <<
    "cost_best" << "\t";
	
  for(int i=0; i<2*SPACE_SIZE; i++) {
    output_file << "best(" << (i+1) << ")" << "\t";
  }
  
  output_file << std::endl;

  EA::Chronometer timer;
  timer.tic();

  GA_Type ga_obj;
  
  ga_obj.problem_mode = EA::GA_MODE::SOGA;
  ga_obj.multi_threading = false;
  ga_obj.dynamic_threading = false;
  ga_obj.idle_delay_us = 0; // Switch between threads quickly.
  ga_obj.verbose = false;
  ga_obj.population = POPULATION_SIZE;
  ga_obj.generation_max = 1000;
  ga_obj.calculate_SO_total_fitness = calculate_SO_total_fitness;
  ga_obj.init_genes = init_genes;
  ga_obj.eval_solution = eval_solution;
  ga_obj.mutate = mutate;
  ga_obj.crossover = crossover;
  ga_obj.SO_report_generation = SO_report_generation;
  ga_obj.best_stall_max = 20;
  ga_obj.average_stall_max = 20;
  ga_obj.tol_stall_best = 1e-6;
  ga_obj.tol_stall_average = 1e-6;
  ga_obj.elite_count = 10;
  ga_obj.crossover_fraction = 0.7;
  ga_obj.mutation_rate = 0.1;
  
  ga_obj.solve();

  std::cout << "The problem is optimized in " << timer.toc() << " seconds." << std::endl;

  output_file.close();
  
  return EXIT_SUCCESS;
}
