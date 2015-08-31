#!/bin/bash

mapFile="$1"
dotFile="$2"

##### DO NOT EDIT BELOW ########

# Find out if line is whitelisted
function is_white_listed 
{
  if [[ "$1" = "libcairo" ]] || \
     [[ "$1" = "libgtk-3" ]] || \
     [[ "$1" = "libclutter-1" ]] || \
     [[ "$1" = "libpthread" ]] || \
     [[ "$1" = "ld-linux-x86-64" ]] || \
     [[ "$1" = "libc" ]]
  then
    return 1
  else
    return 0
  fi
  return -1
}


# Write the graph to a file in DOT format.
function write_to_file
{
    echo "strict digraph {"         > "$1"
    echo -e "$GRAPH" | sort | uniq >> "$1"
    echo -e "$COLORS"              >> "$1"
    echo "}"                       >> "$1"
}

# Add colors to the graph nodes
function color_known_entities
{
    COLORS+="  \"libmemory-patient\" [shape=egg, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libmemory-study\" [shape=egg, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libmemory-serie\" [shape=egg, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libmemory-slice\" [shape=egg, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libmemory-tree\" [shape=egg, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libmemory-io\" [shape=egg, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libcommon-algebra\" [shape=egg, style=filled, fillcolor=brown3]\n"
    COLORS+="  \"libcommon-debug\" [shape=egg, style=filled, fillcolor=brown3]\n"
    COLORS+="  \"libcommon-tree\" [shape=egg, style=filled, fillcolor=brown3]\n"
    COLORS+="  \"libcommon-history\" [shape=egg, style=filled, fillcolor=brown3]\n"
    COLORS+="  \"libcommon-list\" [shape=egg, style=filled, fillcolor=brown3]\n"
    COLORS+="  \"libcairo\" [shape=egg, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"libclutter-1\" [shape=egg, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"libgtk-3\" [shape=egg, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"libc\" [shape=egg, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"libpthread\" [shape=egg, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"ld-linux-x86-64\" [shape=egg, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"libconfiguration\" [shape=egg, style=filled, fillcolor=dodgerblue4, fontcolor=white]\n"
    COLORS+="  \"libpixeldata-plugin\" [shape=egg, style=filled, fillcolor=burlywood3]\n"
    COLORS+="  \"libpixeldata\" [shape=egg, style=filled, fillcolor=burlywood3]\n"
    COLORS+="  \"mainwindow\" [shape=egg, style=filled, fillcolor=darksalmon]\n"
    COLORS+="  \"main\" [shape=egg, style=filled, fillcolor=darksalmon]\n"
    COLORS+="  \"libviewer\" [shape=egg, style=filled, fillcolor=grey84]\n"
    COLORS+="  \"libhistogram\" [shape=egg, style=filled, fillcolor=khaki]\n"
    COLORS+="  \"libio-nifti\" [shape=egg, style=filled, fillcolor=hotpink3]\n"
    COLORS+="  \"libio-dicom\" [shape=egg, style=filled, fillcolor=hotpink3]\n"
}

# Filter and prettify the graph nodes.
function add_graph_line
{
  is_external=0
  if [[ "$1$2" = */usr/lib* ]]
  then
    is_external=1
  fi

  if [[ "$1" = *"("*")"* ]]
  then
    ##
    pretty_first=$(echo "$1" | awk -F "(" '{ print $2 }' | tr -d ")" | cut -d "." -f 1)
  else
    pretty_first=$(basename "$1" | cut -d "." -f 1)
  fi

  if [[ "$2" = *"("*")"* ]]
  then
    ## 
    pretty_second=$(echo "$2" | awk -F "(" '{ print $2 }' | tr -d ")" | cut -d "." -f 1)
  else
    pretty_second=$(basename "$2" | cut -d "." -f 1)
  fi

  if [ $is_external -eq 1 ]
  then
    is_white_listed "$pretty_first"
    if  [ $? -lt 1 ]
    then
      return 1
    fi

    is_white_listed "$pretty_second"
    if  [ $? -lt 1 ]
    then
      return 1
    fi
  fi

    # Make sure the items are escaped so they can contain '-'.
  if [[ $pretty_first != *\"* ]]
  then
    pretty_first="\"$pretty_first\""
  fi

  if [[ $pretty_second != *\"* ]]; 
  then
    pretty_second="\"$pretty_second\""
  fi
  
  GRAPH+="  $pretty_first -> $pretty_second\n" #[label=\"$3\"]
}

## Extract relationships from the map file.
function extract_graph
{
  lineNumber=$(grep -n "Symbol" "$mapFile" | awk -F ":" '{ print $1 }')
  numberOfLines=$(cat "$mapFile" | wc -l)
  numberOfLinesFromTail=$(($numberOfLines - $lineNumber))

  while read LINE
  do
    numberOfWords=$(echo "$LINE" | wc -w)
    if [ $numberOfWords -eq 2 ]
    then
      lastModule=$(echo "$LINE" | awk '{print $2}')
    elif [ $numberOfWords -eq 1 ]
    then 
      relationModule=$(echo "$LINE" | awk '{print $1}')
      if [ ! -z "$lastModule" ]
      then
        add_graph_line "$relationModule" "$lastModule"
      fi
      lastModule=""
    fi
  done < <(tail -n "$numberOfLinesFromTail" "$mapFile")
}

if [ ! -f "$mapFile" ] 
then
  echo "Map file does not exist, please use valid map file"
  exit
fi

if [ -z "$dotFile" ] 
then
  echo "Dot file argument is empty, please parse a argument"
  exit
fi

## Check if parsed map file contains a is cross reference table
if [ $(grep -c "Cross Reference Table" "$mapFile") -eq 0 ]
then
  echo "Parsed map file is not suitable for this script"
  echo "It doesn't contain a cross reference table"
  exit
fi

if [ $(grep "Symbol" "$mapFile" | grep -c "File") -eq 0 ]
then
  echo "Parsed map file is not suitable for this script"
  echo "It doesn't contain a cross reference table"
  exit
fi

extract_graph
color_known_entities
write_to_file "$dotFile"
