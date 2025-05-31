#!/bin/bash

# update_build_data.sh
# Safely update build data while ensuring only README.md and build_data.csv exist

set -e

echo "📊 Updating build data in clean build_data branch"

# Configuration
BRANCH_NAME="build_data"
BUILD_DATA_FILE="build_data.csv"
README_FILE="README.md"

# Store current branch
CURRENT_BRANCH=$(git branch --show-current)
echo "📍 Current branch: $CURRENT_BRANCH"

# Function to cleanup and exit
cleanup_and_exit() {
    local exit_code=$1
    echo "🔄 Returning to original branch: $CURRENT_BRANCH"
    git checkout "$CURRENT_BRANCH" 2>/dev/null || true
    exit $exit_code
}

# Trap to ensure we return to original branch on exit
trap 'cleanup_and_exit $?' EXIT

# Validate required environment variables
if [ -z "$TOTAL_SIZE" ] || [ -z "$GIT_COMMIT" ]; then
    echo "❌ Required environment variables not set (TOTAL_SIZE, GIT_COMMIT)"
    exit 1
fi

# Get additional size information if available
BOOTLOADER_SIZE=${BOOTLOADER_SIZE:-"0"}
APP_SIZE=${APP_SIZE:-"0"}
PARTITION_TABLE_SIZE=${PARTITION_TABLE_SIZE:-"0"}

# Create timestamp
TIMESTAMP=$(date -u +"%Y-%m-%d %H:%M:%S UTC")

echo "📋 Build data to record:"
echo "  - Timestamp: $TIMESTAMP"
echo "  - Commit: $GIT_COMMIT"
echo "  - Total Size: $TOTAL_SIZE bytes"
echo "  - Bootloader Size: $BOOTLOADER_SIZE bytes"
echo "  - App Size: $APP_SIZE bytes"
echo "  - Partition Table Size: $PARTITION_TABLE_SIZE bytes"

# Fetch the build_data branch or create it
if ! git show-ref --verify --quiet refs/remotes/origin/$BRANCH_NAME; then
    echo "🌿 Creating new $BRANCH_NAME branch"
    git checkout --orphan $BRANCH_NAME
    
    # Remove all files from the new orphan branch
    git rm -rf . 2>/dev/null || true
    
    # Create initial README.md
    cat > $README_FILE << 'EOF'
# Build Data

This branch contains build size tracking data for the ESP-IDF project.

## Files

- `build_data.csv` - Historical build size data
- `README.md` - This documentation file

## Purpose

This branch tracks binary size changes over time to help monitor firmware growth and optimize builds.

## Data Format

The CSV contains the following columns:
- `timestamp` - When the build was completed (UTC)
- `commit` - Git commit hash
- `total_size` - Total binary size in bytes
- `bootloader_size` - Bootloader binary size in bytes
- `app_size` - Application binary size in bytes
- `partition_table_size` - Partition table size in bytes
EOF
    
    # Create initial build_data.csv with headers
    echo "timestamp,commit,total_size,bootloader_size,app_size,partition_table_size" > $BUILD_DATA_FILE
    
    git add $README_FILE $BUILD_DATA_FILE
    git commit -m "Initialize build_data branch

- Added README.md with documentation
- Added build_data.csv with column headers
- This branch will only contain these two files"
    
    echo "✅ Created and initialized $BRANCH_NAME branch"
    
else
    echo "🔄 Fetching and switching to $BRANCH_NAME branch"
    git fetch origin $BRANCH_NAME
    git checkout $BRANCH_NAME
    
    # Verify branch integrity - only allowed files should exist
    ALL_FILES=($(git ls-files))
    ALLOWED_FILES=("$README_FILE" "$BUILD_DATA_FILE")
    
    echo "🔍 Verifying branch integrity..."
    unauthorized_files=()
    
    for file in "${ALL_FILES[@]}"; do
        file_allowed=false
        for allowed in "${ALLOWED_FILES[@]}"; do
            if [ "$file" == "$allowed" ]; then
                file_allowed=true
                break
            fi
        done
        
        if [ "$file_allowed" == false ]; then
            unauthorized_files+=("$file")
        fi
    done
    
    # Remove unauthorized files if found
    if [ ${#unauthorized_files[@]} -gt 0 ]; then
        echo "🗑️  Found unauthorized files, removing them:"
        for file in "${unauthorized_files[@]}"; do
            echo "  - Removing: $file"
            git rm "$file"
        done
        
        git commit -m "Clean unauthorized files from build_data branch

Removed unauthorized files:
$(printf '- %s\n' "${unauthorized_files[@]}")

Only README.md and build_data.csv are allowed in this branch."
        
        echo "✅ Cleaned ${#unauthorized_files[@]} unauthorized files"
    fi
    
    # Ensure required files exist
    if [ ! -f "$BUILD_DATA_FILE" ]; then
        echo "📊 Creating missing $BUILD_DATA_FILE"
        echo "timestamp,commit,total_size,bootloader_size,app_size,partition_table_size" > $BUILD_DATA_FILE
        git add $BUILD_DATA_FILE
    fi
    
    if [ ! -f "$README_FILE" ]; then
        echo "📝 Creating missing $README_FILE"
        cat > $README_FILE << 'EOF'
# Build Data

This branch contains build size tracking data for the ESP-IDF project.

## Files

- `build_data.csv` - Historical build size data
- `README.md` - This documentation file

## Purpose

This branch tracks binary size changes over time to help monitor firmware growth and optimize builds.
EOF
        git add $README_FILE
    fi
fi

# Add new build data entry
echo "📝 Adding new build data entry..."
NEW_ENTRY="\"$TIMESTAMP\",$GIT_COMMIT,$TOTAL_SIZE,$BOOTLOADER_SIZE,$APP_SIZE,$PARTITION_TABLE_SIZE"
echo "$NEW_ENTRY" >> $BUILD_DATA_FILE

# Check if this is a duplicate entry (same commit)
DUPLICATE_LINES=$(grep -c ",$GIT_COMMIT," $BUILD_DATA_FILE || true)
if [ "$DUPLICATE_LINES" -gt 1 ]; then
    echo "⚠️  Found duplicate entries for commit $GIT_COMMIT, keeping only the latest"
    # Keep header + remove all lines with this commit + add our new entry
    head -1 $BUILD_DATA_FILE > temp_build_data.csv
    grep -v ",$GIT_COMMIT," $BUILD_DATA_FILE | tail -n +2 >> temp_build_data.csv || true
    echo "$NEW_ENTRY" >> temp_build_data.csv
    mv temp_build_data.csv $BUILD_DATA_FILE
fi

# Sort by timestamp (keeping header)
echo "🔄 Sorting build data by timestamp..."
head -1 $BUILD_DATA_FILE > temp_sorted.csv
tail -n +2 $BUILD_DATA_FILE | sort -t',' -k1 >> temp_sorted.csv
mv temp_sorted.csv $BUILD_DATA_FILE

# Commit the changes
git add $BUILD_DATA_FILE
git commit -m "Update build data for commit $GIT_COMMIT

- Total size: $TOTAL_SIZE bytes
- Bootloader: $BOOTLOADER_SIZE bytes  
- App: $APP_SIZE bytes
- Partition table: $PARTITION_TABLE_SIZE bytes
- Timestamp: $TIMESTAMP"

# Final verification that only authorized files exist
FINAL_FILES=($(git ls-files))
echo "🔍 Final verification - files in branch:"
for file in "${FINAL_FILES[@]}"; do
    if [ "$file" == "$README_FILE" ] || [ "$file" == "$BUILD_DATA_FILE" ]; then
        echo "  ✅ $file (authorized)"
    else
        echo "  ❌ $file (unauthorized - this should not happen!)"
        exit 1
    fi
done

# Push the changes
echo "📤 Pushing updated build data..."
git push origin $BRANCH_NAME

echo "✅ Successfully updated build data branch with only authorized files"
echo "📊 Build data entry added for commit: $GIT_COMMIT"
