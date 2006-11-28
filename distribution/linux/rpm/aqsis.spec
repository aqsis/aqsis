Name:           aqsis
Version:        1.1.0
Release:        1%{?dist}
Summary:        Open source RenderMan-compliant 3D rendering solution


Group:          Applications/Graphics
License:        GPL
URL:            http://www.aqsis.org
Source0:        %{name}-%{version}.tar.gz
#Source1:        http://download.aqsis.org/stable/source/tar/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  /sbin/ldconfig, libtiff >= 3.5.7, libjpeg >= 6b, zlib >= 1.1.4, fltk
Requires:       libtiff >= 3.5.7, libjpeg >= 6b, zlib >= 1.1.4, fltk


%description
Aqsis is a cross-platform photorealistic 3D rendering solution, based on the
RenderMan interface standard defined by Pixar Animation Studios.

Focusing on stability and production usage features include constructive solid
geometry, depth-of-field, extensible shading engine (DSOs), instancing,
level-of-detail, motion blur, NURBS, procedural plugins, programmable shading,
subdivision surfaces, subpixel displacements and more.

This package contains a command-line renderer, a shader compiler for shaders
written using the RenderMan shading language, a texture pre-processor for
optimizing textures and a RIB processor.

Aqsis is an open source project licensed under the GPL, with some parts under
the LGPL.


%package devel
Requires:		%{name} = %{version}
Summary:        Development files for the open source RenderMan-compliant Aqsis 3D rendering solution
Group:          Applications/Graphics


%description devel
Aqsis is a cross-platform photorealistic 3D rendering solution, based on the
RenderMan interface standard defined by Pixar Animation Studios

Focusing on stability and production usage features include constructive solid
geometry, depth-of-field, extensible shading engine (DSOs), instancing,
level-of-detail, motion blur, NURBS, procedural plugins, programmable shading,
subdivision surfaces, subpixel displacements and more.

This package contains various developer libraries to enable integration with
third-party applications.

Aqsis is an open source project licensed under the GPL, with some parts under
the LGPL.


%package data
Requires:		%{name} = %{version}
Summary:        Example content for the open source RenderMan-compliant Aqsis 3D rendering solution
Group:          Applications/Multimedia


%description data
Aqsis is a cross-platform photorealistic 3D rendering solution, based on the
RenderMan interface standard defined by Pixar Animation Studios.

Focusing on stability and production usage features include constructive solid
geometry, depth-of-field, extensible shading engine (DSOs), instancing, 
level-of-detail, motion blur, NURBS, procedural plugins, programmable shading,
subdivision surfaces, subpixel displacements and more.

This package contains example content, including additional scenes and shaders

Aqsis is an open source project licensed under the GPL, with some parts under
the LGPL.


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
%attr(755,root,root)%{_bindir}/mpanalyse.p*
%{_libdir}/%{name}/*.so
%{_libdir}/libaqsis.so
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

* Wed Nov 22 2006 Tobias Sauerwein <cgtobi@gmail.com> - 1.1.0-1
- Make an RPM

