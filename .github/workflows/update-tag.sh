#!/bin/bash
set -eux

if [ "${1:-}" == "" ]; then
  echo "Usage: ${0} new_image_tag"
  exit 1
fi 

root=$(realpath "$(dirname ${0})/../..")
 
new_image_tag=${1}

sed -i "s/epmlsop\/indigo-tester:.*$/epmlsop\/indigo-tester:${new_image_tag}/g" ${root}/.github/workflows/indigo-ci.yaml
sed -i "s/epmlsop\/buildpack-centos7:.*$/epmlsop\/buildpack-centos7:${new_image_tag}/g" ${root}/.github/workflows/indigo-ci.yaml
sed -i "s/epmlsop\/buildpack-arm64:.*$/epmlsop\/buildpack-arm64:${new_image_tag}/g" ${root}/.github/workflows/indigo-ci.yaml
