from _spyder import Turn, TurnList, compute_jer

import click
from tabulate import tabulate

from collections import defaultdict


class JERMetrics:
    def __init__(self, metrics):
        self.duration = metrics.duration
        self.jer = metrics.jer

    def __repr__(self):
        return "JERMetrics(" f"duration={self.duration:.2f}," f"jer={self.jer:.2%})"


def JER(ref, hyp):
    ref_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in ref])
    hyp_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in hyp])
    metrics = JERMetrics(compute_jer(ref_turns, hyp_turns))
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
def compute_jer_from_rttm(ref_rttm, hyp_rttm, per_file=False, skip_missing=False):
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
        metrics = JER(ref_turns[reco_id], hyp_turns[reco_id])
        all_metrics.append([reco_id, metrics.duration, metrics.jer])

    print(f"Evaluated {len(all_metrics)} recordings. Results:")

    total_duration = sum([x[1] for x in all_metrics])
    total_jer = sum([x[2] for x in all_metrics])
    jer = 100 * total_jer / total_duration
    all_metrics.append(["Overall", total_duration, jer])

    selected_metrics = all_metrics if per_file else [all_metrics[-1]]
    print(
        tabulate(
            selected_metrics,
            headers=["Recording", "Duration (s)", "JER",],
            tablefmt="fancy_grid",
            floatfmt=[None, ".2f", ".2%"],
        )
    )
