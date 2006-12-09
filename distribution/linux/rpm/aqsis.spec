# Title: Aqsis Package for Linux (RPM)
# Author: Aqsis Team (packages@aqsis.org)
# Info: 
# Other: 1. To make updates easier, all message strings have been placed within the top 10-80 lines of this file.
#        2. To build using the 'Official' tarball comment line 18 and uncomment line 19.


Name:           aqsis
Version:        1.1.0
Release:        1%{?dist}
Summary:        Open source RenderMan-compliant 3D rendering solution

Group:          Applications/Multimedia
License:        GPL
URL:            http://www.aqsis.org
Vendor:			Aqsis Team
Packager:		Aqsis Team <packages@aqsis.org>
Source:         %{name}-%{version}.tar.gz
#Source:        http://download.aqsis.org/stable/source/tar/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%if 0%{?mandriva_version}
BuildRequires:	bison, flex >= 2.5.4, OpenEXR-devel, boost-devel >= 1.32.0, fltk-devel >= 1.1.0, libjpeg-devel >= 6b, libtiff3-devel >= 3.5.7, libxslt-proc, scons >= 0.96.1, zlib-devel >= 1.1.4
%else
BuildRequires:	bison, flex >= 2.5.4, OpenEXR-devel, boost-devel >= 1.32.0, fltk-devel >= 1.1.0, libjpeg-devel >= 6b, libtiff3-devel >= 3.5.7, libxslt, scons >= 0.96.1, zlib-devel >= 1.1.4
%endif
Requires:		libtiff >= 3.5.7, libjpeg >= 6b, zlib >= 1.1.4, fltk >= 1.1.0, OpenEXR


%description
Aqsis is a cross-platform photorealistic 3D rendering solution, based on the RenderMan interface standard defined by Pixar Animation Studios.

Focusing on stability and production usage features include constructive solid geometry, depth-of-field, extensible shading engine (DSOs), instancing, level-of-detail, motion blur, NURBS, procedural plugins, programmable shading, subdivision surfaces, subpixel displacements and more.

This package contains a command-line renderer, a shader compiler for shaders written using the RenderMan shading language, a texture pre-processor for optimizing textures and a RIB processor.

Aqsis is an open source project licensed under the GPL, with some parts under the LGPL.


%package devel
Requires:		%{name} = %{version}
Summary:        Development files for the open source RenderMan-compliant Aqsis 3D rendering solution
Group:          Applications/Multimedia


%description devel
Aqsis is a cross-platform photorealistic 3D rendering solution, based on the RenderMan interface standard defined by Pixar Animation Studios

Focusing on stability and production usage features include constructive solid geometry, depth-of-field, extensible shading engine (DSOs), instancing, level-of-detail, motion blur, NURBS, procedural plugins, programmable shading, subdivision surfaces, subpixel displacements and more.

This package contains various developer libraries to enable integration with third-party applications.

Aqsis is an open source project licensed under the GPL, with some parts under the LGPL.


%package data
Requires:		%{name} = %{version}
Summary:        Example content for the open source RenderMan-compliant Aqsis 3D rendering solution
Group:          Applications/Multimedia


%description data
Aqsis is a cross-platform photorealistic 3D rendering solution, based on the RenderMan interface standard defined by Pixar Animation Studios.

Focusing on stability and production usage features include constructive solid geometry, depth-of-field, extensible shading engine (DSOs), instancing, level-of-detail, motion blur, NURBS, procedural plugins, programmable shading, subdivision surfaces, subpixel displacements and more.

This package contains example content, including additional scenes and shaders.

Aqsis is an open source project licensed under the GPL, with some parts under the LGPL.


%prep
%setup -q


%build
scons build destdir=$RPM_BUILD_ROOT \
	install_prefix=%{_prefix} \
	sysconfdir=%{_sysconfdir}


%install
rm -rf $RPM_BUILD_ROOT
scons install


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING INSTALL README ReleaseNotes
%attr(755,root,root)%{_bindir}/aqsis
%attr(755,root,root)%{_bindir}/aqsl
%attr(755,root,root)%{_bindir}/aqsltell
%attr(755,root,root)%{_bindir}/miqser
%attr(755,root,root)%{_bindir}/teqser
%attr(755,root,root)%{_bindir}/mpanalyse.py
%{_libdir}/%{name}/*.so
%{_libdir}/libaqsis.so*
%{_sysconfdir}/aqsisrc
%{_datadir}/%{name}/shaders/


%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/
%{_libdir}/%{name}/*.a


%files data
%defattr(-,root,root,-)
%{_datadir}/%{name}/content/ribs/features/layeredshaders/
%{_datadir}/%{name}/content/ribs/scenes/vase/
%{_datadir}/%{name}/content/shaders/displacement/
%{_datadir}/%{name}/content/shaders/light/


%changelog
* Fri Dec 09 2006 - latkinson@aqsis.org
- Added Mandriva (2006 tested) support to SPEC file.
* Wed Nov 22 2006 - cgtobi@gmail.com
- Initial RPM/SPEC.

