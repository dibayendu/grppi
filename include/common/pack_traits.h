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

#ifndef GRPPI_COMMON_PACK_TRAITS_H
#define GRPPI_COMMON_PACK_TRAITS_H

namespace grppi {

namespace internal {

template <int Index, typename ... T>
using requires_index_last =
  std::enable_if_t<(Index == (sizeof...(T) - 1)), int>;

template <int Index, typename ... T>
using requires_index_not_last =
  std::enable_if_t<(Index < sizeof...(T) - 1), int>;

} // end namespace internal

} // end namespace grppi

#endif
