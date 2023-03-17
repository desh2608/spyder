from collections import defaultdict

import click
import numpy as np
from tabulate import tabulate

from _spyder import Metrics, Turn, TurnList, compute_der


class DERMetrics:
    def __init__(self, metrics):
        self.duration = metrics.duration
        self.miss = metrics.miss
        self.falarm = metrics.falarm
        self.conf = metrics.conf
        self.der = metrics.der
        self.ref_map = metrics.ref_map
        self.hyp_map = metrics.hyp_map

    def __repr__(self):
        return (
            "DERMetrics("
            f"duration={self.duration:.2f},"
            f"miss={self.miss:.2%},"
            f"falarm={self.falarm:.2%},"
            f"conf={self.conf:.2%},"
            f"der={self.der:.2%})"
        )


def _DER(ref, hyp, uem, regions="all", collar=0.0):
    ref_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in ref])
    hyp_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in hyp])
    uem_turns = TurnList([Turn("dummy", turn[0], turn[1]) for turn in uem])
    metrics = DERMetrics(
        compute_der(ref_turns, hyp_turns, uem_turns, regions=regions, collar=collar)
    )
    return metrics


def _DER_multi(
    ref_turns: dict,
    hyp_turns: dict,
    uem_turns: dict,
    per_file=False,
    skip_missing=False,
    regions="all",
    collar=0.0,
    print_speaker_map=False,
    verbose=True,
):
    all_metrics = []
    speaker_maps = {}
    for reco_id in ref_turns:
        if reco_id not in hyp_turns:
            if skip_missing:
                if verbose:
                    print(
                        f"Skipping recording {reco_id} since not present in hypothesis"
                    )
                continue
            else:
                hyp_turns[reco_id] = []
        metrics = _DER(
            ref_turns[reco_id],
            hyp_turns[reco_id],
            uem_turns[reco_id],
            regions=regions,
            collar=collar,
        )
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
        speaker_maps[reco_id] = {"ref": metrics.ref_map, "hyp": metrics.hyp_map}

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
    if verbose:
        print(f"Evaluated {len(all_metrics)} recordings on `{regions}` regions.")
        if print_speaker_map:
            from pprint import pprint

            print("Speaker map:")
            pprint(speaker_maps, width=1)
        print("DER metrics:")
        print(
            tabulate(
                selected_metrics,
                headers=[
                    "Recording",
                    "Duration (s)",
                    "Miss.",
                    "F.Alarm.",
                    "Conf.",
                    "DER",
                ],
                tablefmt="fancy_grid",
                floatfmt=[None, ".2f", ".2%", ".2%", ".2%", ".2%"],
            )
        )
    return {m[0]: DERMetrics(Metrics(*m[1:-1])) for m in selected_metrics}


def get_uem_turns(ref_turns, hyp_turns):
    """
    Get UEM turns from ref and hyp turns.
    `ref_turns` and `hyp_turns` can be either a list of turns or a dict of
    recording id to list of turns.
    """
    if isinstance(ref_turns, list) and isinstance(hyp_turns, list):
        start = min([t[1] for t in ref_turns + hyp_turns])
        end = max([t[2] for t in ref_turns + hyp_turns])
        uem_turns = [(start, end)]
    else:
        uem_turns = defaultdict(list)
        for rec in ref_turns:
            turns = ref_turns[rec][:]
            if rec in hyp_turns:
                turns += hyp_turns[rec][:]
            start = min([t[1] for t in turns])
            end = max([t[2] for t in turns])
            uem_turns[rec].append((start, end))
    return uem_turns


def DER(
    ref,
    hyp,
    uem=None,
    per_file=False,
    skip_missing=False,
    regions="all",
    collar=0.0,
    verbose=False,
):
    """
    Compute DER between ref and hyp.
    The following formats are supported for `ref` and `hyp`:
        - list of tuples: list of turns of single recording
        - dict: {recording_id: list of turns}
        - list of ndarrays: list of numpy arrays; each array contains turns of a recording

    Args:
        ref (dict or list): Reference turns.
        hyp (dict or list): Hypothesis turns.
        uem (dict or list): UEM turns. If None, we will use the union of ref and hyp.
        per_file (bool): If True, return DER for each file. Otherwise, return overall DER.
        skip_missing (bool): If True, skip missing files in hypothesis from evaluation.
        regions (str): Regions to evaluate. Possible options are:
            - 'all': Evaluate on all regions.
            - 'single': Evaluate on single speaker regions (ignore silence and multiple speaker).
            - 'overlap': Evaluate on regions with multiple speakers in the reference.
            - 'nonoverlap': Evaluate on regions without multiple speakers in the reference,
                i.e. single speaker regions and silence regions.
        collar (float): Collar size in seconds.
        verbose (bool): If True, print DER for each file.

    Returns:
        dict: {recording_id: DERMetrics} if per_file is True, otherwise {overall: DERMetrics}.
        DERMetrics is a named tuple with the following fields:
            - duration: Duration of the scored regions.
            - miss: Missed speech %.
            - falarm: False alarm %.
            - conf: Confusion error %.
            - der: Diarization error rate %.
            - ref_map: Speaker map from reference to common labels.
            - hyp_map: Speaker map from hypothesis to common labels.
    """
    if uem is None:
        uem = get_uem_turns(ref, hyp)
    if isinstance(ref, dict) and isinstance(hyp, dict):
        assert isinstance(uem, dict), "UEM must be dict if ref and hyp are dict"
        metrics = _DER_multi(
            ref, hyp, uem, per_file, skip_missing, regions, collar, verbose
        )
    elif np.ndim(ref[-1]) == 2 and np.ndim(hyp[-1]) == 2:
        # the first dimension is the number of utterances
        # in this case ref and hyp must have same length
        if len(ref) != len(hyp):
            raise ValueError("if ref and hyp are not dict, they must have same length")

        assert len(uem) == len(ref), "uem must have same length as ref and hyp"

        # convert list into dict, indexed by utterance index
        ref_turns = dict(zip(range(len(ref)), ref))
        hyp_turns = dict(zip(range(len(hyp)), hyp))
        uem_turns = dict(zip(range(len(uem)), uem))

        metrics = _DER_multi(
            ref_turns,
            hyp_turns,
            uem_turns,
            per_file,
            skip_missing,
            regions,
            collar,
            verbose,
        )
    elif np.ndim(ref[-1]) == 1 and np.ndim(hyp[-1]) == 1:
        assert isinstance(uem, list), "UEM must be list if ref and hyp are list"
        # only one utterance
        metrics = _DER(ref, hyp, uem, regions, collar)
        if verbose:
            print(metrics)
    else:
        raise ValueError("ref and hyp must be either dict or list")
    return metrics


@click.command()
@click.argument("ref_rttm", nargs=1, type=click.Path(exists=True))
@click.argument("hyp_rttm", nargs=1, type=click.Path(exists=True))
@click.option(
    "--uem",
    "-u",
    nargs=1,
    type=click.Path(exists=True),
    default=None,
    help="UEM file (format: <recording_id> <channel> <start> <end>)",
)
@click.option(
    "--per-file",
    "-p",
    is_flag=True,
    default=False,
    show_default=True,
    help="If this flag is set, print per file results.",
)
@click.option(
    "--skip-missing",
    "-s",
    is_flag=True,
    default=False,
    show_default=True,
    help="Skip recordings which are missing in hypothesis (i.e., not counted in missed speech).",
)
@click.option(
    "--regions",
    "-r",
    type=click.Choice(["all", "single", "overlap", "nonoverlap"]),
    default="all",
    show_default=True,
    help="Only evaluate on the selected region type. Default is all. "
    " - all: all regions. "
    " - single: only single-speaker regions (ignore silence and multiple speaker). "
    " - overlap: only regions with multiple speakers in the reference. "
    " - nonoverlap: only regions without multiple speakers in the reference.",
)
@click.option(
    "--collar",
    "-c",
    type=click.FloatRange(min=0.0),
    default=0.0,
    show_default=True,
    help="Collar size.",
)
@click.option(
    "--print-speaker-map",
    "-m",
    is_flag=True,
    default=False,
    show_default=True,
    help="Print speaker mapping for reference and hypothesis speakers.",
)
def compute_der_from_rttm(
    ref_rttm,
    hyp_rttm,
    uem=None,
    per_file=False,
    skip_missing=False,
    regions="all",
    collar=0.0,
    print_speaker_map=False,
    verbose=True,
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

    uem_turns = defaultdict(list)
    if uem is not None:
        with open(uem, "r") as f:
            for line in f:
                parts = line.strip().split()
                start = float(parts[2])
                end = float(parts[3])
                uem_turns[parts[0]].append((start, end))
    else:
        uem_turns = get_uem_turns(ref_turns, hyp_turns)

    _DER_multi(
        ref_turns,
        hyp_turns,
        uem_turns,
        per_file,
        skip_missing,
        regions,
        collar,
        print_speaker_map,
        verbose,
    )
