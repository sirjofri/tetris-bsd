# Maintainer: Joel Fridolin Meyer <sirjofri@sirjofri.de>
pkgname=tetris-bsd
pkgver=1.17
pkgrel=1
pkgdesc="Tetris BSD updated version"
arch=('x86_64')
url="https://github.com/sirjofri/tetris-bsd/"
license=('BSD')
depends=('ncurses')
conflicts=('bsdgames')
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
