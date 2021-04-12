// spyder/jer.cc

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

#ifndef SPYDER_JER_CC
#define SPYDER_JER_CC

#include "jer.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "float.h"
#include "hungarian.h"

namespace spyder {

TurnList get_difference(TurnList& A, TurnList& B) {
    // Check if single-speaker lists
    A.build_speaker_set();
    B.build_speaker_set();
    if (A.speaker_set.size() > 1 || B.speaker_set.size() > 1) {
        throw std::runtime_error("Cannot compute difference between turn lists");
    }
    std::vector<Token> tokens = get_tokens_from_turns(A, B);
    std::sort(tokens.begin(), tokens.end());
    TurnList diff;
    float start = -1;
    for (auto& token : tokens) {
        if (token.system == REF) {
            if (token.type == START) {
                start = token.timestamp;
            } else {
                if (start > 0) {
                    diff.turns.push_back(Turn(token.spk, start, token.timestamp));
                    start = -1;
                }
            }
        } else {
            if (token.type == START) {
                if (start > 0) {
                    diff.turns.push_back(Turn(token.spk, start, token.timestamp));
                    start = -1;
                }
            } else {
                if (start > 0) {
                    start = token.timestamp;
                }
            }
        }
    }
    return diff;
}

// JER computation pseudocode is taken from pyannote.metrics:
// https://pyannote.github.io/pyannote-metrics/_modules/pyannote/metrics/diarization.html#JaccardErrorRate
void compute_jer_mapped(TurnList& ref, TurnList& hyp, JERMetrics& metrics) {
    // Build speaker sets for ref and hyp
    ref.build_speaker_set();
    hyp.build_speaker_set();

    // compute JER metrics per speaker
    std::unordered_map<std::string, float> speaker_jer;
    float total, falarm, miss, jer, total_jer = 0, total_dur = 0;
    for (auto& spk : ref.speaker_set) {
        TurnList r = TurnList(ref, spk);
        TurnList h = TurnList(hyp, spk);
        total_dur += r.duration();
        if (hyp.speaker_set.find(spk) == hyp.speaker_set.end()) {
            // Reference speaker not paired with any hypothesis speaker
            // JER = 1
            jer = 1;
        } else {
            // total is the duration of the union of reference and hypothesis
            // speaker segments
            total = r.get_union(h).duration();
            // falarm is the total hyp speaker time not attributed to the
            // reference speaker
            falarm = h.duration() - get_difference(h, r).duration();
            // miss is the total reference speaker time not attributed to
            // the system speaker
            miss = r.duration() - get_difference(r, h).duration();
            jer = (falarm + miss) / total;
        }
        speaker_jer.insert(std::make_pair(spk, jer));
        total_jer += jer;
    }

    total_jer /= speaker_jer.size();
    metrics.jer = total_jer;
    metrics.duration = total_dur;
    return;
}

JERMetrics compute_jer(TurnList& ref, TurnList& hyp) {
    ref.build_speaker_index();
    hyp.build_speaker_index();
    std::vector<std::vector<double>> cost_matrix = build_cost_matrix(ref, hyp);
    HungarianAlgorithm hungarian_solver;
    std::vector<int> assignment;
    double cost = hungarian_solver.Solve(cost_matrix, assignment);
    map_labels(ref, hyp, assignment);
    JERMetrics metrics;
    compute_jer_mapped(ref, hyp, metrics);
    return metrics;
}

}  // end namespace spyder

#endif