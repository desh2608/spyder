// spyder/containers.cc

// Copyright 2021  Johns Hopkins University (Author: Desh Raj)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

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
#include <stack>
#include <stdexcept>
#include <string>

#include "float.h"

namespace spyder {

bool Turn::operator<(const Turn &other) const {
    if (fabs(start - other.start) > DBL_EPSILON) {
        return (start < other.start);
    } else {
        return (spk.compare(other.spk) < 0);
    }
}

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

TurnList::TurnList(TurnList tl, std::string spk) {
    TurnList spk_tl;
    for (auto &turn : tl.turns) {
        if (turn.spk == spk)
            spk_tl.turns.push_back(turn);
    }
}

TurnList TurnList::get_union(TurnList &other) {
    std::vector<Turn> merged_turns = turns;
    merged_turns.insert(merged_turns.end(), other.turns.begin(), other.turns.end());
    std::sort(merged_turns.begin(), merged_turns.end());
    TurnList res;
    res.turns = merged_turns;
    res.build_speaker_set();
    std::map<std::string, std::stack<std::pair<float, float>>> stacks;
    for (auto &spk : res.speaker_set) {
        stacks.insert(std::make_pair(spk, std::stack<std::pair<float, float>>()));
    }
    for (auto &turn : turns) {
        if (stacks[turn.spk].empty() || turn.start > stacks[turn.spk].top().second) {
            // If speaker stack is empty or current interval does not overlap with
            // last interval, push it
            stacks[turn.spk].push(std::make_pair(turn.start, turn.end));
        } else {
            // update top interval by merging current interval
            stacks[turn.spk].top().second = turn.end;
        }
    }
    std::vector<Turn>().swap(res.turns);
    for (auto &spk : speaker_set) {
        while (!stacks[spk].empty()) {
            res.turns.push_back(Turn(spk, stacks[spk].top().first, stacks[spk].top().second));
            stacks[spk].pop();
        }
    }
    return res;
}

float TurnList::duration() {
    float duration = 0;
    for (auto &turn : turns) {
        duration += (turn.end - turn.start);
    }
    return duration;
}

TurnList::~TurnList() {
    std::vector<Turn>().swap(turns);
    std::set<std::string>().swap(speaker_set);
}

void TurnList::build_speaker_set() {
    std::set<std::string>().swap(speaker_set);
    for (auto &turn : turns) {
        speaker_set.insert(turn.spk);
    }
}

void TurnList::build_speaker_index() {
    // Get set of speakers
    build_speaker_set();
    // Assign an integer to each speaker
    int idx = 0;
    std::map<std::string, int>().swap(forward_index);
    std::map<int, std::string>().swap(reverse_index);
    for (auto &spk : speaker_set) {
        forward_index.insert(std::pair<std::string, int>(spk, idx));
        reverse_index.insert(std::pair<int, std::string>(idx, spk));
        idx += 1;
    }
}

int TurnList::size() {
    return turns.size();
}

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
        return (timestamp < other.timestamp);
    } else {
        return (type.compare(other.type) < 0);
    }
}

std::vector<Token> get_tokens_from_turns(TurnList &ref, TurnList &hyp) {
    // Create a list of tokens combining reference and hypothesis
    std::vector<Token> tokens(2 * (ref.size() + hyp.size()));
    int i = -1;
    for (auto &turn : ref.turns) {
        tokens[++i] = Token(START, REF, turn.spk, turn.start);
        tokens[++i] = Token(END, REF, turn.spk, turn.end);
    }
    for (auto &turn : hyp.turns) {
        tokens[++i] = Token(START, HYP, turn.spk, turn.start);
        tokens[++i] = Token(END, HYP, turn.spk, turn.end);
    }
    return tokens;
}

Region::~Region() {
    std::vector<std::string>().swap(ref_spk);
    std::vector<std::string>().swap(hyp_spk);
}

double Region::duration() {
    return (end - start);
}

int Region::num_correct() {
    int N_correct = 0;
    for (auto &ref : ref_spk) {
        if (std::find(hyp_spk.begin(), hyp_spk.end(), ref) != hyp_spk.end())
            N_correct++;
    }
    return N_correct;
}

std::vector<Region> get_regions(std::vector<Token> tokens) {
    std::vector<Region> regions;
    double region_start = tokens[0].timestamp;
    std::vector<std::string> ref_spk, hyp_spk;
    if (tokens[0].system == REF) {
        ref_spk.push_back(tokens[0].spk);
    } else {
        hyp_spk.push_back(tokens[0].spk);
    }
    for (int i = 1; i < tokens.size(); ++i) {
        if (tokens[i].timestamp - region_start > DBL_EPSILON) {
            regions.push_back(Region(region_start, tokens[i].timestamp, ref_spk, hyp_spk));
        }
        if (tokens[i].type == START) {
            if (tokens[i].system == REF) {
                ref_spk.push_back(tokens[i].spk);
            } else {
                hyp_spk.push_back(tokens[i].spk);
            }
        } else {
            if (tokens[i].system == REF) {
                ref_spk.erase(std::find(ref_spk.begin(), ref_spk.end(), tokens[i].spk));
            } else {
                hyp_spk.erase(std::find(hyp_spk.begin(), hyp_spk.end(), tokens[i].spk));
            }
        }
        region_start = tokens[i].timestamp;
    }
    return regions;
}

}  // end namespace spyder

#endif