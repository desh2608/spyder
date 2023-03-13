// spyder/containers.h

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

#ifndef SPYDER_CONTAINERS_H
#define SPYDER_CONTAINERS_H

#include <map>
#include <set>
#include <string>
#include <vector>

namespace spyder {

// Some strings used during DER computation.
const std::string START = "start";
const std::string END = "end";
const std::string REF = "ref";
const std::string HYP = "hyp";

// Stores speaker turns as provided in the input reference and hypothesis.
class Turn {
public:
  std::string spk;
  double start;
  double end;
  Turn(std::string spk, double start, double end)
      : spk(spk), start(start), end(end) {}
  ~Turn() {}
  // Overload less than operator to enable sorting on start time
  bool operator<(const Turn &other) const;
};

class TurnList {
private:
  // check input (used in constructor)
  bool check_input(std::vector<Turn> turns_list);

public:
  // list of turns
  std::vector<Turn> turns;

  // speaker set in the list of turns
  std::set<std::string> speaker_set;

  // forward index mapping speakers to integers
  std::map<std::string, int> forward_index;

  // reverse index mapping integers to original speaker labels
  std::map<int, std::string> reverse_index;

  TurnList(std::vector<Turn> turns);
  ~TurnList();

  // Merge overlapping turns from the same speaker in the list of turns.
  void merge_same_speaker_turns();

  // Build index of speakers. Each speaker is mapped to a natural number, i.e.,
  // 0,1,2,.. and so on. This is needed to build the cost matrix and apply
  // the Hungarian algorithm.
  void build_speaker_index();

  // Returns total number of turns
  int size();

  // map speaker labels using provided mapping
  // \param label_map, a mapping from old label to new label
  void map_labels(std::map<std::string, std::string> &label_map);
};

// Denotes a timestamp (or boundary marker).
class Token {
public:
  std::string type;
  std::string system;
  std::string spk;
  double timestamp;
  Token() {}
  Token(std::string type, std::string system, std::string spk, double timestamp)
      : type(type), system(system), spk(spk), timestamp(timestamp) {}
  ~Token() {}
  // Overload less than operator to enable sorting on timestamp and type
  bool operator<(const Token &other) const;
};

// Each "region" is a homogeneous segment, i.e., no speaker change happens
// within a region, in either the reference or the hypothesis.
class Region {
public:
  double start;
  double end;
  std::vector<std::string> ref_spk;
  std::vector<std::string> hyp_spk;
  Region(double start, double end, std::vector<std::string> ref_spk,
         std::vector<std::string> hyp_spk)
      : start(start), end(end), ref_spk(ref_spk), hyp_spk(hyp_spk) {}
  ~Region();

  // region duration
  double duration();

  // number of correct speakers in region
  int num_correct();
};

} // end namespace spyder

#endif
