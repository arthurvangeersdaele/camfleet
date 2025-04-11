#!/bin/bash

# Define the repository URL as a variable
REPO_URL="https://github.com/arthurvangeersdaele/camfleet.git"

# Function to echo text in blue
echo_blue() {
  echo -e "\033[0;34m$1\033[0m"
}

echo_blue "Running history deletion script, wait for END to appear before closing this window."

# Ensure the script is run from inside a Git repository
if [ ! -d ".git" ]; then
  echo_blue "This is not a Git repository. Please run the script inside a Git repository."
  exit 1
fi

# Update your local repo 
git fetch origin
git reset --hard origin/main
git clean -fd
git pull origin main
echo_blue "Local git synced."

# Remove the entire Git history
rm -rf .git
echo_blue "Git history removed."

# Reinitialize a new Git repository
git init
echo_blue "Git repository reinitialized."

# Rename the default branch to main
git branch -m main
echo_blue "Branch renamed to 'main'."

# Add all files to the staging area
git add .
echo_blue "Files added to staging area."

# Commit the changes with a message
git commit -m "Initial commit (reset history)"
echo_blue "Initial commit made."

# Set the remote origin URL using the variable
if git remote | grep -q origin; then
  git remote remove origin
fi
git remote add origin "$REPO_URL"

# Push the changes to the remote repository with force
git push --force origin main
echo_blue "Changes pushed to remote repository."

echo_blue "END - Git history reset and repository pushed successfully."
read -p "Press Enter to close this window..."