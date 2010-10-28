import xml.etree.ElementTree as etree
import re
import exceptions

#-------------------------------------------------------------------------------
def parseXml(fileName):
    return etree.parse(fileName)


#-------------------------------------------------------------------------------
# Code formatting functions
def commentBanner(str, width=70, fillchar='='):
    comment = '// '
    return comment + (' ' + str + ' ').center(width - len(comment), fillchar)

def commentBannerC(str, width=70, fillchar='='):
    return '/* ' + (' ' + str + ' ').center(width - 6, fillchar) + ' */'

def wrapDecl(str, maxWidth, wrapIndent=None):
    '''Wrap a one-line declaration onto multiple lines.'''
    splits = re.split(', *', str);
    if len(splits) == 1:
        return str
    line = splits.pop(0)
    if wrapIndent is None:
        indent = (line.find('(') + 1)*' '
    else:
        indent = ' ' * wrapIndent
    result = []
    while len(line) != 0:
        while len(splits) > 0 and len(line) + 3 + len(splits[0]) <= maxWidth:
            line += ', '
            line += splits.pop(0)
        result += [line]
        if len(splits) == 0:
            line = ''
        else:
            line = indent + splits.pop(0)
    return ',\n'.join(result)



#-------------------------------------------------------------------------------
# Code generation functions with etree objects as arguments.
#
def ribArgs(procXml):
    return [a for a in procXml.findall('Arguments/Argument')
            if not a.findall('RibValue')]

def cArgs(procXml):
    return [a for a in procXml.findall('Arguments/Argument')
            if not a.findall('RibOnly')]

def cName(procXml):
    if procXml.findall('CName'):
        return procXml.findtext('CName')
    else:
        return 'Ri' + procXml.findtext('Name')

def formalArg(arg):
    type = arg.findtext('Type')
    name = arg.findtext('Name')
    if type is None:
        return name
    if type == 'RtTokenOrInt':
        type = 'RtToken'
    if type.endswith('Array'):
        return 'const %sArray& %s' % (type[2:-5], name)
    else:
        type = re.sub(r'Rt(Token|String|Color|Point(?!er)|Matrix|Bound|Basis)',
                      r'RtConst\1', type)
    return '%s %s' % (type, name)

def formalArgC(arg, arraySuffix=''):
    type = arg.findtext('Type')
    name = arg.findtext('Name')
    if type.endswith('Array'):
        return '%s %s[]' % (type[:-5], name + arraySuffix)
    else:
        return '%s %s' % (type, name)

def riCxxMethodDecl(procXml, className=None, useDefaults=False):
    args = [formalArg(arg) for arg in ribArgs(procXml)]
    if procXml.findall('Arguments/ParamList'):
        if procXml.findall('Arguments/ParamList/Optional') and useDefaults:
            args += ['const ParamList& pList = ParamList()']
        else:
            args += ['const ParamList& pList']
    procName = procXml.findtext('Name')
    if className is not None:
        procName = className + '::' + procName
    return 'RtVoid %s(%s)' % (procName, ', '.join(args))


def wrapperCallArgList(procXml):
    args = [arg.findtext('Name') for arg in ribArgs(procXml)]
    if procXml.findall('Arguments/ParamList'):
        args += ['pList']
    return args


#------------------------------------------------------------------------------
# Renderman interface stuff
paramDeclRegex = re.compile(
'(uniform|constant|varying|vertex|facevarying|facevertex)? *\
(float|point|color|integer|string|vector|normal|hpoint|matrix|mpoint)\
(?:\[([0-9]+)\])? *([a-zA-Z]+)')

def parseParamDecl(declString):
    g = list(paramDeclRegex.match(declString).groups())
    if g[0] is None:
        g[0] = 'uniform'
    if g[2] is None:
        g[2] = '1'
    return g

def paramDeclStringToCxx(declString):
    toks = parseParamDecl(declString)
    iclassToCxxIClass = {
        'uniform':     'Uniform',
        'constant':    'Constant',
        'varying':     'Varying',
        'vertex':      'Vertex',
        'facevarying': 'FaceVarying',
        'facevertex':  'FaceVertex',
    }
    typeToCxxType = {
        'float'   : 'Float',
        'point'   : 'Point',
        'color'   : 'Color',
        'integer' : 'Integer',
        'string'  : 'String',
        'vector'  : 'Vector',
        'normal'  : 'Normal',
        'hpoint'  : 'HPoint',
        'matrix'  : 'Matrix',
        'mpoint'  : 'MPoint',
    }
    return ('Ri::TypeSpec(Ri::TypeSpec::%s, Ri::TypeSpec::%s, %s)' % \
            (iclassToCxxIClass[toks[0]], typeToCxxType[toks[1]], toks[2]), \
            toks[3])

# vi: set et:
