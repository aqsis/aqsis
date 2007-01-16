# Title: Aqsis Package for Linux (RPM)
# Author: Aqsis Team (packages@aqsis.org)
# Info: 
# Other: 1. To make updates easier, all message strings have been placed within the top 10-40 lines of this file.
#        2. To build using the 'Official' tarball comment line 24 and uncomment line 25.


%define PRODUCT_NAME aqsis
%define PRODUCT_VERSION_MAJOR 1
%define PRODUCT_VERSION_MINOR 2
%define PRODUCT_VERSION_BUILD 0
%define PRODUCT_VERSION_RELEASE 0.1
%define PRODUCT_WEB_SITE http://www.aqsis.org
%define PRODUCT_WEB_UPDATE http://download.aqsis.org/stable/source/tar

Name:		%{PRODUCT_NAME}
Version:	%{PRODUCT_VERSION_MAJOR}.%{PRODUCT_VERSION_MINOR}.%{PRODUCT_VERSION_BUILD}
Release:	%{PRODUCT_VERSION_RELEASE}%{?dist}
Summary:	Open source RenderMan-compliant 3D rendering solution
Group:		Applications/Multimedia

License:	GPL
URL:		%{PRODUCT_WEB_SITE}
Source:		%{name}-%{version}-%{release}.tar.gz
#Source:		%{PRODUCT_WEB_UPDATE}/%{name}-%{version}-%{release}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)


BuildRequires:	bison, boost-devel >= 1.32.0, flex >= 2.5.4, fltk-devel >= 1.1.0,
BuildRequires:	gcc-c++, libjpeg-devel >= 6b, libtiff-devel >= 3.7.1, libxslt,
BuildRequires:	OpenEXR-devel, scons >= 0.96.1, zlib-devel >= 1.1.4

Requires:		fltk >= 1.1.0, libjpeg >= 6b, libtiff >= 3.7.1, OpenEXR, zlib >= 1.1.4


%description
Aqsis is a cross-platform photorealistic 3D rendering solution, based 
on the RenderMan interface standard defined by Pixar Animation Studios.

This package contains a command-line renderer, a shader compiler for shaders 
written using the RenderMan shading language, a texture pre-processor for 
optimizing textures and a RIB processor.


%package devel
Requires:	%{name} = %{version}-%{release}
Summary:	Development files for the open source RenderMan-compliant Aqsis 3D rendering solution
Group:		Development/Libraries


%description devel
Aqsis is a cross-platform photorealistic 3D rendering solution, based 
on the RenderMan interface standard defined by Pixar Animation Studios.

This package contains various developer libraries to enable integration with 
third-party applications.


%package data
Requires:	%{name} = %{version}-%{release}
Summary:	Example content for the open source RenderMan-compliant Aqsis 3D rendering solution
Group:		Applications/Multimedia


%description data
Aqsis is a cross-platform photorealistic 3D rendering solution, based 
on the RenderMan interface standard defined by Pixar Animation Studios.

This package contains example content, including additional scenes and shaders.


%prep
%setup -q


%build
export CFLAGS=$RPM_OPT_FLAGS
export CXXFLAGS=$RPM_OPT_FLAGS
scons %{?_smp_mflags} \
		destdir=$RPM_BUILD_ROOT \
		install_prefix=%{_prefix} \
		sysconfdir=%{_sysconfdir} \
		no_rpath=true \
		build


%install
rm -rf $RPM_BUILD_ROOT
scons install
chmod a+rx $RPM_BUILD_ROOT%{_datadir}/%{name}/scripts/mpanalyse.py
chmod a+rx $RPM_BUILD_ROOT%{_datadir}/%{name}/content/ribs/scenes/vase/render.sh
chmod a+rx $RPM_BUILD_ROOT%{_datadir}/%{name}/content/ribs/features/layeredshaders/render.sh


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
%{_libdir}/%{name}/
%{_libdir}/*.so.*
%config %{_sysconfdir}/aqsisrc
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/shaders/
%{_datadir}/%{name}/scripts/


%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/
%{_libdir}/*.so


%files data
%defattr(-,root,root,-)
%{_datadir}/%{name}/content/
%exclude %{_datadir}/%{name}/content/ribs/*/*/*.bat


%changelog
* Thu Jan 11 2007 Leon Tony Atkinson <latkinson@aqsis.org> 1.2.0-0.1.alpha2
- Fedora-specific clean-up/optimisation and release update

* Sat Dec 23 2006 Tobias Sauerwein <tsauerwein@aqsis.org> 1.2.0-0.3.alpha1
- Tuning to meet Fedora-Extras requirements

* Thu Dec 21 2006 Tobias Sauerwein <tsauerwein@aqsis.org> 1.2.0-0.2.alpha1
- Fedora-specific clean-up/optimisation

* Thu Dec 14 2006 Tobias Sauerwein <tsauerwein@aqsis.org> 1.2.0-0.1.alpha1
- Clean-up/optimisation

* Wed Nov 22 2006 Tobias Sauerwein <tsauerwein@aqsis.org> 1.1.0-1
- Initial RPM/SPEC.
