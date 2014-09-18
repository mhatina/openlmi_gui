ABOUT
===========
Keep in mind the program is still in alpha stage, so bugs and install issues are still very common.

LATEST VERSION
===============

Latest sources of the software can be found at: [openlmi_gui][]

INSTALLATION
============

LMI Command Center has several dependencies:

- C++ compiler, which supports C++98 standard
- tog-pegasus (2.12.0)
- tog-pegasus-libs (2.12.0)
- tog-pegasus-devel (2.12.0)
- boost-thread (1.54.0)
- boost-devel (1.54.0)
- openslp (1.2.1)
- gnome-keyring (3.10.0)
- build tools (autoconf, automake, make, qmake, ...)

To install the module, follow these steps:

    $ cd src/logger
    $ qmake
    $ make
    # make install
    $ cd -
    $ qmake CONFIG+=manual_install
    $ make
    # make install

BUG REPORTS
===========

Report bugs to [mhatina@redhat.com](mailto:mhatina@redhat.com) or [openlmi issues][].

[openlmi_gui]: https://github.com/mhatina/openlmi_gui "openlmi_gui repository on github"
[openlmi issues]: https://github.com/mhatina/openlmi/issues "Report a bug"
[LMI Meta-Command]: https://git.fedorahosted.org/git/openlmi-tools.git
