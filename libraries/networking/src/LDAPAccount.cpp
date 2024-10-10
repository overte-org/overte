//
//  DomainAccountManager.cpp
//  libraries/networking/src
//
//  Created by Armored Dragon in October 2024.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "LDAPAccount.h"

#include "NetworkLogging.h"

#include <iostream>
#include <cstring>
#include <cctype>

#include <ldap.h>

bool LDAPAccount::isValidCredentials(const QString& username, const QString& password) {
    // TODO FIXME: Check to see if username/password is null
    // TODO FIXME: Get domain URL or LDAP url?

    // Change the QString to char*
    const char* usernameChar = toChar(username);
    const char* passwordChar = toChar(password);

    LDAP* ldapHandle;
    const char* ldapURI = "ldap://localhost:3389";
    const char* bindDN = usernameChar;
    const char* ldapPassword = passwordChar;
    // const char* saslMechanism = "LDAP_SASL_SIMPLE";
    ulong version = LDAP_VERSION3;

    // Prepare the password as a berval.
    berval creds{};
    creds.bv_val = const_cast<char*>(ldapPassword);  // Cast to char*
    creds.bv_len = strlen(ldapPassword);             // Set length of password

    // Initialize the LDAP library
    int result = ldap_initialize(&ldapHandle, ldapURI);
    if (result != LDAP_SUCCESS) {
        qDebug(networking) << "LDAP could not be initialized";
        return false;
    }

    // Set the LDAP version to 3
    result = ldap_set_option(ldapHandle, LDAP_OPT_PROTOCOL_VERSION, &version);
    if (result != LDAP_SUCCESS) {
        qDebug(networking) << "Could not update LDAP version";
        return false;
    }

    ldap_connect(ldapHandle);

    // if (connectSuccess == LDAP_SUCCESS) {
    //     qDebug(networking) << "Successfully connected to LDAP";
    // }

    // Perform SASL bind
    result = ldap_sasl_bind_s(ldapHandle, bindDN, LDAP_SASL_SIMPLE, &creds, nullptr, nullptr, nullptr);
    if (result != LDAP_SUCCESS) {
        // Login info is invalid
        qDebug(networking) << "Failed trying to bind to LDAP. Status: " << result;
        ldap_unbind_ext(ldapHandle, nullptr, nullptr);
        return false;
    }

    qDebug(networking) << "Successfully signed in '" << username << "' into the LDAP server";

    // Unbind and free the resources
    ldap_unbind_ext(ldapHandle, nullptr, nullptr);

    return true;
}

const char* LDAPAccount::toChar(const QString& str) {
    const char* ch = str.toLocal8Bit().constData();
    return ch;
}
