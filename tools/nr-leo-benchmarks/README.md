# NR LEO Benchmarks

Tools to benchmark the execution time of the IoD_Sim simulator across a grid of
NR LEO scenario parameters (number of satellites, number of vehicles, mobility
precision, NR stack on/off, duration), and to plot the results.

## 1. Compile the simulator binary first

`run_bench.py` does not build the simulator, it only runs an already
compiled binary. Before running any benchmark you must produce it via the
Docker build in [`tools/compile`](../compile):

```bash
cd tools/compile
docker compose up --build
```

This builds the `ns3.48-scenario-default` binary and drops it into
`tools/compile/bin/`. `run_bench.py` picks the alphabetically-first binary in
that directory, so if it's empty or missing it will exit immediately with an
error asking you to compile first.

## 2. Install Python dependencies

```bash
cd tools/leo-benchmarks
pip install -r requirements.txt
```

(`PyYAML` for `run_bench.py`, `pandas`/`matplotlib`/`seaborn` for
`plot_benchmarks.py`.)

## 3. Configure the benchmark grid

Edit [`benchmark_config.yaml`](benchmark_config.yaml). Every combination of
the listed values is run (Cartesian product):

```yaml
num_satellites: [0, 5, 10, 100]
num_vehicles: [0, 5, 10, 100, 300]
mobility_precisions: ["100ms", "50ms"]
enable_nr: [true, false]
durations: [0.3]
repetitions: 3
```

- `repetitions` controls how many times each configuration is run so the
  execution time can be averaged out (simulator warm-up, host noise, etc.).
  Defaults to `1` if omitted.

## 4. Run the benchmarks

```bash
python3 run_bench.py [-j N]
```

- `-j/--jobs`: number of scenarios to run concurrently (default `1`). Each
  worker runs one full configuration (all its `repetitions` runs) sequentially
  before moving to the next task, so total parallelism is bounded by CPU/RAM
  available for concurrent simulator processes.
- For each configuration, a scenario JSON is generated once into
  `scenarios/` and the simulator binary is invoked against it `repetitions`
  times.
- Results are appended to `benchmark_results.csv` (one row per run), with
  columns: `num_satellites, num_vehicles, mobility_precision, enable_nr,
  duration, run, execution_time_s, status`.
- Simulator output for each run is written under `results/` (one
  timestamped directory per run).
- On non-Linux hosts, the script re-executes itself inside a Docker
  container automatically.

The CSV is append-only across invocations — delete or move
`benchmark_results.csv` if you want a clean run instead of accumulating rows.

## 5. Plot the results

```bash
python3 plot_benchmarks.py [--csv benchmark_results.csv] [--output-dir graphs] \
                            [--enable-nr true|false] [--precision 100ms]
```

- Execution times are averaged across all `repetitions` of the same
  configuration (only `SUCCESS` runs count) before plotting.
- `--enable-nr` / `--precision` select which configuration slice to use for
  the two trend plots (execution time vs. vehicles, vs. satellites) and the
  NR-impact comparison plot — no longer hardcoded to NR-enabled/100ms.
- `--list-configs` prints every available configuration (with its averaged
  execution time and number of successful runs) instead of plotting, useful
  to check what values are valid for `--enable-nr`/`--precision`.

Generated plots are saved as PNGs in the output directory (`graphs/` by
default):

- `trend_exec_vs_vehicles.png`
- `trend_exec_vs_satellites.png`
- `trend_exec_nr_global_comparison.png`
