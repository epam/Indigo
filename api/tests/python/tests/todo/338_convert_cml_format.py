import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *
from threading import local
import re

tls = local()

##indigo = Indigo()

def printInfo(mol):
    print("Internal molecule: %s" % mol.name())
    print("Atoms:")
    with mol.iterateAtoms() as iterator:
        while iterator.hasNext() :
            atom = iterator.next()
            is_pseudo = atom.isPseudoatom()
            atomic_number = 0 if (is_pseudo) else atom.atomicNumber()
            print('%s, number: %d, is pseudo: %d' % (atom.symbol(), atomic_number, is_pseudo))


indigo_defaults = {
    'ignore-stereochemistry-errors': 'true',
    'smart-layout': 'true',
    'gross-formula-add-rsites': 'true',
    'mass-skip-error-on-pseudoatoms': 'false',
}

# Copy-paste from utils/indigo-service/service/v2/indigo_api.py#62
def indigo_init(options={}):
    try:
        tls.indigo = Indigo()
##      tls.indigo.inchi = IndigoInchi(tls.indigo)
##      tls.indigo.renderer = IndigoRenderer(tls.indigo)
        for option, value in indigo_defaults.items():
            tls.indigo.setOption(option, value)
        for option, value in options.items():
            # TODO: Remove this when Indigo API supports smiles type option
            if option in {'smiles', }:
                continue
            tls.indigo.setOption(option, value)
        return tls.indigo
    except Exception as e:
##      indigo_api_logger.error('indigo-init: {0}'.format(e))
        print('indigo-init: {0}'.format(e))
        return None

# Copy-paste from utils/indigo-service/service/v2/indigo_api.py#80
class MolData:
    is_query = False
    is_rxn = False
    struct = None
    substruct = None

# Copy-paste from utils/indigo-service/service/v2/indigo_api.py#87
def is_rxn(molstr):
    return '>>' in molstr or molstr.startswith('$RXN') or '<reactantList>' in molstr

# Copy-paste from utils/indigo-service/service/v2/indigo_api.py#263
def load_moldata(molstr, indigo=None, options={}, query=False, mime_type=None, selected=[]):
    if not indigo:
        try:
            indigo = indigo_init(options)
        except Exception as e:
##          raise HttpException('Problem with Indigo initialization: {0}'.format(e), 501)
            print('Problem with Indigo initialization: {0}'.format(e))

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


# Copy-paste from utils/indigo-service/service/v2/indigo_api.py#306
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


# Copy-paste from utils/indigo-service/service/v2/indigo_api.py#358
def get_response(md, output_struct_format, json_output, options):
    output_mol = save_moldata(md, output_struct_format, options)
##  LOG_DATA('[RESPONSE]', output_struct_format, options, output_mol.encode('utf-8'))

    if json_output:
        return jsonify({'struct': output_mol, 'format': output_struct_format}), 200, {'Content-Type': 'application/json'}
    else:
        return output_mol ##, 200, {'Content-Type': output_struct_format}


# Copy-paste from utils/indigo-service/service/v2/indigo_api.py#545
    """
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
    """
def convert(data:dict):
##  if request.method == 'POST':
##      data = IndigoRequestSchema(strict=True).load(get_request_data(request)).data

##      LOG_DATA('[REQUEST] /convert', data['input_format'], data['output_format'], data['struct'].encode('utf-8'), data['options'])
        md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])
        printInfo(md.struct)
        return get_response(md, data['output_format'], data['json_output'], data['options'])
##  elif request.method == 'GET':
##
##      input_dict = {
##          'struct': request.args['struct'],
##          'output_format' : request.args['output_format'] if 'output_format' in request.args else 'chemical/x-mdl-molfile',
##      }
##
##
##      data = IndigoRequestSchema(strict=True).load(input_dict).data
##
##      LOG_DATA('[REQUEST] /convert', data['input_format'], data['output_format'], data['struct'].encode('utf-8'), data['options'])
##      md = load_moldata(data['struct'], mime_type=data['input_format'], options=data['options'])
##
##      if 'json_output' in request.args:
##          data['json_output'] = True
##      else:
##          data['json_output'] = False
##
##      return get_response(md, data['output_format'], data['json_output'], data['options'])


try:
    with open(joinPath('molecules/file_338.cml'), 'r') as file:
        str_load = file.read()
    print("Input molecule:\n", str_load)

    data = {
        'struct': str_load,
        'output_format': 'chemical/x-mdl-molfile',
        'input_format' : 'chemical/x-cml',
        'json_output' : False,
        'options' : { 'ignore-stereochemistry-errors':True, }
    }
    response = convert(data)
    
    print("Output molecule:\n", response)
except IndigoException as e:
    print("caught " + getIndigoExceptionText(e))
