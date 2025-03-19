#!/bin/bash
set -e

# Usage message
usage() {
    echo "Usage:"
    echo "  $0 install                    # Install dependencies"
    echo "  $0 build                      # Build the full planner"
    echo "  $0 update                     # Update (rebuild) pandaEngine (02-planner)"
    echo "  $0 run DOMAINFILE PROBLEMFILE PLANFILE"
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
    echo "Building 03-lama-planner (pandaEngine)..."
    cd 03-lama-planner/src
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
    echo "Updating pandaEngine in 03-lama-planner..."
    cd 03-lama-planner/bin
    make -j
    mv pandaPIengine ../../lamaPIengine
    cd ../..
    echo "pandaEngine update completed."
}

# Run the planner on given input files
run_planner() {
    if [ "$#" -ne 3 ]; then
        echo "Usage: $0 run DOMAINFILE PROBLEMFILE PLANFILE OPTION"
        echo "OPTION:"
        echo "  1: lmCut (standard)"
        echo "  2: lmc-BD"
        echo "  3: lmc-GZD"
        echo "  4: lmc-BDpGZD"
        echo "  5: lmc-VDM"
        exit 1
    fi

    DOMAINFILE="$1"
    PROBLEMFILE="$2"
    OPTION="$3"

    echo "Running planner on:"
    echo "  Domain file: $DOMAINFILE"
    echo "  Problem file: $PROBLEMFILE"
    echo "  Heuristic option: $OPTION"

    # Run parser to create a temporary parsed file
    ./pandaPIparser "$DOMAINFILE" "$PROBLEMFILE" temp.parsed
    if [ ! -f temp.parsed ]; then
        echo "Parsing failed."
        exit 101
    fi

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


    
    # Determine the heuristic based on the OPTION parameter
    if [ "$OPTION" -eq 1 ]; then
        LM_OPTION="lmc"
        ./lamaPIengine --heuristic="lama(lazy=false;ha=false;lm=lmc;useLMOrd=false;h=none;search=gbfs)" "$grounded_file" --noPlanOutput #> panda.log
    elif [ "$OPTION" -eq 2 ]; then
        LM_OPTION="lmc-BD"
        ./lamaPIengine --heuristic="lama(lazy=false;ha=false;lm=lmc-BD;useLMOrd=false;h=none;search=gbfs)" \
                   "$grounded_file" --noPlanOutput
    elif [ "$OPTION" -eq 3 ]; then
        LM_OPTION="lmc-GZD"
        ./lamaPIengine --heuristic="lama(lazy=false;ha=false;lm=lmc-GZD;useLMOrd=false;h=none;search=gbfs)" \
                   "$grounded_file" --noPlanOutput
    elif [ "$OPTION" -eq 4 ]; then
        LM_OPTION="lmc-BDpGZD"
        ./lamaPIengine --heuristic="lama(lazy=false;ha=false;lm=lmc-BDpGZD;useLMOrd=false;h=none;search=gbfs)" \
                   "$grounded_file" --noPlanOutput
    elif [ "$OPTION" -eq 5 ]; then
        LM_OPTION="lmc-VDM"
        ./lamaPIengine --heuristic="lama(lazy=false;ha=false;lm=lmc-VDM;useLMOrd=false;h=none;search=gbfs)" \
                   "$grounded_file" --noPlanOutput
    else
        echo "Invalid OPTION: $OPTION. Choose between 1 (lmc), 2 (lmc-BD), 3 (lmc-GZD), 4 (lmc-BDpGZD)."
        exit 1
    fi

    # echo "Selected LM heuristic: $LM_OPTION"

    # ./lamaPIengine --heuristic="lama(lazy=false;ha=false;lm=${LM_OPTION};useLMOrd=false;h=none;search=gbfs)" \
    #                "$grounded_file" --noPlanOutput > panda.log

    #./pandaPIparser panda.log "$PLANFILE"
    # echo "Planner finished. Plan is in $PLANFILE."
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
