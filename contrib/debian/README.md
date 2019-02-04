
Debian
====================
This directory contains files used to package growd/grow-qt
for Debian-based Linux systems. If you compile growd/grow-qt yourself, there are some useful files here.

## grow: URI support ##


grow-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install grow-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your grow-qt binary to `/usr/bin`
and the `../../share/pixmaps/pivx128.png` to `/usr/share/pixmaps`

grow-qt.protocol (KDE)

