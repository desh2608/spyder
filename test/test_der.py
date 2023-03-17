from test.conftest import *

import pytest

from spyder.der import *


@pytest.mark.parametrize(
    "collar, regions, expected",
    [
        (0.0, "all", 0.2639),
        (0.0, "single", 0.1907),
        (0.0, "overlap", 0.3727),
        (0.2, "all", 0.2426),
        (0.2, "single", 0.1935),
    ],
)
def test_der(ref_turns, hyp_turns, collar, regions, expected):
    der = DER(ref_turns, hyp_turns, collar=collar, regions=regions)
    assert der["Overall"].der == pytest.approx(expected=expected, rel=1e-2)


def test_der_with_uem(ref_turns, hyp_turns, uem_turns):
    der = DER(ref_turns, hyp_turns, uem=uem_turns)
    assert der["Overall"].duration == pytest.approx(13.79, rel=1e-2)
    assert der["Overall"].der == pytest.approx(expected=0.0967, rel=1e-2)
