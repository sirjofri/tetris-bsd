tetris-bsd
==========

This is an updated version of the original tetris-bsd. The code is a fork of
[BSDGames](https://github.com/vattam/BSDGames), I extracted the tetris code,
because I don't need the other games.

Installation
------------

Please adjust the `config.mk` to your needs. Especially the `SCOREFILE` and
the paths `BINDIR`/`MANDIR` are important. If everything fits, execute:

```
make
sudo make install
```

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
