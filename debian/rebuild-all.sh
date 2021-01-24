#!/bin/sh

set -euf

# Make sure debian/changelog is committed -- we'll revert it to cleanup our
# temporary changes, and don't want that to erase any real changes
# Based on https://stackoverflow.com/a/3879077/1797496
git update-index --refresh 
git diff-index --quiet HEAD -- debian/changelog

rebuild_release () {
  version=$1
  codename=$2
  set -x
  dch --distribution=$codename "" --local "~$version."
  debuild -i -S
  git checkout -- debian/changelog
}

rebuild_release 16.04 xenial
rebuild_release 18.04 bionic
rebuild_release 20.04 focal
