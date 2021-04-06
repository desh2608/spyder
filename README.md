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

metrics2 = spyder.DER(ref, hyp, regions="single")
print(metrics2)
# DERMetrics(miss=0.00,falarm=26.83,conf=19.51,der=46.34)
```

Alternatively, __spyder__ can also be invoked from the command line to compute the per-file
and average DERs between reference and hypothesis RTTMs.

```shell
Usage: spyder [OPTIONS] REF_RTTM HYP_RTTM

Options:
  --per-file                      If this flag is set, print per file results.
                                  [default: False]

  --skip-missing                  Skip recordings which are missing in
                                  hypothesis (i.e., not counted in missed
                                  speech).  [default: False]

  --regions [all|single|overlap]  Only evaluate on the selected region type.
                                  For example, if `single` is selected, all
                                  overlapping regions are ignored for
                                  evaluation.  [default: all]

  --help                          Show this message and exit.
```

Examples:

```shell
> spyder ref_rttm hyp_rttm
Evaluated 16 recordings on `all` regions. Results:
╒═════════════╤════════════════╤═════════╤════════════╤═════════╤════════╕
│ Recording   │   Duration (s) │   Miss. │   F.Alarm. │   Conf. │    DER │
╞═════════════╪════════════════╪═════════╪════════════╪═════════╪════════╡
│ Overall     │       33952.95 │  11.48% │      2.27% │   9.81% │ 23.56% │
╘═════════════╧════════════════╧═════════╧════════════╧═════════╧════════╛

> spyder ref_rttm hyp_rttm --regions single --per-file
Evaluated 16 recordings on `single` regions. Results:
Evaluated 16 recordings on `single` regions. Results:
╒═════════════════════╤════════════════╤═════════╤════════════╤═════════╤════════╕
│ Recording           │   Duration (s) │   Miss. │   F.Alarm. │   Conf. │    DER │
╞═════════════════════╪════════════════╪═════════╪════════════╪═════════╪════════╡
│ EN2002a.Mix-Headset │        2910.97 │   0.04% │      6.24% │   8.59% │ 14.86% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ EN2002b.Mix-Headset │        2173.78 │   0.03% │      6.61% │   9.00% │ 15.65% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ EN2002c.Mix-Headset │        3551.64 │   0.02% │      3.30% │   2.58% │  5.90% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ EN2002d.Mix-Headset │        3042.98 │   0.04% │      6.76% │  10.92% │ 17.71% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004a.Mix-Headset │        1051.71 │   0.03% │      3.58% │   7.21% │ 10.82% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004b.Mix-Headset │        2403.80 │   0.02% │      1.77% │   2.74% │  4.53% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004c.Mix-Headset │        2439.53 │   0.02% │      1.17% │   2.67% │  3.86% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004d.Mix-Headset │        2258.48 │   0.04% │      4.10% │  13.24% │ 17.37% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009a.Mix-Headset │         771.77 │   0.03% │      7.99% │   7.01% │ 15.03% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009b.Mix-Headset │        2074.64 │   0.02% │      3.13% │   1.92% │  5.08% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009c.Mix-Headset │        1680.34 │   0.03% │      4.06% │   2.21% │  6.29% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009d.Mix-Headset │        1891.66 │   0.03% │      4.48% │   5.42% │  9.94% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003a.Mix-Headset │        1209.19 │   0.03% │      0.04% │  14.43% │ 14.50% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003b.Mix-Headset │        2011.71 │   0.03% │      0.87% │   1.91% │  2.81% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003c.Mix-Headset │        2086.65 │   0.02% │      2.71% │   3.17% │  5.90% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003d.Mix-Headset │        2394.10 │   0.04% │      4.30% │   5.38% │  9.73% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ Overall             │       33952.95 │   0.03% │      3.85% │   5.94% │  9.82% │
╘═════════════════════╧════════════════╧═════════╧════════════╧═════════╧════════╛
```

## Why spyder?

* __Fast:__ Implemented in pure C++, and faster than the alternatives (md-eval.pl,
dscore, pyannote.metrics). See this [benchmark](https://desh2608.github.io/2021-03-05-spyder/) 
for comparisons with other tools.
* __Stand-alone:__ It has no dependency on any other library. We have our own 
implementation of the Hungarian algorithm, for example, instead of using `scipy`.
* __Easy-to-use:__ No need to write the reference and hypothesis turns to files and
read md-eval output with complex regex patterns.
* __Overlap:__ Spyder supports overlapping speech in reference and hypothesis. In addition,
you can compute metrics on just the single-speaker or overlap regions by passing the
keyword argument `regions="single"` or `regions="overlap"`, respectively.


## Bugs/issues

Please raise an issue in the [issue tracker](https://github.com/desh2608/spyder/issues).
