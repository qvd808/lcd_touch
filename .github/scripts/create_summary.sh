#!/bin/bash
# .github/scripts/create_summary.sh
# Create build summary for GitHub Actions

set -e

echo "=== Creating build summary ==="

# Get target from environment
TARGET=${TARGET:-unknown}

# Create build summary
echo "## 🔨 ESP-IDF Build Results ($TARGET)" >> $GITHUB_STEP_SUMMARY
echo "| Component | Size | Change |" >> $GITHUB_STEP_SUMMARY
echo "|-----------|------|---------|" >> $GITHUB_STEP_SUMMARY
echo "| Bootloader | ${BOOTLOADER_SIZE} bytes | - |" >> $GITHUB_STEP_SUMMARY
echo "| Partition Table | ${PARTITION_TABLE_SIZE} bytes | - |" >> $GITHUB_STEP_SUMMARY
echo "| Main Binary | ${APP_SIZE} bytes | - |" >> $GITHUB_STEP_SUMMARY
echo "| **Total** | **${TOTAL_SIZE} bytes** | - |" >> $GITHUB_STEP_SUMMARY
echo "" >> $GITHUB_STEP_SUMMARY
echo "📊 **Build Data**: Updated in [\`build-data\`](https://github.com/${GITHUB_REPOSITORY}/tree/build-data) branch" >> $GITHUB_STEP_SUMMARY

if [ "${SIZES_CHANGED}" = "true" ]; then
  echo "✅ **Status**: Size changes detected - artifacts uploaded" >> $GITHUB_STEP_SUMMARY
else
  echo "ℹ️ **Status**: No size changes - skipped artifact upload" >> $GITHUB_STEP_SUMMARY
fi

echo "✅ Build summary created for target: $TARGET"
