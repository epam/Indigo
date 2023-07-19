# EPAM LifeSciences Open Source Products #

## Introduction ##

This repository includes [indigo](http://lifescience.opensource.epam.com/indigo/) based solution for [KNIME](https://www.knime.org/knime).
This solution represents an implementation of nodes to build high-perfomance workflows to process chemical data like molecules, reactions, query molecules and reactions. 
The nodes support standard chemical formats as input: Mol, Smiles, CML, SMARTS, SDF, InChi and RXN. For more details see [the introduction](https://tech.knime.org/community/indigo).

## Build Indigo nodes ##

1. First you need to run installation script to download referenced libraries:
  - Linux: `install-indigo.sh`
  - Windows: `install-indigo.bat`
2. Then having an appropriate [platform](https://www.knime.org/downloads/overview) (Eclipse from 4.5.x, KNIME SDK from 3.1.x) 
with Java Runtime Environment (Oracle Java 1.8.x) make sure you have installed:
  * KNIME SDK
  * KNIME Distance Matrix
  * KNIME Base Chemistry Types & Nodes
3. Then import project and run it as an Eclipse Application or export a deployable plug-in or feature by mean of standard Eclipse's tool. After exporting this plug-in/feature might be installed into the Eclipse based platform you use
to have an access to Indigo nodes.

Another way to have access to Indigo nodes is to install them from [update site](https://tech.knime.org/community) (see versions 3.1 and higher).

  
## Source code organization ##

  * **_\com.epam.indigo.knime.feature_** directory contains files required to create the Eclipse fetaure
  * **_\com.epam.indigo.knime.plugin_** directory contains files required to create Eclipse plug-in
    * **_\icons_** directory contains icon for categories and nodes to be represented in the KNIME prespective
    * **_\META-INF_** directory contains manifest
    * **_\src_** directory contains code sources

  - **_\tests_** directory contains `run-indigo-knime-tests.bat` and `run-indigo-knime-tests.sh` files with script to run tests on Windows and Linux respectively
    - **_\workflows_** directory contains test workflows


## Branches ##

 * **_gga\_old\_v1\_1\_13_** branch with contains Indigo nodes based on KNIME 2.x. Use JRE 1.6.x and Eclipse 3.x to build and run these nodes. This nodes are not supported anymore.
 * **_master_** branch contains Indigo nodes based on KNIME 3.x (3.1 and higher, not for 3.0). This branch currently is under construction.

## Notes ##