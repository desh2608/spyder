<h1 align="center">SPYDER</h1>

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

# compute DER on full recording
print(spyder.DER(ref, hyp))
# DERMetrics(duration=5.10,miss=9.80%,falarm=21.57%,conf=25.49%,der=56.86%)

# compute DER on single-speaker regions only
print(spyder.DER(ref, hyp, regions="single"))
# DERMetrics(duration=4.10,miss=0.00%,falarm=26.83%,conf=19.51%,der=46.34%)

# compute DER using UEM segments
uem = [(0.5, 5.0)]
print(spyder.DER(ref, hyp, uem=uem))
# DERMetrics(duration=4.50,miss=11.11%,falarm=22.22%,conf=26.67%,der=60.00%)

# compute DER using collar
print(spyder.DER(ref, hyp, collar=0.2))
# DERMetrics(duration=3.10,miss=3.23%,falarm=12.90%,conf=19.35%,der=35.48%)

# get speaker mapping between reference and hypothesis
metrics = spyder.DER(ref, hyp)
print(f"Reference speaker map: {metrics.ref_map}")
print(f"Hypothesis speaker map: {metrics.hyp_map}")
# Reference speaker map: {'A': '0', 'B': '1'}
# Hypothesis speaker map: {'1': '0', '2': '2', '3': '1'}
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

Additionally, you can provide UEM and collar parameters similar to single pair case.

### Compute per-file and overall DERs between reference and hypothesis RTTMs using command line tool

Alternatively, __spyder__ can also be invoked from the command line to compute the per-file
and average DERs between reference and hypothesis RTTMs.

```shell
Usage: spyder [OPTIONS] REF_RTTM HYP_RTTM

Options:
  -u, --uem PATH                  UEM file (format: <recording_id> <channel>
                                  <start> <end>)

  -p, --per-file                  If this flag is set, print per file results.
                                  [default: False]

  -s, --skip-missing              Skip recordings which are missing in
                                  hypothesis (i.e., not counted in missed
                                  speech).  [default: False]

  -r, --regions [all|single|overlap|nonoverlap]
                                  Only evaluate on the selected region type.
                                  Default is all.  - all: all regions.  -
                                  single: only single-speaker regions (ignore
                                  silence and multiple speaker).  - overlap:
                                  only regions with multiple speakers in the
                                  reference.  - nonoverlap: only regions
                                  without multiple speakers in the reference.
                                  [default: all]

  -c, --collar FLOAT RANGE        Collar size.  [default: 0.0]
  -m, --print-speaker-map         Print speaker mapping for reference and
                                  hypothesis speakers.  [default: False]

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

> spyder ref_rttm hyp_rttm -r single -p -c 0.25
Evaluated 16 recordings on `single` regions. Results:
╒═════════════════════╤════════════════╤═════════╤════════════╤═════════╤════════╕
│ Recording           │   Duration (s) │   Miss. │   F.Alarm. │   Conf. │    DER │
╞═════════════════════╪════════════════╪═════════╪════════════╪═════════╪════════╡
│ EN2002a.Mix-Headset │        1032.05 │   0.00% │      2.98% │   4.97% │  7.94% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ EN2002b.Mix-Headset │         853.56 │   0.00% │      3.40% │   5.39% │  8.80% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ EN2002c.Mix-Headset │        1641.68 │   0.00% │      1.42% │   1.05% │  2.47% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ EN2002d.Mix-Headset │        1006.27 │   0.00% │      3.12% │   7.14% │ 10.26% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004a.Mix-Headset │         539.48 │   0.00% │      1.62% │   5.12% │  6.74% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004b.Mix-Headset │        1582.05 │   0.00% │      0.82% │   1.39% │  2.21% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004c.Mix-Headset │        1526.84 │   0.00% │      0.45% │   1.27% │  1.72% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ ES2004d.Mix-Headset │        1172.72 │   0.00% │      1.77% │   9.60% │ 11.37% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009a.Mix-Headset │         425.51 │   0.00% │      3.94% │   4.60% │  8.54% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009b.Mix-Headset │        1412.03 │   0.00% │      1.23% │   0.85% │  2.08% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009c.Mix-Headset │        1283.21 │   0.00% │      2.74% │   1.00% │  3.75% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ IS1009d.Mix-Headset │        1164.49 │   0.00% │      2.27% │   3.37% │  5.64% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003a.Mix-Headset │         804.27 │   0.00% │      0.00% │  11.28% │ 11.28% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003b.Mix-Headset │        1509.49 │   0.00% │      0.36% │   0.75% │  1.11% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003c.Mix-Headset │        1566.84 │   0.00% │      1.76% │   1.74% │  3.50% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ TS3003d.Mix-Headset │        1357.45 │   0.00% │      2.42% │   2.93% │  5.35% │
├─────────────────────┼────────────────┼─────────┼────────────┼─────────┼────────┤
│ Overall             │       18877.94 │   0.00% │      1.72% │   3.29% │  5.01% │
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
