# spyder

A simple Python package for fast DER computation.

## Installation

```shell
pip install spy-der
```

To install version with latest features directly from Github:

```shell
pip install git+https://github.com/desh2608/spyder.git@main
```

For development, clone this repository and run:

```shell
pip install --editable .
```

## Usage
### Compute DER for a single pair of reference and hypothesis

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
# DERMetrics(duration=5.10,miss=9.80%,falarm=21.57%,conf=25.49%,der=56.86%)

print (f"{metrics.miss:.3f}, {metrics.falarm:.3f}, {metrics.conf:3f}, {metrics.der:.3f}")
# 0.098, 0.216, 0.254, 0.569

metrics2 = spyder.DER(ref, hyp, regions="single")
print(metrics2)
# DERMetrics(duration=4.10,miss=0.00%,falarm=26.83%,conf=19.51%,der=46.34%)
```

### Compute DER for multiple pairs of reference and hypothesis

```python
import spyder

# for multiple pairs, reference and hypothesis should be lists or dicts
# if lists, ref and hyp must have same length

# reference (ground truth)
ref = {"uttr0":[("A", 0.0, 2.0), # (speaker, start, end)
                ("B", 1.5, 3.5),
                ("A", 4.0, 5.1)],
       "uttr2":[("A", 0.0, 4.3), # (speaker, start, end)
                ("C", 6.0, 8.1),
                ("B", 2.0, 8.5)]}

# hypothesis (diarization result from your algorithm)
hyp = {"uttr0":[("1", 0.0, 0.8),
                ("2", 0.6, 2.3),
                ("3", 2.1, 3.9),
                ("1", 3.8, 5.2)],
       "uttr2":[("1", 0.0, 4.5),
                ("2", 2.5, 8.7)]}

metrics = spyder.DER(ref, hyp)
print(metrics)
# {'Overall': DERMetrics(duration=18.00,miss=17.22%,falarm=8.33%,conf=7.22%,der=32.78%)}

metrics2 = spyder.DER(ref, hyp, per_file=True, verbose=True)  # verbose=True to prints per-file results
```
Output:
```
Evaluated 2 recordings on `all` regions. Results:
╒═════════════╤════════════════╤═════════╤════════════╤═════════╤════════╕
│ Recording   │   Duration (s) │   Miss. │   F.Alarm. │   Conf. │    DER │
╞═════════════╪════════════════╪═════════╪════════════╪═════════╪════════╡
│ uttr0       │           5.10 │   9.80% │     21.57% │  25.49% │ 56.86% │
├─────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ uttr2       │          12.90 │  20.16% │      3.10% │   0.00% │ 23.26% │
├─────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ Overall     │          18.00 │  17.22% │      8.33% │   7.22% │ 32.78% │
╘═════════════╧════════════════╧═════════╧════════════╧═════════╧════════╛
```

### Compute per-file and overall DERs between reference and hypothesis RTTMs using command line tool

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
╒═════════════════════╤════════════════╤═════════╤════════════╤═════════╤════════╕
│ Recording           │   Duration (s) │   Miss. │   F.Alarm. │   Conf. │    DER │
╞═════════════════════╪════════════════╪═════════╪════════════╪═════════╪════════╡
│ EN2002a.Mix-Headset │        1290.50 │   0.04% │      6.24% │   8.59% │ 14.86% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ EN2002b.Mix-Headset │        1041.65 │   0.03% │      6.61% │   9.00% │ 15.65% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ EN2002c.Mix-Headset │        1906.56 │   0.02% │      3.30% │   2.58% │  5.90% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ EN2002d.Mix-Headset │        1258.10 │   0.04% │      6.76% │  10.92% │ 17.71% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004a.Mix-Headset │         644.22 │   0.03% │      3.58% │   7.21% │ 10.82% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004b.Mix-Headset │        1774.77 │   0.02% │      1.77% │   2.74% │  4.53% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004c.Mix-Headset │        1730.05 │   0.02% │      1.17% │   2.67% │  3.86% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004d.Mix-Headset │        1422.67 │   0.04% │      4.10% │  13.24% │ 17.37% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009a.Mix-Headset │         506.56 │   0.03% │      7.99% │   7.01% │ 15.03% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009b.Mix-Headset │        1565.30 │   0.02% │      3.13% │   1.92% │  5.08% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009c.Mix-Headset │        1413.05 │   0.03% │      4.06% │   2.21% │  6.29% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009d.Mix-Headset │        1364.54 │   0.03% │      4.48% │   5.42% │  9.94% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003a.Mix-Headset │         925.10 │   0.03% │      0.04% │  14.43% │ 14.50% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003b.Mix-Headset │        1685.06 │   0.03% │      0.87% │   1.91% │  2.81% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003c.Mix-Headset │        1731.52 │   0.02% │      2.71% │   3.17% │  5.90% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003d.Mix-Headset │        1651.62 │   0.04% │      4.30% │   5.38% │  9.73% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ Overall             │       21911.26 │   0.03% │      3.52% │   5.48% │  9.03% │
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


## Contributing

Contributions for core improvements or new recipes are welcome. Please run the following
before creating a pull request.

```bash
pre-commit install
pre-commit run # Running linter checks
```


## Bugs/issues

Please raise an issue in the [issue tracker](https://github.com/desh2608/spyder/issues).
