#!/usr/bin/env python
# -*- coding: utf-8 -*

import collections
from functools import wraps
from flask import Blueprint, request, jsonify
import itertools
import json
import logging
from marshmallow.exceptions import ValidationError
import re
import sys
import traceback

from indigo import Indigo, IndigoException
from indigo_inchi import IndigoInchi
from indigo_renderer import IndigoRenderer

from .common.util import highlight
from .validation import IndigoRendererSchema, IndigoRequestSchema, IndigoAutomapSchema, IndigoCheckSchema, IndigoCalculateSchema
from math import sqrt
from threading import local
import base64
import config

tls = local()

indigo_api = Blueprint('indigo_api', __name__)
indigo_api.indigo_defaults = {
    'ignore-stereochemistry-errors': 'true',
    'smart-layout': 'true',
    'gross-formula-add-rsites': 'true',
    'mass-skip-error-on-pseudoatoms': 'false',
}
indigo_api_logger = logging.getLogger('indigo')


LOG_MAX_SYMBOLS = 50

def get_shorten_text(intext):
    if len(intext) < LOG_MAX_SYMBOLS:
        return intext
    return "{} ...".format(intext.strip().ljust(LOG_MAX_SYMBOLS)[:LOG_MAX_SYMBOLS])


def LOG_DATA(*args):
    format_shorten = []
    format_str = []

    for a in args:
        if config.__dict__['LOG_LEVEL'] == 'DEBUG':
            format_str.append(str(a))
        else:
            format_shorten.append(get_shorten_text(str(a)))

    if config.__dict__['LOG_LEVEL'] == 'DEBUG':
        indigo_api_logger.debug("\"{}\"".format('" "'.join(format_str)))
    else:
        indigo_api_logger.info("\"{}\"".format('" "'.join(format_shorten)))


def indigo_init(options={}):
    try:
        tls.indigo = Indigo()
        tls.indigo.inchi = IndigoInchi(tls.indigo)
        tls.indigo.renderer = IndigoRenderer(tls.indigo)
        for option, value in indigo_api.indigo_defaults.items():
            tls.indigo.setOption(option, value)
        for option, value in options.items():
            # TODO: Remove this when Indigo API supports smiles type option
            if option in {'smiles', }:
                continue
            tls.indigo.setOption(option, value)
        return tls.indigo
    except Exception as e:
        indigo_api_logger.error('indigo-init: {0}'.format(e))
        return None


class MolData:
    is_query = False
    is_rxn = False
    struct = None
    substruct = None


def is_rxn(molstr):
    return '>>' in molstr or molstr.startswith('$RXN') or '<reactantList>' in molstr


def qmol_to_mol(m, selected):
    for atom in m.iterateAtoms():
        if not atom.index() in selected:
            atom.resetAtom('C')
    for bond in m.iterateBonds():
        if not (bond.source().index() in selected or bond.destination().index() in selected):
            m.removeBonds([bond.index(), ])
    return m.dispatcher.loadMolecule(m.clone().molfile())


class ImplicitHCalcExpection(IndigoException):
    pass


def remove_implicit_h_in_selected_components(m, selected):
    if m.countRSites():
        for index in selected:
            if m.getAtom(index).isRSite():
                raise ImplicitHCalcExpection(b'Cannot calculate properties for RGroups')
    if m.countAttachmentPoints():
        count = m.countAttachmentPoints()
        for order in range(1, count + 1):
            for ap in m.iterateAttachmentPoints(order):
                if ap.index() in selected:
                    raise ImplicitHCalcExpection(b'Cannot calculate properties for RGroups')
    removed_atoms = set()
    implicit_h_decrease = collections.defaultdict(int)
    for c in m.iterateComponents():
        for a in c.iterateAtoms():
            a_index = a.index()
            if a_index not in selected:
                continue
            for n in a.iterateNeighbors():
                n_index = n.index()
                if n_index not in selected:
                    bond_order = None
                    for bond in m.iterateBonds():
                        if all(i in [a_index, n_index] for i in (bond.source().index(), bond.destination().index())):
                            bond_order = bond.bondOrder()
                            break
                    if bond_order not in (1, 2, 3):
                        raise ImplicitHCalcExpection(b"Cannot calculate implicit hydrogens on single atom with query or aromatic bonds")
                    implicit_h_decrease[a_index] += bond_order
                    removed_atoms.add(n_index)
    m.removeAtoms(list(removed_atoms))
    for index, value in implicit_h_decrease.items():
        a = m.getAtom(index)
        implicit_h_count = a.countImplicitHydrogens() - value
        if implicit_h_count < 0:
            raise ImplicitHCalcExpection(b'Cannot calculate implicit hydrogenes on atom with bad valence')
        a.setImplicitHCount(implicit_h_count)
    return m


def iterate_selected_submolecules(r, selected):
    atomCounter = 0
    for m in r.iterateMolecules():
        moleculeAtoms = []
        for atom in selected:
            if atomCounter <= atom < atomCounter + m.countAtoms():
                moleculeAtoms.append(atom - atomCounter)
        atomCounter += m.countAtoms()
        if moleculeAtoms:
            if r.dbgInternalType() == '#05: <query reaction>':
                m = qmol_to_mol(m, moleculeAtoms)
                m = remove_implicit_h_in_selected_components(m, moleculeAtoms)
            yield m.getSubmolecule(moleculeAtoms).clone()


def iterate_selected_components(m, selected):
    for c in m.iterateComponents():
        for atom in c.iterateAtoms():
            if atom.index() in selected:
                yield c
                break


def do_calc(m, func_name, precision):
    try:
        value = getattr(m, func_name)()
    except IndigoException as e:
        value = 'error: {0}'.format(e.value.split(': ')[-1])
    if type(value) == float:
        value = round(value, precision)
    return str(value)


def molecule_calc(m, func_name, precision=None):
    if m.dbgInternalType() == '#03: <query molecule>':
        return 'Cannot calculate properties for structures with query features'
    if m.countRGroups() or m.countAttachmentPoints():
        return 'Cannot calculate properties for RGroups'
    results = []
    for c in m.iterateComponents():
        results.append(do_calc(c.clone(), func_name, precision))
    return '; '.join(results)


def reaction_calc(rxn, func_name, precision=None):
    if rxn.dbgInternalType() == '#05: <query reaction>':
        return 'Cannot calculate properties for structures with query features'
    for m in rxn.iterateMolecules():
        if m.countRGroups() or m.countAttachmentPoints():
            return 'Cannot calculate properties for RGroups'
    reactants_results = []
    for r in rxn.iterateReactants():
        reactants_results.append('[{0}]'.format(molecule_calc(r, func_name, precision)))
    product_results = []
    for p in rxn.iterateProducts():
        product_results.append('[{0}]'.format(molecule_calc(p, func_name, precision)))
    return '{0} > {1}'.format(' + '.join(reactants_results), ' + '.join(product_results))


def selected_molecule_calc(m, selected, func_name, precision=None):
    if m.dbgInternalType() == '#03: <query molecule>':
        try:
            m = qmol_to_mol(m, selected)
        except IndigoException as e:
            return 'Cannot calculate properties for structures with query features'
    if m.countRGroups() and max(selected) >= m.countAtoms():
        return 'Cannot calculate properties for RGroups'
    try:
        m = remove_implicit_h_in_selected_components(m, selected)
    except ImplicitHCalcExpection as e:
        return str(e.value)
    results = []
    for c in iterate_selected_components(m, selected):
        cc = c.clone()
        if cc.countRSites() or cc.countAttachmentPoints():
            return 'Cannot calculate properties for RGroups'
        results.append(do_calc(cc, func_name, precision))
    return '; '.join(results)


def selected_reaction_calc(r, selected, func_name, precision=None):
    results = []
    total_atoms_count = sum([m.countAtoms() for m in r.iterateMolecules()])
    total_rgroups_count = sum([m.countRGroups() for m in r.iterateMolecules()])
    if total_rgroups_count and max(selected) >= total_atoms_count:
        return 'Cannot calculate properties for RGroups'
    try:
        for csm in iterate_selected_submolecules(r, selected):
            if csm.countRSites() or csm.countAttachmentPoints():
                return 'Cannot calculate properties for RGroups'
            results.append(do_calc(csm, func_name, precision))
    except ImplicitHCalcExpection as e:
        return str(e.value)
    except IndigoException as e:
        return 'Cannot calculate properties for structures with query features'
    return '; '.join(results)


def remove_unselected_repeating_units_m(m, selected):
    for ru in m.iterateRepeatingUnits():
        for atom in ru.iterateAtoms():
            if atom.index() not in selected:
                ru.remove()
                break


def remove_unselected_repeating_units_r(r, selected):
    atomCounter = 0
    for m in r.iterateMolecules():
        moleculeAtoms = []
        for atom in selected:
            if atomCounter <= atom < atomCounter + m.countAtoms():
                moleculeAtoms.append(atom - atomCounter)
        atomCounter += m.countAtoms()
        if moleculeAtoms:
            remove_unselected_repeating_units_m(m, moleculeAtoms)


def load_moldata(molstr, indigo=None, options={}, query=False, mime_type=None, selected=[]):
    if not indigo:
        try:
            indigo = indigo_init(options)
        except Exception as e:
            raise HttpException('Problem with Indigo initialization: {0}'.format(e), 501)
    md = MolData()

    # TODO: rewrite to use indigo function
    # params = ''
    # if mime_type == 'chemical/x-daylight-smarts':
    #     params = 'smarts'
    #     md.is_query = True

    # md.struct = indigo.loadStructure(molstr, params)
    # md.is_query = md.struct.checkQuery()

    if molstr.startswith('InChI'):
        md.struct = indigo.inchi.loadMolecule(molstr)
        md.is_rxn = False
        md.is_query = False
    else:
        md.is_rxn = is_rxn(molstr)
        if mime_type == 'chemical/x-daylight-smarts':
            md.struct = indigo.loadReactionSmarts(molstr) if md.is_rxn else indigo.loadSmarts(molstr)
            md.is_query = True
        else:
            if not query:
                try:
                    md.struct = indigo.loadReaction(molstr) if md.is_rxn else indigo.loadMolecule(molstr)
                    md.is_query = False
                except:
                    message = str(sys.exc_info()[1])
                    if not re.search('loader.+quer(y|ies)', message):
                        raise
                    md.struct = indigo.loadQueryReaction(molstr) if md.is_rxn else indigo.loadQueryMolecule(molstr)
                    md.is_query = True
            else:
                md.struct = indigo.loadQueryReaction(molstr) if md.is_rxn else indigo.loadQueryMolecule(molstr)
                md.is_query = True
    return md


def save_moldata(md, output_format=None, options={}):
    if output_format in ('chemical/x-mdl-molfile', 'chemical/x-mdl-rxnfile'):
        return md.struct.rxnfile() if md.is_rxn else md.struct.molfile()
    elif output_format == 'chemical/x-daylight-smiles':
        if options.get('smiles') == 'canonical':
            return md.struct.canonicalSmiles()
        else:
            return md.struct.smiles()
    elif output_format == 'chemical/x-chemaxon-cxsmiles':
        if options.get('smiles') == 'canonical':
            return md.struct.canonicalSmiles()
        else:
            return md.struct.smiles()
    elif output_format == 'chemical/x-daylight-smarts':
        return md.struct.smarts()
    elif output_format == 'chemical/x-cml':
        return md.struct.cml()
    elif output_format == 'chemical/x-inchi':
        return md.struct.dispatcher.inchi.getInchi(md.struct)
    elif output_format == 'chemical/x-inchi-aux':
        res = md.struct.dispatcher.inchi.getInchi(md.struct)
        aux = md.struct.dispatcher.inchi.getAuxInfo()
        return "{}\n{}".format(res, aux)
    raise HttpException("Format %s is not supported" % output_format, 400)


class HttpException(Exception):
    def __init__(self, value, code, headers={'Content-Type': 'text/plain'}):
        self.value = value
        self.code = code
        self.headers = headers


def get_request_data(request):
    request_data = {}
    if request.content_type == 'application/json':
        request_data = json.loads(request.data.decode('utf-8'))
        request_data['json_output'] = True
    else:
        request_data['struct'] = request.data.decode('utf-8')
        request_data['input_format'] = request.headers['Content-Type'] if 'Content-Type' in request.headers else None

        request_data['output_format'] = 'chemical/x-mdl-molfile'

        if 'Accept' in request.headers and request.headers['Accept'] != '*/*':
            request_data['output_format'] = request.headers['Accept']


        request_data['json_output'] = False
    return request_data


def get_response(md, output_struct_format, json_output, options):
    output_mol = save_moldata(md, output_struct_format, options)
    LOG_DATA('[RESPONSE]', output_struct_format, options, output_mol.encode('utf-8'))

    if json_output:
        return jsonify({'struct': output_mol, 'format': output_struct_format}), 200, {'Content-Type': 'application/json'}
    else:
        return output_mol, 200, {'Content-Type': output_struct_format}


def get_error_response(value, error_code, json_output=False):
    if json_output:
        return jsonify({'error': value}), error_code, {'Content-Type': 'application/json'}
    else:
        return value, error_code, {'Content-Type': 'text/plain'}


def check_exceptions(f):
    @wraps(f)
    def wrapper(*args, **kwargs):
        json_output = (('Accept' in request.headers and request.headers['Accept'] == 'application/json') or
                       ('Content-Type' in request.headers and request.headers['Content-Type'] == 'application/json'))
        try:
            return f(*args, **kwargs)
        except ValidationError as e:
            indigo_api_logger.error('[RESPONSE-400] validation error: {0}'.format(e.messages))
            indigo_api_logger.debug(traceback.format_exc())
            if json_output:
                return jsonify({'error': e.messages}), 400, {'Content-Type': 'application/json; charset=utf-8'}
            else:
                return 'ValidationError: {0}'.format(str(e.messages)), 400, {'Content-Type': 'text/plain'}
        except HttpException as e:
            indigo_api_logger.error('HttpException: {0}'.format(e.value))
            indigo_api_logger.debug(traceback.format_exc())
            if json_output:
                return jsonify({'error': e.value}), e.code, e.headers.update({'Content-Type': 'application/json'})
            else:
                return e.value, e.code, e.headers.update({'Content-Type': 'text/plain'})
        except IndigoException as e:
            indigo_api_logger.error('IndigoException: {0}'.format(e.value))
            indigo_api_logger.debug(traceback.format_exc())
            if json_output:
                return jsonify({'error': 'IndigoException: {0}'.format(e.value)}), 400, {'Content-Type': 'application/json'}
            else:
                return 'IndigoException: {0}'.format(e.value), 400, {'Content-Type': 'text/plain'}
        except Exception as e:
            indigo_api_logger.error('Exception: {0}'.format(e))
            indigo_api_logger.debug(traceback.format_exc())
            if json_output:
                return jsonify({'error': '{0}'.format(e)}), 500, {'Content-Type': 'application/json'}
            else:
                return 'Exception: {0}'.format(e), 500, {'Content-Type': 'text/plain'}
    return wrapper


@indigo_api.route('/info')
@check_exceptions
def info():
    """
    Get information about Indigo version
    ---
    tags:
      - indigo
    responses:
      200:
        description: JSON with Indigo version
    """
    indigo_api_logger.info("[REQUEST] /info")
    indigo = indigo_init()
    return jsonify({'Indigo': {'version': indigo.version()}}), 200, {'Content-Type': 'application/json'}


@indigo_api.route('/aromatize', methods=['POST'])
@check_exceptions
def aromatize():
    """
    Aromatize structure
    ---
    tags:
      - indigo
    parameters:
      - name: json_request
        in: body
        required: true
        schema:
          id: IndigoRequest
          required:
            - struct
          properties:
            struct:
              type: string
              default: C1=CC=CC=C1
            output_format:
              type: string
              default: chemical/x-daylight-smiles
    responses:
      200:
        description: Aromatized molecule
        schema:
          id: IndigoResponse
          required:
            - struct
            - format
          properties:
            struct:
              type: string
            format:
              type: string
              default: chemical/x-mdl-molfile
      400:
        description: 'A problem with supplied client data'
        schema:
          id: Error
          required:
            - error
          properties:
            error:
              type: string
      500:
        description: 'A problem on server side'
        schema:
          id: Error
          required:
            - error
          properties:
            error:
              type: string
    """

    request_data = get_request_data(request)
    indigo_api_logger.debug("[RAW REQUEST] {}".format(request_data))

    data = IndigoRequestSchema(strict=True).load(request_data).data

    LOG_DATA('[REQUEST] /aromatize', data['input_format'], data['output_format'], data['struct'], data['options'])

    md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])

    md.struct.aromatize()
    return get_response(md, data['output_format'], data['json_output'], data['options'])


@indigo_api.route('/dearomatize', methods=['POST'])
@check_exceptions
def dearomatize():
    """
    Dearomatize structure
    ---
    tags:
      - indigo
    parameters:
      - name: json_request
        in: body
        required: true
        schema:
          id: IndigoDearomatizeRequest
          properties:
            struct:
              type: string
              default: c1ccccc1
            output_format:
              type: string
              default: chemical/x-daylight-smiles
    responses:
      200:
        description: Aromatized molecule
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_IndigoResponse"
      400:
        description: 'A problem with supplied client data'
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_Error"
    """
    data = IndigoRequestSchema(strict=True).load(get_request_data(request)).data

    LOG_DATA('[REQUEST] /dearomatize', data['input_format'], data['output_format'], data['struct'], data['options'])

    md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])

    if md.is_query:
        return get_error_response("Structures with query features cannot be dearomatized yet", 400, data['json_output'])
    md.struct.dearomatize()
    return get_response(md, data['output_format'], data['json_output'], data['options'])


@indigo_api.route('/convert', methods=['POST', 'GET'])
@check_exceptions
def convert():
    """
    Convert structure to Molfile/Rxnfile, SMILES, CML or InChI
    ---
    tags:
      - indigo
    parameters:
      - name: json_request
        in: body
        required: true
        schema:
          id: IndigoConvertRequest
          properties:
            struct:
              type: string
              default: C1=CC=CC=C1
            output_format:
              type: string
              default: chemical/x-mdl-molfile
              enum:
                - chemical/x-mdl-rxnfile
                - chemical/x-mdl-molfile
                - chemical/x-daylight-smiles
                - chemical/x-chemaxon-cxsmiles
                - chemical/x-cml
                - chemical/x-inchi
                - chemical/x-iupac
                - chemical/x-daylight-smarts
                - chemical/x-inchi-aux'
    responses:
      200:
        description: Aromatized molecule
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_IndigoResponse"
      400:
        description: 'A problem with supplied client data'
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_Error"
    """
    if request.method == 'POST':
        data = IndigoRequestSchema(strict=True).load(get_request_data(request)).data

        LOG_DATA('[REQUEST] /convert', data['input_format'], data['output_format'], data['struct'].encode('utf-8'), data['options'])
        md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])
        return get_response(md, data['output_format'], data['json_output'], data['options'])
    elif request.method == 'GET':

        input_dict = {
            'struct': request.args['struct'],
            'output_format' : request.args['output_format'] if 'output_format' in request.args else 'chemical/x-mdl-molfile',
        }


        data = IndigoRequestSchema(strict=True).load(input_dict).data

        LOG_DATA('[REQUEST] /convert', data['input_format'], data['output_format'], data['struct'].encode('utf-8'), data['options'])
        md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])

        if 'json_output' in request.args:
            data['json_output'] = True
        else:
            data['json_output'] = False

        return get_response(md, data['output_format'], data['json_output'], data['options'])



@indigo_api.route('/layout', methods=['POST'])
@check_exceptions
def layout():
    """
    Layout structure
    ---
    tags:
      - indigo
    parameters:
      - name: json_request
        in: body
        required: true
        schema:
          id: IndigoLayoutRequest
          properties:
            struct:
              type: string
              default: C1=CC=CC=C1
            output_format:
              type: string
              default: chemical/x-inchi
    responses:
      200:
        description: Molecule with calculated correct coordinates
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_IndigoResponse"
      400:
        description: 'A problem with supplied client data'
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_Error"
      500:
        description: 'A problem on server side'
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_Error"
    """
    data = IndigoRequestSchema(strict=True).load(get_request_data(request)).data
    LOG_DATA('[REQUEST] /layout', data['input_format'], data['output_format'], data['struct'], data['options'])
    md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])
    md.struct.layout()
    return get_response(md, data['output_format'], data['json_output'], data['options'])


@indigo_api.route('/clean', methods=['POST'])
@check_exceptions
def clean():
    """
    Clean up structure or selected substructure coordinates
    ---
    tags:
      - indigo
    parameters:
      - name: json_request
        in: body
        required: true
        schema:
          id: IndigoClean2dRequest
          properties:
            struct:
              type: string
              default: C1=CC=CC=C1
            output_format:
              type: string
              default: chemical/x-mdl-molfile
            selected:
              type: array
              default: [1, 2, 3]
    responses:
      200:
        description: Molecule with calculated coordinates
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_IndigoResponse"
      400:
        description: 'A problem with supplied client data'
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_Error"
      500:
        description: 'A problem on server side'
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_Error"
    """
    data = IndigoRequestSchema(strict=True).load(get_request_data(request)).data
    LOG_DATA('[REQUEST] /clean', data['input_format'], data['output_format'], data['struct'], data['options'])

    md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'], selected=data['selected'])
    if md.is_rxn and data['selected']:
        for sm in iterate_selected_submolecules(md.struct, data['selected']):
            sm.clean2d()
    else:
        md.substruct = md.struct.getSubmolecule(data['selected']) if data['selected'] else md.struct
        md.substruct.clean2d()
    return get_response(md, data['output_format'], data['json_output'], data['options'])


@indigo_api.route('/automap', methods=['POST'])
@check_exceptions
def automap():
    data = IndigoAutomapSchema(strict=True).load(get_request_data(request)).data
    LOG_DATA('[REQUEST] /automap', data['input_format'], data['output_format'], data['mode'], data['struct'], data['options'])
    md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])
    md.struct.automap(data['mode'])
    return get_response(md, data['output_format'], data['json_output'], data['options'])


@indigo_api.route('/calculate_cip', methods=['POST'])
@check_exceptions
def calculate_cip():
    data = IndigoRequestSchema(strict=True).load(get_request_data(request)).data
    LOG_DATA('[REQUEST] /calculate_cip', data['input_format'], data['output_format'], data['struct'], data['options'])
    md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])
    md.struct.dispatcher.setOption('molfile-saving-add-stereo-desc', True)
    return get_response(md, data['output_format'], data['json_output'], data['options'])


@indigo_api.route('/check2', methods=['POST', ])
@check_exceptions
def check2():
    data = IndigoCheckSchema(strict=True).load(get_request_data(request)).data
    try:
        indigo = indigo_init(data['options'])
    except Exception as e:
        raise HttpException('Problem with Indigo initialization: {0}'.format(e), 501)
    LOG_DATA('[REQUEST] /check', data['types'], data['struct'], data['options'])
    result = indigo.check(data['struct'], data['types']);
    return result, 200, {'Content-Type': 'application/json'}

@indigo_api.route('/check', methods=['POST', ])
@check_exceptions
def check():
    """
    Verifies input structure according to given types to Molfile/Rxnfile, SMILES, CML or InChI
    ---
    tags:
      - indigo
    parameters:
      - name: json_request
        in: body
        required: true
        schema:
          id: IndigoCheckRequest
          properties:
            struct:
              type: string
              default: C1=CC=CC=C1
            types:
              type: array
              default: ["valence"]
      - name: struct
        in: json_request
        type: string
      - name: types
        in: json_request
        required: true
        type: string
        enum: ['valence', 'ambiguous_h', 'query', 'pseudoatoms', 'radicals', 'stereo', 'overlapping_atoms', 'overlapping_bonds', '3d', 'sgroups', 'v3000', 'rgroups', 'chiral']
    responses:
      200:
        description: Verification messages
        schema:
          $ref: "#/definitions/indigo_api_check_post_IndigoResponse"
      400:
        description: 'A problem with supplied client data'
        schema:
          $ref: "#/definitions/indigo_api_check_post_Error"
    """
    def check_molecule(molecule, rxn_query=False, rxn=False):
        def check_atoms_overlapping(mol):
            def calc_dist(a, b):
                return sqrt(sum((a - b) ** 2 for a, b in zip(a, b)))

            mean_dist = 0
            for bond in mol.iterateBonds():
                a = bond.source().xyz()
                b = bond.destination().xyz()
                dist = calc_dist(a, b)
                mean_dist += dist
            if mol.countBonds() > 0:
                mean_dist = mean_dist / mol.countBonds()
            if not mean_dist and mol.countBonds():
                return True
            atom_xyz = []
            for atom in mol.iterateAtoms():
                a = atom.xyz()
                for b in atom_xyz:
                    dist = calc_dist(a, b)
                    if mean_dist and dist / mean_dist < 0.25:
                        return True
                atom_xyz.append(a)
            return False

        def check_bonds_overlapping(mol):
            def side(a, b, c):
                d = (c[1] - a[1]) * (b[0] - a[0]) - (b[1] - a[1]) * (c[0] - a[0])
                return 1 if d > 0 else (-1 if d < 0 else 0)

            def is_point_in_closed_segment(a, b, c):
                """ Returns True if c is inside closed segment, False otherwise.
                    a, b, c are expected to be collinear"""
                if a[0] < b[0]:
                    return a[0] <= c[0] and c[0] <= b[0]
                if b[0] < a[0]:
                    return b[0] <= c[0] and c[0] <= a[0]
                if a[1] < b[1]:
                    return a[1] <= c[1] and c[1] <= b[1]
                if b[1] < a[1]:
                    return b[1] <= c[1] and c[1] <= a[1]
                return a[0] == c[0] and a[1] == c[1]

            def closed_segment_intersect(a, b, c, d):
                """ Verifies if closed segments a, b, c, d do intersect"""
                if a == c or a == d or b == c or b == d:
                    return False
                s1 = side(a, b, c)
                s2 = side(a, b, d)
                # All points are collinear
                if s1 == 0 and s2 == 0:
                    return (is_point_in_closed_segment(a, b, c) or
                            is_point_in_closed_segment(a, b, d) or
                            is_point_in_closed_segment(c, d, a) or
                            is_point_in_closed_segment(c, d, b))
                # No touching and on the same side
                if s1 and s1 == s2:
                    return False
                s1 = side(c, d, a)
                s2 = side(c, d, b)
                # No touching and on the same side
                if s1 and s1 == s2:
                    return False
                return True

            bond_ids = range(mol.countBonds())
            result = False
            for b1, b2 in itertools.combinations(bond_ids, 2):
                bond1 = mol.getBond(b1)
                bond2 = mol.getBond(b2)
                a = bond1.source().xyz()[0:2]
                b = bond1.destination().xyz()[0:2]
                c = bond2.source().xyz()[0:2]
                d = bond2.destination().xyz()[0:2]
                if closed_segment_intersect(a, b, c, d):
                    result = True
                    break
            return result

        result = {}
        check_list = data['types']
        if 'query' in check_list:
            query = molecule.dbgInternalType() == '#03: <query molecule>' or rxn_query
            if not query:
                for item in itertools.chain(molecule.iterateAtoms(), molecule.iterateBonds()):
                    if item.checkQuery():
                        query = True
                        break
            if query:
                result['query'] = 'Structure contains query features'
        # TODO: Provide more verbose information about check result
        if 'rgroups' in check_list:
            if molecule.checkRGroups():
                result['rgroups'] = 'Structure contains RGroup components'
        if 'pseudoatoms' in check_list:
            pseudoatoms = []
            for atom in molecule.iteratePseudoatoms():
                pseudoatoms.append(atom.symbol())
            if len(pseudoatoms):
                result['pseudoatoms'] = len(pseudoatoms) if rxn else 'Structure contains {0} pseudoatom{1}'.format(str(len(pseudoatoms)), '' if len(pseudoatoms) == 1 else 's')
        if 'valence' in check_list:
            if 'query' in result:
                result['valence'] = 'Structure contains query features, so valency could not be checked'
            elif 'rgroups' in result:
                result['valence'] = 'Structure contains RGroup components, so valency could not be checked'
            else:
                valence_errors = []
                for atom in molecule.iterateAtoms():
                    e = atom.checkValence()
                    if e:
                        valence_errors.append(atom.index())
                if len(valence_errors):
                    result['valence'] = len(valence_errors) if rxn else 'Structure contains {0} atom{1} with bad valence'.format(str(len(valence_errors)), '' if len(valence_errors) == 1 else 's')  # : {1}'.format(str(len(radicals)), ', '.join(str(r) for r in radicals))
        if 'ambiguous_h' in check_list:
            if 'query' in result:
                result['ambiguous_h'] = 'Structure contains query features, so ambiguous H could not be checked'
            else:
                ambiguous_h = molecule.checkAmbiguousH()
                if ambiguous_h:
                    result['ambiguous_h'] = 'Structure has ambiguous hydrogens'
        if 'radicals' in check_list:
            if 'pseudoatoms' in result:
                result['radicals'] = 'Structure contains pseudoatoms, so radicals could not be checked'
            else:
                radicals = []
                for atom in molecule.iterateAtoms():
                    if atom.isRSite():
                        continue
                    r = atom.radicalElectrons()
                    if r:
                        radicals.append(atom.index())
                if len(radicals):
                    result['radicals'] = len(radicals) if rxn else 'Structure contains {0} atom{1} with radical electrons'.format(str(len(radicals)), '' if len(radicals) == 1 else 's')  # : {1}'.format(str(len(radicals)), ', '.join(str(r) for r in radicals))
        if 'stereo' in check_list:
            indigo = indigo_init()
            indigo.setOption('ignore-stereochemistry-errors', False)
            try:
                indigo.loadQueryMolecule(molecule.molfile()) if 'query' in result else indigo.loadMolecule(molecule.molfile())
                if molecule.checkStereo():
                    result['stereo'] = 'Structure contains one or more stereogenic atom(s) with unspecified stereochemistry'
            except IndigoException:
                result['stereo'] = 'Structure has stereochemistry errors'
        if '3d' in check_list:
            if molecule.hasZCoord():
                result['3d'] = 'Structure has Z coordinates'
        if 'sgroups' in check_list:
            for sgroup in molecule.iterateDataSGroups():
                result['sgroups'] = 'Structure has SGroups'
                break
        if 'v3000' in check_list:
            mf = molecule.molfile()
            if mf.find('V3000') != -1:
                result['v3000'] = 'Structure supports only Molfile V3000'
        if 'overlapping_atoms' in check_list:
            if check_atoms_overlapping(molecule):
                result['overlapping_atoms'] = 'Structure contains overlapping atoms'
        if 'overlapping_bonds' in check_list:
            if check_bonds_overlapping(molecule):
                result['overlapping_bonds'] = 'Structure contains overlapping bonds'
        if 'chiral' in check_list:
            if not molecule.checkChirality():
               result['chiral'] = 'Structure has invalid Chiral flag'
            if molecule.isChiral():
                result['chiral'] = 'Structure has Chiral flag'
            if molecule.check3DStereo():
                result['chiral'] = 'Structure has 3D Chiral center'
        return result

    def check_reaction(rxn):
        result = collections.defaultdict(list)
        for m in rxn.iterateMolecules():
            cm = check_molecule(m, rxn_query=rxn.dbgInternalType() == '#05: <query reaction>', rxn=True)
            for key in cm:
                result[key].append(cm[key])
        message_dict = {
            'valence': 'bad valence',
            'radicals': 'radical electrons',
            'pseudoatoms': 'pseudoatoms'
        }
        for key in result:
            if key in ('valence', 'pseudoatoms', 'radicals'):
                str_values = set()
                int_values = []
                for value in result[key]:
                    int_values.append(value) if type(value) is int else str_values.add(value)
                result[key] = ''
                if len(int_values):
                    result[key] = 'Structure contains {0} atom(s) with {1}{2}'.format(sum(int_values), message_dict[key], ', ' if len(str_values) else '')
                result[key] += ', '.join(str_values)
            else:
                result[key] = ', '.join(list(set(result[key])))
        return result

    data = IndigoCheckSchema(strict=True).load(get_request_data(request)).data
    LOG_DATA('[REQUEST] /check', data['types'], data['struct'], data['options'])
    md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'], selected=data['selected'])
    result = check_reaction(md.struct) if md.is_rxn else check_molecule(md.struct)
    if data['json_output']:
        return jsonify(result), 200, {'Content-Type': 'application/json'}
    else:
        return '\n'.join(['{0}: {1}'.format(key, value) for key, value in {k: v for k, v in result.items() if v}.items()]), 200, {'Content-Type': 'text/plain'}


@indigo_api.route('/calculate', methods=['POST', ])
@check_exceptions
def calculate():
    """
    Calculate properites for input structure
    ---
    tags:
      - indigo
    parameters:
      - name: json_request
        in: body
        required: true
        schema:
          id: IndigoCalculateRequest
          properties:
            struct:
              type: string
              default: C1=CC=CC=C1
            'properties':
              type: array
              default: ["molecular-weight"]
      - name: struct
        in: json_request
        type: string
      - name: properties
        in: json_request
        required: true
        type: string
        enum: ['molecular-weight', 'most-abundant-mass', 'monoisotopic-mass', 'gross', 'mass-composition']
        default: ["molecular-weight"]
    responses:
      200:
        description: Calculated properites
        schema:
          $ref: "#/definitions/indigo_api_calculate_post_IndigoResponse"
      400:
        description: 'A problem with supplied client data'
        schema:
          $ref: "#/definitions/indigo_api_calculate_post_Error"
    """
    data = IndigoCalculateSchema(strict=True).load(get_request_data(request)).data
    LOG_DATA('[REQUEST] /calculate', data['properties'], data['selected'], data['struct'], data['options'])
    md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'], selected=data['selected'])
    if data['selected']:
        if md.is_rxn:
            remove_unselected_repeating_units_r(md.struct, data['selected'])
        else:
            remove_unselected_repeating_units_m(md.struct, data['selected'])
    calculate_properties = data['properties']
    result = {}
    precision = data['precision']
    func_name_dict = {
        'molecular-weight': 'molecularWeight',
        'most-abundant-mass': 'mostAbundantMass',
        'monoisotopic-mass': 'monoisotopicMass',
        'mass-composition': 'massComposition',
        'gross': 'grossFormula'
    }
    for p in calculate_properties:
        if md.is_rxn:
            if data['selected']:
                result[p] = selected_reaction_calc(md.struct, data['selected'], func_name_dict[p], precision=precision)
            else:
                result[p] = reaction_calc(md.struct, func_name_dict[p], precision=precision)
        else:
            if data['selected']:
                result[p] = selected_molecule_calc(md.struct, data['selected'], func_name_dict[p], precision=precision)
            else:
                result[p] = molecule_calc(md.struct, func_name_dict[p], precision=precision)
    if data['json_output']:
        return jsonify(result), 200, {'Content-Type': 'application/json'}
    else:
        return '\n'.join(['{0}: {1}'.format(key, value) for key, value in {k: v for k, v in result.items() if v}.items()]), 200, {'Content-Type': 'text/plain'}


@indigo_api.route('/render', methods=['POST', 'GET'])
@check_exceptions
def render():
    """
    Render molecule
    ---
    tags:
      - indigo
    description: 'Returns a molecule image'
    consumes:
      - application/json
      # - chemical/x-mdl-molfile
      # - chemical/x-mdl-rxnfile
      # - chemical/x-daylight-smiles
      # - chemical/x-cml
      # - chemical/x-inchi
    produces:
      - image/png
      - image/svg+xml
      - application/pdf
    parameters:
      - name: json_request
        in: body
        required: true
        schema:
          id: IndigoRenderRequest
          properties:
            struct:
              type: string
              default: C1=CC=CC=C1
            query:
              type: string
              default: CCC
            output_format:
              type: string
              default: image/svg+xml
            options:
              type: array
    responses:
      200:
        description: 'A rendered molecule image'
        schema:
          type: file
      400:
        description: 'A problem with supplied client data'
        schema:
          $ref: "#/definitions/indigo_api_aromatize_post_Error"
    """
    render_format_dict = {
        'image/svg+xml': 'svg',
        'image/png': 'png',
        'application/pdf': 'pdf',
        'image/png;base64': 'png'
    }
    if request.method == 'POST':
        LOG_DATA('[REQUEST] /render', request.headers['Content-Type'], request.headers['Accept'], request.data)
        try:
            if 'application/json' in request.headers['Content-Type']:
                input_dict = json.loads(request.data.decode())
            else:
                input_dict = {
                    'struct': request.data,
                    'output_format': request.headers['Accept'],
                }
        except ValueError:
            return get_error_response('Invalid input JSON: {0}'.format(request.data), 400)

        data = IndigoRendererSchema(strict=True).load(input_dict).data
        if data['struct'] and not data['query']:
            md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])
        elif data['query'] and not data['struct']:
            md = load_moldata(data['query'], options=data['options'])
        else:
            md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])
            mdq = load_moldata(data['query'], mime_type=data['input_format'], indigo=md.struct.dispatcher, query=True)
            try:
                md.struct = highlight(md.struct.dispatcher, md.struct, mdq.struct)
            except RuntimeError:
                pass

    elif request.method == 'GET':

        LOG_DATA('[REQUEST] /render GET', request.args)

        try:
            input_dict = {
            'struct': request.args['struct'] if 'struct' in request.args else None,
            'output_format': request.args['output_format'] if 'output_format' in  request.args else 'image/svg+xml',
            'query': request.args['query']  if 'query' in request.args else None,
            }
            if input_dict['struct'] and not input_dict['query']:
                md = load_moldata(input_dict['struct'])
            elif  input_dict['query'] and not input_dict['struct']:
                mdq = load_moldata(input_dict['query'])
            else:
                md = load_moldata(input_dict['struct'])
                mdq = load_moldata(input_dict['query'], indigo=md.struct.dispatcher, query=True)
                md.struct = highlight(md.struct.dispatcher, md.struct, mdq.struct)
            data = IndigoRendererSchema(strict=True).load(input_dict).data
        except Exception as e:
            return get_error_response('Invalid GET query {}'.format(str(e)), 400)



    indigo = md.struct.dispatcher
    indigo.setOption("render-coloring", True)
    indigo.setOption("render-output-format", render_format_dict[data['output_format']])
    indigo.setOption("render-image-width", data['width'])
    indigo.setOption("render-image-height", data['height'])
    result = indigo.renderer.renderToBuffer(md.struct)
    result = result.tostring() if sys.version_info < (3, 2) else result.tobytes()

    if 'image/png;base64' in data['output_format']:
        png_b64 = base64.b64encode(result)
        result = 'data:image/png;base64, {}'.format(str(png_b64, 'ascii'))

    indigo_api_logger.info("[RESPONSE] Content-Type: {0}".format(data['output_format']))
    return result, 200, {'Content-Type': data['output_format']}



