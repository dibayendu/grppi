/**
* @version		GrPPI v0.1
* @copyright		Copyright (C) 2017 Universidad Carlos III de Madrid. All rights reserved.
* @license		GNU/GPL, see LICENSE.txt
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

#ifndef PPI_COMMON
#define PPI_COMMON

#include <atomic>

#include "is_iterator.hpp"
#include <omp.h>
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include "pool.hpp"

namespace grppi{
/** @brief Indicate the type of execution and the implementation frameworks */
struct execution_model {
  bool ordering = true;
  int num_threads;
  execution_model() : num_threads{} {
    num_threads = std::thread::hardware_concurrency();
  };
  execution_model(int _threads) : num_threads{_threads} {};
  public:
     bool lockfree = false;
  //virtual ~execution_model() = 0;
};
//execution_model::~execution_model() {};

/** @brief Set the execution mode to sequencial */
struct sequential_execution : execution_model {
  /** @brief set num_threads to 1 in order to sequential execution */
  sequential_execution() : execution_model{} {};
};

//extern bool initialised;
//extern thread_pool pool;
/** @brief Set the execution mode to parallel with posix thread implementation 
 */
struct parallel_execution_thr : execution_model {
  public: 
  thread_pool pool;
  
  int get_threadID(){
      while (lock.test_and_set(std::memory_order_acquire));
      auto it = std::find(thid_table.begin(), thid_table.end(), std::this_thread::get_id());
      auto id = std::distance(thid_table.begin(), it);
      lock.clear(std::memory_order_release);  
      return id; 
  }
  
  void register_thread(){
      while (lock.test_and_set(std::memory_order_acquire));
      thid_table.push_back(std::this_thread::get_id());    
      lock.clear(std::memory_order_release);  
  }
  
  void deregister_thread(){
      while (lock.test_and_set(std::memory_order_acquire));
      thid_table.erase(std::remove(thid_table.begin(), thid_table.end(),std::this_thread::get_id()), thid_table.end());
      lock.clear(std::memory_order_release);  
  }
  /** @brief Set num_threads to the maximum number of thread available by the
   *    hardware
   */
  parallel_execution_thr() : execution_model{} { /*if(!initialised)*/ pool.initialise(this->num_threads); };

  /** @brief Set num_threads to _threads in order to run in parallel
   *
   *  @param _threads number of threads used in the parallel mode
   */
  parallel_execution_thr(int _threads) : execution_model{_threads} { /*if(!initialised)*/ pool.initialise (_threads);};

  /** @brief Set num_threads to _threads in order to run in parallel and allows to disable the ordered execution
   *
   *  @param _threads number of threads used in the parallel mode
   *  @param _order enable or disable the ordered execution
   */
  parallel_execution_thr(int _threads, bool order) : execution_model{_threads} { ordering = order; /*if(!initialised)*/ pool.initialise (_threads);};
  private: 
     std::atomic_flag lock = ATOMIC_FLAG_INIT;
     std::vector<std::thread::id> thid_table;

};


/** @brief Set the execution mode to parallel with ompenmp framework 
 *    implementation 
 */
struct parallel_execution_omp : execution_model {
  int get_threadID(){
     return omp_get_thread_num();
  }
  /** @brief Set num_threads to the maximum number of thread available by the
   *    hardware
   */
  parallel_execution_omp() : execution_model{} {};

  /** @brief Set num_threads to _threads in order to run in parallel
   *
   *  @param _threads number of threads used in the parallel mode
   */
  parallel_execution_omp(int _threads) : execution_model{_threads} {};

  /** @brief Set num_threads to _threads in order to run in parallel and allows to disable the ordered execution
   *
   *  @param _threads number of threads used in the parallel mode
   *  @param _order enable or disable the ordered execution
   */
  parallel_execution_omp(int _threads, bool order) : execution_model{_threads} { ordering = order;};

};

/** @brief Set the execution mode to parallel with threading building blocks
 *    (TBB) framework implementation
 */
struct parallel_execution_tbb : execution_model {
  /** @brief Set num_threads to the maximum number of thread available by the
   *    hardware
   */
  parallel_execution_tbb() : execution_model{} {};

  /** @brief Set num_threads to _threads in order to run in parallel
   *
   *  @param _threads number of threads used in the parallel mode
   */
  parallel_execution_tbb(int _threads) : execution_model{_threads} {};
};


#ifdef __CUDACC__


template< typename Policy >
class parallel_execution_thrust_internal {
public:
   int num_gpus = 1;
   Policy policy;
   parallel_execution_thrust_internal(int _gpus, Policy _policy) : num_gpus{_gpus}, policy{_policy} {};
};

template<typename Policy>
parallel_execution_thrust_internal <Policy> parallel_execution_thrust ( int _gpus, Policy policy){
   return parallel_execution_thrust_internal<Policy>(_gpus, policy);
}

parallel_execution_thrust_internal<thrust::system::cuda::detail::par_t > parallel_execution_thrust(){
   return parallel_execution_thrust_internal<thrust::system::cuda::detail::par_t>(1, thrust::cuda::par);
}

parallel_execution_thrust_internal<thrust::system::cuda::detail::par_t > parallel_execution_thrust(int _gpus){
   return parallel_execution_thrust_internal<thrust::system::cuda::detail::par_t>(_gpus, thrust::cuda::par);
}
/*
auto parallel_execution_thrust() -> decltype(parallel_execution_thrust_internal(1, thrust::cuda::par))
{
    return parallel_execution_thrust_internal(1, thrust::cuda::par);
} 
*/
#endif

template <typename T>
class optional {
    public:
        typedef T type;
        typedef T value_type;
        T elem;
        bool end;
        optional(): end(true) { }
        optional(const T& i): elem(i), end(false) { }

        optional& operator=(const optional& o) {
                 elem = o.elem;
                 end = o.end;
                 return *this;
        }

        T& value(){ return elem; }

        constexpr explicit operator bool() const {
            return !end;
        }
};


template <typename InputIt>
void GetStart(int n, int tid, InputIt& in){
    in = in + (n*tid);
}

template <typename InputIt, typename ... MoreIn>
void GetStart(int n, int tid, InputIt& in, MoreIn ... inputs){
    in = in + (n*tid);
    GetStart(n,tid,inputs...);
}

//Update iterators
template <typename InputIt>
void NextInputs(InputIt &in){
   in++;
}

template <typename InputIt, typename ... MoreIn>
void NextInputs(InputIt &in, MoreIn ... inputs){
   in++;
   NextInputs(inputs...);
}


template <typename Stage, typename ... Stages>
class PipelineObj{
   public:
      std::tuple<Stage *, Stages *...> stages;
      
      PipelineObj(Stage s, Stages ... sts):stages(std::make_tuple(&s, &sts...)) {}
};

template <typename E,class TaskFunc, class RedFunc>
class ReduceObj
{
   public:
      TaskFunc * task;
      RedFunc * red;
      E exectype;
      ReduceObj(E s, TaskFunc farm, RedFunc r){exectype=s; task = &farm; red= &r;}
};

template <typename E,class TaskFunc>
class FarmObj
{
   public:
      TaskFunc * task;
      E * exectype;
      int farmtype;
      FarmObj(E &s,TaskFunc f){exectype=&s; task = &f;};


};

template <typename E,class TaskFunc>
class FilterObj
{
   public:
      TaskFunc * task;
      E *exectype;
      int filtertype;
      FilterObj(E& s,TaskFunc f){exectype=&s; task = &f;};
};

}

#endif