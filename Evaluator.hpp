#include "Util.hpp"
#include "WordleEngine.hpp"
#include <cmath>
#include <map>
#include <limits>
#include <chrono>
#include <iomanip>

typedef pair<char, int>           mapkey_t;
typedef multimap<mapkey_t, sid_t> map_t;
typedef pair    <int, string>     score_t;
typedef multimap<int, string,
                 greater<int> >   score_table_t;

class Evaluator : public WordleEngine
{
protected:
  map_t         Map;
  string        Alphabet;
  size_t        Longest;

  int populate_multimap()
  {
    Map.clear();
    Alphabet.clear();
    Longest = 0;
    set<char> ab, cs;
    for ( sid_t i = 0 ; i < WinnablesEnd() ; ++i )
    {
      string const& word = IdToWordMap[i];
      size_t l = word.size();
      Longest = max(Longest, l);
      cs.clear();
      while (l>0)
      {
        char const c = word[--l];
        ab.insert ( c );
        cs.insert ( c );
        Map.insert ( make_pair ( make_pair(c, l        ) , i ) );
      }
      for ( char c : cs )
        Map.insert ( make_pair ( make_pair(c, ANYWHERE ) , i ) );
    }
    copy ( ab.begin(), ab.end(), back_inserter(Alphabet) );
    // all okay
    return 0;
  }

public:
  Evaluator ( list_t const& _w, list_t const& _g )
    : WordleEngine::WordleEngine(_w, _g)
  {
    populate_multimap();
  }

  Evaluator ( string const& wfile, string const& gfile )
    : WordleEngine::WordleEngine(wfile, gfile)
  {
    populate_multimap();
  }

  size_t count_greens(string const& word) const
  {
    size_t matches = 0;
    for ( size_t i = 0 ; i < word.size() ; ++i )
      matches += Map.count(make_pair(word[i], (int)i));
    return matches;
  }

  size_t count_yellows(string const& word) const
  {
    size_t matches = 0;
    for ( size_t i = 0 ; i < word.size() ; ++i )
      matches += Map.count(make_pair(word[i], ANYWHERE));
    return matches;
  }

  int score_candidates(score_table_t& scores) const
  {
    for ( size_t i = 0 ; i < End() ; ++i )
    {
      // drop redundancies
      if ( !UniqueChars(i) )
        continue;
      string const& key = IdToWordMap[i];
      scores.insert ( make_pair ( entropy(i), key) );
    }
    return (int)scores.size(); 
  }

  int score_candidates2(score_table_t& scores) const
  {
    constexpr char const delim = '-';
    string       key;
    size_t const max_status   = binomial_coefficient(End(), 2);
    size_t const status_delta = max_status / 100;
    size_t       next_status  = 100;
    size_t       counter      = 0;
    for ( size_t i = 0 ; i < End() ; ++i )
    {
      // drop redundancies
      if ( !UniqueChars(i) )
        continue;
      for ( size_t j = 0 ; j < i ; ++j, ++counter )
      {
        if ( counter >= next_status )
        {
          time_t t = time(nullptr);
          tm    tm = *localtime(&t);
          cerr << setw(10) << counter << " of " << setw(10) << max_status
               << " : " << put_time(&tm, "%T") << endl;
          next_status += status_delta;
        }
        // drop redundancies
        if ( !UniqueChars(j) )
          continue;
        if ( OverlappingChars(i,j) )
          continue;
        key = IdToWordMap[i] + delim + IdToWordMap[j];
        scores.insert ( make_pair ( coverage(i, j), key ) );
      }
    }
    return (int)scores.size(); 
  }

  int entropy(sid_t id) const
  {
    if ( id >= End() )
      return numeric_limits<int>::min();
    string const& word = IdToWordMap[id];
    set_t s;
    for ( size_t i = 0 ; i < word.size() ; ++i )
    {
      auto p = Map.equal_range ( make_pair(word[i],i) );
      transform ( p.first, p.second, inserter(s, s.end()),
                  [](auto i){ return i.second; } ); 
      p = Map.equal_range ( make_pair(word[i], ANYWHERE) );
      transform ( p.first, p.second, inserter(s, s.end()),
                  [](auto i){ return i.second; } ); 
    }
    double const ratio = (double)s.size() / (double)Winnables.size();
    // return int ( 1000. * log2 (ratio) );
    // return int ( 1000. * (1 - fabs(0.5-ratio)) );
    return int ( 1000. * ratio );
  }

  int coverage(sid_t ai, sid_t bi) const
  {
    string const& a = IdToWordMap[ai];
    string const& b = IdToWordMap[bi];
    set_t s;
    for ( size_t i = 0 ; i < a.size() ; ++i )
    {
      auto p = Map.equal_range ( make_pair(a[i],i) );
      transform ( p.first, p.second, inserter(s, s.end()),
                  [](auto i){ return i.second; } ); 
      p = Map.equal_range ( make_pair(a[i], ANYWHERE) );
      transform ( p.first, p.second, inserter(s, s.end()),
                  [](auto i){ return i.second; } ); 
      p = Map.equal_range ( make_pair(b[i],i) );
      transform ( p.first, p.second, inserter(s, s.end()),
                  [](auto i){ return i.second; } ); 
      p = Map.equal_range ( make_pair(b[i], ANYWHERE) );
      transform ( p.first, p.second, inserter(s, s.end()),
                  [](auto i){ return i.second; } ); 
    }
    return (int)s.size();
  }

};
