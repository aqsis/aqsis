%{!?rel:    %define rel    %(date +%Y%m%d)}

Name: aqsis

Version: 0.9.2_%{rel}
#Version: 0.9.2

Release: %{distro}
Summary: RenderMan(tm)-compatible renderer
License: GPL
Group: Applications/Graphics
Vendor: none
URL: www.aqsis.com
Packager: cgtobix
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root
Requires: /sbin/ldconfig, libtiff >= 3.5.7, libjpeg >= 6b, zlib >= 1.1.4, glut
AutoReqProv: no


%description
A RenderMan(tm)-compatible renderer

%prep
%setup -q

%build
#./bootstrap
export CFLAGS="-O2" 
export CXXFLAGS="-O29" 
./configure $features

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

* Sun Mar 21 2004 cgtobix <cgtobix@users.sourceforge.net>
- Now building without debug information
* Thu Mar 18 2004 cgtobix <cgtobix@users.sourceforge.net>
- Make it working for RedHat9/Fedora
* Wed Oct 23 2002 Damien Miller <djm@mindrot.org>
- Make an RPM




