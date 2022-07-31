#
# Copyright (C) from 2009 to Present EPAM Systems.
#
# This file is part of Indigo toolkit.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from typing import Awaitable, Callable, List, Tuple, Union

from fastapi import FastAPI, HTTPException, Request
from fastapi.responses import JSONResponse
from indigo import IndigoException
from indigo.renderer import IndigoRenderer

from indigo_service import jsonapi, service
from indigo_service.indigo_tools import indigo, indigo_new

app = FastAPI(title="Indigo service JSON:API", version=indigo().version())


BASE_URL_INDIGO = "/indigo"
RESP_HEADER_CONTENT_TYPE = "application/vnd.api+json"


@app.middleware("http")
async def isolate_indigo_session(
    request: Request, call_next: Callable[[Request], Awaitable[JSONResponse]]
) -> JSONResponse:
    with indigo_new():
        response = await call_next(request)
        if not request.scope["path"].startswith(
            ("/docs", "/redoc", f"{BASE_URL_INDIGO}/render")
        ):
            response.headers["Content-Type"] = RESP_HEADER_CONTENT_TYPE
        return response


@app.exception_handler(IndigoException)
def unicorn_exception_handler(
    request: Request, exc: IndigoException  # pylint: disable=unused-argument
) -> JSONResponse:
    error = jsonapi.make_error_response(exc)
    return JSONResponse(status_code=400, content=error.dict())


def compounds(
    request: Union[
        jsonapi.DescriptorRequest,
        jsonapi.ValidationRequest,
        jsonapi.CompoundConvertRequest,
        jsonapi.RenderRequest,
    ],
) -> List[Tuple[str, jsonapi.CompoundFormat]]:
    return service.extract_pairs(request.data.attributes.compound)


def source(
    request: jsonapi.SourceTargetsRequest,
) -> List[Tuple[str, jsonapi.CompoundFormat]]:
    return service.extract_pairs(request.data.attributes.source)


def targets(
    request: jsonapi.SourceTargetsRequest,
) -> List[Tuple[str, jsonapi.CompoundFormat]]:
    return service.extract_pairs(request.data.attributes.targets)


# Indigo Service Methods


@app.get(f"{BASE_URL_INDIGO}/version", response_model=jsonapi.VersionResponse)
def indigo_version() -> jsonapi.VersionResponse:
    return jsonapi.make_version_response(indigo().version())


@app.post(
    f"{BASE_URL_INDIGO}/similarities",
    response_model=jsonapi.SimilaritiesResponse,
    response_model_exclude_unset=True,
)
def similarities(
    request: jsonapi.SimilaritiesRequest,
) -> jsonapi.SimilaritiesResponse:

    fingerprint = request.data.attributes.fingerprint
    alpha = request.data.attributes.alpha
    beta = request.data.attributes.beta
    metric = request.data.attributes.metric

    compound, *_ = service.extract_compounds(source(request))
    target_pairs = targets(request)
    target_compounds = service.extract_compounds(target_pairs)
    comp_fp = compound.fingerprint(fingerprint)
    target_fps = [comp.fingerprint(fingerprint) for comp in target_compounds]

    sim_results = []
    for target_fp in target_fps:
        if jsonapi.SimilarityMetric.TVERSKY == metric:
            result = indigo().similarity(
                comp_fp, target_fp, f"{metric} {alpha} {beta}"
            )
        else:
            result = indigo().similarity(comp_fp, target_fp, metric)
        sim_results.append(result)
    return jsonapi.make_similarities_response(sim_results)


@app.post(
    f"{BASE_URL_INDIGO}/exactMatch",
    response_model=jsonapi.MatchResponse,  # type: ignore
    response_model_exclude_unset=True,
)
def exact_match(request: jsonapi.MatchRequest) -> jsonapi.MatchResponse:
    compound, *_ = service.extract_compounds(source(request))
    target_pairs = targets(request)
    target_compounds = service.extract_compounds(target_pairs)
    output_format = request.data.attributes.outputFormat
    results = []
    for target_compound in target_compounds:
        match = indigo().exactMatch(
            compound, target_compound, request.data.attributes.flag
        )
        try:
            results.append(
                service.map_match_output(
                    match, request.data.attributes.outputFormat, compound
                )
            )
        except AttributeError:
            results.append("")

    return jsonapi.make_match_response(results, output_format)  # type: ignore


@app.post(
    f"{BASE_URL_INDIGO}/convert",
    response_model=jsonapi.CompoundResponse,
    response_model_exclude_unset=True,
)
def convert(
    request: jsonapi.CompoundConvertRequest,
) -> jsonapi.CompoundResponse:
    compound, *_ = service.extract_compounds(
        compounds(request), request.data.attributes.compound.modifiers
    )
    return jsonapi.make_compound_response(
        *service.to_string(compound, request.data.attributes.outputFormat)
    )


@app.post(
    f"{BASE_URL_INDIGO}/validate",
    response_model=jsonapi.ValidationResponse,
    response_model_exclude_unset=True,
)
def validate(request: jsonapi.ValidationRequest) -> jsonapi.ValidationResponse:
    compound, *_ = service.extract_compounds(compounds(request))
    validations = request.data.attributes.validations
    results = {}
    for validation in validations:
        results[validation] = service.validate(compound, validation)
    return jsonapi.make_validation_response(results)


@app.post(
    f"{BASE_URL_INDIGO}/descriptors",
    response_model=jsonapi.DescriptorResponse,
    response_model_exclude_unset=True,
)
def descriptors(
    request: jsonapi.DescriptorRequest,
) -> jsonapi.DescriptorResponse:
    compound, *_ = service.extract_compounds(compounds(request))
    properties = request.data.attributes.descriptors
    results = {}
    for property_ in properties:
        results[property_] = service.get_descriptor(compound, property_)
    return jsonapi.make_descriptor_response(results)


@app.post(
    f"{BASE_URL_INDIGO}/commonBits",
    response_model=jsonapi.CommonBitsResponse,
    response_model_exclude_unset=True,
)
def common_bits(
    request: jsonapi.CommonBitsRequest,
) -> jsonapi.CommonBitsResponse:
    compound, *_ = service.extract_compounds(source(request))
    target_compounds = service.extract_compounds(targets(request))
    source_fp = compound.fingerprint("sim")
    result = []
    for target in target_compounds:
        bits = indigo().commonBits(source_fp, target.fingerprint("sim"))
        result.append(bits)
    return jsonapi.make_common_bits_response(result)


@app.post(f"{BASE_URL_INDIGO}/render", response_model=jsonapi.RenderResponse)
def render(
    request: jsonapi.RenderRequest,
) -> jsonapi.RenderResponse:
    compound, *_ = service.extract_compounds(compounds(request))
    output_format = request.data.attributes.outputFormat
    indigo_renderer = IndigoRenderer(indigo())
    indigo().setOption(
        "render-output-format", jsonapi.rendering_formats.get(output_format)
    )
    options = request.data.attributes.options
    if options:
        for option, value in options.items():
            if option == "render-output-format":
                raise HTTPException(
                    status_code=400, detail="Choose only one output format"
                )
            indigo().setOption(option, value)
    raw_image = indigo_renderer.renderToBuffer(compound)
    return jsonapi.make_render_response(raw_image, output_format)


def run_debug() -> None:
    # Debug server for dev purpose only
    import uvicorn  # pylint: disable=import-outside-toplevel

    uvicorn.run(
        "indigo_service.indigo_http:app",
        host="0.0.0.0",
        port=8080,
        log_level="debug",
        reload=True,
    )


if __name__ == "__main__":
    run_debug()

