%define		module gd
Summary:	Python interface to GD library
Summary(pl):	Interfejs do biblioteki GD dla Pythona
Name:		python-%{module}
Version:	0.56
Release:	1
Epoch:		1
License:	BSD-like
Group:		Development/Languages/Python
Source0:	http://newcenturycomputers.net/projects/download.cgi/gdmodule-%{version}.tar.gz
# Source0-md5:	6a6db28a089d4caf5a921cd266a62b3d
Patch0:		%{name}-lib64.patch
URL:		http://newcenturycomputers.net/projects/gdmodule.html
BuildRequires:	gd-devel >= 2.0.23
BuildRequires:	python-devel >= 1.5
BuildRequires:	python-modules >= 1.5
%pyrequires_eq	python-modules
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
GD module is an interface to the GD library written by Thomas
Bouttel.

It allows your code to quickly draw images complete with lines,
arcs, text, multiple colors, cut and paste from other images,
and flood fills, and write out the result as a PNG or JPEG file.
This is particularly useful in World Wide Web applications,
where PNG and JPEG are two of the formats accepted for inline
images by most browsers.

It has been extended in some ways from the original GD library.

%description -l pl
Modu³ GD jest interfejsem do biblioteki GD autorstwa Thomasa
Bouttela.

Modu³ pozwala na szybkie rysowanie obrazów sk³adaj±cych siê z
linii, ³uków, tekstu, ró¿nych kolorów, fragmentów innych obrazków,
czy wype³nieñ. Wyprodukowany obrazek jest zapisywany do pliku PNG
lub JPEG. Jest to szczególnie przydatne w aplikacjach WWW, gdzie
PNG i JPEG s± dwoma formatami obrazów sk³adowych akceptowanymi
przez wiêkszo¶æ przegl±darek.

Modu³ zosta³ równie¿ rozszerzony w kilku miejscach w stosunku do
oryginalnej biblioteki GD.

%prep
%setup -q -n gdmodule-%{version}
%if "%{_lib}" == "lib64"
%patch0 -p1
%endif

%build
CFLAGS="%{rpmcflags}" \
python Setup.py build

%install
rm -rf $RPM_BUILD_ROOT

python Setup.py install \
	--optimize=2 \
	--root=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%doc README gd-ref.html
%{py_sitedir}/gd.py[co]
%attr(755,root,root) %{py_sitedir}/_gd.so
