if [ "$CI" = "true" ]; then
    echo "CI detected — skipping vendor reload"
    exit 0
fi

git config --global submodule.recurse true
git submodule sync
git submodule update --init --recursive --remote
git pull