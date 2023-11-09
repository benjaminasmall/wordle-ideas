#include "Util.hpp"
#include <regex>
#include <initializer_list>

struct Box
{
  char sides[12];
  byte dict [26];
  uint mask_;
  bool init;
  static byte const INVALID = (byte)4;
  Box(string const& s)
    : mask_(0), init(false)
  {
    fill_n(dict, 26, INVALID);
    static const string sidex = "([a-z]{3})";
    static const string delex = ",";
    static regex const re(sidex + delex + sidex + delex + sidex + delex + sidex);
    smatch m;
    init = regex_match(s, m, re);
    if ( !init )
      return;
    for ( auto i : {0,1,2,3} )
    {
      for ( auto j : {0,1,2} )
      {
        char const c = m[i+1].str()[j];
        sides[i*3+j] = c;
        dict[c-'a'] = (byte)i;
        mask_ |= (1 << (c-'a'));
      }
    }
  }
  byte side(char c) const
  {
    return dict[c-'a'];
  }
  uint mask() const
  {
    return mask_;
  }
  string get_side(size_t s) const
  {
    return string(sides).substr(3*s, 3);
  }
  bool is_valid(string const& word) const
  {
    byte last_side = INVALID;
    for ( char c : word )
    {
      byte const s = side(c);
      if ( s == INVALID || s == last_side )
        return false;
      last_side = s;
    }
    return true;
  }
};

ostream& operator<<(ostream& os, const Box& box)
{
  os << box.get_side(0) << "," << box.get_side(1) << ","
     << box.get_side(2) << "," << box.get_side(3);
  return os;
}

size_t load_dictionary(string const& filename, vector<string>& dict)
{
  dict.clear();
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
    // proper nouns and hyphenates
    if ( !all_of(line.begin(), line.end(), &::islower) )
      continue; 
    // otherwise append it
    dict.push_back(line);
  }
  // all done
  return 0;
}

size_t play ( string const& sides,
              string const& dict_file = "/usr/share/dict/words" )
{
  vector       <string>       dict;
  unordered_map<string, uint> masks;
  Box box(sides);
  load_dictionary(dict_file, dict);
  cerr << "Loaded " << dict.size() << " words from dictionary" << endl;
  generate_letter_masks(dict, masks);
  for ( auto it = masks.begin() ; it != masks.end() ; /**/ )
  {
    if ( (it->second | box.mask()) != box.mask() )
      it = masks.erase(it);
    else
      ++it;
  }
  cerr << "Reduced to " << masks.size() << " possibly usable words" << endl;
  for ( auto it = masks.begin() ; it != masks.end() ; /**/ )
  {
    if ( !box.is_valid(it->first) )
      it = masks.erase(it);
    else
      ++it;
  }
  cerr << "Reduced to " << masks.size() << " valid words" << endl;

  for ( auto w : masks )
  {
    if ( __popcount(w.second) == 12 )
      cout << "SUPER SOLUTION: " << w.first << endl;
  }

  for ( auto w1 : masks )
  {
    for ( auto w2 : masks )
    {
      if ( w1.first.back() != w2.first.front() )
        continue;
      if ( (w1.second | w2.second) == box.mask() )
        cout << "SOLUTION: " << w1.first << "-" << w2.first << endl;
    }
  }

  return 0;
}

size_t find_two_words()
{
  return 0;
}

void test_helper(string const& word, string const& sides)
{
  Box b(sides);
  cout << word << "; " << b << ": ";
  cout << (b.is_valid(word) ? "VALID" : "invalid") << endl;
}

int main ( int argc, char *argv[])
{
  string const SELF (argv[0]);
  int ret = 0;

  if ( argc != 2 )
  {
    cerr << SELF << ": must provide box in 'abc,def,ghi,jkl' format" << endl;
    return -1;
  }

  ret = play(argv[1]);

  switch(ret)
  {
    case -1:
      cerr << SELF << ": not a valid box: " << argv[1] << endl;
  }

  return ret;
}

