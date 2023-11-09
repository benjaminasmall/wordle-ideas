#include "Util.hpp"
#include "Evaluator.hpp"
#include <random>

size_t play ( string const& wins_file = "./shuffled_real_wordles.txt",
              string const& gses_file = "./official_allowed_guesses.txt")
{
  WordleEngine we(wins_file, gses_file);

  we.Play(cin, cout);

  return 0;
}

size_t find_two_words()
{
  string const wins_file = "./shuffled_real_wordles.txt";
  string const gses_file = "./official_allowed_guesses.txt";

  Evaluator eval(wins_file, gses_file);

  score_table_t scores;
  eval.score_candidates2(scores);

  auto it = scores.begin();
  for ( size_t i = 0 ; i < 1000 ; ++i, ++it )
    cout << it->first << " " << it->second << endl;

  return scores.size();
}

int main ( int argc, char *argv[])
{
  string const SELF (argv[0]);
  int ret = 0;

  if ( argc > 1 && !strcmp(argv[1], "play") )
    ret = play();

  if ( argc > 1 && !strcmp(argv[1], "two") )
    ret = find_two_words();

  return 0;
}

