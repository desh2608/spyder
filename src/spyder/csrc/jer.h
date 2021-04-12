// spyder/jer.h

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

#ifndef SPYDER_JER_H
#define SPYDER_JER_H

#include "containers.h"
#include "der.h"

namespace spyder {

// The DER metrics: missed speech, false alarm, speaker confusion (error),
// and diarization error rate (DER).
class JERMetrics {
   public:
    double duration;
    double jer;
    JERMetrics() {}
    JERMetrics(double duration, double jer) : duration(duration), jer(jer) {}
    ~JERMetrics() {}
};

// Given 2 single-speaker lists of turns, compute the difference A\B
// \param A: list of turns (single-speaker)
// \param B: list of turns (single-speaker)
TurnList get_difference(TurnList& A, TurnList& B);

// Compute Jaccard error rate with mapped turn lists.
// \param ref: a list of reference turns
// \param hyp: a list of hypothesis turns
void compute_jer_mapped(TurnList& ref, TurnList& hyp, JERMetrics& metrics);

// Compute diarization error rate. First the lists are mapped to a common
// label space using the Hungarian algorithm.// \param ref: a list of reference turns
// \param hyp: a list of hypothesis turns
JERMetrics compute_jer(TurnList& ref, TurnList& hyp);

}  // end namespace spyder

#endif