from _spyder import Turn, TurnList, compute_der


def DER(ref, hyp):
    ref_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in ref])
    hyp_turns = TurnList([Turn(turn[0], turn[1], turn[2]) for turn in hyp])
    metrics = compute_der(ref_turns, hyp_turns)
    return metrics
