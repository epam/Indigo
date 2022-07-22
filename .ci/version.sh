#!/bin/bash
set -eux

root=$(realpath $(dirname ${0})/..)

if [ $(git branch --show-current) -ne "master" ]; then
  echo "Version update should be done only on master!"
  exit 1
fi 


old_base_version=$(sed -n 1p ${root}/.ci/version.txt)
old_suffix=$(sed -n 2p ${root}/.ci/version.txt)
old_revision=$(sed -n 3p ${root}/.ci/version.txt)
old_java_snapshot=$(sed -n 4p ${root}/.ci/version.txt)
old_hash_sum=$(sed -n 5p ${root}/.ci/version.txt)

check_hash_sum=$(echo "${old_base_version}${old_suffix}${old_revision}${old_java_snapshot}" | sha256sum | awk '{print $1}')
if [ "${check_hash_sum}" != "${old_hash_sum}" ]; then
  echo ".ci/version.txt hash sum check failed! Probably file was manually edited. Cannot continue."
  exit 1
fi

if [ "${old_suffix}" != "" ]; then
  if [ "${old_revision:-}" != "" ]; then
    old_version="${old_base_version}-${old_suffix}.${old_revision}"
  else
    old_version="${old_base_version}-${old_suffix}"
  fi
  old_version_java="${old_version}${old_java_snapshot}"
  old_version_python="${old_base_version}.${old_suffix}${old_revision:-0}"
else
  old_version="${old_base_version}"
  old_version_java="${old_version}"
  old_version_python="${old_version}"
fi 


if ! [[ ${1} =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "First argument (version) should be in MAJOR.MINOR.PATCH format!"
  exit 2
fi
if [[ "${2:-}" != "" ]] && ! [[ ${2} =~ ^[a-z]+$ ]]; then
  echo "Second argument (suffix) should be in [a-z]+ format!"
  exit 2
fi
if [[ "${3:-}" != "" ]] && ! [[ ${3:-} =~ ^[0-9]+$ ]]; then
 echo "Third argument (revision) should be in [0-9]+ format!"
 exit 2
fi
 
new_base_version=${1}
new_suffix=${2:-}
new_java_snapshot=""
if [ "${new_suffix}" != "" ]; then
  
  if [ "${3-unset}" == "unset" ]; then
    new_revision=${3:-$(git rev-list $(git describe --tags --match indigo-* --abbrev=0)..HEAD | wc -l)}
    new_java_snapshot="-SNAPSHOT"
  else
    new_revision="${3}"
    new_java_snapshot=""
  fi
  
  if [ "${new_revision}" == "" ]; then
    new_version="${new_base_version}-${new_suffix}"
  else
    new_version="${new_base_version}-${new_suffix}.${new_revision}"
  fi
  new_version_java="${new_version}${new_java_snapshot}"
  new_version_python="${new_base_version}.${new_suffix}${new_revision:-0}"
else
  new_revision=""
  new_version="${new_base_version}"
  new_version_java="${new_version}"
  new_version_python="${new_version}"
fi 

new_hash_sum=$(echo "${new_base_version}${new_suffix}${new_revision}${new_java_snapshot}" | sha256sum | awk '{print $1}')

# version.txt
printf "${new_base_version}\n${new_suffix}\n${new_revision}\n${new_java_snapshot}\n${new_hash_sum}" > ${root}/.ci/version.txt

# C
sed -i "s/${old_version}/${new_version}/g" ${root}/api/indigo-version.cmake

# .NET
sed -i "s/${old_version}/${new_version}/g" ${root}/api/dotnet/src/Indigo.Net.csproj

# Java
sed -i "s/${old_version_java}/${new_version_java}/g" ${root}/api/java/pom.xml
sed -i "s/${old_version_java}/${new_version_java}/g" ${root}/bingo/bingo-elastic/java/pom.xml

# Python
sed -i "s/${old_version_python}/${new_version_python}/g" ${root}/api/http/setup.py
sed -i "s/${old_version_python}/${new_version_python}/g" ${root}/api/http/requirements.txt
sed -i "s/${old_version_python}/${new_version_python}/g" ${root}/api/python/setup.py
sed -i "s/${old_version_python}/${new_version_python}/g" ${root}/api/python/indigo/__init__.py
sed -i "s/${old_version_python}/${new_version_python}/g" ${root}/bingo/bingo-elastic/python/setup.py
sed -i "s/${old_version_python}/${new_version_python}/g" ${root}/bingo/bingo-elastic/python/bingo_elastic/__init__.py
sed -i "s/${old_version_python}/${new_version_python}/g" ${root}/utils/indigo-ml/setup.py

# R
sed -i "s/${old_version}/${new_version}/g" ${root}/api/r/DESCRIPTION

# JS
sed -i "s/${old_version}/${new_version}/g" ${root}/api/wasm/indigo-ketcher/package.json
sed -i "s/${old_version}/${new_version}/g" ${root}/utils/indigo-service-client/package.json
