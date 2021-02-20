import numpy as np
from lapsolver import solve_dense

from collections import namedtuple, defaultdict


Token = namedtuple('Token', 'type spk timestamp system')
Region = namedtuple('Region', 'start end ref_spk hyp_spk')
Metrics = namedtuple('Metrics', 'miss false conf der')


def check_input(hyp):
    """Check whether a hypothesis/reference is valid.

    Args:
        hyp: a list of tuples, where each tuple is (speaker, start, end)
            of type (string, float, float)

    Raises:
        TypeError: if the type of `hyp` is incorrect
        ValueError: if some tuple has start > end;

    Returns:
        True: if no overlaps are present
        False: if overlaps are present
    """
    if not isinstance(hyp, list):
        raise TypeError("Input must be a list.")
    for element in hyp:
        if not isinstance(element, tuple):
            raise TypeError("Input must be a list of tuples.")
        if len(element) != 3:
            raise TypeError(
                "Each tuple must have the elements: (speaker, start, end).")
        if not isinstance(element[0], str):
            raise TypeError("Speaker must be a string.")
        if not isinstance(element[1], float) or not isinstance(
                element[2], float):
            raise TypeError("Start and end must be float numbers.")
        if element[1] > element[2]:
            raise ValueError("Start must not be larger than end.")


def build_speaker_index(hyp):
    """Build the index for the speakers.

    Args:
        hyp: a list of tuples, where each tuple is (speaker, start, end)
            of type (string, float, float)

    Returns:
        a dict from speaker to integer
    """
    speaker_set = sorted({element[0] for element in hyp})
    index = {speaker: i for i, speaker in enumerate(speaker_set)}
    return index


def compute_intersection_length(A, B):
    """Compute the intersection length of two tuples.
    Args:
        A: a (speaker, start, end) tuple of type (string, float, float)
        B: a (speaker, start, end) tuple of type (string, float, float)
    Returns:
        a float number of the intersection between `A` and `B`
    """
    max_start = max(A[1], B[1])
    min_end = min(A[2], B[2])
    return max(0.0, min_end - max_start)


def build_cost_matrix(ref, hyp):
    """Build the cost matrix.

    Args:
        ref: a list of tuples for the ground truth, where each tuple is
            (speaker, start, end) of type (string, float, float)
        hyp: a list of tuples for the diarization result hypothesis, same type
            as `ref`

    Returns:
        a 2-dim numpy array, whose element (i, j) is the overlap between
            `i`th reference speaker and `j`th hypothesis speaker
    """
    ref_index = build_speaker_index(ref)
    hyp_index = build_speaker_index(hyp)
    cost_matrix = np.zeros((len(ref_index), len(hyp_index)))
    for ref_element in ref:
        for hyp_element in hyp:
            i = ref_index[ref_element[0]]
            j = hyp_index[hyp_element[0]]
            cost_matrix[i, j] += compute_intersection_length(
                ref_element, hyp_element)
    return cost_matrix, ref_index, hyp_index


def compute_der(ref, hyp):
    """Compute DER when overlaps are present

    Args:
        ref: a list of tuples for the ground truth, where each tuple is
            (speaker, start, end) of type (string, float, float)
        hyp: a list of tuples for the diarization result hypothesis, same type
            as `ref`

    Returns:
        DER: Namedtuple containing (miss, false, conf, der)
    """
    tokens = []
    for segment in ref:
        tokens.append(
            Token(type='START', spk=segment[0], timestamp=segment[1], system='ref'))
        tokens.append(
            Token(type='END', spk=segment[0], timestamp=segment[2], system='ref'))
    for segment in hyp:
        tokens.append(
            Token(type='START', spk=segment[0], timestamp=segment[1], system='hyp'))
        tokens.append(
            Token(type='END', spk=segment[0], timestamp=segment[2], system='hyp'))

    sorted_tokens = sorted(tokens, key=lambda x: (x.timestamp, x.type))
    regions = []
    region_start = sorted_tokens[0].timestamp
    running_speakers = [(sorted_tokens[0].spk, sorted_tokens[0].system)]
    for token in sorted_tokens[1:]:
        if token.timestamp > region_start:
            ref_spk = [spk for spk,
                       system in running_speakers if system == 'ref']
            hyp_spk = [spk for spk,
                       system in running_speakers if system == 'hyp']
            if len(ref_spk) > 0 or len(hyp_spk) > 0:
                regions.append(
                    Region(start=region_start, end=token.timestamp, ref_spk=ref_spk, hyp_spk=hyp_spk))
        if token.type == "START":
            running_speakers.append((token.spk, token.system))
        else:
            running_speakers.remove((token.spk, token.system))
        region_start = token.timestamp

    miss = 0
    false = 0
    conf = 0
    total_dur = 0
    for region in regions:
        dur = region.end - region.start
        N_ref = len(region.ref_spk)
        N_hyp = len(region.hyp_spk)
        N_correct = len(list(set(region.ref_spk) & set(region.hyp_spk)))
        miss += dur*(max(0, N_ref-N_hyp))
        false += dur*(max(0, N_hyp-N_ref))
        conf += dur*(min(N_ref, N_hyp)-N_correct)
        total_dur += dur*N_ref

    miss /= total_dur
    false /= total_dur
    conf /= total_dur
    der = miss + false + conf
    return Metrics(miss=miss, false=false, conf=conf, der=der)


def DER(ref, hyp):
    """Compute Diarization Error Rate.

    Args:
        ref: a list of tuples for the ground truth, where each tuple is
            (speaker, start, end) of type (string, float, float)
        hyp: a list of tuples for the diarization result hypothesis, same type
            as `ref`

    Returns:
        DER: Namedtuple containing (miss, false, conf, der)
    """
    check_input(ref)
    check_input(hyp)
    cost_matrix, ref_index, hyp_index = build_cost_matrix(ref, hyp)
    row_index, col_index = solve_dense(-cost_matrix)

    ref_rev_index = {v: k for k, v in ref_index.items()}
    hyp_rev_index = {v: k for k, v in hyp_index.items()}
    ref_index_mapped = dict()
    hyp_index_mapped = dict()
    for k, idx in enumerate(zip(row_index, col_index)):
        i, j = idx
        ref_index_mapped[ref_rev_index[i]] = k
        hyp_index_mapped[hyp_rev_index[j]] = k

    # Map remaining ref speakers
    ref_index_rem = set(ref_index.keys()) - set(ref_index_mapped.keys())
    for spk in ref_index_rem:
        k += 1
        ref_index_mapped[spk] = k

    # Map remaining hyp speakers
    hyp_index_rem = set(hyp_index.keys()) - set(hyp_index_mapped.keys())
    for spk in hyp_index_rem:
        k += 1
        hyp_index_mapped[spk] = k

    ref_mapped = [(ref_index_mapped[turn[0]], turn[1], turn[2])
                  for turn in ref]
    hyp_mapped = [(hyp_index_mapped[turn[0]], turn[1], turn[2])
                  for turn in hyp]

    der = compute_der(ref_mapped, hyp_mapped)
    return der
