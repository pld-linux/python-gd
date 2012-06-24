%define pp_subname gd
Summary:	Python interface to gd library.
Summary(pl):	Interfejs do biblioteki gd dla Pythona
Name:		python-%{pp_subname}
Version:	1.3
# NOTE: module version is 0.22, not 1.3
Release:	2
License:	distributable
Group:		Development/Languages/Python
Source0:	gdmodule.c
Source1:	gd-ref.html
Source2:	python-Makefile.pre.in
Source3:	python-gd-Setup.in
#Source0:	http://newcenturycomputers.net/cgi-bin/download.py/projects/downloads/gdmodule-0.25.tar.gz
#Icon:		linux-python-paint-icon.gif
URL:		http://newcenturycomputers.net/projects/gdmodule.html
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)
Requires:	python >= 1.5
#BuildRequires:	python-devel >= 1.5, sed

%description
Python interface to the gd library 1.3

GD module is an interface to the GD library written by Thomas Bouttel.

'It allows your code to quickly draw images complete with lines, arcs,
text, multiple colors, cut and paste from other images, and flood
fills, and write out the result as a .GIF file. This is particularly
useful in World Wide Web applications, where .GIF is the format used
for inline images.'

It has been extended in some ways from the original GD library.

%description -l pl
Modu� GD jest interfejsem do biblioteki GD autorstwa Thomasa Bouttela.

Modu�, czy biblioteka gd wog�le pozwala na szybkie rysowanie orazow
sk�adaj�cych si� z lini, �uk�w, tekstu, fragment�w innych obrazk�w,
czy wype�nie�. Ca�o�c oczywiscie mo�� by� wielokolorowa, a
wyprodukowany obrazek jest zapisywany do pliku .GIF. Jest to
szczeg�lnie przydatne dla zastosowa� zwi�zanych z WWW, gdzie .GIF jest
standardowym formatem u�ywanym do zapisywania obrazk�w sk�adaj�cych
si� na stron� WWW.

Modu� zosta� r�wnie� rozszerzony w kilku miejscach w stosunku do
oryginalnej biblioteki gd.

%prep
%setup -q -c -T
cp -f %{SOURCE0} .
cp -f %{SOURCE1} .
cp -f %{SOURCE2} Makefile.pre.in
cp -f %{SOURCE3} Setup.in

%build
%{__make} -f Makefile.pre.in boot
%{__make} OPT="%{rpmcflags}"

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_libdir}/python1.5/site-packages/

install gdmodule.so $RPM_BUILD_ROOT%{_libdir}/python1.5/site-packages/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%doc gd-ref.html
%{_libdir}/python1.5/site-packages/gdmodule.so
