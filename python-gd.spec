Summary:       Python interface to gd library.
Summary(pl):   Interfejs do biblioteki gd dla Pythona.
Name:          python-gd
Version:       1.3 
Release:       2
Copyright:     distributable
Group:         Development/Languages/Python
Group(pl):     Programowanie/Jêzyki/Python
Source0:       gdmodule.c 
Source1:       gd-ref.html
Source2:       python-Makefile.pre.in
Source3:       python-gd-Setup.in
#Icon:          linux-python-paint-icon.gif 
BuildRoot:	   /tmp/%{name}-%{version}-root
Requires:      python >= 1.5
#BuildRequires: python-devel >= 1.5, sed

%description
Python interface to the gd library 1.3

GD module is an interface to the GD library written by Thomas Bouttel.

'It allows your code to quickly draw images complete with lines, arcs,
text, multiple colors, cut and paste from other images, and flood fills,
and write out the result as a .GIF file. This is particularly useful in 
World Wide Web applications, where .GIF is the format used for 
inline images.'

It has been extended in some ways from the original GD library.

%description -l pl
Modu³ GD jest interfejsem do biblioteki GD autorstwa Thomasa Bouttela.

Modu³, czy biblioteka gd wogóle pozwala na szybkie rysowanie orazow 
sk³adaj±cych siê z lini, ³uków, tekstu, fragmentów innych obrazków,
czy wype³nieñ. Ca³o¶c oczywiscie mo¿ê byæ wielokolorowa, a wyprodukowany
obrazek jest zapisywany do pliku .GIF.
Jest to szczególnie przydatne dla zastosowañ zwi±zanych z WWW, gdzie .GIF
jest standardowym formatem u¿ywanym do zapisywania obrazków sk³adaj±cych siê
na stronê WWW. 

Modu³ zosta³ równie¿ rozszerzony w kilku miejscach w stosunku do orginalnej 
biblioteki gd.

%prep
%setup -c -T
cp $RPM_SOURCE_DIR/gdmodule.c .
cp $RPM_SOURCE_DIR/gd-ref.html .
cp $RPM_SOURCE_DIR/python-Makefile.pre.in Makefile.pre.in
cp $RPM_SOURCE_DIR/python-gd-Setup.in Setup.in

%build
make -f Makefile.pre.in boot
make "OPT=$RPM_OPT_FLAGS"

%install
mkdir -p $RPM_BUILD_ROOT%{_libdir}/python1.5/site-packages/
install -m 755 gdmodule.so $RPM_BUILD_ROOT%{_libdir}/python1.5/site-packages/
gzip -9nf gd-ref.html

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc gd-ref.html.gz
%{_libdir}/python1.5/site-packages/gdmodule.so
