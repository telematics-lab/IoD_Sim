import argparse
import os

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns


def str2bool(value):
    if value.lower() in ("true", "1", "yes"):
        return True
    if value.lower() in ("false", "0", "no"):
        return False
    raise argparse.ArgumentTypeError(f"Expected a boolean value, got '{value}'")


def parse_args():
    parser = argparse.ArgumentParser(description="Plot leo-benchmarks results")
    parser.add_argument("--csv", default="benchmark_results.csv", help="Path to the benchmark results CSV")
    parser.add_argument("--output-dir", default="graphs", help="Directory where plots will be saved")
    parser.add_argument("--enable-nr", type=str2bool, default=True,
                         help="Which enable_nr configuration to use for the trend plots (true/false)")
    parser.add_argument("--precision", default="100ms",
                         help="Which mobility_precision configuration to use for the trend plots (e.g. 100ms, 50ms)")
    parser.add_argument("--list-configs", action="store_true",
                         help="List available (num_satellites, num_vehicles, enable_nr, mobility_precision, duration) "
                              "configurations found in the CSV, with their run counts, then exit")
    return parser.parse_args()


def add_duration_reference_lines(durations):
    """Draw a red dash-dotted horizontal line for each distinct simulated scenario
    duration present in the plotted data, so it can be compared against the
    (real) execution time."""
    for duration in sorted(pd.unique(durations)):
        plt.axhline(y=duration, color="red", linestyle="-.", linewidth=1.5,
                    label=f"Scenario duration ({duration}s)")


def load_averaged_results(csv_file):
    df = pd.read_csv(csv_file)

    # Only successful runs contribute to the average.
    df = df[df["status"] == "SUCCESS"]

    group_cols = ["num_satellites", "num_vehicles", "mobility_precision", "enable_nr", "duration"]

    # Average execution_time_s across the repeated runs of each configuration.
    df_agg = (
        df.groupby(group_cols)["execution_time_s"]
        .agg(execution_time_s="mean", num_runs="count")
        .reset_index()
    )
    return df_agg


def plot_benchmarks():
    args = parse_args()

    if not os.path.exists(args.csv):
        print(f"Error: {args.csv} not found.")
        return

    df_agg = load_averaged_results(args.csv)

    if args.list_configs:
        print(df_agg.to_string(index=False))
        return

    os.makedirs(args.output_dir, exist_ok=True)
    sns.set_theme(style="whitegrid")

    # To avoid the "amplitude" (confidence intervals) from seaborn averaging different configurations,
    # we will plot the real data points using scatter plots and distinct lines.

    df_trend = df_agg[
        (df_agg["enable_nr"] == args.enable_nr) & (df_agg["mobility_precision"] == args.precision)
    ].copy()

    if df_trend.empty:
        print(
            f"No data found for enable_nr={args.enable_nr}, mobility_precision={args.precision}. "
            f"Use --list-configs to see available configurations."
        )
        return

    # Convert to string to force discrete categorical coloring
    df_trend["num_satellites_str"] = df_trend["num_satellites"].astype(str) + " Sat"
    df_trend["num_vehicles_str"] = df_trend["num_vehicles"].astype(str) + " Veh"

    # 1. Trend: Execution time vs Num Vehicles (grouped by satellites)
    plt.figure(figsize=(10, 6))
    sns.lineplot(data=df_trend, x="num_vehicles", y="execution_time_s", hue="num_satellites_str", marker="o", palette="tab10", errorbar=None)
    add_duration_reference_lines(df_trend["duration"])
    plt.title(f"Trend: Execution Time vs Number of Vehicles (NR={args.enable_nr}, {args.precision})")
    plt.xlabel("Number of Vehicles")
    plt.ylabel("Average Execution Time (s)")
    plt.grid(True)
    plt.legend(title="Satellites")
    plt.tight_layout()
    plt.savefig(os.path.join(args.output_dir, "trend_exec_vs_vehicles.png"))
    plt.close()

    # 2. Trend: Execution time vs Num Satellites (grouped by vehicles)
    plt.figure(figsize=(10, 6))
    sns.lineplot(data=df_trend, x="num_satellites", y="execution_time_s", hue="num_vehicles_str", marker="o", palette="Set1", errorbar=None)
    add_duration_reference_lines(df_trend["duration"])
    plt.title(f"Trend: Execution Time vs Number of Satellites (NR={args.enable_nr}, {args.precision})")
    plt.xlabel("Number of Satellites")
    plt.ylabel("Average Execution Time (s)")
    plt.grid(True)
    plt.legend(title="Vehicles")
    plt.tight_layout()
    plt.savefig(os.path.join(args.output_dir, "trend_exec_vs_satellites.png"))
    plt.close()

    # 3. Global NR vs non-NR comparison (excluding the edge cases with 0 satellites or 0 vehicles)
    df_global_nr = df_agg[
        (df_agg["mobility_precision"] == args.precision)
        & (df_agg["num_satellites"] > 0)
        & (df_agg["num_vehicles"] > 0)
    ].copy()
    if not df_global_nr.empty:
        # Create a proxy complexity metric to sort by
        df_global_nr["scenario_complexity"] = df_global_nr["num_vehicles"] + df_global_nr["num_satellites"]

        # Sort by complexity to ensure the x-axis is ordered correctly
        df_global_nr = df_global_nr.sort_values("scenario_complexity")

        # Create the custom label
        df_global_nr["scenario_label"] = df_global_nr.apply(
            lambda row: f"{int(row['num_vehicles'])}+{int(row['num_satellites'])}\n({int(row['scenario_complexity'])})", axis=1
        )

        plt.figure(figsize=(12, 6))
        sns.lineplot(data=df_global_nr, x="scenario_label", y="execution_time_s", hue="enable_nr", marker="o", palette="Set1", errorbar=None, sort=False)
        add_duration_reference_lines(df_global_nr["duration"])
        plt.title(f"NR Stack Impact: Global Execution Time vs Complexity ({args.precision})")
        plt.xlabel("Vehicles + Satellites\n(Sum)")
        plt.ylabel("Average Execution Time (s)")
        plt.xticks(rotation=45)
        plt.grid(True)
        plt.legend(title="enable_nr")
        plt.tight_layout()
        plt.savefig(os.path.join(args.output_dir, "trend_exec_nr_global_comparison.png"))
        plt.close()

    print(f"Graphs generated successfully in {args.output_dir}/")


if __name__ == "__main__":
    plot_benchmarks()
