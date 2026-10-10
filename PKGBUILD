pkgname=omalaunch
pkgver=0.1.0
pkgrel=1
pkgdesc="SpaceX launch tracker for Omarchy"
arch=('x86_64' 'aarch64')
url="https://github.com/benbrackenbury/omalaunch"
license=('MIT')
depends=('qt6-base' 'qt6-declarative' 'qt6-svg' 'xdg-desktop-portal' 'hicolor-icon-theme')
makedepends=('gcc' 'make' 'qt6-base' 'qt6-declarative')
source=()
md5sums=()

build() {
  qmake6 "$startdir/omalaunch.pro"
  make
}

package() {
  make INSTALL_ROOT="$pkgdir" install
}
