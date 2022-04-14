# Indigo service

`indigo_service` is a new REST JSON:API interface for Indigo molecular library.

## How to run

Official way to run `indigo_service` is using Docker. Dockerimage runs `run_debug`
function by default with `log_level=debug`. It's a good idea to rewrite `CMD` for
your purpose, increase count of workers and setup root endpoint.

`uvicorn --workers=10 indigo_service.indigo_http:app`

## Swagger UI
Open `/docs/` for running [swagger ui](https://swagger.io/tools/swagger-ui/)
or `/redoc/` for [redoc interface](https://github.com/Redocly/redoc)

## Root endpoint

By default, indigo_service runs on root path `/`, go to `/docs/` to get Swagger UI.
To change root path use uvicorn `--root-path`configuration: https://www.uvicorn.org/settings/#http

It's a common way to version `indigo_service` API in your environment.

## Supported functions

1. Similarities
2. Exact Match
3. Convert molecules between Smiles, Molfile, CML
4. Validate molecules: `stereo3D`, `ambiguousH`, `badValence`, `chirality`, `query`,
   `rGroups`, `stereo`, `valence`
5. Molecule descriptors: `countAlleneCenters`, `countAtoms`, `countAttachmentPoints`, `countBonds`,
   `countCatalysts`, `countComponents`, `countDataSGroups`, `countGenericSGroups`, `countHeavyAtoms`,
   `countHydrogens`, `countImplicitHydrogens`, `countMolecules`, `countMultipleGroups`, `countProducts`,
   `countPseudoatoms`, `countRGroups`, `countRSites`, `countReactants`, `countRepeatingUnits`, `countSSSR`,
   `countStereocenters`, `countSuperatoms`, `isChiral`, `isHighlighted`, `molecularWeight`, `monoisotopicMass`,
   `mostAbundantMass`, `name`, `acidPkaValue`, `basicPkaValue`, `grossFormula`
6. Common bits


# Development

1. Install requirement_dev.txt into virtualenv
2. Run `pip3 install -e .` to install indigo_service
3. Run `indigo_service` to start dev uvicorn server
4. Run `pytest` and start development
