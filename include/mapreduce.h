/**
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

#ifndef GRPPI_MAPREDUCE_H 
#define GRPPI_MAPREDUCE_H

#include <utility>

#include "common/execution_traits.h"
#include "common/iterator_traits.h"

namespace grppi {

/**
\addtogroup data_patterns
@{
\defgroup mapreduce_pattern Map/reduce pattern
\brief Interface for applyinng the \ref md_map-reduce.
@{
*/

/**
\brief Invoke \ref md_map-reduce on a data sequence.
\tparam Execution Execution type.
\tparam InputIterator Iterator type used for the input sequence.
\tparam Identity Type for the identity value.
\tparam Transformer Callable type for the transformation operation.
\tparam Combiner Callable type for the combination operation of the reduction.
\param ex Execution policy object.
\param first Iterator to the first element in the input sequence.
\param last Iterator to one past the end of the input sequence.
\param identity Identity value for the combination operation.
\param transf_op Transformation operation.
\param combine_op Combination operation.
\return Result of the map/reduce operation.
*/
template <typename Execution, typename InputIterator, typename Identity, 
          typename Transformer, typename Combiner,
          requires_iterator<InputIterator> = 0>
auto map_reduce(const Execution & ex, 
                InputIterator first, InputIterator last, 
                Identity && identity, 
                Transformer &&  transform_op, Combiner && combine_op)
{
  static_assert(supports_map_reduce<Execution>(),
    "map/reduce not supported on execution type");
  return ex.map_reduce(make_tuple(first), std::distance(first,last), 
      std::forward<Identity>(identity),
      std::forward<Transformer>(transform_op), 
      std::forward<Combiner>(combine_op));
}

/**
\brief Invoke \ref md_map-reduce on multiple data sequences. 
\tparam Execution Execution type.
\tparam InputIterator Iterator type used for the input sequence.
\tparam Identity Type for the identity value.
\tparam Transformer Callable type for the transformation operation.
\tparam Combiner Callable type for the combination operation of the reduction.
\param ex Execution policy object.
\param first Iterator to the first element in the input sequence.
\param last Iterator to one past the end of the input sequence.
\param identity Identity value for the combination operation.
\param transf_op Transformation operation.
\param combine_op Combination operation.
\return Result of the map/reduce operation.
*/
template <typename Execution, typename InputIterator, typename Identity, 
          typename Transformer, typename Combiner,
          typename ... OtherInputIterators,
          requires_iterator<InputIterator> = 0>
auto map_reduce(const Execution & ex, 
                InputIterator first, InputIterator last, 
                Identity && identity, 
                Transformer &&  transform_op, Combiner && combine_op,
                OtherInputIterators ... other_firsts)
{
  static_assert(supports_map_reduce<Execution>(),
    "map/reduce not supported on execution type");
  return ex.map_reduce(make_tuple(first, other_firsts...), 
      std::distance(first,last), 
      std::forward<Identity>(identity),
      std::forward<Transformer>(transform_op), 
      std::forward<Combiner>(combine_op));
}

/**
@}
@}
*/

}

#endif
