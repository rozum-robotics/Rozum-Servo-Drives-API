#!/usr/bin/env bash
set -e
git checkout master
git reset --hard HEAD
git pull
git config --global user.email ${GITLAB_USER_EMAIL}
git config --global user.name ${GITLAB_USER_NAME}
git remote set-url origin "https://$GITLAB_CREDENTIALS@$GITLAB_URL"
git tag -a v$(cat version) -m "Release v$(cat version)"
git push origin v$(cat version)
git config --global credential.helper store
echo "$GITHUB_CREDENTIALS" > ~/.git-credentials
git push -f "$GITHUB_URL" master
git tag
