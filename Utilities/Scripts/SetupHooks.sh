#!/usr/bin/env bash

# Run this script to set up the git hooks for committing changes
# For more information, see:
#   http://public.kitware.com/Wiki/Git/Hooks

current_directory=`pwd`

die() {
  echo 'Failure during hook setup.' 1>&2
  echo '--------------------------' 1>&2
  echo '' 1>&2
  echo "$@" 1>&2
  exit 1
}

# Centralize project variables for each script
project="VTK"
projectUrl="vtk.org"

u=$(cd "$(echo "$0"|sed 's/[^/]*$//')"; pwd)
cd "$u/../../.git/hooks"

# We need to have a git repository to do a pull.
if ! test -d ./.git; then
  git init || die "Could not run git init."
fi

# Grab the hooks.
# Use the local hooks if possible (no local hooks yet, use the ones of VTK).
echo "Pulling the hooks..."


#if GIT_DIR=.. git for-each-ref refs/remotes/origin/hooks 2>/dev/null | \
#  grep -q '\<refs/remotes/origin/hooks$'; then
#  git pull .. remotes/origin/hooks
#else
  git pull http://${projectUrl}/${project}.git hooks || die "Downloading the hooks failed."
#fi

echo "Setting github hooks..."

if ! test -d ./github; then
  mkdir ./github
fi

# Install github issue hook
cp $current_directory/commit-msg.sh ./github/commit-msg
chmod +x ./github/commit-msg

cd ../..

# hooks-config.bash looks in source directory for this file
cp $current_directory/hooks-config.sh .hooks-config.bash
chmod +x .hooks-config.bash

echo "Done."
