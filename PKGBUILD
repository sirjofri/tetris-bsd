# Maintainer: Joel Fridolin Meyer <sirjofri@sirjofri.de>
pkgname=tetris2-bsd
pkgver=2.0
pkgrel=1
pkgdesc="Tetris BSD updated version 2"
arch=('x86_64')
url="https://github.com/sirjofri/tetris-bsd/"
license=('BSD')
depends=('ncurses')
install='tetris.install'

build() {
	cd ..
	make
}

package() {
	cd ..
	make PREFIX="/usr" PACKAGE="$pkgdir" package
	install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
