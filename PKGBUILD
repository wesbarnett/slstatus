pkgname=slstatus
pkgver=e624621
pkgrel=1
pkgdesc="a suckless and lightweight status monitor for window managers which use WM_NAME as statusbar"
url="https://github.com/wesbarnett/slstatus"
arch=('i686' 'x86_64')
license=('custom')
depends=('dwm')
makedepends=('git')
source=('config.h' git+https://github.com/wesbarnett/slstatus.git)
md5sums=('479fb41b81852d73c65bb955c92d62d6'
         'SKIP')

pkgver() {
  cd $srcdir/$pkgname
  git describe --always
}

prepare() {
  cp config.h $srcdir/$pkgname/
}

build() {
  cd $srcdir/$pkgname
  make
}

package() {
  cd $srcdir/$pkgname
  make PREFIX=/usr DESTDIR=$pkgdir install
  install -m644 -D LICENSE $pkgdir/usr/share/licenses/$pkgname/LICENSE
}
