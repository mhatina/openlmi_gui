QMAKE_CXXFLAGS  +=      -ansi -pedantic -Wall -Wextra
DEFINES         +=      PEGASUS_PLATFORM_LINUX_X86_64_GNU
CONFIG          +=      link_pkgconfig
PKGCONFIG       +=      glib-2.0

isEmpty(PREFIX) {
    PREFIX      =       /usr
}
isEmpty(LIB_PATH) {
    LIB_PATH    =       /lib64
}
isEmpty(DOC) {
    DOC         =       /usr/share/doc/lmicc
}
isEmpty(QT_SINGLE_APP) {
    QT_SINGLE_APP =     /usr$$LIB_PATH/qt4/mkspecs/features/qtsingleapplication.prf
}

#exists(/usr$$LIB_PATH/libsecret*) {
#    DEFINES     +=      "HAVE_LIBSECRET"
#    LIBS        +=      -lsecret-1
#    PKGCONFIG   +=      libsecret-1
#}
!contains(DEFINES, HAVE_LIBSECRET) {
    exists(/usr$$LIB_PATH/libgnome-keyring*) {
        DEFINES +=      "HAVE_LIBKEYRING"
        LIBS    +=      -lgnome-keyring
    }
}
