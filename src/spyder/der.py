from _spyder import Turn, TurnList, compute_der


class DERMetrics:
    def __init__(self, metrics):
        self.miss = metrics.miss
        self.falarm = metrics.falarm
        self.conf = metrics.conf
        self.der = metrics.der

    def __repr__(self):
        return (
            f"Missed speech: {self.miss:.3f} \n"
            f"False alarm: {self.falarm:.3f} \n"
            f"Speaker error: {self.conf:.3f} \n"
            f"DER: {self.der:.3f}"
        )


def DER(ref, hyp):
    ref_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in ref])
    hyp_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in hyp])
    metrics = DERMetrics(compute_der(ref_turns, hyp_turns))
    return metrics
