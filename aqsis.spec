Name: aqsis
Version: 0.9.1
Release: cvs
Summary: RenderMan(tm)-compatible renderer
License: GPL
Group: Applications/Graphics
Vendor: none
URL: www.aqsis.com
Packager: cgtobix
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root
Requires: /sbin/ldconfig
AutoReqProv: no

%description
A RenderMan(tm)-compatible renderer

%prep
%setup -q

%build
#./bootstrap
%configure
make
#make check

%install
rm -fr %{buildroot}
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

%clean
rm -fr %{buildroot}

%files
%defattr(-,root,root)
%doc ChangeLog COPYING AUTHORS README ribfiles
%{_bindir}/*
%{_libdir}/*
%dir %{_datadir}/aqsis/shaders
%{_datadir}/aqsis/shaders/*
%{_includedir}/*
%dir %{_sysconfdir}/
%{_sysconfdir}/*

%changelog

* Wed Oct 23 2002 Damien Miller <djm@mindrot.org>
- Make an RPM
* Thu Mar 18 2004 cgtobix
- RPM from aqsis cvs 0.9.1



