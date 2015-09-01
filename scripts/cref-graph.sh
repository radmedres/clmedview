#!/bin/sh
#
# Copyright (C) 2015 Jos Slenter <j.slenter@mumc.nl>,
#                    Roel Janssen <roel@gnu.org>
#
# This file is part of clmedview.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

MAP_FILE="$1"
DOT_FILE="$2"
SVG_FILE="clmedview.svg"
TMP_FILE="clmedview-cref-tmp"

##### DO NOT EDIT BELOW ########

generate_svg_output()
{
    dot -Tsvg $1 -o$2 
}

# Write the graph to a file in DOT format.
write_to_file()
{
    echo "strict digraph {"         > "$1"
    echo -e "$GRAPH" | sort | uniq >> "$1"
    echo -e "$COLORS"              >> "$1"
    echo "}"                       >> "$1"
}

# Add colors to the graph nodes
color_known_entities()
{
    COLORS+="  \"libmemory-patient\" [shape=ellipse, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libmemory-study\" [shape=ellipse, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libmemory-serie\" [shape=ellipse, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libmemory-slice\" [shape=ellipse, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libmemory-tree\" [shape=ellipse, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libmemory-io\" [shape=ellipse, style=filled, fillcolor=goldenrod1]\n"
    COLORS+="  \"libcommon-algebra\" [shape=ellipse, style=filled, fillcolor=brown3]\n"
    COLORS+="  \"libcommon-debug\" [shape=ellipse, style=filled, fillcolor=brown3]\n"
    COLORS+="  \"libcommon-tree\" [shape=ellipse, style=filled, fillcolor=brown3]\n"
    COLORS+="  \"libcommon-history\" [shape=ellipse, style=filled, fillcolor=brown3]\n"
    COLORS+="  \"libcommon-list\" [shape=ellipse, style=filled, fillcolor=brown3]\n"
    COLORS+="  \"libcairo\" [shape=ellipse, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"libclutter-1\" [shape=ellipse, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"libgtk-3\" [shape=ellipse, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"libc\" [shape=ellipse, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"libpthread\" [shape=ellipse, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"ld-linux-x86-64\" [shape=ellipse, style=filled, fillcolor=black, fontcolor=white]\n"
    COLORS+="  \"libconfiguration\" [shape=ellipse, style=filled, fillcolor=dodgerblue4, fontcolor=white]\n"
    COLORS+="  \"libpixeldata-plugin\" [shape=ellipse, style=filled, fillcolor=burlywood3]\n"
    COLORS+="  \"libpixeldata\" [shape=ellipse, style=filled, fillcolor=burlywood3]\n"
    COLORS+="  \"mainwindow\" [shape=ellipse, style=filled, fillcolor=darksalmon]\n"
    COLORS+="  \"main\" [shape=ellipse, style=filled, fillcolor=darksalmon]\n"
    COLORS+="  \"libviewer\" [shape=ellipse, style=filled, fillcolor=grey84]\n"
    #COLORS+="  \"libhistogram\" [shape=ellipse, style=filled, fillcolor=khaki]\n"
    COLORS+="  \"libio-nifti\" [shape=ellipse, style=filled, fillcolor=hotpink3]\n"
    COLORS+="  \"libio-dicom\" [shape=ellipse, style=filled, fillcolor=hotpink3]\n"
    COLORS+="  \"zz\" [shape=ellipse, style=filled, fillcolor=hotpink3]\n"
    COLORS+="  \"zzio\" [shape=ellipse, style=filled, fillcolor=hotpink3]\n"
}

# Filter and prettify the graph nodes.
add_graph_line()
{
    pretty_first=$(basename $(echo $1 | cut -d "(" -f 2 | cut -d "." -f 1))

    # Filter out the first argument
    if [[ "$1" = /usr/lib* ]]
    then
    	if [[ "$pretty_first" != "libcairo" &&
    	      "$pretty_first" != "libgtk-3" &&
    	      "$pretty_first" != "libclutter-1" &&
    	      "$pretty_first" != "libc" &&
    	      "$pretty_first" != "libpthread" &&
    	      "$pretty_first" != "ld-linux-x86-64" ]];
    	then
    	   return 1
    	fi
    fi

    pretty_second=$(basename $(echo $2 | cut -d "(" -f 2 | cut -d "." -f 1))

    # Filter out the second argument
    if [[ "$2" = /usr/lib* ]]
    then
    	if [[ "$pretty_second" != "libcairo" &&
    	      "$pretty_second" != "libgtk-3" &&
    	      "$pretty_second" != "libclutter-1" &&
    	      "$pretty_second" != "libc" &&
    	      "$pretty_second" != "libpthread" &&
    	      "$pretty_second" != "ld-linux-x86-64" ]];
    	then
    	   return 1
    	fi
    fi

    GRAPH+="  \"$pretty_first\" -> \"$pretty_second\"\n" #[label=\"$3\"]
}

## Extract relationships from the map file.
extract_graph()
{
    lineNumber=$(grep -n "Symbol" "$MAP_FILE" | awk -F ":" '{ print $1 }')
    numberOfLines=$(cat "$MAP_FILE" | wc -l)
    numberOfLinesFromTail=$(($numberOfLines - $lineNumber))
    
    tail -n "$numberOfLinesFromTail" "$MAP_FILE" > $TMP_FILE
    while read LINE
    do
	numberOfWords=$(echo "$LINE" | wc -w)
	if [ $numberOfWords -eq 2 ]
	then
	    lastModule=$(echo "$LINE" | awk '{print $2}')
	elif [ $numberOfWords -eq 1 ]
	then
	    add_graph_line "$LINE" "$lastModule" #"$lastSymbol"
	fi
    done < $TMP_FILE

    rm -f $TMP_FILE
}

if [ ! -f "$MAP_FILE" ] 
then
  echo "Map file does not exist, please use valid map file"
  exit
fi

if [ -z "$DOT_FILE" ] 
then
  echo "Dot file argument is empty, please parse a argument"
  exit
fi

## Check if parsed map file contains a is cross reference table
if [ $(grep -c "Cross Reference Table" "$MAP_FILE") -eq 0 ]
then
  echo "Parsed map file is not suitable for this script"
  echo "It doesn't contain a cross reference table"
  exit
fi

if [ $(grep "Symbol" "$MAP_FILE" | grep -c "File") -eq 0 ]
then
  echo "Parsed map file is not suitable for this script"
  echo "It doesn't contain a cross reference table"
  exit
fi

extract_graph
color_known_entities
write_to_file "$DOT_FILE"
generate_svg_output "$DOT_FILE" "$SVG_FILE"

rm -f "$DOT_FILE"
