.. include:: ../../common/external.rst


====================
Using Aqsis Codebase
====================

File Naming Convention
======================

To avoid any problems with non-case sensitive filesystems, such as Windows, all filenames should use the following rules where possible:

* All filenames should be completely lowercase
* All C/C++ header files will use the extension ``.h``
* All C source files will use the extension ``.c``
* All C++ source files will use the extension ``.cpp``

The only allowable exceptions are for files sourced from a third-party that are not under the direct control of the project, such as header files for a third-party library.


Style Guidelines
================

The following sections provide a brief rundown of some parts of the aqsis coding conventions. It's important to also inspect our codebase which contains extra information missing here.


Naming Conventions
------------------

All code written specifically for the Aqsis project should use the following common naming convention for code elements.


Name prefixes
~~~~~~~~~~~~~

When browsing our codebase, you will notice the common occurrence of the prefixes ``Cq``, ``Sq``, ``Eq``, ``Tq``, ``Iq``, ``Xq``, and ``m_``. These prefixes are applied to:

* All classes, unless otherwise specified, use the prefix ``Cq``
* All structs use the prefix ``Sq``
* All enumerations use the prefix ``Eq``
* All typedefs use the prefix ``Tq``
* All interfaces (pure virtual base classes) use the prefix ``Iq``
* All exceptions use the prefix ``Xq``
* All private class member data (note: classes should not have public member data!) use the prefix ``m_``


Naming of types, member data etc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Apart from the prefixes described above, the remainder of names should adhere to the following policies:

* All ``types`` (classes, enums, etc) should be named with *CamelCase*
* All ``functions``, including member functions, should be named with *lowerCamelCase*
* All ``variables`` (including member data) should be named with *lowerCamelCase*
* Individual ``enumerations`` elements should be named with a common *CamelCase* prefix, followed by a single underscore, and the enumeration name using *CamelCase*

.. warning::
   `Hungarian Notation`_ is not acceptable.


Examples
~~~~~~~~

Here are some examples adhering to the above conventions:

* ``class CqSurfacePointsPolygons``
* ``struct SqSamplePoint``
* ``enum EqPlaneSide``
  - Enumeration element: ``Side_Outside``
* typedef: ``TqFloat``
* Class member data: ``m_pointerToData``
* Member function: ``pointerToData()``


Indenting, bracket placement
----------------------------

The Aqsis code base uses a custom `Artistic Style`_ settings file in an attempt to ensure that the coding style remains constant throughout:

.. include:: astylerc
   :literal:

Please use this program to adapt your personal coding style (in particular, indenting type/style and bracket placement) to our accepted style before committing changes to the codebase.

