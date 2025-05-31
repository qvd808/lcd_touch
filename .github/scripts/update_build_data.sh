#!/bin/bash
# .github/scripts/update_build_data.sh
# Update build data only when build sizes change compared to last commit in main

set -e

echo "📊 Updating build data using Git index operations"

# Get values from GitHub Actions environment
TIMESTAMP=$(date -u '+%Y-%m-%d %H:%M:%S UTC')
COMMIT_HASH=${GIT_COMMIT:-$(git rev-parse --short HEAD)}
TOTAL_SIZE=${TOTAL_SIZE:-0}
BOOTLOADER_SIZE=${BOOTLOADER_SIZE:-0}
APP_SIZE=${APP_SIZE:-0}
PARTITION_TABLE_SIZE=${PARTITION_TABLE_SIZE:-0}

echo "📋 Build data to record:"
echo "	- Timestamp: $TIMESTAMP"
echo "	- Commit: $COMMIT_HASH"
echo "	- Total Size: $TOTAL_SIZE bytes"
echo "	- Bootloader Size: $BOOTLOADER_SIZE bytes"
echo "	- App Size: $APP_SIZE bytes"
echo "	- Partition Table Size: $PARTITION_TABLE_SIZE bytes"

# Store current branch info
ORIGINAL_BRANCH=$(git branch --show-current)
echo "🔄 Working from branch: $ORIGINAL_BRANCH"

# Function to clean up temporary files
cleanup() {
    echo "🧹 Cleaning up temporary files..."
    rm -f build_data_temp.csv build_data_filtered.csv
}
trap cleanup EXIT

# Check if build_data branch exists remotely
if ! (git ls-remote --heads origin build_data | grep -q build_data); then
    echo "🆕 Creating new build_data branch using Git index..."
    
    # Create initial files in memory
    cat > build_data_temp.csv << EOF
timestamp,commit,total_size,bootloader_size,app_size,partition_table_size
$TIMESTAMP,$COMMIT_HASH,$TOTAL_SIZE,$BOOTLOADER_SIZE,$APP_SIZE,$PARTITION_TABLE_SIZE
EOF
    
    cat > README_temp.md << EOF
# Build Data Branch

This branch contains build size tracking data.
EOF
    
    # Create orphan branch using index operations
    git checkout --orphan build_data
    git rm -rf . --quiet || true  # Remove all files from index
    
    # Add our new files to the index
    git add build_data_temp.csv
    git add README_temp.md
    
    # Move files to proper names in index
    git mv build_data_temp.csv build_data.csv
    git mv README_temp.md README.md
    
    # Commit and push
    git commit -m "Initialize build_data branch with README.md and build_data.csv"
    git push origin build_data
    
    echo "✅ Successfully created and pushed new build_data branch"
    echo "📊 Done. Initial build data created for commit: $COMMIT_HASH"
    
    # Clean exit - return to original branch
    git checkout "$ORIGINAL_BRANCH"
    exit 0
fi

echo "📡 Found existing build_data branch remotely"
echo "📥 Fetching latest build_data branch..."
git fetch origin build_data

# Use Git index operations to work with build_data branch files without checkout
echo "🔍 Extracting build_data.csv from build_data branch..."

# Extract the current build_data.csv from the build_data branch
if git show origin/build_data:build_data.csv > build_data_temp.csv 2>/dev/null; then
    echo "✅ Successfully extracted build_data.csv"
else
    echo "⚠️ build_data.csv not found, creating new one"
    echo "timestamp,commit,total_size,bootloader_size,app_size,partition_table_size" > build_data_temp.csv
fi

# Clean and filter build_data.csv: keep only entries whose commit is in main
should_append=true
latest_main_commit=""
last_total=0
last_boot=0
last_app=0
last_part=0

if [ -f build_data_temp.csv ] && [ $(wc -l < build_data_temp.csv) -gt 1 ]; then
    echo "🧹 Filtering build_data.csv to only include commits in $ORIGINAL_BRANCH"
    head -n 1 build_data_temp.csv > build_data_filtered.csv  # Keep header
    mapfile -t lines < <(tail -n +2 build_data_temp.csv)

    for entry in "${lines[@]}"; do
        IFS=',' read -r timestamp commit total boot app part <<< "$entry"
        # Check if commit exists in current branch
        if git merge-base --is-ancestor "$commit" HEAD 2>/dev/null; then
            echo "$entry" >> build_data_filtered.csv
            # Track the latest valid entry
            latest_main_commit="$commit"
            last_total=$total
            last_boot=$boot
            last_app=$app
            last_part=$part
        else
            echo "🗑️ Removing entry for commit not in $ORIGINAL_BRANCH: $commit"
        fi
    done

    mv build_data_filtered.csv build_data_temp.csv

    if [ -n "$latest_main_commit" ]; then
        echo "✅ Latest commit in $ORIGINAL_BRANCH for comparison: $latest_main_commit"

        if [ "$TOTAL_SIZE" -eq "$last_total" ] && \
           [ "$BOOTLOADER_SIZE" -eq "$last_boot" ] && \
           [ "$APP_SIZE" -eq "$last_app" ] && \
           [ "$PARTITION_TABLE_SIZE" -eq "$last_part" ]; then
            echo "🔄 Build sizes unchanged. Skipping update."
            should_append=false
        else
            echo "📈 Build sizes changed:"
            echo "	- Total: $last_total -> $TOTAL_SIZE"
            echo "	- Bootloader: $last_boot -> $BOOTLOADER_SIZE"
            echo "	- App: $last_app -> $APP_SIZE"
            echo "	- Partition Table: $last_part -> $PARTITION_TABLE_SIZE"
        fi
    else
        echo "⚠️ No entries found that exist in $ORIGINAL_BRANCH. Appending as first valid entry."
    fi
fi

# Append new data if needed and commit using index operations
if [ "$should_append" = true ]; then
    echo "📝 Appending new build data entry..."
    echo "$TIMESTAMP,$COMMIT_HASH,$TOTAL_SIZE,$BOOTLOADER_SIZE,$APP_SIZE,$PARTITION_TABLE_SIZE" >> build_data_temp.csv

    # Verify the file was modified correctly
    if tail -n 1 build_data_temp.csv | grep -q "$COMMIT_HASH"; then
        echo "✅ Successfully added new entry to build_data.csv"
        
        # Use Git index to commit to build_data branch without checkout
        echo "🔄 Committing changes to build_data branch using Git index..."
        
        # Get the current build_data branch commit
        BUILD_DATA_COMMIT=$(git rev-parse origin/build_data)
        
        # Create a new tree with our updated file
        BLOB_HASH=$(git hash-object -w build_data_temp.csv)
        
        # Get the current tree from build_data branch
        CURRENT_TREE=$(git rev-parse origin/build_data^{tree})
        
        # Create new tree with updated build_data.csv
        NEW_TREE=$(git read-tree --index-output=/tmp/git-index $CURRENT_TREE && \
                   GIT_INDEX_FILE=/tmp/git-index git update-index --add --cacheinfo 100644 $BLOB_HASH build_data.csv && \
                   GIT_INDEX_FILE=/tmp/git-index git write-tree)
        
        # Create commit
        NEW_COMMIT=$(git commit-tree $NEW_TREE -p $BUILD_DATA_COMMIT -m "Update build data for commit $COMMIT_HASH")
        
        # Update the build_data branch reference
        git update-ref refs/heads/build_data $NEW_COMMIT
        
        echo "✅ Committed build data changes (commit: ${NEW_COMMIT:0:8})"

        echo "📤 Pushing updated build data..."
        git push origin build_data
        echo "✅ Successfully pushed build data to remote"
        
        # Clean up temporary index file
        rm -f /tmp/git-index
    else
        echo "❌ Failed to add entry to build_data.csv"
        exit 1
    fi
else
    echo "⏭️ No changes needed, skipping commit and push"
fi

echo "📊 Done. Build data processed for commit: $COMMIT_HASH"
