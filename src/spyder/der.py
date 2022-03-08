from _spyder import Turn, TurnList, compute_der

import click
from tabulate import tabulate

from collections import defaultdict


class DERMetrics:
    def __init__(self, metrics):
        self.duration = metrics.duration
        self.miss = metrics.miss
        self.falarm = metrics.falarm
        self.conf = metrics.conf
        self.der = metrics.der

    def __repr__(self):
        return (
            "DERMetrics("
            f"duration={self.duration:.2f},"
            f"miss={self.miss:.2%},"
            f"falarm={self.falarm:.2%},"
            f"conf={self.conf:.2%},"
            f"der={self.der:.2%})"
        )


def DER(ref, hyp, regions="all"):
    ref_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in ref])
    hyp_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in hyp])
    metrics = DERMetrics(compute_der(ref_turns, hyp_turns, regions=regions))
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
@click.option(
    "--skip-missing",
    is_flag=True,
    default=False,
    show_default=True,
    help="Skip recordings which are missing in hypothesis (i.e., not counted in missed speech).",
)
@click.option(
    "--regions",
    type=click.Choice(["all", "single", "overlap", "nonoverlap"]),
    default="all",
    show_default=True,
    help="Only evaluate on the selected region type. Default is all. "
    " - all: all regions. "
    " - single: only single-speaker regions (ignore silence and multiple speaker). "
    " - overlap: only regions with multiple speakers in the reference. "
    " - nonoverlap: only regions without multiple speakers in the reference.",
)
def compute_der_from_rttm(
    ref_rttm, hyp_rttm, per_file=False, skip_missing=False, regions="all"
):
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

    all_metrics = []
    for reco_id in ref_turns:
        if reco_id not in hyp_turns:
            if skip_missing:
                print(f"Skipping recording {reco_id} since not present in hypothesis")
                continue
            else:
                hyp_turns[reco_id] = []
        metrics = DER(ref_turns[reco_id], hyp_turns[reco_id], regions=regions)
        all_metrics.append(
            [
                reco_id,
                metrics.duration,
                metrics.miss,
                metrics.falarm,
                metrics.conf,
                metrics.der,
            ]
        )

    print(f"Evaluated {len(all_metrics)} recordings on `{regions}` regions. Results:")

    total_duration = sum([x[1] for x in all_metrics])
    total_miss = sum([x[1] * x[2] for x in all_metrics])  # duration*miss
    total_falarm = sum([x[1] * x[3] for x in all_metrics])  # duration*falarm
    total_conf = sum([x[1] * x[4] for x in all_metrics])  # duration*conf

    miss = total_miss / total_duration
    falarm = total_falarm / total_duration
    conf = total_conf / total_duration
    der = miss + falarm + conf
    all_metrics.append(["Overall", total_duration, miss, falarm, conf, der])

    selected_metrics = all_metrics if per_file else [all_metrics[-1]]
    print(
        tabulate(
            selected_metrics,
            headers=["Recording", "Duration (s)", "Miss.", "F.Alarm.", "Conf.", "DER",],
            tablefmt="fancy_grid",
            floatfmt=[None, ".2f", ".2%", ".2%", ".2%", ".2%"],
        )
    )
