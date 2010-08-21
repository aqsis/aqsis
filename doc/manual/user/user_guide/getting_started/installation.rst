.. include:: ../../common.rst

.. _runtime libraries: http://www.microsoft.com/downloads/details.aspx?familyid=A5C84275-3B97-4AB7-A40D-3802B2AF5FC2&displaylang=en
.. _openSUSE Build Service: http://software.opensuse.org/search?q=aqsis
.. _SourceForge: http://downloads.sourceforge.net/aqsis

.. _installation:

.. index:: install 

============
Installation
============

.. index:: install; Windows, Windows; install

Windows
-------

On Windows systems, |Aqsis| is available as a self installing executable. Simply run the setup to begin the installation, you will be presented with a license agreement, and a number of configuration options.

System PATH
	You can choose to add the installation folder to your system or user PATH environment variable, or not modify the PATH at all. Choosing to modify the PATH will make sure that |Aqsis| can be located from a command line environment as well as by thirdparty plugin_ tools and applications that might need to locate the |Aqsis| tools.

Installation Location
	By default, |Aqsis| will be installed to the standard program files location on your system, however, you can modify the installation location here if you want it installed to a non-standard drive/folder.

Start Menu Folder
	The installer can optionally place shortcuts to some |Aqsis| tools and documentation in the Windows Start Menu, you can choose which folder to place these shortcuts, or choose to not add the shortcuts at all.

.. note:: The installer version of |Aqsis| is built with the MicroSoft Visual Studio tools, it might be necessary to install the appropriate `runtime libraries`_ if you don't already have them installed.


.. index:: install; Mac OSX, Mac OSX; install, Mac OSX; Universal binary, Universal binary

OSX
---

|Aqsis| is available as a Universal binary from http://downloads.sourceforge.net/aqsis.

Once the disk image (\*.dmg) is mounted you can launch the |Aqsis| application, and optionally install it in the "Applications" folder. When launched, the application provides the option to either open an |Aqsis| shell or open the graphical front-end eqsl_.

.. note:: The Aqsis shell updates the session **PATH** and **AQSISHOME** environment variables on each execution, meaning the application bundle can be safely moved around your system without breaking functionality.


.. index:: install; Linux, Linux; install

Linux
-----

|Aqsis| is part of the official package repositories for a number of Linux distributions, including Fedora, openSUSE, Debian (including Ubuntu) and Gentoo. Most users can refer to their native package manager to install Aqsis.

.. note:: As of October 2009, these Linux distribution packages are very *outdated* and still contain version 1.2 of Aqsis. Currently there are no installation packages for Linux provided on our SourceForge site. Therefore, the instructions in this section should be disregarded for now. Please refer to our instructions on how to build Aqsis 1.6 from source. 

Red Hat Enterprise Linux
^^^^^^^^^^^^^^^^^^^^^^^^

Packages can be found at the following location(s):

* `openSUSE Build Service`_
* SourceForge_

Fedora
^^^^^^

Open a console/terminal window and enter the following command::

	yum install aqsis

openSUSE
^^^^^^^^

Packages can be found at the following location(s):

* `openSUSE Build Service`_
* SourceForge_

Debian
^^^^^^

Open your package manager of choice and install the package "aqsis", or open a terminal window and enter the following command::

	sudo apt-get install aqsis

Gentoo
^^^^^^

A gentoo ebuild is available in portage.  Install in the usual way using the command::

	emerge aqsis

