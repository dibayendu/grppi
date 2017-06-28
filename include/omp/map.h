/*
* @version		GrPPI v0.2
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

#ifndef GRPPI_MAP_OMP_H
#define GRPPI_MAP_OMP_H

#ifdef GRPPI_OMP

namespace grppi{

/**
\addtogroup map_pattern
@{
\addtogroup map_pattern_omp OpenMP parallel map pattern.
Implementation of map pattern for OpenMP parallel back-end.
@{
*/

/**
\brief Invoke [map pattern](@ref map-pattern) on a data sequence with OpenMP
parallel execution.
\tparam InputIt Iterator type used for input sequence.
\tparam OtuputIt Iterator type used for the output sequence.
\tparam Operation Callable type for the transformation operation.
\param ex Sequential execution policy object
\param first Iterator to the first element in the input sequence.
\param last Iterator to one past the end of the input sequence.
\param first_out Iterator to first elemento of the output sequence.
\param op Transformation operation.
*/
template <typename InputIt, typename OutputIt, typename Operation>
void map(parallel_execution_omp & ex, 
         InputIt first, InputIt last, 
         OutputIt first_out, 
         Operation && op)
{
  int numElements = last - first;

  int elemperthr = numElements/ex.num_threads;

  #pragma omp parallel
  {
   #pragma omp single nowait
   {
    for(int i=1;i<ex.num_threads;i++){
      


      #pragma omp task firstprivate(i)
      {
        auto begin = first + (elemperthr * i);
        auto end = first + (elemperthr * (i+1));
        if(i == ex.num_threads -1 ) end = last;
        auto out = first_out + (elemperthr * i);
        while(begin!=end){
          *out = op(*begin);
          begin++;
          out++;
        }
      }
     }
      //Map main threads
      auto beg =first;
      auto out = first_out;
      auto end = first+elemperthr;
      while(beg!=end){
            *out = op(*beg);
            beg++;
            out++;
      }
      #pragma omp taskwait
    }
  }
}

template <typename InputIt, typename OutputIt, typename ... MoreIn, typename Operation>
void internal_map(parallel_execution_omp & ex, 
                  InputIt first, InputIt last, 
                  OutputIt first_out,
                  Operation &&op, 
                  int i, 
                  int elemperthr, MoreIn ... inputs)
{
  //Calculate local input and output iterator 
  auto begin = first + (elemperthr * i);
  auto end = first + (elemperthr * (i+1));
  if( i == ex.num_threads-1) end = last;
  auto out = first_out + (elemperthr * i);
  advance_iterators(elemperthr*i, inputs ...);
  while(begin!=end){
    *out = op(*begin, *inputs ...);
    advance_iterators(inputs ...);
    begin++;
    out++;
  }
}

/**
\brief Invoke [map pattern](@ref map-pattern) on a data sequence with OpenMP
execution.
\tparam InputIt Iterator type used for input sequence.
\tparam OtuputIt Iterator type used for the output sequence.
\tparam Operation Callable type for the transformation operation.
\param ex Sequential execution policy object
\param first Iterator to the first element in the input sequence.
\param last Iterator to one past the end of the input sequence.
\param first_out Iterator to first elemento of the output sequence.
\param op Transformation operation.
\param more_firsts Additional iterators with first elements of additional sequences.
*/
template <typename InputIt, typename OutputIt, 
          typename ... MoreIn, typename Operation>
void map(parallel_execution_omp & ex, 
         InputIt first, InputIt last, 
         OutputIt first_out, 
         Operation && op, 
         MoreIn ... inputs)
{
  //Calculate number of elements per thread
  int numElements = last - first;
  int elemperthr = numElements/ex.num_threads;

  //Create tasks
  #pragma omp parallel
  {
    #pragma omp single nowait
    {
      for(int i=1;i<ex.num_threads;i++){

      #pragma omp task firstprivate(i)
      {
        internal_map(ex, first, last, first_out, std::forward<Operation>(op) , i, elemperthr, inputs ...);
      }
      //End task
     }

     //Map main thread
     internal_map(ex, first,last, first_out, std::forward<Operation>(op), 0, elemperthr, inputs ...);

    //Join threads
    #pragma omp taskwait
    }
  }
}

/**
@}
@}
*/
}

#endif

#endif
