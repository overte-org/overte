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


#ifndef LDAPACCOUNT_H
#define LDAPACCOUNT_H

#include <ldap.h>
#include <QtCore/QObject>

class LDAPAccount : public QObject {
    Q_OBJECT
private:
    static LDAP* initialize();
    static int login(LDAP* ldapHandle, const QString& username, const QString& password);
    static QString buildDNFromUsername(const QString& username);
    static QString ldapServerURL;
    static QString ldapServerUserBase;
    static QString ldapServerGroupBase;
    static QString readLDAPServerURL() {return ldapServerURL;}
    static QString readLDAPUserBase() {return ldapServerUserBase;}
    static QString readLDAPGroupBase() {return ldapServerGroupBase;}
public:
    static bool isValidCredentials(const QString& username, const QString& password);
    static std::vector<std::string> getRolesAsStrings(const QString& username, const QString& password);
    static void setLDAPServerURL(const QString& url) {ldapServerURL = url;}
    static void setLDAPUserBase(const QString& base) {ldapServerUserBase = base;}
    static void setLDAPGroupBase(const QString& base) {ldapServerGroupBase = base;}
};
#endif //LDAPACCOUNT_H
