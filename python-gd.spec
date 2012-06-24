%define		module	gd
Summary:	Python interface to GD library
Summary(pl):	Interfejs do biblioteki GD dla Pythona
Name:		python-%{module}
Version:	0.52
Release:	1
Epoch:		1
License:	BSD-like
Group:		Development/Languages/Python
Source0:	http://newcenturycomputers.net/projects/download.cgi/gdmodule-%{version}.tar.gz
# Source0-md5:	7322e7cc82c21765989053fd9a55e1ac
URL:		http://newcenturycomputers.net/projects/gdmodule.html
BuildRequires:	gd-devel >= 1.8.3
BuildRequires:	python-devel >= 1.5
BuildRequires:	python-modules >= 1.5
Requires:	python >= 1.5
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
Modu� GD jest interfejsem do biblioteki GD autorstwa Thomasa
Bouttela.

Modu� pozwala na szybkie rysowanie obraz�w sk�adaj�cych si� z
linii, �uk�w, tekstu, r�nych kolor�w, fragment�w innych obrazk�w,
czy wype�nie�. Wyprodukowany obrazek jest zapisywany do pliku PNG
lub JPEG. Jest to szczeg�lnie przydatne w aplikacjach WWW, gdzie
PNG i JPEG s� dwoma formatami obraz�w sk�adowych akceptowanymi
przez wi�kszo�� przegl�darek.

Modu� zosta� r�wnie� rozszerzony w kilku miejscach w stosunku do
oryginalnej biblioteki GD.

%prep
%setup -q -n gdmodule-%{version}

%build
CFLAGS="%{rpmcflags}" %__python Setup.py build

%install
rm -rf $RPM_BUILD_ROOT
%__python Setup.py install \
	--optimize=2 \
	--root=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%doc README gd-ref.html
%{py_sitedir}/gd.py[c,o]
%attr(755,root,root) %{py_sitedir}/_gd.so
