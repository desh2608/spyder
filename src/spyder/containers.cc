// spyder/containers.cc

// Copyright 2021  Johns Hopkins University (Author: Desh Raj)

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

#ifndef SPYDER_CONTAINERS_CC
#define SPYDER_CONTAINERS_CC

#include "containers.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "GroupBy.h"
#include "float.h"

namespace spyder {

bool Turn::operator<(const Turn &other) const { return start < other.start; }

bool TurnList::check_input(std::vector<Turn> turns_list) {
  for (auto &turn : turns_list) {
    if (turn.start > turn.end) {
      return false;
    }
  }
  return true;
}

TurnList::TurnList(std::vector<Turn> turns_list) {
  if (check_input(turns_list))
    turns = turns_list;
  else
    throw std::invalid_argument("start time cannot be greater than end time");
}

TurnList::~TurnList() {
  std::vector<Turn>().swap(turns);
  std::set<std::string>().swap(speaker_set);
}

void TurnList::merge_same_speaker_turns() {
  std::vector<Turn> new_turns;
  // Group the list of turns by speaker
  std::map<std::string, std::vector<Turn>> const turns_by_speaker =
      groupBy(turns.begin(), turns.end(), [](Turn const &t) { return t.spk; });
  for (auto &it : turns_by_speaker) {
    std::vector<Turn> spk_turns = it.second;
    // Sort the turns by start time
    std::sort(spk_turns.begin(), spk_turns.end());
    // Merge overlapping intervals in interval tree
    std::vector<Turn> merged_turns;
    double prev_end = -1;
    for (auto &turn : spk_turns) {
      if (prev_end < 0) {
        prev_end = turn.end;
        merged_turns.push_back(turn);
      } else {
        if (turn.start <= prev_end) {
          prev_end = std::max(prev_end, turn.end);
          merged_turns.back().end = prev_end;
        } else {
          prev_end = turn.end;
          merged_turns.push_back(turn);
        }
      }
    }
    // Add merged turns to new list
    new_turns.insert(new_turns.end(), merged_turns.begin(), merged_turns.end());
  }
  // Replace the old list with the new list
  turns.swap(new_turns);
}

void TurnList::build_speaker_index() {
  // Get set of speakers
  for (auto &turn : turns) {
    speaker_set.insert(turn.spk);
  }
  // Assign an integer to each speaker
  int idx = 0;
  for (auto &spk : speaker_set) {
    forward_index.insert(std::pair<std::string, int>(spk, idx));
    reverse_index.insert(std::pair<int, std::string>(idx, spk));
    idx += 1;
  }
}

int TurnList::size() { return turns.size(); }

void TurnList::map_labels(std::map<std::string, std::string> &label_map) {
  std::string old_label, new_label;
  for (auto &turn : turns) {
    old_label = turn.spk;
    new_label = label_map.find(old_label)->second;
    turn.spk = new_label;
  }
}

bool Token::operator<(const Token &other) const {
  if (fabs(timestamp - other.timestamp) > DBL_EPSILON) {
    // case 1: timestamps are different
    return (timestamp < other.timestamp);
  } else if (type != other.type) {
    // case 2: timestamps are same but types are different
    return (type.compare(other.type) < 0);
  } else {
    // case 3: timestamps and types are same, but system is different
    if (type == START) {
      // case 3a: start token -> UEM < ref < hyp
      return (system.compare(other.system) > 0);
    } else {
      // case 3b: end token -> hyp < ref < UEM
      return (system.compare(other.system) < 0);
    }
  }
}

Region::~Region() {
  std::vector<std::string>().swap(ref_spk);
  std::vector<std::string>().swap(hyp_spk);
}

double Region::duration() { return (end - start); }

int Region::num_correct() {
  int N_correct = 0;
  for (auto &ref : ref_spk) {
    if (std::count(hyp_spk.begin(), hyp_spk.end(), ref) > 0) {
      N_correct += 1;
    }
  }
  return N_correct;
}

}  // end namespace spyder

#endif
