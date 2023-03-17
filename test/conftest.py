from collections import defaultdict

import pytest

__all__ = ["ref_turns", "hyp_turns", "uem_turns"]


@pytest.fixture(scope="module")
def ref_turns():
    ref_turns = defaultdict(list)
    with open("test/fixtures/ref.rttm", "r") as f:
        for line in f:
            parts = line.strip().split()
            spk = parts[7]
            start = float(parts[3])
            end = start + float(parts[4])
            ref_turns[parts[1]].append((spk, start, end))
    return ref_turns


@pytest.fixture(scope="module")
def hyp_turns():
    hyp_turns = defaultdict(list)
    with open("test/fixtures/hyp.rttm", "r") as f:
        for line in f:
            parts = line.strip().split()
            spk = parts[7]
            start = float(parts[3])
            end = start + float(parts[4])
            hyp_turns[parts[1]].append((spk, start, end))
    return hyp_turns


@pytest.fixture(scope="module")
def uem_turns():
    uem_turns = defaultdict(list)
    with open("test/fixtures/ref.uem", "r") as f:
        for line in f:
            parts = line.strip().split()
            start = float(parts[2])
            end = float(parts[3])
            uem_turns[parts[0]].append((start, end))
    return uem_turns
