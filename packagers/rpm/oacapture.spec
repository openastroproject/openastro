Name:           oacapture
Version:        1.2.0
Release:        1
Summary:        planetary capture application
License:        GPL-3
URL:            http://www.openastroproject.org/
Prefix:         %{_prefix}
Provides:       oacapture = %{version}-%{release}
Obsoletes:      oacapture <= 1.1.0
Requires:       libtiff
Requires:       libdc1394
Requires:       systemd
Requires:       cfitsio
Requires:       qt
Requires:       qt-x11
Requires:       xz-libs
Requires:       libjpeg-turbo
Requires:       libftdi
Requires:       libasicamera
Requires:       libuvc
Requires:       libpng
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  systemd-devel
BuildRequires:  libv4l-devel
BuildRequires:  libtiff-devel
BuildRequires:  libdc1394-devel
BuildRequires:  systemd-devel
BuildRequires:  cfitsio-devel
BuildRequires:  xz-devel
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  libftdi-devel
BuildRequires:  libasicamera-devel
BuildRequires:  libuvc-devel
BuildRequires:  libpng-devel
BuildRequires:  qt
BuildRequires:  qt-config
BuildRequires:  qt-devel
BuildRequires:  qt-x11
BuildRequires:  yasm
BuildRequires:  autoconf
BuildRequires:  autoconf-archive
BuildRequires:  libtool
BuildRequires:  desktop-file-utils
Source:         oacapture-%{version}.tar.bz2

%description
An application and associated tools for controlling cameras for planetary
capture and related astronomy imaging

%undefine _hardened_build
%define _unpackaged_files_terminate_build 0

%{?systemd_requires}
BuildRequires: systemd
%prep
%setup -q

%build
%configure
%make_build

%install
%make_install
desktop-file-validate %{buildroot}/%{_datadir}/applications/oacapture.desktop

%files
/usr/bin/oacapture
/lib/udev/rules.d/*
/lib/firmware/qhy/*
/usr/share/applications/oacapture.desktop
/usr/share/icons/hicolor/*/apps/*

%post
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
%systemd_post udev.service

%postun
%systemd_postun udev.service
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
