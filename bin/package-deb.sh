#!/usr/bin/env bash

cd `dirname $0`/..

version=$1

echo "Preparing directories"

rm -rf build

mkdir -p "build/rozum-update-tool_${version}/DEBIAN"
mkdir -p "build/rozum-update-tool_${version}/opt/rozum-update-tool"
mkdir -p "build/rozum-update-tool_${version}/opt/rozum-update-tool/src"

echo "Collect build source"

update_tool_path=c

cp -r ${update_tool_path}/* "build/rozum-update-tool_${version}/opt/rozum-update-tool/src"

echo "Copy build config"
for file in `find debian -type f`; do
    sed -e "s/\${version}/${version}/" ${file} >> "build/rozum-update-tool_${version}/DEBIAN/`basename ${file}`"
done

echo "Packaging Firmware Installer"

chmod 0555 build/rozum-update-tool_${version}/DEBIAN/postinst

dpkg-deb --build "build/rozum-update-tool_${version}"
