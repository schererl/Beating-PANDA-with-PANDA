#!/usr/bin/env python3
import argparse
import csv
import os
from collections import defaultdict

def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Aggregate benchmark landmark info by Domain and Experiment Name (summing over problems), and add a total row per experiment."
    )
    parser.add_argument("-i", "--input_file", required=True, help="Path to the input CSV file.")
    parser.add_argument("-o", "--output_file", required=True, help="Path to the output aggregated CSV file.")
    return parser.parse_args()

def aggregate_data(input_file):
    """
    Reads the input CSV and aggregates rows by (Domain, Experiment Name).
    Assumes the CSV has columns:
      Domain, Problem, Experiment Name, Total Landmarks, Individual LMs, Disjunctions, Overall.
    Returns:
      aggregated: dict keyed by (domain, experiment) with sums for each numeric column.
      counts: dict keyed by (domain, experiment) with the count of problems.
    """
    numeric_columns = ["Total Landmarks", "Individual LMs", "Disjunctions", "Overall"]
    aggregated = defaultdict(lambda: {col: 0 for col in numeric_columns})
    counts = defaultdict(int)

    with open(input_file, "r", newline="", encoding="utf-8") as csvfile:
        reader = csv.DictReader(csvfile, delimiter=",")
        reader.fieldnames = [field.strip() for field in reader.fieldnames]
        for row in reader:
            domain = row.get("Domain", "").strip()
            experiment = row.get("Experiment Name", "").strip()
            key = (domain, experiment)
            counts[key] += 1
            for col in numeric_columns:
                try:
                    value = float(row.get(col, "0").strip())
                except ValueError:
                    value = 0
                aggregated[key][col] += value

    return aggregated, counts

def write_aggregated_csv(aggregated, counts, output_file):
    """
    Writes aggregated rows and, for each experiment, an additional "Total" row (summing across domains).
    Each row contains:
      Domain, Experiment Name, Count, Sum Total Landmarks, Sum Individual LMs, Sum Disjunctions, Sum Overall.
    """
    header = [
        "Domain",
        "Experiment Name",
        "Count",
        "Sum Total Landmarks",
        "Sum Individual LMs",
        "Sum Disjunctions",
        "Sum Overall"
    ]
    
    # We'll also aggregate totals by experiment across domains.
    experiment_totals = defaultdict(lambda: {col: 0 for col in ["Count", "Total Landmarks", "Individual LMs", "Disjunctions", "Overall"]})
    
    with open(output_file, "w", newline="", encoding="utf-8") as outfile:
        writer = csv.writer(outfile)
        writer.writerow(header)
        # Write each (Domain, Experiment) row
        for (domain, experiment), sums in aggregated.items():
            count = counts.get((domain, experiment), 0)
            row = [
                domain,
                experiment,
                count,
                sums["Total Landmarks"],
                sums["Individual LMs"],
                sums["Disjunctions"],
                sums["Overall"]
            ]
            writer.writerow(row)
            # Update experiment totals
            experiment_totals[experiment]["Count"] += count
            experiment_totals[experiment]["Total Landmarks"] += sums["Total Landmarks"]
            experiment_totals[experiment]["Individual LMs"] += sums["Individual LMs"]
            experiment_totals[experiment]["Disjunctions"] += sums["Disjunctions"]
            experiment_totals[experiment]["Overall"] += sums["Overall"]
        
        # Write a blank row for separation (optional)
        writer.writerow([])
        # Write total rows for each experiment
        for experiment, totals in experiment_totals.items():
            total_row = [
                "Total",  # Domain column is "Total"
                experiment,
                totals["Count"],
                totals["Total Landmarks"],
                totals["Individual LMs"],
                totals["Disjunctions"],
                totals["Overall"]
            ]
            writer.writerow(total_row)
    print(f"Aggregated results saved to {output_file}")

def main():
    args = parse_arguments()
    if not os.path.isfile(args.input_file):
        print(f"Error: The specified input file does not exist: {args.input_file}")
        return
    aggregated, counts = aggregate_data(args.input_file)
    write_aggregated_csv(aggregated, counts, args.output_file)

if __name__ == "__main__":
    main()
