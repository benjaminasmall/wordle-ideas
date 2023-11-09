#pragma once

#include <string>
#include <cctype>
#include <utility>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <random>

using namespace std;

struct string5_hash;

typedef size_t                      sid_t;
typedef vector<string>              list_t;
typedef set<sid_t>                  set_t;
typedef unordered_map<string,
                      sid_t>        dict_t;
//                      string5_hash> dict_t;

static constexpr int const ANYWHERE = -1;

enum class Color
{
  None   = ' ',
  Gray   = '.',
  Yellow = '?',
  Green  = '!'
};

class Move
{
  string        word;
  vector<Color> cols;

public:
  Move(string const& _w)
  : word(_w),
    cols(word.size(), Color::None)
  { }
  
  Move(string const& _w, Color _c[])
  : word(_w),
    cols(_c, _c + word.size()/sizeof(Color))
  { }

  Move(string const& _w, initializer_list<Color> _c)
  : word(_w),
    cols(_c)
  { }
  
  template <class ...Class>
  Move(char x, Color y, Class... rest)
  : word(),
    cols()
  {
    word.push_back(x);
    cols.push_back(y);
    Move(rest...);
  }

  bool win() const
  {
    return all_of ( cols.begin(), cols.end(),
                    [](Color const& c) { return c == Color::Green; } );
  }

  friend ostream& operator<<(ostream&, Move const&);
  Move& operator |= (string const& w)
  {
    cols = vector ( word.size(), Color::None);
    if ( w.size() != word.size() )
      return *this;
    set<char> s;
    copy ( w.begin(), w.end(), inserter(s, s.end()) );
    for ( size_t i = 0 ; i < word.size() ; ++i )
    {
      if ( word[i] == w[i] )
      {
        cols[i] = Color::Green;
        continue;
      }
      if ( s.find(word[i]) != s.end() )
      {
        cols[i] = Color::Yellow;
        continue;
      }
      cols[i] = Color::Gray;
    } 
    return *this;
  }
};

ostream& operator<<(ostream& os, Move const& m)
{
  for ( size_t i = 0 ; i < m.word.size() ; ++i )
  {
    os << "|" << "\033[1m";
    switch (m.cols[i])
    {
      case Color::None:
        break;
      case Color::Gray:
        os << "\033[48;5;7m";
        break;
      case Color::Yellow:
        os << "\033[48;5;11m";
        break;
      case Color::Green:
        os << "\033[48;5;10m";
        break;
    }
    os << (char)toupper(m.word[i]);
    os << "\033[0m";
  }
  os << "|";
  return os;
}

/****
 * Class for playing Wordle and supporting functions; not for strategy
 * discovery, evaluation, or other optimizations.
 ****/

class WordleEngine
{
protected:
  set_t          Winnables;
  set_t          Playables;
  list_t         IdToWordMap;
  dict_t         WordToIdMap;
  vector<size_t> Lengths, Counts;
  vector<int>    Chars;

  random_device                            r;
  mutable mt19937                          e;
  mutable uniform_int_distribution<size_t> u_d;

  struct string5_hash
  {
    size_t operator()(string const& w) const
    {
      size_t val = 0;
      for ( size_t i = 0 ; i < 5 ; ++i )
      {
        val *= 26;
        val += w[i] - 'a';
      }
      return val;
    }
  };

  template <class Container>
  void populate_containers(Container const& c, bool winners)
  {
    size_t idx = IdToWordMap.size();
    for ( string word : c )
    {
      IdToWordMap.push_back(word);
      WordToIdMap.insert(make_pair(word,idx));
      if ( winners )
        Winnables.insert(idx);
      Playables.insert(idx);
      auto p = character_bits(word);
      Lengths.push_back(word.size());
      Counts.push_back (p.first);
      Chars.push_back  (p.second);
      ++idx;
    }
  }

public:
  WordleEngine ( list_t const& _w, list_t const& _g )
    : r(), e(r()), u_d(0,_w.size()-1)
  {
    populate_containers(_w, true);
    populate_containers(_g, false);
    cout << "WordleEngine: initialized with " << Winnables.size()
         << " winnables and " << Playables.size()
         << " playable words." << endl;
  }

  WordleEngine ( string const& wfile, string const& gfile )
    : r(), e(r())
  {
    list_t w, g;
    WordleEngine::load_list(wfile, w);
    WordleEngine::load_list(gfile, g);
    populate_containers(w, true);
    populate_containers(g, false);
    u_d = uniform_int_distribution<size_t>(0, w.size()-1);
    cout << "WordleEngine: initialized with " << Winnables.size()
         << " winnables and " << Playables.size()
         << " playable words." << endl;
  }


protected:
  sid_t WinnablesEnd() const
  {
    return Winnables.size();
  }

  sid_t End() const
  {
    return IdToWordMap.size();
  }

  sid_t Find(string const& w) const
  {
    auto it = WordToIdMap.find(w);
    return it == WordToIdMap.end() ? End() : it->second;
  }

public:
  bool UniqueChars(sid_t id) const
  {
    return Lengths[id] == Counts[id];
  }

  bool OverlappingChars(sid_t a, sid_t b) const
  {
    return Chars[a] & Chars[b];
  }

protected: 
  // count of unique letters and unique letters as bit set
  static pair<size_t, int> character_bits(string const& word)
  {
    int    mask  = 0;
    int    bit   = 0;
    size_t count = 0;
    for ( char const c : word )
    {
      bit = 1 << (c-'a');
      count += bit & mask ? 0 : 1;
      mask |= bit;
    }
    return make_pair(count, mask);
  } 

  static int load_list ( string const& filename, list_t& vec )
  {
    vec.clear();
    string line;
    ifstream file(filename);
    if (!file.is_open())
      return 1;
    while ( getline (file, line) )
    {
      // comments or garbage
      string::size_type n = line.find_first_of(" \t#");
      if (n != string::npos)
        line.erase(n);
      if ( line.empty() )
        continue;
      WordleEngine::Canonize(line);
      vec.push_back(line);
    }
    // all done
    return 0;
  }

public:
  static void Canonize(string& w)
  {
    transform ( w.begin(), w.end(), w.begin(),
                [](char c) { return tolower(c); } );
  }
  sid_t Random() const
  {
    return u_d(e);
  }
  size_t Play(istream& is, ostream& os, size_t const turns = 6)
  {
    static string const salutation[] = { "Impossibly lucky",
                                       "Really lucky",
                                       "Splendid",
                                       "Amazing",
                                       "Great",
                                       "Phew" };
    string const& win  = IdToWordMap[Random()];
    string line;
    for ( size_t turn = 1 ; turn <= turns ; ++turn )
    {
      sid_t it = End();
      while ( it == End() )
      {
        os << turn << "> ";
        is >> line;
        WordleEngine::Canonize(line); 
        it = Find(line);
        cout << it << endl;
      }
      // now play it
      Move move(line);
      move |= win;
      os << move;
      if ( move.win() )
      {
        os << " " << salutation[turn-1] << " !" << endl;
        return turn;
      }
      os << endl;
    } 
    os << "Sorry (" << win << ")." << endl;
    return 0;
  }
};

