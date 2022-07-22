# Indigo version script

Indigo uses Semantic Versioning 2.0.0.

To change the version of Indigo packages we use the script `.ci/version.sh`.
It has the following interface:

```bash
./.ci/version.sh <MAJOR.MINOR.PATCH> [<SUFFIX>] [<REVISION>]
```

Release version:

```bash
./.ci/version.sh 1.8.0
```

Development version with automatic revision number retrieval:

```bash
./.ci/version.sh 1.8.0 dev
```

Development version without revision:

```bash
./.ci/version.sh 1.8.0 dev ""
```

Release candidate version with defined revision:

```bash
./.ci/version.sh 1.8.0 rc 1
```

Script does the following:

1. Automatically reads the old version parameters from `.ci/version.txt`.
   NOTE: never change `.ci/version.txt` manually! It's protected against
   manual editing by hashsum check.
2. Prepares new versions depending on package language (see below for details).
3. Updates `.ci/version.txt` and all package files with new version.

## `version.txt`

`version.txt` stores current version and has 5 lines:

1. Version in MAJOR.MINOR.PATCH format
2. Optional suffix or empty line
3. Optional revision or empty line
4. Optional Java snapshot suffix
5. SHA256 hash sum of 4 previous lines to check that file was not manually
   modified

## Versions

Common version is strictly following semver and has the following format:

```
MAJOR.MINOR.PATH[-SUFFIX][REVISION]
```

If suffix is provided and revision is not, then revision will be
automatically calculated as number of commits since last tag
(`4` in example).

| Input          | Version     |
|----------------|-------------|
| `1.8.0`        | 1.8.0       |
| `1.8.0 dev`    | 1.8.0-dev.4 |
| `1.8.0 dev ""` | 1.8.0-dev   |
| `1.8.0 rc 1`   | 1.8.0-rc.1  |

### Java

Java version follows common rules with one exception: if suffix is provided,
and revision is not, then it adds a `-SNAPSHOT` ending to the version:

| Input          | Java Version         |
|----------------|----------------------|
| `1.8.0`        | 1.8.0                |
| `1.8.0 dev`    | 1.8.0-dev.4-SNAPSHOT |
| `1.8.0 dev ""` | 1.8.0-dev            |
| `1.8.0 rc 1`   | 1.8.0-rc.1           |

### Python

Python has [own versioning standard](https://peps.python.org/pep-0440/),
that slightly differs from semver, so we need to adapt to its requirements.
It could be written as

```
MAJOR.MINOR.PATCH[.SUFFIX][REVISION]
```

| Input          | Python Version |
|----------------|----------------|
| `1.8.0`        | 1.8.0          |
| `1.8.0 dev`    | 1.8.0.dev4     |
| `1.8.0 dev ""` | 1.8.0.dev0     |
| `1.8.0 rc 1`   | 1.8.0.rc1      |
