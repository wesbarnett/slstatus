pkgname=slstatus
pkgver=96d54a4
pkgrel=1
pkgdesc="a suckless and lightweight status monitor for window managers which use WM_NAME as statusbar"
url="https://github.com/wesbarnett/slstatus"
arch=('i686' 'x86_64')
license=('custom')
depends=('dwm')
makedepends=('git')
source=('config.h' git+https://github.com/wesbarnett/slstatus.git)
md5sums=('ffb032afe7f125474438d8ca7ae47563'
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
