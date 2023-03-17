// spyder/utils.cc

// Copyright 2023  Johns Hopkins University (Author: Desh Raj)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef SPYDER_UTILS_CC
#define SPYDER_UTILS_CC

#include "utils.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "float.h"

namespace spyder {

double compute_intersection_length(Turn A, Turn B) {
  double max_start = std::max(A.start, B.start);
  double min_end = std::min(A.end, B.end);
  return std::max(0.0, min_end - max_start);
}

std::vector<std::vector<double>> build_cost_matrix(TurnList &ref, TurnList &hyp) {
  int M = ref.forward_index.size();
  int N = hyp.forward_index.size();
  std::vector<std::vector<double>> cost_matrix(M, std::vector<double>(N));

  int i, j;
  for (auto &ref_turn : ref.turns) {
    for (auto &hyp_turn : hyp.turns) {
      i = ref.forward_index.find(ref_turn.spk)->second;
      j = hyp.forward_index.find(hyp_turn.spk)->second;
      cost_matrix[i][j] -= compute_intersection_length(ref_turn, hyp_turn);
    }
  }
  return cost_matrix;
}

std::vector<std::vector<double>> build_cost_matrix(TurnList &ref, TurnList &hyp,
                                                   std::vector<Region> &regions) {
  int M = ref.forward_index.size();
  int N = hyp.forward_index.size();
  std::vector<std::vector<double>> cost_matrix(M, std::vector<double>(N));

  int i, j;
  for (auto &region : regions) {
    for (auto &ref_spk : region.ref_spk) {
      for (auto &hyp_spk : region.hyp_spk) {
        i = ref.forward_index.find(ref_spk)->second;
        j = hyp.forward_index.find(hyp_spk)->second;
        cost_matrix[i][j] -= region.duration();
      }
    }
  }
  return cost_matrix;
}

void map_labels(TurnList &ref, TurnList &hyp, std::vector<int> &assignment,
                std::map<std::string, std::string> &ref_map,
                std::map<std::string, std::string> &hyp_map) {
  int k = 0;
  std::set<std::string> ref_spk_remaining = ref.speaker_set;
  std::set<std::string> hyp_spk_remaining = hyp.speaker_set;
  std::string ref_spk, hyp_spk;
  for (int i = 0; i < assignment.size(); ++i) {
    if (assignment[i] != -1) {
      ref_spk = ref.reverse_index.find(i)->second;
      hyp_spk = hyp.reverse_index.find(assignment[i])->second;
      ref_map.insert(std::pair<std::string, std::string>(ref_spk, std::to_string(k)));
      hyp_map.insert(std::pair<std::string, std::string>(hyp_spk, std::to_string(k)));
      k += 1;
      ref_spk_remaining.erase(ref_spk);
      hyp_spk_remaining.erase(hyp_spk);
    }
  }
  for (auto &spk : ref_spk_remaining) {
    ref_map.insert(std::pair<std::string, std::string>(spk, std::to_string(k++)));
  }
  for (auto &spk : hyp_spk_remaining) {
    hyp_map.insert(std::pair<std::string, std::string>(spk, std::to_string(k++)));
  }
  ref.map_labels(ref_map);
  hyp.map_labels(hyp_map);
}

std::vector<Region> get_eval_regions(TurnList &ref, TurnList &hyp, TurnList &uem) {
  // Create a list of tokens combining reference, hypothesis, and UEM segments
  std::vector<Token> tokens(2 * (ref.size() + hyp.size() + uem.size()));
  int i = -1;
  for (auto &turn : uem.turns) {
    tokens[++i] = Token(START, UEM, turn.spk, turn.start);
    tokens[++i] = Token(END, UEM, turn.spk, turn.end);
  }
  for (auto &turn : ref.turns) {
    tokens[++i] = Token(START, REF, turn.spk, turn.start);
    tokens[++i] = Token(END, REF, turn.spk, turn.end);
  }
  for (auto &turn : hyp.turns) {
    tokens[++i] = Token(START, HYP, turn.spk, turn.start);
    tokens[++i] = Token(END, HYP, turn.spk, turn.end);
  }

  // Sort the tokens. They will be sorted first by timestamp and then
  // by type (i.e. "end" tokens before "start"), since we overloaded
  // the Token "<" (less than) operator.
  std::sort(tokens.begin(), tokens.end());

  // Create list of evaluation regions
  std::vector<Region> regions;
  double region_start = tokens[0].timestamp;
  std::unordered_set<std::string> ref_spk, hyp_spk;
  bool evaluate = false;

  for (int i = 0; i < tokens.size(); ++i) {
    // If the evaluate flag is set and the region is not empty, add it to the
    // list of regions
    if (evaluate && tokens[i].timestamp - region_start > DBL_EPSILON) {
      std::vector<std::string> ref_spk_list(ref_spk.begin(), ref_spk.end());
      std::vector<std::string> hyp_spk_list(hyp_spk.begin(), hyp_spk.end());
      regions.push_back(Region(region_start, tokens[i].timestamp, ref_spk_list, hyp_spk_list));
    }

    // Update the list of ref and hyp speakers in the current region
    if (tokens[i].system == REF) {
      if (tokens[i].type == START) {
        ref_spk.insert(tokens[i].spk);
      } else {
        ref_spk.erase(tokens[i].spk);
      }
    } else if (tokens[i].system == HYP) {
      if (tokens[i].type == START) {
        hyp_spk.insert(tokens[i].spk);
      } else {
        hyp_spk.erase(tokens[i].spk);
      }
    } else {
      // If it is a UEM token, update the evaluate flag
      evaluate = (tokens[i].type == START);
    }

    // Update the region start time
    region_start = tokens[i].timestamp;
  }
  // free up memory
  std::vector<Token>().swap(tokens);
  return regions;
}

void add_collar_to_uem(TurnList &uem, TurnList &ref, float collar) {
  // Create a list of tokens combining reference and UEM segments
  std::vector<Token> tokens(4 * ref.size() + 2 * uem.size());
  int i = -1;
  for (auto &turn : uem.turns) {
    tokens[++i] = Token(START, UEM, turn.spk, turn.start);
    tokens[++i] = Token(END, UEM, turn.spk, turn.end);
  }
  for (auto &turn : ref.turns) {
    tokens[++i] = Token(END, REF, turn.spk, turn.start - collar);
    tokens[++i] = Token(START, REF, turn.spk, turn.start + collar);
    tokens[++i] = Token(END, REF, turn.spk, turn.end - collar);
    tokens[++i] = Token(START, REF, turn.spk, turn.end + collar);
  }

  // Sort the tokens. They will be sorted first by timestamp and then
  // by type (i.e. "end" tokens before "start"), since we overloaded
  // the Token "<" (less than) operator.
  std::sort(tokens.begin(), tokens.end());

  std::vector<Turn> uem_turns;

  double region_start = tokens[0].timestamp;
  std::unordered_set<std::string> ref_spk, hyp_spk;
  std::string dummy_spk = uem.turns[0].spk;
  int evaluate = 0;

  for (int i = 0; i < tokens.size(); ++i) {
    // If it is a START token, increment the evaluate flag
    if (tokens[i].type == START) {
      evaluate += 1;
      if (evaluate == 1) {
        region_start = tokens[i].timestamp;
      }
    } else {
      evaluate -= 1;
      if (evaluate == 0 && tokens[i].timestamp - region_start > DBL_EPSILON) {
        uem_turns.push_back(Turn(dummy_spk, region_start, tokens[i].timestamp));
      }
    }
  }
  // free up memory
  std::vector<Token>().swap(tokens);

  // Replace the old list with the new list
  uem.turns.swap(uem_turns);
}

}  // end namespace spyder

#endif
