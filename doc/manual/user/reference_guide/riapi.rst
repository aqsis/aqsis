******
RI API
******

.. highlight:: RIB

The operation of a RenderMan renderer is mostly controlled by the Options and
Attributes mechanisms, both of which associate named values with the various
stages of the rendering pipeline.

**Options** are associated with the scene as a whole.  Once the
``RiWorldBegin`` directive has been reached, the options are fixed, it is then
illegal to call any Ri directives that can modify the options state.
**Attributes** are associated with the attribute stack, and as such are
assigned to light sources and geometric primitives.  The value of these
attributes are pushed and popped along with other attribute state by the
``RiAttributeBegin/End`` directives.

Options and attributes are stored as a named set of name/value pairs. That is,
each option or attribute has a unique name, and can contain any number of
values, each with a unique name and type. The general format of an option or
attribute directive is::

  Option "<option name>" "<value1 typespec>" [<value1 value(s)>] "<value2 typespec>" [<value2 value(s)>]

where ``typespec`` is either the name of an already declared value, or an
inline declaration.  For example::

  Declare "string mystringvalue"
  Option "myopt" "mystringvalue" ["some string"] "float myfloatvalue" [1.0]

Note from the second example above that it is perfectly reasonable to specify
multiple name/value pairs in a single Option directive, each will add a new
value to the same containing option or attribute.

There are a number of predefined options and attributes that Aqsis recognizes
for use internally to configure the operation of the renderer; these are listed
below.  In addition, Aqsis fully supports the specification of arbitrary
user-defined name/value pairs for both options and attributes.  These may be
queried from the shading language using the standard ``option()`` and
``attribute()`` functions.


.. toctree::

    options
    attributes
    extensions
