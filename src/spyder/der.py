from _spyder import Turn, TurnList, compute_der

import click
from collections import defaultdict


class DERMetrics:
    def __init__(self, metrics):
        self.miss = metrics.miss
        self.falarm = metrics.falarm
        self.conf = metrics.conf
        self.der = metrics.der

    def __repr__(self):
        return (
            "DERMetrics("
            f"miss={100*self.miss:.2f},"
            f"falarm={100*self.falarm:.2f},"
            f"conf={100*self.conf:.2f},"
            f"der={100*self.der:.2f})"
        )


def DER(ref, hyp):
    ref_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in ref])
    hyp_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in hyp])
    metrics = DERMetrics(compute_der(ref_turns, hyp_turns))
    return metrics


@click.command()
@click.argument("ref_rttm", nargs=1, type=click.Path(exists=True))
@click.argument("hyp_rttm", nargs=1, type=click.Path(exists=True))
@click.option(
    "--per-file",
    is_flag=True,
    default=False,
    show_default=True,
    help="If this flag is set, print per file results.",
)
def compute_der_from_rttm(ref_rttm, hyp_rttm, per_file=False):
    ref_turns = defaultdict(list)
    hyp_turns = defaultdict(list)

    with open(ref_rttm, "r") as f:
        for line in f:
            parts = line.strip().split()
            spk = parts[7]
            start = float(parts[3])
            end = start + float(parts[4])
            ref_turns[parts[1]].append((spk, start, end))

    with open(hyp_rttm, "r") as f:
        for line in f:
            parts = line.strip().split()
            spk = parts[7]
            start = float(parts[3])
            end = start + float(parts[4])
            hyp_turns[parts[1]].append((spk, start, end))

    total_duration = 0
    total_miss = 0
    total_falarm = 0
    total_conf = 0
    for reco_id in ref_turns:
        if reco_id not in hyp_turns:
            print(f"Recording {reco_id} not present in hypothesis")
            continue
        metrics = DER(ref_turns[reco_id], hyp_turns[reco_id])
        if per_file:
            print(f"{reco_id}: {metrics}")
        duration = sum([turn[2] - turn[1] for turn in ref_turns[reco_id]])
        total_duration += duration
        total_miss += duration * metrics.miss
        total_falarm += duration * metrics.falarm
        total_conf += duration * metrics.conf

    miss = total_miss / total_duration
    falarm = total_falarm / total_duration
    conf = total_conf / total_duration
    der = miss + falarm + conf
    print("Average error rates:")
    print("----------------------------------------------------")
    print(f"Missed speaker time = {miss:.2%}")
    print(f"False alarm speaker time = {falarm:.2%}")
    print(f"Speaker error time = {conf:.2%}")
    print(f"Diarization error rate (DER) = {der:.2%}")
