#ifndef RANDOM_PICK_H
#define RANDOM_PICK_H

#include <vector>
#include <map>

#include "Utility.h"

namespace rnd
{
  template <typename T>
  struct random_pick_entry_t
  {
    T value;
    int probability_weight;
  };

  namespace detail
  {
    template <typename T>
    int prop_sum(const std::vector< random_pick_entry_t<T> >& entries)
    {
      int sum = 0;
      for (auto it = entries.begin(); it != entries.end(); ++it)
      {
        sum += it->probability_weight;
      }
      return sum;
    }
  }

  template <typename T>
  T random_pick(const std::vector< random_pick_entry_t<T> >& entries)
  {
    std::map<int, T> values;

    int sum = detail::prop_sum(entries);

    int range_start = 0;
    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
      int range_end = range_start + (int)(((float)it->probability_weight / (float)sum) * 100.0f);

      values[range_end] = it->value;

      range_start = range_end + 1;
    }

    typename std::map<int, T>::iterator search_it;
    do
    {
      search_it = values.upper_bound(random_range(0, 100));
    } while (search_it == values.end());

    return search_it->second;
  }
}

#endif
