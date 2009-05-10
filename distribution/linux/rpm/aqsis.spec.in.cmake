# Title: Aqsis package for Linux (RPM)
# Author: Aqsis Team (packages@aqsis.org)

%define name          ${CMAKE_PROJECT_NAME}
%define version       ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}
%define release       1%{?dist}


Name:           %{name}
Version:        %{version}
Release:        %{release}
License:        GPLv2, LGPLv2
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
BuildRequires:  boost-devel >= 1.34.0
BuildRequires:  cmake >= 2.4.6
BuildRequires:  flex >= 2.5.4
BuildRequires:  gcc-c++
BuildRequires:  libtiff-devel >= 3.7.1
BuildRequires:  libjpeg-devel >= 6
BuildRequires:  bison >= 1.35.0
%if 0%{?suse_version} >= 1020
BuildRequires:  fltk-devel >= 1.1.0
%else
BuildRequires:  fltk-fluid >= 1.1.0
%endif
%if 0%{?mandriva_version}
BuildRequires:  libxslt-proc
%else
BuildRequires:  libxslt
%endif
BuildRequires:  OpenEXR-devel
BuildRequires:  zlib-devel >= 1.1.4
Requires:       boost >= 1.34.0
Requires:       fltk >= 1.1.0
Requires:       xdg-utils >= 1.0.0
Requires:       %{name}-libs = %{version}
%if 0%{?suse_version} >= 1020
Group:          Productivity/Graphics/Visualization/Other
%else
Group:          Applications/Multimedia
%endif
Summary:        Open source RenderMan-compliant 3D rendering solution
Url:            http://www.aqsis.org
Source:         %{name}-%{version}.tar.gz
#Source:         http://downloads.sourceforge.net/aqsis/%{name}-%{version}.tar.gz
#Source:         http://download.aqsis.org/builds/stable/source/tar/%{name}-%{version}.tar.gz

%description
Aqsis is a cross-platform photorealistic 3D rendering solution, based
on the RenderMan interface standard defined by Pixar Animation Studios.

This package contains a command-line renderer, a shader compiler for shaders
written using the RenderMan shading language, a texture pre-processor for
optimizing textures and a RIB processor.


%package devel
Requires:   %{name}-libs = %{version}
%if 0%{?suse_version} >= 1020
Group:      Development/Libraries/C and C++
%else
Group:      Development/Libraries
%endif
Summary:    Development files for Aqsis

%description devel
Aqsis is a cross-platform photorealistic 3D rendering solution, based
on the RenderMan interface standard defined by Pixar Animation Studios.

This package contains various developer files to enable integration with
third-party applications.


%package libs
%if 0%{?suse_version} >= 1020
Group:      Development/Libraries/C and C++
%else
Group:      Development/Libraries
%endif
Summary:    Development libraries for Aqsis

%description libs
Aqsis is a cross-platform photorealistic 3D rendering solution, based
on the RenderMan interface standard defined by Pixar Animation Studios.

This package contains various developer libraries to enable integration with
third-party applications.


%package data
Requires:   %{name} = %{version}
%if 0%{?suse_version} >= 1020
Group:      Productivity/Graphics/Visualization/Other
%else
Group:      Applications/Multimedia
%endif
Summary:    Example content for Aqsis

%description data
Aqsis is a cross-platform photorealistic 3D rendering solution, based
on the RenderMan interface standard defined by Pixar Animation Studios.

This package contains example content, including additional scenes and shaders.


%prep
%setup -q


%build
export CFLAGS=$RPM_OPT_FLAGS
export CXXFLAGS=$RPM_OPT_FLAGS
mkdir build
cd build
%if 0%{?suse_version} <= 1030
cmake -DCMAKE_INSTALL_PREFIX="%{_prefix}" -DLIBDIR="%{_libdir}" -DSYSCONFDIR="%{_sysconfdir}/%{name}" -DDEFAULT_DISPLAYPATH="%{_libdir}/%{name}" ../
%else
cmake -DCMAKE_INSTALL_PREFIX="%{_prefix}" -DLIBDIR="%{_libdir}" -DSYSCONFDIR="%{_sysconfdir}/%{name}" -DDEFAULT_DISPLAYPATH="%{_libdir}/%{name}" ../
%endif
make %{?_smp_mflags}


## The %install section needs revising, moving the manual creation and population of dirs to CMake!
%install
cd build
make DESTDIR="$RPM_BUILD_ROOT" install


%post
/sbin/ldconfig
xdg-icon-resource install --novendor --size 192 %{_datadir}/pixmaps/aqsis.png aqsis
xdg-icon-resource install --novendor --context mimetypes --size 192 %{_datadir}/pixmaps/aqsis-doc.png application-x-slx
xdg-icon-resource install --novendor --context mimetypes --size 192 %{_datadir}/pixmaps/aqsis-doc.png model-x-rib
xdg-icon-resource install --novendor --context mimetypes --size 192 %{_datadir}/pixmaps/aqsis-doc.png model-x-rib-gzip
xdg-icon-resource install --novendor --context mimetypes --size 192 %{_datadir}/pixmaps/aqsis-doc.png text-x-sl
xdg-desktop-menu install --novendor %{_datadir}/applications/aqsis.desktop
xdg-desktop-menu install --novendor %{_datadir}/applications/aqsl.desktop
xdg-desktop-menu install --novendor %{_datadir}/applications/aqsltell.desktop
xdg-desktop-menu install --novendor %{_datadir}/applications/eqsl.desktop
xdg-desktop-menu install --novendor %{_datadir}/applications/piqsl.desktop
xdg-mime install --novendor %{_datadir}/mime/packages/aqsis.xml


%postun
/sbin/ldconfig


%preun
xdg-icon-resource uninstall --size 192 aqsis
xdg-icon-resource uninstall --context mimetypes --size 192 application-x-slx
xdg-icon-resource uninstall --context mimetypes --size 192 model-x-rib
xdg-icon-resource uninstall --context mimetypes --size 192 model-x-rib-gzip
xdg-icon-resource uninstall --context mimetypes --size 192 text-x-sl
xdg-desktop-menu uninstall %{_datadir}/applications/aqsis.desktop
xdg-desktop-menu uninstall %{_datadir}/applications/aqsl.desktop
xdg-desktop-menu uninstall %{_datadir}/applications/aqsltell.desktop
xdg-desktop-menu uninstall %{_datadir}/applications/eqsl.desktop
xdg-desktop-menu uninstall %{_datadir}/applications/piqsl.desktop
xdg-mime uninstall %{_datadir}/mime/packages/aqsis.xml


%clean
rm -rf "$RPM_BUILD_ROOT"


%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog.txt COPYING README ReleaseNotes
%{_bindir}/aqsis
%{_bindir}/aqsl
%{_bindir}/aqsltell
%{_bindir}/eqsl
%{_bindir}/miqser
%{_bindir}/piqsl
%{_bindir}/teqser
%dir %{_sysconfdir}/%{name}
%config %{_sysconfdir}/%{name}/aqsisrc
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/scripts/
%{_datadir}/%{name}/shaders/
%{_datadir}/applications/
%{_datadir}/mime/packages/
%{_datadir}/pixmaps/


%files devel
%defattr(-,root,root)
%{_includedir}/%{name}/


%files libs
%defattr(-,root,root)
%{_libdir}/%{name}/
%{_libdir}/*.so.*
%{_libdir}/*.so
#%{_libdir}/libri2rib.s*      # Licensed under LGPLv2


%files data
%defattr(-,root,root)
%{_datadir}/%{name}/content/


%changelog
* Sun Jun 29 2008 Leon Tony Atkinson < latkinson [at] aqsis [dot] org > - 1.3.0.alpha
- New SPEC for latest Aqsis release, with some parts based on original SPEC by Nicolas Chauvet & Tobias Sauerwein
