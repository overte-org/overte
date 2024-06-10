//
//  NetworkingConstants.h
//  libraries/networking/src
//
//  Created by Stephen Birarda on 2015-03-31.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_NetworkingConstants_h
#define hifi_NetworkingConstants_h

#include <QtCore/QProcessEnvironment>
#include <QtCore/QUrl>

namespace NetworkingConstants {
    // If you want to use STAGING instead of STABLE,
    // links from the Domain Server web interface (like the connect account token generation)
    // will still point at stable unless you ALSO change the Domain Server Directory Services URL inside of:
    // <Overte repo>\domain-server\resources\web\js\shared.js

    // You can avoid changing that and still effectively use a connected domain on staging
    // if you manually generate a personal access token for the domains scope
    // at https://staging.highfidelity.com/user/tokens/new?for_domain_server=true

    const QString WEB_ENGINE_VERSION = "Chrome/83.0.4103.122";

    // For now we only have one directory server.
    const QUrl METAVERSE_SERVER_URL_STABLE { "https://mv.overte.org/server" };
    const QUrl METAVERSE_SERVER_URL_STAGING { "https://mv.overte.org/server" };

    // Web Engine requests to this parent domain have an account authorization header added
    const QString AUTH_HOSTNAME_BASE = "overte.org";
    const QStringList IS_AUTHABLE_HOSTNAME = { "overte.org" };

    // Use a custom User-Agent to avoid ModSecurity filtering, e.g. by hosting providers.
    const QByteArray OVERTE_USER_AGENT = "Mozilla/5.0 (OverteInterface)";

    const QString WEB_ENGINE_USER_AGENT = "Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) " + WEB_ENGINE_VERSION + " Mobile Safari/537.36";
    const QString MOBILE_USER_AGENT = "Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) " + WEB_ENGINE_VERSION + " Mobile Safari/537.36";

    // WebEntity Defaults
    const QString WEB_ENTITY_DEFAULT_SOURCE_URL = "https://overte.org/";
    const QString WEB_ENTITY_DEFAULT_USER_AGENT = WEB_ENGINE_USER_AGENT;

    // Builds URLs
    const QUrl BUILDS_XML_URL("");
    const QUrl MASTER_BUILDS_XML_URL("");

    const QString DEFAULT_AVATAR_COLLISION_SOUND_URL = "https://hifi-public.s3.amazonaws.com/sounds/Collisions-otherorganic/Body_Hits_Impact.wav";

    // CDN URLs
    const QString HF_CONTENT_CDN_URL = "";
    const QString HF_MPASSETS_CDN_URL = "";
    const QString HF_PUBLIC_CDN_URL = "";
    const QString HF_MARKETPLACE_CDN_HOSTNAME = "";
    const QString OVERTE_CONTENT_CDN_URL = "https://content.overte.org/";
    const QString OVERTE_COMMUNITY_APPLICATIONS = { "https://more.overte.org/applications" };
    const QString OVERTE_TUTORIAL_SCRIPTS = { "https://more.overte.org/tutorial" };

#if USE_STABLE_GLOBAL_SERVICES
    const QString ICE_SERVER_DEFAULT_HOSTNAME = "ice.overte.org";

    const QString STUN_SERVER_DEFAULT_HOSTNAME = "stun1.l.google.com";
    const unsigned short STUN_SERVER_DEFAULT_PORT = 19302;
#else
    const QString ICE_SERVER_DEFAULT_HOSTNAME = "ice.overte.org";

    const QString STUN_SERVER_DEFAULT_HOSTNAME = "stun2.l.google.com";
    const unsigned short STUN_SERVER_DEFAULT_PORT = 19302;
#endif

    const QUrl HELP_COMMUNITY_URL{ "https://overte.org" };
    const QUrl HELP_DOCS_URL{ "https://docs.overte.org" };
    const QUrl HELP_FORUM_URL{ "https://overte.org" };
    const QUrl HELP_SCRIPTING_REFERENCE_URL{ "https://apidocs.overte.org/" };
    const QUrl HELP_RELEASE_NOTES_URL{ "https://docs.overte.org/release-notes.html" };
    const QUrl HELP_BUG_REPORT_URL{ "https://github.com/overte-org/overte/issues" };

    const QString DEFAULT_OVERTE_ADDRESS = "file:///~/serverless/tutorial.json";
    const QString DEFAULT_HOME_ADDRESS = "file:///~/serverless/tutorial.json";
    const QString REDIRECT_HIFI_ADDRESS = "file:///~/serverless/redirect.json";
}

const QString HIFI_URL_SCHEME_ABOUT = "about";
const QString URL_SCHEME_OVERTE = "hifi";
const QString URL_SCHEME_OVERTEAPP = "hifiapp";
const QString URL_SCHEME_DATA = "data";
const QString URL_SCHEME_QRC = "qrc";
const QString HIFI_URL_SCHEME_FILE = "file";
const QString HIFI_URL_SCHEME_HTTP = "http";
const QString HIFI_URL_SCHEME_HTTPS = "https";
const QString HIFI_URL_SCHEME_FTP = "ftp";
const QString URL_SCHEME_ATP = "atp";

#endif // hifi_NetworkingConstants_h
