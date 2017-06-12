/**
* @version    GrPPI v0.1
* @copyright    Copyright (C) 2017 Universidad Carlos III de Madrid. All rights reserved.
* @license    GNU/GPL, see LICENSE.txt
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You have received a copy of the GNU General Public License in LICENSE.txt
* also available in <http://www.gnu.org/licenses/gpl.html>.
*
* See COPYRIGHT.txt for copyright notices and details.
*/
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <string>
#include <numeric>
#include <stdexcept>

#include "common/polymorphic_execution.h"
#include "map.h"

template <typename I1, typename I2, typename F>
void mymap_helper(grppi::polymorphic_execution & e , I1 i1, I1 f1, I2 i2, F && f) {
}

template <typename E, typename ... O, typename I1, typename I2, typename F,
          std::enable_if_t<!grppi::is_supported<E>(),int> = 0>
void mymap_helper(grppi::polymorphic_execution & e , I1 i1, I1 f1, I2 i2, F && f) {
  mymap_helper<O...>(e, i1, f1, i2, f);
}

template <typename E, typename ... O, typename I1, typename I2, typename F,
          std::enable_if_t<grppi::is_supported<E>(),int> = 0>
void mymap_helper(grppi::polymorphic_execution & e , I1 i1, I1 f1, I2 i2, F && f) {
  if (typeid(E) == e.type()) {
    grppi::map(*e.execution_ptr<E>(), i1, f1, i2, f);
  }
  else
    mymap_helper<O...>(e, i1, f1, i2, f);
}

template <typename I1, typename I2, typename F>
void mymap(grppi::polymorphic_execution & e, I1 i1, I1 f1, I2 i2, F && f) {
  mymap_helper<
    grppi::sequential_execution,
    grppi::parallel_execution_thr,
    grppi::parallel_execution_omp,
    grppi::parallel_execution_tbb
  >(e, i1, f1, i2, f);
}


// EXAMPLE STARTS HERE

grppi::polymorphic_execution execution_mode(const std::string & opt) {
  using namespace grppi;
  if ("seq" == opt) 
    return make_polymorphic_execution<grppi::sequential_execution>();
  if ("thr" == opt) 
    return make_polymorphic_execution<grppi::parallel_execution_thr>();
  if ("omp" == opt) 
    return make_polymorphic_execution<grppi::parallel_execution_omp>();
  if ("tbb" == opt) 
    return make_polymorphic_execution<grppi::parallel_execution_tbb>();
  return polymorphic_execution{};
}

template <typename F, typename ...Args>
bool run_test(const string & mode, F && f, Args && ... args) {
  auto e = execution_mode(mode);
  if (e.has_execution()) {
    f(e, args...);
    return true;
  }
  return false;
}


void test_map(grppi::polymorphic_execution & e, int n) {
  using namespace std;

  vector<int> in(n);
  iota(in.begin(), in.end(), 0);

  vector<int> out(n);

  mymap(e, begin(in), end(in), begin(out),
    [](int i) { return i*2; });

  copy(begin(out), end(out), ostream_iterator<int>(cout, " "));
  cout << endl;
}

void print_message(const std::string & prog, const std::string & msg) {
  using namespace std;
  cerr << msg << endl;
  cerr << "Usage: " << prog << " size mode" << endl;
  cerr << "  size: Integer value with problem size" << endl;
  cerr << "  mode:" << endl;
  cerr << "    seq -> Sequential execution" << endl;
  cerr << "    thr -> ISO Threads backend" << endl;
  cerr << "    tbb -> Intel TBB backend" << endl;
  cerr << "    omp -> OpenMP backend" << endl;
}


int main(int argc, char **argv) {
    
  using namespace std;

  if(argc < 3){
    print_message(argv[0], "Invalid number of arguments.");
    return -1;
  }

  int n = stoi(argv[1]);
  if(n <= 0){
    print_message(argv[0], "Invalid problem size. Use a positive number.");
    return -1;
  }

  if (!run_test(argv[2], test_map, n)) {
    print_message(argv[0], "Invalid policy.");
    return -1;
  }

    return 0;
}
