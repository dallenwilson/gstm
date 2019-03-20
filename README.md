gstm - Gnome SSH Tunnel Manager
---

#### Gtk3 Edition
This is a Gtk3 port of gstm, based on v1.2 available at: https://sourceforge.net/projects/gstm/

Dependencies on deprecated libgnome libraries have also been removed.

##### Packaging & Releases
- For Gentoo users, an ebuild is available in my layman overlay: https://github.com/dallenwilson/trolltoo

- Ubuntu users (and Mint, Debian and other related distros) can find deb packages in my launchpad ppa: https://launchpad.net/~dallen.wilson/+archive/ubuntu/ppa

- For everyone else, release tarballs are available from Github.

##### Building from Source
If you're building from a release tarball, you'll need pkg-config, gtk3, libxml2, and intltool along with their associated dev packages. The configure script and makefiles are included in the tarball.

Building direct from the git repo, you'll need the above as well as autoconf (>=2.69) and automake. There is a bash script (autogen.sh) to handle the autoconf/automake work and generate the configure script and various makefiles. Run it, then the usual ./configure && make.

##### Bugs and Issues
Bugs can be reported using GitHub's Issues tab. Before opening a new issue, try reproducing the problem after building the lastest source from the master branch. Also, take a look through the closed issues to see if it's been reported before. This is doubly important if you're using packages from the ppa; Those tend to lag a bit behind.
