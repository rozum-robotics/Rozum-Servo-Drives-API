#!/usr/bin/env bash
set -e
git checkout master
git reset --hard HEAD
git pull
git config --global user.email "gitlab@rozum.com"
git config --global user.name "gitlab-bot"
git remote set-url origin "https://$GITLAB_CREDENTIALS@dev.rozum.com/rozum-mcu/Software/userapi.git"
git tag -a v$(cat version) -m "Release v$(cat version)"
git push origin v$(cat version)
git config --global credential.helper store
echo "$GITHUB_CREDENTIALS" > ~/.git-credentials
git push -f "$GITHUB_URL" master
git tag
