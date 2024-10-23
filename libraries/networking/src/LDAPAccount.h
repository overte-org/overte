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
public:
    static bool isValidCredentials(const QString& username, const QString& password);
    static const char* toChar(const QString& str);
    static std::vector<std::string> getRolesAsStrings(const QString& username, const QString& password);
private:
    static LDAP* initialize();
};
#endif //LDAPACCOUNT_H
