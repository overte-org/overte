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
#include <vector>
#include <regex>
#include <string>

#include <ldap.h>

QString LDAPAccount::ldapServerURL;
QString LDAPAccount::ldapServerUserBase;
QString LDAPAccount::ldapServerGroupBase;

bool LDAPAccount::isValidCredentials(const QString& username, const QString& password) {
    LDAP* ldapHandle = initialize(); // Initialize the ldap connection.
    if (ldapHandle == nullptr) {return false;}
    int loginResult = login(ldapHandle, username, password); // Log in
    ldap_unbind_ext(ldapHandle, nullptr, nullptr);

    if (loginResult != LDAP_SUCCESS){
        // Login failed
        return false;
    }

    return true;
}

std::vector<std::string> LDAPAccount::getRolesAsStrings(const QString& username, const QString& password) {
    berval** memberOf;
    std::regex cnRegex(R"(cn=([^,]+))");
    std::smatch matches;
    LDAPMessage *result, *entry;
    std::vector<std::string> roles;

    LDAP* ldapHandle = initialize(); // Initialize the ldap connection.
    int loginResult = login(ldapHandle, username, password); // Log in

    if (loginResult != 0){
        // Login failed
        qDebug() << "Failed trying to get roles as string. LDAP error code: " << loginResult << ". " << ldap_err2string(loginResult);
        return {};
    }

    QString usernameDN = buildDNFromUsername(username);
    int rc;

    char *attrs[] = { "memberOf", nullptr };

    rc = ldap_search_ext_s(
        ldapHandle,
        usernameDN.toLocal8Bit().constData(),
        LDAP_SCOPE_SUBTREE,
        nullptr,
        attrs,
        0,
        nullptr,
        nullptr,
        nullptr,
        0,
        &result
    );

    if (rc != LDAP_SUCCESS) {
        // Failed preforming the LDAP search
        qDebug(networking) << "LDAP search failed: "<< ldap_err2string(rc);
        return {};
    }

    entry = ldap_first_entry(ldapHandle, result);
    if (entry == nullptr) {
        // Failed to find the user
        qDebug(networking) << "LDAP search returned no users.";
        return {};
    }

    memberOf = ldap_get_values_len(ldapHandle, entry, "memberOf");
    if (memberOf == nullptr) {
        // User does not have any roles to return
        qDebug(networking) << "User does not have any roles.";
        return {};
    }

    for (int i = 0; memberOf[i] != nullptr; i++) {
        const std::string role = memberOf[i]->bv_val;

        if (std::regex_search(role, matches, cnRegex)){
            std::string cn = matches[0]; // Extract the CN value
    	    cn.erase(0, 3); // Remove the first three characters (cn=)
            roles.emplace_back(cn); // Append to the list
        }
    }

    return roles;
}

LDAP* LDAPAccount::initialize(){
    // TODO FIXME: Check to see if username/password is null
    // TODO FIXME: Get domain URL or LDAP url?

    LDAP* ldapHandle;
    // const char* saslMechanism = "LDAP_SASL_SIMPLE";
    ulong version = LDAP_VERSION3;

    // Initialize the LDAP library
    int result = ldap_initialize(&ldapHandle, readLDAPServerURL().toLocal8Bit().constData());
    if (result != LDAP_SUCCESS) {
        // LDAP could not be initialized for some reason.
        qDebug(networking) << "LDAP could not be initialized: Error: "<< result << ". "<< ldap_err2string(result);;
        return {};
    }

    // Set the LDAP version to 3
    result = ldap_set_option(ldapHandle, LDAP_OPT_PROTOCOL_VERSION, &version);
    if (result != LDAP_SUCCESS) {
        // Could not update the LDAP version for some reason.
        qDebug(networking) << "Could not update LDAP version";
        return {};
    }
    return ldapHandle;
}
QString LDAPAccount::buildDNFromUsername(const QString& username) {
    // Build the DN
    QString loginDN = "cn=" + username + "," + readLDAPUserBase();
    return loginDN;
}

int LDAPAccount::login(LDAP* ldapHandle, const QString& username, const QString& password){
    QString loginDN = buildDNFromUsername(username);

    // Prepare the password as a berval.
    berval creds{};
    creds.bv_val = const_cast<char*>(password.toLocal8Bit().constData());
    creds.bv_len = strlen(password.toLocal8Bit().constData());

    // Perform SASL bind
    int saslBind = ldap_sasl_bind_s(ldapHandle, loginDN.toLocal8Bit().constData(), LDAP_SASL_SIMPLE, &creds, nullptr, nullptr, nullptr);
    if (saslBind != LDAP_SUCCESS) {
        // Login info is invalid
        qDebug(networking) << "Failed trying to bind to LDAP. Status: " << saslBind << ". " << ldap_err2string(saslBind);
        return saslBind;
    }

    qDebug(networking) << "Successfully signed in '" << username << "' into the LDAP server";
    return saslBind; // Should be '0'
}