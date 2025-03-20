#!/bin/bash

# Folder containing benchmark domains (adjust as needed)
folder_benchmarks="benchmarks/*"

# Time and memory limits (in seconds and kilobytes)
TIME_LIMIT=1800
MEM_LIMIT=8388608

# List of domains to ignore
ignored_domains=("Blocksworld-GTOHP" "Depots" "AssemblyHierarchical" "Barman-BDI" "Blocksworld-HPDDL" "Freecell-Learned-ECAI-16" "Factories-simple")

# Iterate over each domain directory
for domain_dir in $folder_benchmarks; do
    if [ -d "$domain_dir" ]; then
        domain_name=$(basename "$domain_dir")
        if [[ " ${ignored_domains[@]} " =~ " ${domain_name} " ]]; then
            echo "Skipping ignored domain: $domain_name"
            continue
        fi
        
        echo "Processing domain: $domain_name"
        
        # Find all .hddl files in the domain directory (excluding grounded files)
        files=($(find "$domain_dir" -maxdepth 1 -type f -name "*.hddl" ! -iname "*-grounded*"))
        
        domain_files=()
        problem_files=()
        for file in "${files[@]}"; do
            if [[ "$(basename "$file")" == *domain* ]]; then
                domain_files+=("$file")
            else
                problem_files+=("$file")
            fi
        done

        # If there is only one domain file, use it for all problem files
        if [ ${#domain_files[@]} -eq 1 ]; then
            domain_file="${domain_files[0]}"
            for problem_file in "${problem_files[@]}"; do
                for option in {1..5}; do
                    echo "Domain: $domain_name, Problem: $(basename "$problem_file"), Option: $option"
                    (
                        ulimit -t $TIME_LIMIT
                        ulimit -v $MEM_LIMIT
                        ./run-lama-panda.sh run "$domain_file" "$problem_file" "$option"
                    )
                    echo "@"
                done
                # Clean up temporary files
                rm -f ./*.psas
            done
        else
            # If multiple domain files exist, pair each with its corresponding problem(s)
            domain_files=($(find "$domain_dir" -maxdepth 1 -type f -name "*-domain.hddl" -o -name "*-domain-grounded.hddl"))
            for domain_file in "${domain_files[@]}"; do
                base_name=$(basename "$domain_file" | sed 's/-domain-grounded.hddl$//;s/-domain.hddl$//')
                corresponding_problems=($(find "$domain_dir" -maxdepth 1 -type f -name "${base_name}.hddl" ! -name "*-domain*"))
                for problem_file in "${corresponding_problems[@]}"; do
                    for option in {1..5}; do
                        echo "Domain: $domain_name, Problem: $(basename "$problem_file"), Option: $option"
                        (
                            ulimit -t $TIME_LIMIT
                            ulimit -v $MEM_LIMIT
                            ./run-lama-panda.sh run "$domain_file" "$problem_file" "$option"
                        )
                        echo "@"
                    done
                    rm -f ./*.psas
                done
            done
        fi
    fi
done
