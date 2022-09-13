git config --global submodule.recurse true
git submodule sync
git submodule update --init --recursive --remote
git pull