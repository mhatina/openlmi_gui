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

#include "logger.h"
#include "passwordstorage.h"

PasswordStorage::PasswordStorage(QMutex *kernel_mutex) :
    m_mutex(kernel_mutex)
{
#ifdef HAVE_LIBSECRET
    m_collection = NULL;
    m_res = NULL;
#endif
}


#ifdef HAVE_LIBSECRET
PasswordStorage::~PasswordStorage()
{
    if (m_collection) {
        g_object_unref(m_collection);
    }
}

const SecretSchema *get_schema()
{
    static const SecretSchema passwd_schema = {
        "com.redhat.openlmi.lmicc.passwd_schema", SECRET_SCHEMA_NONE,
        {
            {"system", SECRET_SCHEMA_ATTRIBUTE_STRING},
            {"username", SECRET_SCHEMA_ATTRIBUTE_STRING},
            {"NULL", SECRET_SCHEMA_ATTRIBUTE_STRING},
        }
    };
    return &passwd_schema;
}

void PasswordStorage::resetErr()
{
    if (m_res) {
        g_error_free(m_res);
        m_res = NULL;
    }
}

bool PasswordStorage::unlock()
{
    resetErr();

    SecretService *service = secret_service_get_sync(
                                 SECRET_SERVICE_LOAD_COLLECTIONS,
                                 NULL,
                                 &m_res);

    if (m_res != NULL) {
        g_object_unref(service);
        return false;
    }

    GList *list = NULL;
    list = g_list_append(list, m_collection);
    if (!secret_service_unlock_sync(service, list, NULL, NULL, &m_res)) {
        g_object_unref(service);
        g_list_free(list);
        return false;
    }

    g_object_unref(service);
    g_list_free(list);

    return true;
}

bool PasswordStorage::createStorage()
{
    if (m_collection) {
        g_object_unref(m_collection);
    }
    resetErr();
    SecretService *service = secret_service_get_sync(
                                 SECRET_SERVICE_LOAD_COLLECTIONS,
                                 NULL,
                                 &m_res);

    if (m_res != NULL) {
        g_object_unref(service);
        return false;
    }

    if (secret_service_load_collections_sync(service, NULL, NULL) == FALSE) {
        g_object_unref(service);
        return false;
    }

    GList *collections = secret_service_get_collections(service);
    SecretCollection *item = NULL;

    bool collection_exists = false;
    for (GList *list = collections; list && (item = (SecretCollection *) list->data, TRUE); list = list->next) {
        gchar *label = secret_collection_get_label(item);
        if (strcmp(label, OPENLMI_KEYRING_DEFAULT) == 0) {
            collection_exists = true;
            m_collection = item;
            g_object_ref(m_collection);

            g_free(label);
            break;
        }
        g_free(label);
    }

    if (!collection_exists) {
        m_collection = secret_collection_create_sync(
                           service,
                           OPENLMI_KEYRING_DEFAULT,
                           "",
                           SECRET_COLLECTION_CREATE_NONE,
                           NULL,
                           &m_res);

        if (m_collection == NULL) {
            g_list_free_full(collections, g_object_unref);
            g_object_unref(service);
            return false;
        }
    }

    g_object_unref(service);
    g_list_free_full(collections, g_object_unref);

    return true;
}

bool PasswordStorage::resetStorage()
{
    resetErr();
    if (secret_collection_delete_sync(m_collection, NULL, &m_res)) {
        g_object_unref(m_collection);
        m_collection = NULL;
        return createStorage();
    }

    return false;
}

bool PasswordStorage::setUserData(String &system, String &username, String &password)
{
    resetErr();
    GHashTable *attributes = secret_attributes_build(PASSWD_SCHEMA,
                             "system", system.asConstChar(),
                             "username", username.asConstChar(),
                             NULL);

    SecretValue *secret = secret_value_new(password, -1, "text/plain");

    SecretItem *item =
        secret_item_create_sync(
            m_collection,
            PASSWD_SCHEMA,
            attributes,
            OPENLMI_KEYRING_DEFAULT,
            secret,
            SECRET_ITEM_CREATE_REPLACE,
            NULL,
            &m_res);

    g_hash_table_destroy(attributes);
    secret_value_unref(secret);

    if (item == NULL) {
        return false;
    }

    g_object_unref(item);

    return true;
}

passwd_state PasswordStorage::getUserData(String &system, String &username, String &password)
{
    GHashTable *attributes = secret_attributes_build(PASSWD_SCHEMA,
                             "system", system.asConstChar(),
                             NULL);
    GList *list;
    resetErr();

    m_mutex->lock();
    list = secret_service_search_sync(NULL, PASSWD_SCHEMA, attributes, SECRET_SEARCH_ALL, NULL, &m_res);
    m_mutex->unlock();

    if (m_res != NULL) {
        g_hash_table_unref(attributes);
        return PASSWD_ERR;
    } else if (list == NULL) {
        g_hash_table_unref(attributes);
        return PASSWD_NO_MATCH;
    }

    m_mutex->lock();
    SecretItem *item = ((SecretItem *) list->data);
    GHashTable *attr = secret_item_get_attributes(item);
    GList *keys =  g_hash_table_get_keys(attr);
    guint cnt = g_list_length(keys);
    for (guint i = 0; i < cnt; i++) {
        if (strcmp((gchar *) keys->data, "username") == 0) {
            username = (gchar *) g_hash_table_lookup(attr, keys->data);
            break;
        }

        keys = keys->next;
    }
    m_mutex->unlock();

    gchar *gch_passwd;
    m_mutex->lock();
    gch_passwd = secret_password_lookup_sync(
                     PASSWD_SCHEMA, NULL, &m_res,
                     "system", system.asConstChar(),
                     "username", username.asConstChar(),
                     NULL);
    password = gch_passwd;
    m_mutex->unlock();

    return PASSWD_OK;
}

bool PasswordStorage::deletePassword(String &system)
{
    resetErr();
    return secret_password_clear_sync(PASSWD_SCHEMA, NULL, &m_res,
                                      "system", system.asConstChar(),
                                      NULL);
}

String PasswordStorage::errString()
{
    if (m_res != NULL) {
        return m_res->message;
    } else {
        return "";
    }
}

#elif defined HAVE_LIBKEYRING
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

extern const GnomeKeyringPasswordSchema *GNOME_KEYRING_NETWORK_PASSWORD;

PasswordStorage::~PasswordStorage()
{

}

bool PasswordStorage::unlock()
{
}

bool PasswordStorage::createStorage()
{
    m_res = gnome_keyring_create_sync(OPENLMI_KEYRING_DEFAULT, NULL);
    return m_res == GNOME_KEYRING_RESULT_OK || m_res == GNOME_KEYRING_RESULT_KEYRING_ALREADY_EXISTS;
}

bool PasswordStorage::resetStorage()
{
    m_res = gnome_keyring_delete_sync(OPENLMI_KEYRING_DEFAULT);
    if (m_res == GNOME_KEYRING_RESULT_OK) {
        return createStorage();
    }

    return false;
}

bool PasswordStorage::setUserData(String &system, String &username, String &password)
{
    m_res = gnome_keyring_store_password_sync(
                GNOME_KEYRING_NETWORK_PASSWORD,
                OPENLMI_KEYRING_DEFAULT,
                system,
                password,
                "user", username.asConstChar(),
                "server", system.asConstChar(),
                NULL
            );

    return m_res == GNOME_KEYRING_RESULT_OK;
}

passwd_state PasswordStorage::getUserData(String &system, String &username, String &password)
{
    GList *res_list;

    m_mutex->lock();
    GnomeKeyringAttributeList *list = gnome_keyring_attribute_list_new();
    gnome_keyring_attribute_list_append_string(list, "server", system);

    GnomeKeyringResult res = gnome_keyring_find_items_sync(
                                 GNOME_KEYRING_ITEM_NETWORK_PASSWORD,
                                 list,
                                 &res_list
                             );
    m_mutex->unlock();

    if (res == GNOME_KEYRING_RESULT_NO_MATCH) {
        gnome_keyring_found_list_free(res_list);
        return PASSWD_NO_MATCH;
    } else if (res != GNOME_KEYRING_RESULT_OK) {
        gnome_keyring_found_list_free(res_list);
        return PASSWD_ERR;
    }

    m_mutex->lock();
    GnomeKeyringFound *keyring = ((GnomeKeyringFound *) res_list->data);
    guint cnt = g_array_get_element_size(keyring->attributes);
    for (guint i = 0; i < cnt; i++) {
        GnomeKeyringAttribute tmp;
        if (strcmp((tmp = g_array_index(keyring->attributes, GnomeKeyringAttribute,
                                        i)).name, "user") == 0) {
            username = gnome_keyring_attribute_get_string(&tmp);
            break;
        }
    }
    m_mutex->unlock();

    gchar *gch_passwd;
    m_mutex->lock();
    gnome_keyring_find_password_sync(
        GNOME_KEYRING_NETWORK_PASSWORD,
        &gch_passwd,
        "user", username.asConstChar(),
        "server", system.asConstChar(),
        NULL
    );
    password = gch_passwd;
    gnome_keyring_found_list_free(res_list);
    m_mutex->unlock();

    return PASSWD_OK;
}

bool PasswordStorage::deletePassword(String &system)
{
    m_res = gnome_keyring_delete_password_sync(
                GNOME_KEYRING_NETWORK_PASSWORD,
                "server", system.asConstChar(),
                NULL
            );
    return m_res == GNOME_KEYRING_RESULT_OK;
}

String PasswordStorage::errString()
{
    return gnome_keyring_result_to_message(m_res);
}

#endif
