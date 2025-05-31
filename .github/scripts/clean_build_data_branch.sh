#!/bin/bash

# clean_build_data_branch.sh
# Script to ensure only README.md and build_data.csv exist in the build_data branch

set -e

echo "🧹 Cleaning build_data branch to only contain README.md and build_data.csv"

# Configuration
BRANCH_NAME="build_data"
ALLOWED_FILES=("README.md" "build_data.csv")

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

# Check if build_data branch exists, create if not
if ! git show-ref --verify --quiet refs/heads/$BRANCH_NAME; then
    echo "🌿 Creating new $BRANCH_NAME branch"
    git checkout --orphan $BRANCH_NAME
    git rm -rf . 2>/dev/null || true
    
    # Create initial README.md if it doesn't exist
    if [ ! -f "README.md" ]; then
        cat > README.md << 'EOF'
# Build Data

This branch contains build size tracking data for the ESP-IDF project.

## Files

- `build_data.csv` - Historical build size data
- `README.md` - This documentation file

## Purpose

This branch tracks binary size changes over time to help monitor firmware growth and optimize builds.
EOF
        echo "📝 Created initial README.md"
    fi
    
    # Create initial build_data.csv if it doesn't exist
    if [ ! -f "build_data.csv" ]; then
        echo "timestamp,commit,total_size,bootloader_size,app_size,partition_table_size" > build_data.csv
        echo "📊 Created initial build_data.csv with headers"
    fi
    
    git add README.md build_data.csv
    git commit -m "Initialize build_data branch with README.md and build_data.csv"
    echo "✅ Initialized $BRANCH_NAME branch"
    
else
    echo "🔄 Switching to $BRANCH_NAME branch"
    git checkout $BRANCH_NAME
    
    # Get list of all files in the branch (excluding .git)
    ALL_FILES=($(git ls-files))
    
    echo "📋 Current files in $BRANCH_NAME branch:"
    for file in "${ALL_FILES[@]}"; do
        echo "  - $file"
    done
    
    # Find files that should be removed
    FILES_TO_REMOVE=()
    for file in "${ALL_FILES[@]}"; do
        should_keep=false
        for allowed in "${ALLOWED_FILES[@]}"; do
            if [ "$file" == "$allowed" ]; then
                should_keep=true
                break
            fi
        done
        
        if [ "$should_keep" == false ]; then
            FILES_TO_REMOVE+=("$file")
        fi
    done
    
    # Remove unwanted files
    if [ ${#FILES_TO_REMOVE[@]} -gt 0 ]; then
        echo "🗑️  Removing unwanted files:"
        for file in "${FILES_TO_REMOVE[@]}"; do
            echo "  - Removing: $file"
            git rm "$file"
        done
        
        # Commit the removal
        git commit -m "Clean up build_data branch - remove unauthorized files

Removed files:
$(printf '- %s\n' "${FILES_TO_REMOVE[@]}")

Only README.md and build_data.csv should exist in this branch."
        
        echo "✅ Removed ${#FILES_TO_REMOVE[@]} unauthorized files"
    else
        echo "✅ No unauthorized files found - branch is clean"
    fi
    
    # Ensure required files exist
    missing_files=()
    for required in "${ALLOWED_FILES[@]}"; do
        if [ ! -f "$required" ]; then
            missing_files+=("$required")
        fi
    done
    
    if [ ${#missing_files[@]} -gt 0 ]; then
        echo "⚠️  Missing required files, creating them:"
        
        for missing in "${missing_files[@]}"; do
            case "$missing" in
                "README.md")
                    cat > README.md << 'EOF'
# Build Data

This branch contains build size tracking data for the ESP-IDF project.

## Files

- `build_data.csv` - Historical build size data
- `README.md` - This documentation file

## Purpose

This branch tracks binary size changes over time to help monitor firmware growth and optimize builds.
EOF
                    echo "  - Created README.md"
                    ;;
                "build_data.csv")
                    echo "timestamp,commit,total_size,bootloader_size,app_size,partition_table_size" > build_data.csv
                    echo "  - Created build_data.csv with headers"
                    ;;
            esac
        done
        
        git add "${missing_files[@]}"
        git commit -m "Add missing required files to build_data branch

Added:
$(printf '- %s\n' "${missing_files[@]}")"
        
        echo "✅ Added ${#missing_files[@]} missing required files"
    fi
fi

# Final verification
echo "🔍 Final verification of $BRANCH_NAME branch contents:"
FINAL_FILES=($(git ls-files))
all_good=true

for file in "${FINAL_FILES[@]}"; do
    file_allowed=false
    for allowed in "${ALLOWED_FILES[@]}"; do
        if [ "$file" == "$allowed" ]; then
            file_allowed=true
            break
        fi
    done
    
    if [ "$file_allowed" == true ]; then
        echo "  ✅ $file (allowed)"
    else
        echo "  ❌ $file (should not exist!)"
        all_good=false
    fi
done

# Check if all required files are present
for required in "${ALLOWED_FILES[@]}"; do
    if [[ ! " ${FINAL_FILES[@]} " =~ " ${required} " ]]; then
        echo "  ❌ Missing required file: $required"
        all_good=false
    fi
done

if [ "$all_good" == true ]; then
    echo "🎉 Build data branch is clean and contains only authorized files!"
    
    # Push the cleaned branch
    echo "📤 Pushing cleaned $BRANCH_NAME branch..."
    git push origin $BRANCH_NAME --force-with-lease
    echo "✅ Successfully pushed clean $BRANCH_NAME branch"
else
    echo "❌ Build data branch cleanup failed - manual intervention required"
    exit 1
fi

echo "🏁 Build data branch cleanup completed successfully"
