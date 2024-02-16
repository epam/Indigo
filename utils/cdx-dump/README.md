[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# CDX_DUMP #

Copyright (c) 2009-2024 EPAM Systems, Inc.

Licensed under the [Apache License version 2.0](LICENSE)

## Introduction ## 

This small tool created to convert binary CDX format file to human readable JSON file which contains CDX object TAGs, IDs and CDX properties tags, len and content as hex string.
Too create pretty justified JSON please use option "-p".

This tool also can convert back generated JSON to CDX file. Please note that right now only tag, id, len and hex string writed to CDX, no check that len corresond to HEX string len. No check for other parsed values. Only HEX string data writed. So, if you want to change some data - change HEX string and fix len if needed.
Also you can copy and paste any CDX property/object into CDX object. For details see CDX documentation.

TODO: Add available CDX documentation as folder with html files.

## Available options ##

To convert CDX to JSON just call
```
cdx-dump file.cdx
```
Result JSON will be writed to STDOUT, if you whant to save it in file - use output redirection:
```
cdx-dump file.cdx > out_file.json
```
It will generate compact JSON without new lines and justify. If you want pretty-formated JSON use "-p"  option:
```
cdx-dump -p file.cdx > out_file.json
```

To convert JSON back to CDX use "-r" option:
```
cdx-dump -r file.json  out_file.cdx
```

## Build instruction ##

Create build folder and use cmake with desired options. For instance:

```
Indigo/build>cmake .. -DBUILD_INDIGO=ON -DBUILD_INDIGO_WRAPPERS=ON -DBUILD_INDIGO_UTILS=ON
```

To build from console:
```
Indigo/build>cmake --build . --config Release --target cdx-dump
```