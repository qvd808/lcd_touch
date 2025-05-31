#!/bin/bash
# .github/scripts/extract_sizes.sh
# Extract binary file sizes and export to GitHub environment

set -e

echo "=== Extracting binary file sizes ==="

# Get current timestamp
TIMESTAMP=$(date -u '+%Y-%m-%d %H:%M:%S UTC')

# Get git commit hash
GIT_COMMIT=$(git rev-parse --short HEAD)

# Extract file sizes (in bytes)
BOOTLOADER_SIZE=0
PARTITION_SIZE=0
MAIN_BINARY_SIZE=0

if [ -f "build/bootloader/bootloader.bin" ]; then
  BOOTLOADER_SIZE=$(stat -c%s "build/bootloader/bootloader.bin")
  echo "Bootloader size: $BOOTLOADER_SIZE bytes"
fi

if [ -f "build/partition_table/partition-table.bin" ]; then
  PARTITION_SIZE=$(stat -c%s "build/partition_table/partition-table.bin")
  echo "Partition table size: $PARTITION_SIZE bytes"
fi

# Find the main binary (should be *.bin in build root, excluding bootloader and partition)
MAIN_BINARY=$(find build/ -maxdepth 1 -name "*.bin" -not -name "bootloader.bin" -not -name "partition-table.bin" | head -1)
if [ -n "$MAIN_BINARY" ] && [ -f "$MAIN_BINARY" ]; then
  MAIN_BINARY_SIZE=$(stat -c%s "$MAIN_BINARY")
  echo "Main binary ($MAIN_BINARY) size: $MAIN_BINARY_SIZE bytes"
fi

# Calculate total size
TOTAL_SIZE=$((BOOTLOADER_SIZE + PARTITION_SIZE + MAIN_BINARY_SIZE))
echo "Total binary size: $TOTAL_SIZE bytes"

# Export to GitHub environment for use in later steps
echo "TOTAL_SIZE=$TOTAL_SIZE" >> $GITHUB_ENV
echo "BOOTLOADER_SIZE=$BOOTLOADER_SIZE" >> $GITHUB_ENV
echo "PARTITION_SIZE=$PARTITION_SIZE" >> $GITHUB_ENV
echo "MAIN_BINARY_SIZE=$MAIN_BINARY_SIZE" >> $GITHUB_ENV
echo "TIMESTAMP=$TIMESTAMP" >> $GITHUB_ENV
echo "GIT_COMMIT=$GIT_COMMIT" >> $GITHUB_ENV

echo "✅ Size extraction completed"
