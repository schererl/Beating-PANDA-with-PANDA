#!/usr/bin/env python3
import argparse
import re
import sys

def parse_block(block):
    """
    Parse a log block and extract the desired fields.
    Returns a dictionary with keys:
    Domain, Problem, Experiment Name, Total Landmarks,
    Individual LMs, Disjunctions, Overall, Unary Methods,
    Unary Action, Overall Action, Overall Method.
    """
    data = {}

    # Domain and Problem
    m = re.search(r'Domain:\s*([^,]+),\s*Problem:\s*([^,]+)', block)
    if m:
        data['Domain'] = m.group(1).strip()
        data['Problem'] = m.group(2).strip()
    else:
        data['Domain'] = ""
        data['Problem'] = ""

    # Experiment Name
    m = re.search(r'Experiment Name:\s*(.+)', block)
    data['Experiment Name'] = m.group(1).strip() if m else ""

    # Total Landmarks: from "- Found 17 landmark(s)" pattern
    m = re.search(r'- Found\s+(\d+)\s+landmark\(s\)', block)
    data['Total Landmarks'] = m.group(1).strip() if m else ""

    # Individual LMs
    m = re.search(r'- individual LMs\s+(\d+)', block)
    data['Individual LMs'] = m.group(1).strip() if m else ""

    # Disjunctions
    m = re.search(r'- disjunctions\s+(\d+)', block)
    data['Disjunctions'] = m.group(1).strip() if m else ""

    # Overall value
    m = re.search(r'- overall:\s+(\d+)', block)
    data['Overall'] = m.group(1).strip() if m else ""

    # Unary Methods
    m = re.search(r'- unary method LMs:\s+(\d+)', block)
    data['Unary Methods'] = m.group(1).strip() if m else ""

    # Unary Action
    m = re.search(r'- unary action LMs:\s+(\d+)', block)
    data['Unary Action'] = m.group(1).strip() if m else ""

    # Overall Action
    m = re.search(r'- overall action LMs:\s+(\d+)', block)
    data['Overall Action'] = m.group(1).strip() if m else ""

    # Overall Method
    m = re.search(r'- overall method LMs:\s+(\d+)', block)
    data['Overall Method'] = m.group(1).strip() if m else ""

    return data

def main():
    parser = argparse.ArgumentParser(description='Parse log file and output CSV.')
    parser.add_argument('-i', '--input', required=True, help='Input log file')
    parser.add_argument('-o', '--output', required=True, help='Output CSV file')
    args = parser.parse_args()

    try:
        with open(args.input, "r") as f:
            content = f.read()
    except Exception as e:
        sys.stderr.write(f"Error reading file {args.input}: {e}\n")
        sys.exit(1)

    # Split the log into blocks using "@" as a separator.
    blocks = re.split(r'@+', content)
    blocks = [block for block in blocks if block.strip()]

    # Define the fields (order matters for CSV)
    fields = [
        "Domain", "Problem", "Experiment Name", "Total Landmarks",
        "Individual LMs", "Disjunctions", "Overall",
        "Unary Methods", "Unary Action", "Overall Action", "Overall Method"
    ]

    try:
        with open(args.output, "w") as out_file:
            # Write CSV header
            out_file.write(",".join(fields) + "\n")
            # Process each block and write a CSV row
            for block in blocks:
                data = parse_block(block)
                row = [data.get(field, "") for field in fields]
                out_file.write(",".join(row) + "\n")
    except Exception as e:
        sys.stderr.write(f"Error writing to file {args.output}: {e}\n")
        sys.exit(1)

if __name__ == '__main__':
    main()
