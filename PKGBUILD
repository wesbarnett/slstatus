pkgname=slstatus
pkgver=ac1a57e
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
         '9e286e804a608db4c93f4cfde401f383'
         '998aafdc42b897e589314115783f3d51')

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
