#!/bin/bash

# Check if sufficient arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <original_filename> <new_filename>"
    exit 1
fi

# Assign CLI arguments to variables
filename="$1"
new_filename="$2"

# Ensure the file exists
if [ ! -f "$filename" ]; then
    echo "Error: File '$filename' not found."
    exit 1
fi

filesize=$(stat -c %s "$filename")  # Get the file size in bytes

# Ensure the size is treated as a signed 32-bit integer, take the absolute value explicitly
filesize_signed=$(perl -e "use POSIX; print abs(int($filesize));")

# Check if the absolute value exceeds the maximum for a 32-bit signed integer
if (( filesize_signed > 2147483647 )); then
    echo "File size exceeds the maximum value for a 32-bit signed integer."
    exit 1
fi

# Write the signed size to a temp file in binary format (little-endian)
printf '%08x' "$filesize_signed" | xxd -r -p > temp_size

# Concatenate the size file and the original file into the new file
cat temp_size "$filename" > "$new_filename"

# Clean up the temporary size file
rm temp_size

echo "Prepended the signed file size to $new_filename"

