#!/bin/bash

# update_build_data.sh - Safe version that doesn't checkout branches
# Uses git worktree to avoid conflicts with uncommitted changes

set -e

echo "📊 Updating build data without branch checkout conflicts"

# Configuration
BRANCH_NAME="build_data"
BUILD_DATA_FILE="build_data.csv"
README_FILE="README.md"
WORKTREE_DIR="/tmp/build_data_worktree_$$"

# Validate required environment variables
if [ -z "$TOTAL_SIZE" ] || [ -z "$GIT_COMMIT" ]; then
    echo "❌ Required environment variables not set (TOTAL_SIZE, GIT_COMMIT)"
    exit 1
fi

# Cleanup function
cleanup() {
    if [ -d "$WORKTREE_DIR" ]; then
        echo "🧹 Cleaning up worktree..."
        git worktree remove "$WORKTREE_DIR" --force 2>/dev/null || true
        rm -rf "$WORKTREE_DIR" 2>/dev/null || true
    fi
}

# Trap cleanup on exit
trap cleanup EXIT

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

# Check if build_data branch exists remotely
BRANCH_EXISTS=false
if git ls-remote --heads origin | grep -q "refs/heads/$BRANCH_NAME"; then
    BRANCH_EXISTS=true
    echo "📡 Found existing $BRANCH_NAME branch remotely"
else
    echo "🌿 $BRANCH_NAME branch doesn't exist, will create it"
fi

if [ "$BRANCH_EXISTS" = true ]; then
    # Fetch the latest build_data branch
    echo "📥 Fetching latest $BRANCH_NAME branch..."
    git fetch origin $BRANCH_NAME:$BRANCH_NAME 2>/dev/null || git fetch origin $BRANCH_NAME
    
    # Create worktree for build_data branch
    echo "🌳 Creating worktree for $BRANCH_NAME branch..."
    git worktree add "$WORKTREE_DIR" $BRANCH_NAME
    
    cd "$WORKTREE_DIR"
    
    # Verify and clean unauthorized files
    echo "🔍 Verifying branch integrity..."
    if [ -f "$BUILD_DATA_FILE" ] || [ -f "$README_FILE" ]; then
        # Remove any unauthorized files
        UNAUTHORIZED_REMOVED=false
        for file in *; do
            if [ -f "$file" ] && [ "$file" != "$BUILD_DATA_FILE" ] && [ "$file" != "$README_FILE" ]; then
                echo "🗑️  Removing unauthorized file: $file"
                rm "$file"
                UNAUTHORIZED_REMOVED=true
            fi
        done
        
        if [ "$UNAUTHORIZED_REMOVED" = true ]; then
            git add -A
            git commit -m "Remove unauthorized files from build_data branch

Only README.md and build_data.csv are allowed in this branch."
            echo "✅ Removed unauthorized files"
        fi
    fi
    
else
    # Create new orphan branch using worktree
    echo "🌿 Creating new $BRANCH_NAME branch with worktree..."
    git worktree add --detach "$WORKTREE_DIR"
    cd "$WORKTREE_DIR"
    
    # Create orphan branch
    git checkout --orphan $BRANCH_NAME
    git rm -rf . 2>/dev/null || true
fi

# Ensure we're in the worktree directory
cd "$WORKTREE_DIR"

# Create or verify README.md
if [ ! -f "$README_FILE" ]; then
    echo "📝 Creating $README_FILE..."
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
fi

# Create or update build_data.csv
if [ ! -f "$BUILD_DATA_FILE" ]; then
    echo "📊 Creating $BUILD_DATA_FILE with headers..."
    echo "timestamp,commit,total_size,bootloader_size,app_size,partition_table_size" > $BUILD_DATA_FILE
fi

# Add new build data entry
echo "📝 Adding new build data entry..."
NEW_ENTRY="\"$TIMESTAMP\",$GIT_COMMIT,$TOTAL_SIZE,$BOOTLOADER_SIZE,$APP_SIZE,$PARTITION_TABLE_SIZE"

# Check if this commit already exists and remove old entries
if grep -q ",$GIT_COMMIT," "$BUILD_DATA_FILE"; then
    echo "⚠️  Removing existing entries for commit $GIT_COMMIT..."
    head -1 "$BUILD_DATA_FILE" > temp_build_data.csv
    grep -v ",$GIT_COMMIT," "$BUILD_DATA_FILE" | tail -n +2 >> temp_build_data.csv || true
    mv temp_build_data.csv "$BUILD_DATA_FILE"
fi

# Add the new entry
echo "$NEW_ENTRY" >> "$BUILD_DATA_FILE"

# Sort by timestamp (keeping header)
echo "🔄 Sorting build data by timestamp..."
head -1 "$BUILD_DATA_FILE" > temp_sorted.csv
tail -n +2 "$BUILD_DATA_FILE" | sort -t',' -k1 >> temp_sorted.csv
mv temp_sorted.csv "$BUILD_DATA_FILE"

# Final verification - ensure only authorized files exist
echo "🔍 Final verification..."
UNAUTHORIZED_COUNT=0
for file in *; do
    if [ -f "$file" ]; then
        if [ "$file" != "$README_FILE" ] && [ "$file" != "$BUILD_DATA_FILE" ]; then
            echo "❌ Unauthorized file found: $file"
            UNAUTHORIZED_COUNT=$((UNAUTHORIZED_COUNT + 1))
        else
            echo "✅ Authorized file: $file"
        fi
    fi
done

if [ $UNAUTHORIZED_COUNT -gt 0 ]; then
    echo "❌ Found $UNAUTHORIZED_COUNT unauthorized files - build_data branch is compromised!"
    exit 1
fi

# Commit the changes
git add "$README_FILE" "$BUILD_DATA_FILE"

# Check if there are any changes to commit
if git diff --staged --quiet; then
    echo "ℹ️  No changes to commit - build data may already be up to date"
else
    git commit -m "Update build data for commit $GIT_COMMIT

- Total size: $TOTAL_SIZE bytes
- Bootloader: $BOOTLOADER_SIZE bytes  
- App: $APP_SIZE bytes
- Partition table: $PARTITION_TABLE_SIZE bytes
- Timestamp: $TIMESTAMP"
    
    echo "✅ Committed build data changes"
fi

# Push the changes
echo "📤 Pushing updated build data..."
git push origin $BRANCH_NAME

echo "✅ Successfully updated build data branch"
echo "📊 Build data entry processed for commit: $GIT_COMMIT"
echo "🏠 Returning to main working directory..."
