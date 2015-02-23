/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

#ifndef PASSWORDSTORAGE_H
#define PASSWORDSTORAGE_H

#include "lmi_string.h"

#include <QMutex>

#define OPENLMI_KEYRING_DEFAULT "openlmi"

#ifdef HAVE_LIBSECRET
#   undef signals
#       include <libsecret-1/libsecret/secret.h>
#   define signals Q_SIGNALS

#   include <glib/gerror.h>

    typedef GError * StorageResult;

    const SecretSchema *get_schema();
#   define PASSWD_SCHEMA  get_schema()
#elif defined HAVE_LIBKEYRING
#   include <gnome-keyring-1/gnome-keyring.h>

    typedef GnomeKeyringResult StorageResult;
#elif !defined HAVE_LIBSECRET && !defined HAVE_LIBKEYRING
#   error Password storage library required. (libsecret || libgnome-keyring)
    typedef void *StorageResult;
#endif


typedef enum {
    PASSWD_OK = 0,
    PASSWD_NO_MATCH = 1,
    PASSWD_ERR = -1
} passwd_state;


class PasswordStorage
{
private:
#ifdef HAVE_LIBSECRET
    SecretCollection *m_collection;

    void resetErr();
#endif

    QMutex *m_mutex;
    StorageResult m_res;

public:
    PasswordStorage(QMutex *kernel_mutex);
    ~PasswordStorage();

    bool createStorage();
    bool resetStorage();
    bool setUserData(String &system, String &username, String &password);
    passwd_state getUserData(String &system, String &username, String &password);
    bool deletePassword(String &system);

    String errString();
};

#endif // PASSWORDSTORAGE_H
