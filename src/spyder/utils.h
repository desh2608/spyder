// spyder/utils.h

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

#ifndef SPYDER_UTILS_H
#define SPYDER_UTILS_H

#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "containers.h"

namespace spyder {

// Compute intersection length of two turn tuples.
// \param A: a Turn tuple
// \param B: a Turn tuple
double compute_intersection_length(Turn A, Turn B);

// Build cost matrix given reference and hypothesis lists.
// \param ref: a list of reference turns
// \param hyp: a list of hypothesis turns
std::vector<std::vector<double>> build_cost_matrix(TurnList& ref, TurnList& hyp);

// Build cost matrix given reference and hypothesis lists, based on a set of
// evaluation regions.
// \param ref: a list of reference turns
// \param hyp: a list of hypothesis turns
// \param regions: a list of evaluation regions
std::vector<std::vector<double>> build_cost_matrix(TurnList& ref, TurnList& hyp,
                                                   std::vector<Region>& regions);

// Map reference and hypothesis labels to common space based on assignment
// vector.
// \param ref, reference list of turns
// \param hyp, hypothesis list of turns
// \param assignment, vector of assignments from ref to hyp
// \param ref_map, map from reference labels to common labels
// \param hyp_map, map from hypothesis labels to common labels
void map_labels(TurnList& ref, TurnList& hyp, std::vector<int>& assignment,
                std::map<std::string, std::string>& ref_map,
                std::map<std::string, std::string>& hyp_map);

// Compute the evaluation regions based on the reference, hypothesis, and the UEM
// segments.
// \param ref: a list of reference turns.
// \param hyp: a list of hypothesis turns.
// \param uem: a list of UEM segments.
// \param collar: the collar size in seconds.
// \return a list of evaluation regions
std::vector<Region> get_eval_regions(TurnList& ref, TurnList& hyp, TurnList& uem);

// Add reference collars to the UEM. This basically updates the UEM segments to exclude
// the reference regions that are in the collar.
// \param ref: a list of reference turns.
// \param uem: a list of UEM segments.
// \param collar: the collar size in seconds.
void add_collar_to_uem(TurnList& uem, TurnList& ref, float collar = 0.0);

}  // end namespace spyder

#endif
