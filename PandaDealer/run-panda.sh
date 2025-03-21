#!/bin/bash

# Usage message
usage() {
    echo "Usage:"
    echo "  $0 install                    # Install dependencies"
    echo "  $0 build                      # Build the full planner"
    echo "  $0 update                     # Update (rebuild) pandaEngine (02-planner)"
    echo "  $0 run DOMAINFILE PROBLEMFILE OPTION"
    echo " OPTION:"
        echo "  1: lmCut (standard)"
        echo "  2: lmc-BD"
        echo "  3: lmc-GZD"
        echo "  4: lmc-BDpGZD"
        echo "  5: lmc-VDM"
    exit 1
}

# Install necessary dependencies (requires sudo)
install_dependencies() {
    echo "Updating package list and installing dependencies..."
    sudo apt-get update
    sudo apt-get -y install make g++ bison cmake gengetopt flex unzip patch
    echo "Dependencies installed."
}

# Build the full planner from source
build_planner() {
    echo "Building planner..."

    # Build 00-parser
    echo "Building 00-parser..."
    cd 00-parser
    make
    mv pandaPIparser ..
    cd ..

    # Build 01-grounder
    echo "Building 01-grounder..."
    cd 01-grounder/cpddl
    make boruvka opts bliss lpsolve
    make
    cd ../src
    make
    cd ../
    mv pandaPIgrounder ../
    cd ../

    # Build 02-planner (pandaEngine)
    echo "Building 02-planner (pandaEngine)..."
    cd 02-planner/src
    mkdir -p bin
    cd bin
    cmake ../
    make
    mv pandaPIengine ../..
    cd ../..
    
    echo "Build completed."
}

# Update only the pandaEngine in 02-planner
update_engine() {
    echo "Updating pandaEngine in 02-planner..."
    cd 02-planner/bin
    make -j
    mv pandaPIengine ../..
    cd ../..
    echo "pandaEngine update completed."
}

# Run the planner on given input files
run_planner() {
    if [ "$#" -ne 3 ]; then
        usage
    fi

    DOMAINFILE="$1"
    PROBLEMFILE="$2"
    OPTION="$3"

    echo "Running planner on:"
    echo "  Domain file: $DOMAINFILE"
    echo "  Problem file: $PROBLEMFILE"
    

    # Run parser to create a temporary parsed file
    ./pandaPIparser "$DOMAINFILE" "$PROBLEMFILE" temp.parsed
    if [ ! -f temp.parsed ]; then
        echo "Parsing failed."
        exit 101
    fi

    # Construct output name for the grounded file
    base_domain=$(basename "$DOMAINFILE" .hddl)
    base_problem=$(basename "$PROBLEMFILE" .hddl)
    grounded_file="${base_domain}-${base_problem}.psas"

    # Run grounder
    ./pandaPIgrounder -q temp.parsed "$grounded_file"
    rm temp.parsed
    if [ ! -f "$grounded_file" ]; then
        echo "Grounding failed."
        exit 102
    fi

    # Run engine and log to panda.log
    if [ "$OPTION" -eq 1 ]; then
        echo "Experiment Name: lmc"
        ./pandaPIengine --heuristic="rc2(lmc;cost)" --gValue=action --pruneDeadEnds "$grounded_file" --noPlanOutput  #>> panda.log
    elif [ "$OPTION" -eq 2 ]; then
        echo "Experiment Name: lmc-BD"
        ./pandaPIengine --heuristic="rc2(lmc-BD;cost)" --gValue=action --pruneDeadEnds "$grounded_file" --noPlanOutput  #>> panda.log
    elif [ "$OPTION" -eq 3 ]; then
        echo "Experiment Name: lmc-GZD"
        ./pandaPIengine --heuristic="rc2(lmc-GZD;cost)" --gValue=action --pruneDeadEnds "$grounded_file" --noPlanOutput  #>> panda.log
    elif [ "$OPTION" -eq 4 ]; then
        echo "Experiment Name: lmc-BDpGZD"
        ./pandaPIengine --heuristic="rc2(lmc-BDpGZD;cost)" --gValue=action --pruneDeadEnds "$grounded_file" --noPlanOutput  #>> panda.log
    elif [ "$OPTION" -eq 5 ]; then
        echo "Experiment Name: lmc-VDM"
        ./pandaPIengine --heuristic="rc2(lmc-VDM;cost)" --gValue=action --pruneDeadEnds "$grounded_file" --noPlanOutput  #>> panda.log
    else
        echo "Invalid OPTION: $OPTION. Choose between 1 (lmc), 2 (lmc-BD), 3 (lmc-GZD), 4 (lmc-BDpGZD)."
        exit 1
    fi
    # Run parser again with log file and write final plan to PLANFILE
    #./pandaPIparser -c panda.log "$PLANFILE"
    #echo "Planner finished. Plan is in $PLANFILE."
}

# Main: Check for a valid command
if [ "$#" -lt 1 ]; then
    usage
fi

COMMAND="$1"
shift

case "$COMMAND" in
    install)
        install_dependencies
        ;;
    build)
        build_planner
        ;;
    update)
        update_engine
        ;;
    run)
        run_planner "$@"
        ;;
    *)
        usage
        ;;
esac
