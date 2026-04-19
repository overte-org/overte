//
//  URLClass.h
//  libraries/script-engine/src/
//
//  Created by Ada <ada@thingvellir.net> on 2026-04-09
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_URLClass_h
#define hifi_URLClass_h

#include <QUrl>

#include "ScriptEngine.h"
#include "ScriptValue.h"

class ScriptContext;

/*@jsdoc
 * Provides a means to parse and construct URLs. It is a near-complete implementation
 * of <a href="https://developer.mozilla.org/en-US/docs/Web/API/URL">the Web URL API
 * as described on MDN.</a>
 *
 * <p><code>URL.createObjectURL</code> and <code>URL.revokeObjectURL</code>
 * are not currently supported.</p>
 *
 * @class URL
 * @param {string} url - An absolute or relative URL. If relative, then <code>base</code> must be specified.
 * @param {string} [base] - The absolute base URL to resolve <code>url</code> against.
 *
 * @hifi-interface
 * @hifi-client-entity
 * @hifi-avatar
 * @hifi-server-entity
 * @hifi-assignment-client
 *
 * @property {string} hash - The fragment part.
 * @property {string} host - The domain hostname and port separated by ':', if present.
 * @property {string} hostname - The domain hostname.
 * @property {string} href - Contains the whole URL as a string.
 * @property {string} origin - The scheme, domain hostname, and port. <em>Read-only.</em>
 * @property {string} password - The password in the userinfo part, before the hostname.
 * @property {string} pathname - The path part, starting with '/' and not including the query or fragment parts.
 * @property {string} port - A string of the host port number.
 * @property {string} protocol - The protocol scheme of the URL including the trailing ':': i.e. <code>https:</code> or <code>file:</code>.
 * @property {string} search - The query part of the URL, as one whole string.
 * @property {string} username - The username in the userinfo part, before the hostname.
 */
class URLClass : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString hash READ getHash WRITE setHash)
    Q_PROPERTY(QString host READ getHost WRITE setHost)
    Q_PROPERTY(QString hostname READ getHostname WRITE setHostname)
    Q_PROPERTY(QString href READ getHref WRITE setHref)
    Q_PROPERTY(QString origin READ getOrigin)
    Q_PROPERTY(QString password READ getPassword WRITE setPassword)
    Q_PROPERTY(QString pathname READ getPathname WRITE setPathname)
    Q_PROPERTY(QString port READ getPort WRITE setPort)
    Q_PROPERTY(QString protocol READ getProtocol WRITE setProtocol)
    Q_PROPERTY(QString search READ getSearch WRITE setSearch)
    // TODO: ScriptValue Map support?
    // Q_PROPERTY(ScriptValue searchParams READ getSearchParams)
    Q_PROPERTY(QString username READ getUsername WRITE setUsername)

public:
    URLClass(ScriptEngine* engine, QUrl url);

    operator QString() const { return getHref(); }

    static ScriptValue constructor(ScriptContext* context, ScriptEngine* engine);

    /*@jsdoc
     * Allows you to check if a URL is valid without needing to use a <code>try...catch</code>
     * block with the <code>URL</code> constructor.
     * <strong>Static method</strong>. Only available on the global <code>URL</code> object.
     * @function URL.canParse
     * @param {string} url
     * @param {string} [base]
     * @returns {boolean} <code>true</code> if a URL string can be parsed and <code>false</code> otherwise.
     */
    static ScriptValue canParse(ScriptContext* context, ScriptEngine* engine);
    static bool canParse(const QString& url, const QString& base = QString());

    /*@jsdoc
     * A non-throwing alternative to the <code>URL</code> constructor.
     * <strong>Static method</strong>. Only available on the global <code>URL</code> object.
     * @function URL.parse
     * @param {string} url
     * @param {string} [base]
     * @returns {?URL} A <code>URL</code> object, or <code>null</code> if the URL can't be parsed.
     */
    static ScriptValue parse(ScriptContext* context, ScriptEngine* engine);
    static ScriptValue parse(ScriptEngine* engine, const QString& url, const QString& base = QString());

    /*@jsdoc
     * Equivalent to <code>URL.href</code>.
     * @function URL#toString
     * @returns {string} A string containing the whole URL.
     */
    Q_INVOKABLE QString toString() const { return getHref(); }

    /*@jsdoc
     * Equivalent to <code>URL.href</code>.
     * @function URL#toJSON
     * @returns {string} A string containing the whole URL.
     */
    Q_INVOKABLE QString toJSON() const { return getHref(); }

private:
    QString getHash() const;
    QString getHost() const;
    QString getHostname() const;
    QString getHref() const;
    QString getOrigin() const;
    QString getPassword() const;
    QString getPathname() const;
    QString getPort() const;
    QString getProtocol() const;
    QString getSearch() const;
    QString getUsername() const;

    void setHash(const QString& value);
    void setHost(const QString& value);
    void setHostname(const QString& value);
    void setHref(const QString& value);
    void setPassword(const QString& value);
    void setPathname(const QString& value);
    void setPort(const QString& value);
    void setProtocol(const QString& value);
    void setSearch(const QString& value);
    void setUsername(const QString& value);

    ScriptEngine* _engine;
    QUrl _url;
};

#endif // hifi_URLCLass_h

/// @}
