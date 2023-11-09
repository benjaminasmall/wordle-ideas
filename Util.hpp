#pragma once

#include <iostream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <functional>
#include <bit>
#include <vector>
#include <unordered_map>

using namespace std;

size_t binomial_coefficient(size_t n, size_t k) // n C k
{
  if ( k == 0 || n == 0 )
    return 1;
  if ( k > n )
    return 0;
  if ( k > n - k )
    return binomial_coefficient(n, n-k); // symmetry!
  // otherwise
  return ( n * binomial_coefficient(n-1, k-1) ) / k;
}

size_t generate_letter_masks ( vector<string> const& dict,
                               unordered_map<string, uint>& masks )
{
  masks.clear();
  uint mask;
  for ( string const& word : dict )
  {
    mask = 0;
    for ( char c : word )
    {
      mask |= (1 << (c-'a'));
    }
    masks.insert(make_pair(word, mask));
  }
  return masks.size();
}
