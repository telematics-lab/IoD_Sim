import pandas as pd
import matplotlib.pyplot as plt
import os
import seaborn as sns

def plot_benchmarks():
    csv_file = 'benchmark_results.csv'
    if not os.path.exists(csv_file):
        print(f"Error: {csv_file} not found.")
        return

    df = pd.read_csv(csv_file)
    
    # Filter only SUCCESS status
    df = df[df['status'] == 'SUCCESS']
    
    # Create graphs directory
    os.makedirs('graphs', exist_ok=True)
    
    # Setting the theme
    sns.set_theme(style="whitegrid")

    # To avoid the "amplitude" (confidence intervals) from seaborn averaging different configurations,
    # we will plot the real data points using scatter plots and distinct lines.

    # Aggregate data by taking the mean to remove any duplicate runs and confusing error bands
    df_agg = df.groupby(['num_satellites', 'num_vehicles', 'enable_nr', 'mobility_precision'])['execution_time_s'].mean().reset_index()

    # We will focus on NR=True and 100ms for the clearest trend
    df_trend = df_agg[(df_agg['enable_nr'] == True) & (df_agg['mobility_precision'] == '100ms')].copy()

    if not df_trend.empty:
        # Convert to string to force discrete categorical coloring
        df_trend['num_satellites_str'] = df_trend['num_satellites'].astype(str) + ' Sat'
        df_trend['num_vehicles_str'] = df_trend['num_vehicles'].astype(str) + ' Veh'

        # 1. Trend: Execution time vs Num Vehicles (grouped by satellites)
        plt.figure(figsize=(10, 6))
        sns.lineplot(data=df_trend, x='num_vehicles', y='execution_time_s', hue='num_satellites_str', marker='o', palette='tab10', errorbar=None)
        plt.title('Trend: Execution Time vs Number of Vehicles (NR=True, 100ms)')
        plt.xlabel('Number of Vehicles')
        plt.ylabel('Average Execution Time (s)')
        plt.grid(True)
        plt.legend(title='Satellites')
        plt.tight_layout()
        plt.savefig('graphs/trend_exec_vs_vehicles.png')
        plt.close()

        # 2. Trend: Execution time vs Num Satellites (grouped by vehicles)
        plt.figure(figsize=(10, 6))
        sns.lineplot(data=df_trend, x='num_satellites', y='execution_time_s', hue='num_vehicles_str', marker='o', palette='Set1', errorbar=None)
        plt.title('Trend: Execution Time vs Number of Satellites (NR=True, 100ms)')
        plt.xlabel('Number of Satellites')
        plt.ylabel('Average Execution Time (s)')
        plt.grid(True)
        plt.legend(title='Vehicles')
        plt.tight_layout()
        plt.savefig('graphs/trend_exec_vs_satellites.png')
        plt.close()

        # 3. Confronto Globale NR vs Non-NR (escludendo i casi limite con 0 satelliti o 0 veicoli)
        df_global_nr = df_agg[(df_agg['mobility_precision'] == '100ms') & 
                              (df_agg['num_satellites'] > 0) & 
                              (df_agg['num_vehicles'] > 0)].copy()
        if not df_global_nr.empty:
            # Create a proxy complexity metric to sort by
            df_global_nr['scenario_complexity'] = df_global_nr['num_vehicles'] + df_global_nr['num_satellites']
            
            # Sort by complexity to ensure the x-axis is ordered correctly
            df_global_nr = df_global_nr.sort_values('scenario_complexity')
            
            # Create the custom label
            df_global_nr['scenario_label'] = df_global_nr.apply(
                lambda row: f"{int(row['num_vehicles'])}+{int(row['num_satellites'])}\n({int(row['scenario_complexity'])})", axis=1
            )
            
            plt.figure(figsize=(12, 6))
            sns.lineplot(data=df_global_nr, x='scenario_label', y='execution_time_s', hue='enable_nr', marker='o', palette='Set1', errorbar=None, sort=False)
            plt.title('NR Stack Impact: Global Execution Time vs Complexity')
            plt.xlabel('Vehicles + Satellites\n(Sum)')
            plt.ylabel('Average Execution Time (s)')
            plt.xticks(rotation=45)
            plt.grid(True)
            plt.tight_layout()
            plt.savefig('graphs/trend_exec_nr_global_comparison.png')
            plt.close()

    print("Graphs generated successfully in tools/leo-benchmarks/graphs/")

if __name__ == "__main__":
    plot_benchmarks()
