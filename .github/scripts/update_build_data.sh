#!/bin/bash
# .github/scripts/update_build_data.sh
# Update build data only when build sizes change

set -e

echo "📊 Updating build data without branch checkout conflicts"

# Get values from GitHub Actions environment
TIMESTAMP=$(date -u '+%Y-%m-%d %H:%M:%S UTC')
COMMIT_HASH=${GIT_COMMIT:-$(git rev-parse --short HEAD)}
TOTAL_SIZE=${TOTAL_SIZE:-0}
BOOTLOADER_SIZE=${BOOTLOADER_SIZE:-0}
APP_SIZE=${APP_SIZE:-0}
PARTITION_TABLE_SIZE=${PARTITION_TABLE_SIZE:-0}

echo "📋 Build data to record:"
echo "  - Timestamp: $TIMESTAMP"
echo "  - Commit: $COMMIT_HASH"
echo "  - Total Size: $TOTAL_SIZE bytes"
echo "  - Bootloader Size: $BOOTLOADER_SIZE bytes"
echo "  - App Size: $APP_SIZE bytes"
echo "  - Partition Table Size: $PARTITION_TABLE_SIZE bytes"

# Check if build_data branch exists remotely
if git ls-remote --heads origin build_data | grep -q build_data; then
    echo "📡 Found existing build_data branch remotely"
    echo "📥 Fetching latest build_data branch..."
    git fetch origin build_data:build_data
else
    echo "🆕 Creating new build_data branch..."
    git checkout --orphan build_data
    echo "# Build Data Branch" > README.md
    echo "" >> README.md
    echo "This branch contains build size tracking data." >> README.md
    echo "timestamp,commit,total_size,bootloader_size,app_size,partition_table_size" > build_data.csv
    git add README.md build_data.csv
    git commit -m "Initialize build_data branch with README.md and build_data.csv"
    git push origin build_data
    git checkout main
    git fetch origin build_data:build_data
fi

# Create a worktree for the build_data branch
WORKTREE_DIR=$(mktemp -d)
echo "🌳 Creating worktree for build_data branch..."
git worktree add "$WORKTREE_DIR" build_data

# Change to worktree directory
cd "$WORKTREE_DIR"

echo "🔍 Verifying branch integrity..."
ALL_FILES=($(git ls-files))
ALLOWED_FILES=("README.md" "build_data.csv")

unauthorized_found=false
for file in "${ALL_FILES[@]}"; do
    file_allowed=false
    for allowed in "${ALLOWED_FILES[@]}"; do
        if [ "$file" == "$allowed" ]; then
            file_allowed=true
            break
        fi
    done
    
    if [ "$file_allowed" == false ]; then
        echo "❌ Unauthorized file found: $file"
        unauthorized_found=true
    else
        echo "✅ Authorized file: $file"
    fi
done

if [ "$unauthorized_found" == true ]; then
    echo "❌ Build data branch contains unauthorized files!"
    cd - > /dev/null
    git worktree remove "$WORKTREE_DIR" --force
    exit 1
fi

# Check if sizes have changed from last entry
should_append=true
if [ -f build_data.csv ] && [ $(wc -l < build_data.csv) -gt 1 ]; then
    last_entry=$(tail -1 build_data.csv)
    IFS=',' read -r _ last_commit last_total last_boot last_app last_part <<< "$last_entry"
    
    if [ "$TOTAL_SIZE" -eq "$last_total" ] && \
       [ "$BOOTLOADER_SIZE" -eq "$last_boot" ] && \
       [ "$APP_SIZE" -eq "$last_app" ] && \
       [ "$PARTITION_TABLE_SIZE" -eq "$last_part" ]; then
        echo "🔄 Build sizes unchanged from last entry. Skipping update."
        should_append=false
    else
        echo "📈 Build sizes changed from last entry:"
        echo "  - Total: $last_total -> $TOTAL_SIZE"
        echo "  - Bootloader: $last_boot -> $BOOTLOADER_SIZE"
        echo "  - App: $last_app -> $APP_SIZE"
        echo "  - Partition Table: $last_part -> $PARTITION_TABLE_SIZE"
    fi
fi

# Append new entry only if sizes changed
if [ "$should_append" = true ]; then
    echo "📝 Adding new build data entry..."
    echo "$TIMESTAMP,$COMMIT_HASH,$TOTAL_SIZE,$BOOTLOADER_SIZE,$APP_SIZE,$PARTITION_TABLE_SIZE" >> build_data.csv

    echo "🔄 Sorting build data by timestamp..."
    {
        head -1 build_data.csv
        tail -n +2 build_data.csv | sort -t, -k1,1
    } > build_data_sorted.csv
    mv build_data_sorted.csv build_data.csv

    echo "📊 Verifying CSV structure..."
    echo "Total entries (including header): $(wc -l < build_data.csv)"
    echo "Last 3 entries:"
    tail -3 build_data.csv

    echo "🔍 Final verification..."
    for file in "${ALL_FILES[@]}"; do
        file_allowed=false
        for allowed in "${ALLOWED_FILES[@]}"; do
            if [ "$file" == "$allowed" ]; then
                file_allowed=true
                break
            fi
        done
        
        if [ "$file_allowed" == false ]; then
            echo "❌ Unauthorized file found after update: $file"
            cd - > /dev/null
            git worktree remove "$WORKTREE_DIR" --force
            exit 1
        fi
    done

    # Commit and push changes
    git add build_data.csv
    git commit -m "Update build data for commit $COMMIT_HASH"
    echo "✅ Committed build data changes"

    echo "📤 Pushing updated build data..."
    git push origin build_data
    echo "✅ Successfully updated build data branch"
fi

echo "📊 Build data processed for commit: $COMMIT_HASH"

# Cleanup
echo "🏠 Returning to main working directory..."
cd - > /dev/null

echo "🧹 Cleaning up worktree..."
git worktree remove "$WORKTREE_DIR" --force
