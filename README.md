tetris-bsd 2.0
==============

This is an updated version 2.0 of the original tetris-bsd. The code is a fork
of [BSDGames](https://github.com/vattam/BSDGames), I extracted the tetris
code, because I don't need the other games.

As of version 2.0 this package does _no longer_ conflict with the original
bsdgames collection. Use `tetris2` for this updated tetris version or call
`tetris` for the original version.

Installation
------------

Please adjust the `config.mk` to your needs. Especially the `SCOREFILE` and
the paths `BINDIR`/`MANDIR` are important. If everything fits, execute:

```
make
sudo make install
```

### Arch Linux

This package provides a `PKGBUILD` file. You can install this package with
`makepkg -si` on Arch Linux.

Uninstallation
--------------

```
sudo make uninstall
```

Make sure the `config.mk` is the same.

Clean
-----

This removes the built files from the repository. All other files are ignored.

License
=======

See the individual files for copyright information.
