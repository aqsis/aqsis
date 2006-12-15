# Title: Aqsis Package for Linux (RPM)
# Author: Aqsis Team (packages@aqsis.org)
# Info: 
# Other: 1. To make updates easier, all message strings have been placed within the top 10-40 lines of this file.
#        2. To build using the 'Official' tarball comment line 18 and uncomment line 19.

# norootforbuild


%define PRODUCT_NAME aqsis
%define PRODUCT_VERSION_MAJOR 1
%define PRODUCT_VERSION_MINOR 1
%define PRODUCT_VERSION_BUILD 0
%define PRODUCT_VERSION_RELEASE 1
%define PRODUCT_WEB_SITE http://www.aqsis.org
#%define PRODUCT_WEB_UPDATE http://download.aqsis.org/stable/source/tar

%if 0%{?fedora_version}
%define PRODUCT_GROUP_DATA Applications/Multimedia
%define PRODUCT_GROUP_DEVELOP Development/Libraries
%define PRODUCT_GROUP_MAIN Applications/Multimedia
%else
%if 0%{?mandriva_version}
%define PRODUCT_GROUP_DATA Graphics
%define PRODUCT_GROUP_DEVELOP Development/C++
%define PRODUCT_GROUP_MAIN Graphics
%else
%if 0%{?suse_version}
%define PRODUCT_GROUP_DATA Productivity/Graphics/Visualization/Other
%define PRODUCT_GROUP_DEVELOP Development/Libraries/C and C++
%define PRODUCT_GROUP_MAIN Productivity/Graphics/Visualization/Other
%else
%define PRODUCT_GROUP_DATA Applications/Multimedia
%define PRODUCT_GROUP_DEVELOP Development/Libraries
%define PRODUCT_GROUP_MAIN Applications/Multimedia
%endif
%endif
%endif

Name:		%{PRODUCT_NAME}
Version:	%{PRODUCT_VERSION_MAJOR}.%{PRODUCT_VERSION_MINOR}.%{PRODUCT_VERSION_BUILD}
Release:	%{PRODUCT_VERSION_RELEASE}%{?dist}
Summary:	Open source RenderMan-compliant 3D rendering solution
Group:		%{PRODUCT_GROUP_MAIN}

License:	GPL
URL:		%{PRODUCT_WEB_SITE}
Source:		%{name}-%{version}.tar.gz
#Source:	http://download.aqsis.org/stable/source/tar/%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)


# Install XSLT processor under Mandriva
%if 0%{?mandriva_version}
BuildRequires:	libxslt-proc
%endif

# Install Python distutils under SUSE 10.1 (and lower)
%if 0%{?suse_version} <= 1010
BuildRequires:	python-devel
%endif

BuildRequires:	bison, flex >= 2.5.4, boost-devel >= 1.32.0, fltk-devel >= 1.1.0, gcc-c++, libjpeg-devel >= 6b, libtiff-devel >= 3.7.1, libxslt-devel, OpenEXR-devel, scons >= 0.96.1, zlib-devel >= 1.1.4
Requires:		fltk >= 1.1.0, libjpeg >= 6b, libtiff >= 3.7.1, OpenEXR, zlib >= 1.1.4


%description
Aqsis is a cross-platform photorealistic 3D rendering solution, based 
on the RenderMan interface standard defined by Pixar Animation Studios.

Focusing on stability and production usage features include constructive 
solid geometry, depth-of-field, extensible shading engine (DSOs), instancing, 
level-of-detail, motion blur, NURBS, procedural plugins, programmable shading, 
subdivision surfaces, subpixel displacements and more.

This package contains a command-line renderer, a shader compiler for shaders 
written using the RenderMan shading language, a texture pre-processor for 
optimizing textures and a RIB processor.

Aqsis is an open source project licensed under the GPL, with some parts under 
the LGPL.


%package devel
Requires:	%{name} = %{version}
Summary:	Development files for the open source RenderMan-compliant Aqsis 3D rendering solution
Group:		%{PRODUCT_GROUP_DEVELOP}


%description devel
Aqsis is a cross-platform photorealistic 3D rendering solution, based 
on the RenderMan interface standard defined by Pixar Animation Studios.

Focusing on stability and production usage features include constructive 
solid geometry, depth-of-field, extensible shading engine (DSOs), instancing, 
level-of-detail, motion blur, NURBS, procedural plugins, programmable shading, 
subdivision surfaces, subpixel displacements and more.

This package contains various developer libraries to enable integration with 
third-party applications.

Aqsis is an open source project licensed under the GPL, with some parts under 
the LGPL.


%package data
Requires:	%{name} = %{version}
Summary:	Example content for the open source RenderMan-compliant Aqsis 3D rendering solution
Group:		%{PRODUCT_GROUP_DATA}


%description data
Aqsis is a cross-platform photorealistic 3D rendering solution, based 
on the RenderMan interface standard defined by Pixar Animation Studios.

Focusing on stability and production usage features include constructive 
solid geometry, depth-of-field, extensible shading engine (DSOs), instancing, 
level-of-detail, motion blur, NURBS, procedural plugins, programmable shading, 
subdivision surfaces, subpixel displacements and more.

This package contains example content, including additional scenes and shaders.

Aqsis is an open source project licensed under the GPL, with some parts under 
the LGPL.


%prep
%setup -q


%build
export CFLAGS=$RPM_OPT_FLAGS
export CXXFLAGS=$RPM_OPT_FLAGS
scons %{?_smp_mflags} destdir=$RPM_BUILD_ROOT \
		install_prefix=%{_prefix} \
		sysconfdir=%{_sysconfdir} \
		no_rpath=true \
		build


%install
rm -rf $RPM_BUILD_ROOT
export CFLAGS=$RPM_OPT_FLAGS
export CXXFLAGS=$RPM_OPT_FLAGS
scons install
chmod a+rx $RPM_BUILD_ROOT%{_bindir}/mpanalyse.py
touch $RPM_BUILD_ROOT%{_bindir}/mpanalyse.pyc
touch $RPM_BUILD_ROOT%{_bindir}/mpanalyse.pyo


%clean
rm -rf $RPM_BUILD_ROOT


%post   -p /sbin/ldconfig
%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING INSTALL README ReleaseNotes
%{_bindir}/aqsis
%{_bindir}/aqsl
%{_bindir}/aqsltell
%{_bindir}/miqser
%{_bindir}/teqser
%{_bindir}/mpanalyse.py
%exclude %{_bindir}/mpanalyse.pyo
%exclude %{_bindir}/mpanalyse.pyc
%{_libdir}/%{name}/*.so
%{_libdir}/libaqsis.so.*
%config %{_sysconfdir}/aqsisrc
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/shaders/


%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/
%{_libdir}/%{name}/*.a
%{_libdir}/libaqsis.so


%files data
%defattr(-,root,root,-)
%{_datadir}/%{name}/content/
%exclude %{_datadir}/%{name}/content/ribs/*/*/*.bat


%changelog
* Thu Dec 14 2006 Tobias Sauerwein <cgtobi@gmail.com> 1.2.0-0.1.a
- More clean-up/optimisation.

* Mon Dec 11 2006 Leon Tony Atkinson <latkinson@aqsis.org> 1.2.0-1a
- Added Fedora (Core 5 tested) and OpenSUSE (10.2 tested) support to SPEC file.
- Cleaned-up/optimised SPEC file.

* Fri Dec 09 2006 Leon Tony Atkinson <latkinson@aqsis.org> 1.2.0-1a
- Added Mandriva (2006 tested) support to SPEC file.

* Wed Nov 22 2006 Tobias Sauerwein <cgtobi@gmail.com> 1.2.0-1a
- Initial RPM/SPEC.
