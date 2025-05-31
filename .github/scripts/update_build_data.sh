#!/bin/bash
# .github/scripts/update_build_data.sh
# Handle CSV creation and update on build-data branch (data-only orphan branch)

set -e

echo "=== Updating build data CSV ==="

# Configure git
git config --local user.email "action@github.com"
git config --local user.name "GitHub Action"

# Save current commit for later reference
ORIGINAL_COMMIT=$(git rev-parse HEAD)

# Check if build-data branch exists
if git ls-remote --heads origin build-data | grep -q build-data; then
  # Stash any local changes before switching branches
  echo "Stashing local changes..."
  git stash push -m "CI: Auto-stash before build-data checkout" || true

  echo "build-data branch exists, checking it out"
  git fetch origin build-data
  git checkout build-data

  # Clean up any project files that might have accidentally been added
  find . -maxdepth 1 -type f ! -name "*.csv" ! -name "*.md" ! -name ".git*" -delete 2>/dev/null || true
  find . -maxdepth 1 -type d ! -name ".git" ! -name "." -exec rm -rf {} + 2>/dev/null || true
else
  echo "Creating new orphan build-data branch (data-only)"
  git checkout --orphan build-data

  # Remove all files from the orphan branch
  git rm -rf . 2>/dev/null || true

  # Clear any staged changes
  git reset --hard
fi

# Create CSV header if file doesn't exist
if [ ! -f build_sizes.csv ]; then
  echo "timestamp,bootloader_size,partition_table_size,main_binary_size,total_size,git_commit,branch" > build_sizes.csv
  echo "Created new build_sizes.csv with header"
fi

# Calculate entry count for README
ENTRY_COUNT=0
if [ -f build_sizes.csv ] && [ $(wc -l < build_sizes.csv) -gt 1 ]; then
  ENTRY_COUNT=$(($(wc -l < build_sizes.csv) - 1))
fi

# Always ensure README.md exists and is up-to-date with current stats
cat > README.md << EOL
# Build Data Branch

This branch contains only build-related data and metrics for the ESP-IDF project.

## Files:
- \`build_sizes.csv\` - Historical build size data
- \`README.md\` - This file

## Statistics:
- Total build records: ${ENTRY_COUNT}
- Last updated: $(date -u '+%Y-%m-%d %H:%M:%S UTC')
- Source commit: ${GIT_COMMIT}

## CSV Format:
\`\`\`
timestamp,bootloader_size,partition_table_size,main_binary_size,total_size,git_commit,branch
\`\`\`

This is an orphan branch with no connection to the main project code.
EOL

# Create new CSV entry
NEW_ENTRY="\"${TIMESTAMP}\",${BOOTLOADER_SIZE},${PARTITION_SIZE},${MAIN_BINARY_SIZE},${TOTAL_SIZE},${GIT_COMMIT},main"
echo "New entry: $NEW_ENTRY"

# Check if this entry already exists (compare commit)
SIZES_CHANGED=true
if [ -f build_sizes.csv ] && [ $(wc -l < build_sizes.csv) -gt 1 ]; then
  LAST_ENTRY=$(tail -n 1 build_sizes.csv)
  LAST_COMMIT=$(echo "$LAST_ENTRY" | cut -d',' -f6)
  CURRENT_COMMIT="${GIT_COMMIT}"

  echo "Last commit: $LAST_COMMIT"
  echo "Current commit: $CURRENT_COMMIT"

  if [ "$LAST_COMMIT" = "$CURRENT_COMMIT" ]; then
	echo "Same commit as last entry - skipping CSV update"
	SIZES_CHANGED=false
  else
	echo "New commit detected - will update CSV"
  fi
else
  echo "First build or empty CSV - will add entry"
fi

# Add entry and commit if sizes changed
if [ "$SIZES_CHANGED" = true ]; then
  echo "$NEW_ENTRY" >> build_sizes.csv
  echo "SIZES_CHANGED=true" >> $GITHUB_ENV
  
  # Recalculate entry count after adding new entry
  ENTRY_COUNT=$(($(wc -l < build_sizes.csv) - 1))
  
  # Update README.md with new stats
  cat > README.md << EOL
# Build Data Branch

This branch contains only build-related data and metrics for the ESP-IDF project.

## Files:
- \`build_sizes.csv\` - Historical build size data
- \`README.md\` - This file

## Statistics:
- Total build records: ${ENTRY_COUNT}
- Last updated: $(date -u '+%Y-%m-%d %H:%M:%S UTC')
- Source commit: ${GIT_COMMIT}

## CSV Format:
\`\`\`
timestamp,bootloader_size,partition_table_size,main_binary_size,total_size,git_commit,branch
\`\`\`

This is an orphan branch with no connection to the main project code.
EOL
  
  # Display current CSV contents (last 10 lines)
  echo "=== Updated build_sizes.csv (last 10 entries) ==="
  tail -n 10 build_sizes.csv

  # Commit changes
  git add .
  if ! git diff --staged --quiet; then
	git commit -m "Add build data: Total ${TOTAL_SIZE}B (commit ${GIT_COMMIT})"
	git push origin build-data
	echo "Changes committed and pushed to build-data branch"
  else
	echo "No changes to commit"
  fi
else
  echo "SIZES_CHANGED=false" >> $GITHUB_ENV
fi

# Switch back to main branch
git checkout "$ORIGINAL_COMMIT"

echo "✅ Build data update completed"
