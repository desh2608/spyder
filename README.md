# spyder

A simple Python package for fast DER computation.

## Installation

```shell
python -m pip install -e .
```

**Note:** **spyder** will soon be made available for installation from the PyPi repository. Stay tuned!

## Usage

```python
import spyder

# reference (ground truth)
ref = [("A", 0.0, 2.0), # (speaker, start, end)
       ("B", 1.5, 3.5),
       ("A", 4.0, 5.1)]

# hypothesis (diarization result from your algorithm)
hyp = [("1", 0.0, 0.8),
       ("2", 0.6, 2.3),
       ("3", 2.1, 3.9),
       ("1", 3.8, 5.2)]

metrics = spyder.DER(ref, hyp)
print(metrics)
# Missed speech: 0.098 
# False alarm: 0.216 
# Speaker error: 0.255 
# DER: 0.569

print (f"{metrics.miss:.3f}, {metrics.falarm:.3f}, {metrics.conf:3f}, {metrics.der:.3f}")
# 0.098, 0.216, 0.254902, 0.569
```

## Why spyder?

* __Fast:__ Implemented in pure C++, and faster than the alternatives (md-eval.pl,
dscore, pyannote.metrics).
* __Stand-alone:__ It has no dependency on any other library. We have our own 
implementation of the Hungarian algorithm, for example, instead of using `scipy`.
* __Easy-to-use:__ No need to write the reference and hypothesis turns to files and
read md-eval output with complex regex patterns.
* __Overlap:__ Spyder supports overlapping speech in reference and hypothesis.

## TODOs

- [] Benchmark speed comparisons with alternatives
- [] Provide binary for direct use from shell

## Bugs/issues

Please raise an issue in the [issue tracker](https://github.com/desh2608/spyder/issues).
