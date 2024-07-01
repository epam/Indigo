#!/bin/bash
set -x

for file in ./dist/indigo-ketcher-*.tgz;do
    if [[ "$file" == *"dev."* ]]; then
        npm publish $file --tag dev
    elif [[ "$file" == *"rc."* ]]; then
        npm publish $file --tag beta
    else
        npm publish $file
    fi
done
