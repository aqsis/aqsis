# pygments lexers for RIB and RSL.
#
# Install: Link this file to the pygments/lexers directory, cd to that
# directory and run the script _mapping.py
#
# Then also link the file into the python module search path (ugh!)
#
# On my machine:
#
# cd /usr/share/pyshared/pygments/lexers
# ln -s $AQSIS_SRC/doc/manual/user/renderman_lexers.py .
# python _mapping.py
# ln -s $PWD/renderman_lexers.py /usr/lib/pymodules/python2.6/pygments/lexers/
#
# (there doesn't seem to be better way!?)

from pygments.lexer import RegexLexer
from pygments.token import *

# Register lexers so that the pygments _mapping.py script can find them.
__all__ = ['RibLexer', 'RslLexer']


class RibLexer(RegexLexer):
    '''
    Lexer Renderman Interface Bytestream (RIB) scene definition files
    '''

    name = 'RenderManRIB'
    aliases = ['RIB', 'rib']
    filenames = ['*.rib']

    tokens = {
        'root': [
            (r'\n', Text),
            (r'\s+', Text),
            (r'[][]', Punctuation),
            (r'##.*\n', Comment.Special),
            (r'#.*\n', Comment.Single),
            (r'"', String, 'string'),
            (r'[+-]?(\d+\.\d*)([eE][+-]?\d+)?', Number.Float),
            (r'[+-]?\d+', Number.Integer),
            (r'[^\s#"]+', Keyword),
        ],
        'string': [
            (r'"', String, '#pop'),
            (r'\\([\\nrtbf"\']|[0-7]{1,3})', String.Escape),
            (r'[^\\"\n]+', String), # all other characters
            (r'\\\n', String), # line continuation
            (r'\\', String), # stray backslash
        ],
    }


class RslLexer(RegexLexer):
    '''
    Lexer for Renderman Shading Language with C preprocessor directives.  This
    includes all keywords and types defined by RISpec version 3.2 (ie, RSL
    1.0), plus one or two postdating the spec.  No attempt is made to
    highlight additional keywords from the RSL 2.0 language.
    '''
    name = 'RenderManSL'
    aliases = ['rsl', 'sl']
    filenames = ['*.sl']

    tokens = {
        'root': [
            (r'^\s*#if\s+0', Comment.Preproc, 'if0'),
            (r'^\s*#', Comment.Preproc, 'macro'),
            (r'\n', Text),
            (r'\s+', Text),
            (r'\\\n', Text), # line continuation
            (r'/(\\\n)?/(\n|(.|\n)*?[^\\]\n)', Comment.Single),
            (r'/(\\\n)?[*](.|\n)*?[*](\\\n)?/', Comment.Multiline),
            (r'[{}]', Punctuation),
            (r'"', String, 'string'),
            (r'(\d+\.\d*|\.\d+|\d+)([eE][+-]?\d+)?', Number.Float),
            (r'[~!%^&*+=|?:<>/-]', Operator),
            (r'[()\[\],.;]', Punctuation),
            (r'(break|continue|else|for|if|return|while|illuminate|'
             r'illuminance|solar|gather|'
             r'surface|volume|displacement|imager|light)\b', Keyword),
            (r'(float|point|vector|normal|string|void|matrix|color|'
             r'uniform|varying|output|extern)\b',
             Keyword.Type),
            ('[a-zA-Z_][a-zA-Z0-9_]*', Name),
        ],
        'string': [
            (r'"', String, '#pop'),
            (r'\\([\\abfnrtv"\']|x[a-fA-F0-9]{2,4}|[0-7]{1,3})', String.Escape),
            (r'[^\\"\n]+', String), # all other characters
            (r'\\\n', String), # line continuation
            (r'\\', String), # stray backslash
        ],
        'macro': [
            (r'[^/\n]+', Comment.Preproc),
            (r'/[*](.|\n)*?[*]/', Comment.Multiline),
            (r'//.*?\n', Comment.Single, '#pop'),
            (r'/', Comment.Preproc),
            (r'(?<=\\)\n', Comment.Preproc),
            (r'\n', Comment.Preproc, '#pop'),
        ],
        'if0': [
            (r'^\s*#if.*?(?<!\\)\n', Comment.Preproc, '#push'),
            (r'^\s*#endif.*?(?<!\\)\n', Comment.Preproc, '#pop'),
            (r'.*?\n', Comment),
        ]
    }

