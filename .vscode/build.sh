#!/bin/bash

# Define color codes
RED="\e[31m"
ORANGE="\e[33m"
GREEN="\e[32m"
WHITE_BG="\e[47m"
RESET="\e[0m"

# Function to print colored messages
print_color() {
    local color="$1"
    local message="$2"
    echo -e "${color}${message}${RESET}"
}


# Define directories
srcDir="$1"

# Check if directory is not empty
if [ -z "$srcDir" ]; then
    print_color $RED "❌ Error: Argument 1 not provided"
    exit 1
fi

# Check if directory does not exist
if [ ! -d "$srcDir" ]; then
    print_color $RED "❌ Error: Directory '$srcDir' not found"
    exit 1
fi

# Make amxxpc happy to find amxxpc32.so nearly
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$srcDir/scripting

destinationDir="$2"

# Check if directory is not empty
if [ -z "$destinationDir" ]; then
    print_color $RED "❌ Error: Argument 2 not provided"
    exit 1
fi

# Check if directory does not exist
if [ ! -d "$destinationDir" ]; then
    print_color $RED "❌ Error: Directory '$destinationDir' not found"
    exit 1
fi

scriptingDir="$destinationDir/scripting"



# Function to compile a .sma file
compile_sma() {
    local smaFile="$1"
    local outputPluginDir="$2"

    pluginName=$(basename "${smaFile%.sma}")
    relativeDir=$(dirname "${smaFile#$srcDir}")
    outputPlugin="$outputPluginDir/${pluginName}.amxx"

    # Create the output plugin directory if it doesn't exist
    mkdir -p "$outputPluginDir"

    # Print the name of the .sma file with white background
    # print_color $WHITE_BG " - Compiling: $(basename $smaFile)"

    # Get the last modification time of the output plugin file
    lastModTime=$(stat -c %Y "$smaFile" 2>/dev/null)
    now=$(date +%s)
    diff=$((now-lastModTime))

    # Check if the file exists and its last modification time is within the last minute

    # Compile the .sma file and capture its output, excluding the lines with version and copyright info
    compile_output=$("$scriptingDir/amxxpc" \
        "$smaFile" \
        -i"$srcDir/scripting" \
        -i"$srcDir/scripting/include" \
        -i"$srcDir/scripting/ReDeathmatch" \
        -o"$outputPlugin" 2>&1 | grep -vE "AMX Mod X Compiler|Copyright|Could not locate output file")

    # Check if there are any errors or warnings in the compile output
    if echo "$compile_output" | grep -qi "error"; then
        error_lines=$(echo "$compile_output" | grep -i "error" | sed 's/.*scripting\///')
        warning_lines=$(echo "$compile_output" | grep -i "warning" | sed 's/.*scripting\///')
        print_color $RED "❌ $error_lines"
        if [ -n "$warning_lines" ]; then
            print_color $ORANGE "⚠️ $warning_lines"
        fi
    elif echo "$compile_output" | grep -qi "warning"; then
        warning_lines=$(echo "$compile_output" | grep -i "warning" | sed 's/.*scripting\///')
        print_color $ORANGE "⚠️ $warning_lines"
    else
        print_color $GREEN "  ✅ Compiled: $(basename $smaFile)"
    fi

}

# Find and compile all .sma files in the source directory and its subdirectories
find $srcDir -name "*.sma" -type f | while read smaFile; do
    relativeDir=$(dirname "${smaFile#$srcDir/scripting}")
    outputPluginDir="$destinationDir/plugins$relativeDir"
    compile_sma "$smaFile" "$outputPluginDir"
done

needCopyOther=$3
if [ "$needCopyOther" ]; then
    echo ""

    # Copy directories without confirmation with green messages
    print_color $GREEN " - Copying configs..."
    cp -an $srcDir/configs/* $destinationDir/configs/
    print_color $GREEN " - Copying data..."
    cp -an $srcDir/data/* $destinationDir/data/
fi