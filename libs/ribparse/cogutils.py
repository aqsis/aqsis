import xml.etree.ElementTree as etree
import re
import exceptions

#-------------------------------------------------------------------------------
# XML functions

def patchEtreeElementClass():
    '''Monky patch the etree interface to include some handy methods'''
    # The element class type is left undefined in the docs, so we use the
    # Element() factory function to get an instance & then retrive the exact
    # element class for modification :-)
    elementClass = etree.Element('dummy').__class__
    def haschild(element, name):
        return element.find(name) is not None
    elementClass.haschild = haschild
#    def elementGetAttr(element, name):
#        if name.startswith('__'):
#            raise AttributeError('special method?')
#        elements = element.findall(name)
#        if len(elements) == 0:
#            raise AttributeError('element "%s" has no child named "%s"'
#                                 % (element.tag, name))
#        elif len(elements) == 1:
#            return elements[0]
#        else:
#            return elements
#    elementClass.__getattr__ = elementGetAttr

# WARNING: Dynamically patching the Element class is a bit of an ugly hack, but
# how else can we get methods attached to element objects?
#
# It does make the resulting code much nicer to read ;-)
patchEtreeElementClass()

def parseXmlTree(fileName):
    return etree.ElementTree().parse(fileName)


#-------------------------------------------------------------------------------
# Code formatting functions
def commentBanner(str, width=70, fillchar='='):
    comment = '// '
    return comment + (' ' + str + ' ').center(width - len(comment), fillchar)

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
def ribArgs(argList):
    return filter(lambda arg: not arg.haschild('RibValue'), argList)

def cName(procXml):
    if procXml.haschild('CName'):
        return procXml.findtext('CName')
    else:
        return 'Ri' + procXml.findtext('Name')

def formalArg(arg):
    type = arg.findtext('Type')
    name = arg.findtext('Name')
    if type is None:
        return name
    type = type.replace('RtToken', 'RtConstToken')
    type = type.replace('RtString', 'RtConstString')
    if type.endswith('Array'):
        return 'const Array<%s>& %s' % (type[:-5], name)
    return '%s %s' % (type, name)

def riCxxMethodDecl(procXml, className=None):
    args = [formalArg(arg) for arg in ribArgs(procXml.findall('Arguments/Argument'))]
    if procXml.haschild('Arguments/ParamList'):
        args += ['const ParamList& pList']
    procName = procXml.findtext('Name')
    if className is not None:
        procName = className + '::' + procName
    return '%s %s(%s)' % (procXml.findtext('ReturnType'), procName, ', '.join(args))



##------------------------------------------------------------------------------
### The following handy class factory is stolen from python-2.6
def namedtuple(typename, field_names, verbose=False):
    from operator import itemgetter as _itemgetter
    from keyword import iskeyword as _iskeyword
    import sys as _sys
    """Returns a new subclass of tuple with named fields.

    >>> Point = namedtuple('Point', 'x y')
    >>> Point.__doc__                   # docstring for the new class
    'Point(x, y)'
    >>> p = Point(11, y=22)             # instantiate with positional args or keywords
    >>> p[0] + p[1]                     # indexable like a plain tuple
    33
    >>> x, y = p                        # unpack like a regular tuple
    >>> x, y
    (11, 22)
    >>> p.x + p.y                       # fields also accessable by name
    33
    >>> d = p._asdict()                 # convert to a dictionary
    >>> d['x']
    11
    >>> Point(**d)                      # convert from a dictionary
    Point(x=11, y=22)
    >>> p._replace(x=100)               # _replace() is like str.replace() but targets named fields
    Point(x=100, y=22)

    """

    # Parse and validate the field names.  Validation serves two purposes,
    # generating informative error messages and preventing template injection attacks.
    if isinstance(field_names, basestring):
        field_names = field_names.replace(',', ' ').split() # names separated by whitespace and/or commas
    field_names = tuple(map(str, field_names))
    for name in (typename,) + field_names:
        if not all(c.isalnum() or c=='_' for c in name):
            raise ValueError('Type names and field names can only contain alphanumeric characters and underscores: %r' % name)
        if _iskeyword(name):
            raise ValueError('Type names and field names cannot be a keyword: %r' % name)
        if name[0].isdigit():
            raise ValueError('Type names and field names cannot start with a number: %r' % name)
    seen_names = set()
    for name in field_names:
        if name.startswith('_'):
            raise ValueError('Field names cannot start with an underscore: %r' % name)
        if name in seen_names:
            raise ValueError('Encountered duplicate field name: %r' % name)
        seen_names.add(name)

    # Create and fill-in the class template
    numfields = len(field_names)
    argtxt = repr(field_names).replace("'", "")[1:-1]   # tuple repr without parens or quotes
    reprtxt = ', '.join('%s=%%r' % name for name in field_names)
    dicttxt = ', '.join('%r: t[%d]' % (name, pos) for pos, name in enumerate(field_names))
    template = '''class %(typename)s(tuple):
        '%(typename)s(%(argtxt)s)' \n
        __slots__ = () \n
        _fields = %(field_names)r \n
        def __new__(_cls, %(argtxt)s):
            return _tuple.__new__(_cls, (%(argtxt)s)) \n
        @classmethod
        def _make(cls, iterable, new=tuple.__new__, len=len):
            'Make a new %(typename)s object from a sequence or iterable'
            result = new(cls, iterable)
            if len(result) != %(numfields)d:
                raise TypeError('Expected %(numfields)d arguments, got %%d' %% len(result))
            return result \n
        def __repr__(self):
            return '%(typename)s(%(reprtxt)s)' %% self \n
        def _asdict(t):
            'Return a new dict which maps field names to their values'
            return {%(dicttxt)s} \n
        def _replace(_self, **kwds):
            'Return a new %(typename)s object replacing specified fields with new values'
            result = _self._make(map(kwds.pop, %(field_names)r, _self))
            if kwds:
                raise ValueError('Got unexpected field names: %%r' %% kwds.keys())
            return result \n
        def __getnewargs__(self):
            return tuple(self) \n\n''' % locals()
    for i, name in enumerate(field_names):
        template += '        %s = _property(_itemgetter(%d))\n' % (name, i)
    if verbose:
        print template

    # Execute the template string in a temporary namespace and
    # support tracing utilities by setting a value for frame.f_globals['__name__']
    namespace = dict(_itemgetter=_itemgetter, __name__='namedtuple_%s' % typename,
                     _property=property, _tuple=tuple)
    try:
        exec template in namespace
    except SyntaxError, e:
        raise SyntaxError(e.message + ':\n' + template)
    result = namespace[typename]

    # For pickling to work, the __module__ variable needs to be set to the frame
    # where the named tuple is created.  Bypass this step in enviroments where
    # sys._getframe is not defined (Jython for example).
    if hasattr(_sys, '_getframe'):
        result.__module__ = _sys._getframe(1).f_globals.get('__name__', '__main__')

    return result
