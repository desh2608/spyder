# spyder

A simple Python package for fast DER computation.

## Installation

```shell
pip install spy-der
```

For development, clone this repository and run:

```shell
pip install --editable .
```

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
# DERMetrics(miss=0.098,falarm=0.216,conf=0.255,der=0.569) 

print (f"{metrics.miss:.3f}, {metrics.falarm:.3f}, {metrics.conf:3f}, {metrics.der:.3f}")
# 0.098, 0.216, 0.254, 0.569
```

Alternatively, __spyder__ can also be invoked from the command line to compute the per-file
and average DERs between reference and hypothesis RTTMs.

```shell
Usage: spyder [OPTIONS] REF_RTTM HYP_RTTM

Options:
  --per-file      If this flag is set, print per file results.  [default:
                  False]

  --skip-missing  Skip recordings which are missing in hypothesis (i.e., not
                  counted in missed speech).  [default: False]

  --help          Show this message and exit.
```

Example:

```shell
> spyder ref_rttm hyp_rttm
Evaluated 16 recordings. Results:
╒═════════════╤════════════════╤═════════╤════════════╤═════════╤════════╕
│ Recording   │   Duration (s) │   Miss. │   F.Alarm. │   Conf. │    DER │
╞═════════════╪════════════════╪═════════╪════════════╪═════════╪════════╡
│ Overall     │       33952.95 │  11.48% │      2.27% │   9.81% │ 23.56% │
╘═════════════╧════════════════╧═════════╧════════════╧═════════╧════════╛
```

## Why spyder?

* __Fast:__ Implemented in pure C++, and faster than the alternatives (md-eval.pl,
dscore, pyannote.metrics). See this [benchmark](https://desh2608.github.io/2021-03-05-spyder/) 
for comparisons with other tools.
* __Stand-alone:__ It has no dependency on any other library. We have our own 
implementation of the Hungarian algorithm, for example, instead of using `scipy`.
* __Easy-to-use:__ No need to write the reference and hypothesis turns to files and
read md-eval output with complex regex patterns.
* __Overlap:__ Spyder supports overlapping speech in reference and hypothesis.


## Bugs/issues

Please raise an issue in the [issue tracker](https://github.com/desh2608/spyder/issues).
