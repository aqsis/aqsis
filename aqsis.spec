Name: aqsis
Version: 0.7.0
Release: 1
Summary: RenderMan(tm)-compatible renderer
License: GPL
Group: Applications/Multimedia
BuildRoot: %{_tmppath}/%{name}-root
Source: %{name}-%{version}.tar.gz
BuildPreReq: libargparse

%description
A RenderMan(tm)-compatible renderer

%prep
%setup -q

%build
./bootstrap
%configure
make

%clean
rm -fr $RPM_BUILD_ROOT

%install
rm -fr $RPM_BUILD_ROOT
export LD_LIBRARY_PATH=$RPM_BUILD_ROOT/usr/lib 
%makeinstall
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/aqsis
cat << EOF > $RPM_BUILD_ROOT/%{_sysconfdir}/aqsis/ddmsock.ini

file                    /usr/bin/filebuffer
framebuffer             /usr/bin/aqsis_framebuffer_glut
zfile                   /usr/bin/shadowmap
zframebuffer            /usr/bin/aqsis_framebuffer_glut_z
shadow                  /usr/bin/shadowmap
tiff                    /usr/bin/filebuffer
EOF

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc ChangeLog COPYING AUTHORS README ribfiles
%{_bindir}/*
%{_libdir}/*
%dir %{_datadir}/shaders
%{_datadir}/shaders/*
%{_includedir}/*
%dir %{_sysconfdir}/aqsis
%{_sysconfdir}/aqsis/*

%changelog
* Wed Oct 23 2002 Damien Miller <djm@mindrot.org>
- Make an RPM
