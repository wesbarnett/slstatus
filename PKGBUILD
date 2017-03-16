pkgname=slstatus
pkgver=b11dd2d
pkgrel=1
pkgdesc="a suckless and lightweight status monitor for window managers which use WM_NAME as statusbar"
url="https://github.com/drkhsh/slstatus"
arch=('i686' 'x86_64')
license=('custom')
depends=('dwm')
source=(git+https://github.com/drkhsh/slstatus.git
    patch.diff
	config.h)
md5sums=('SKIP'
         'c5c919c7b23fb9a412e2f302ad61fc74'
         'aa77d51049d696caf66362166af37326')

pkgver(){
  cd $srcdir/$pkgname
  git rev-parse --short HEAD
}

prepare() {
  cd $srcdir/$pkgname
  cp $srcdir/config.h config.h
  patch -Np1 <"$srcdir/patch.diff"
}

build() {
  cd $srcdir/$pkgname
  make
}

package() {
  cd $srcdir/$pkgname
  make PREFIX=/usr DESTDIR=$pkgdir install
  install -m644 -D LICENSE $pkgdir/usr/share/licenses/$pkgname/LICENSE
  install -m644 -D README.md $pkgdir/usr/share/doc/$pkgname/README
}
