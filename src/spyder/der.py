from _spyder import Turn, TurnList, compute_der


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
