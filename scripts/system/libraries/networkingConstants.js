//  networkingConstants.js
//
//  Created by Kalila L. on 8/27/20
//  Copyright 2020 Vircadia contributors.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

// Interface Metadata Source
//var INTERFACE_METADATA_SOURCE = "https://cdn.vircadia.com/dist/launcher/vircadiaMeta.json";
var INTERFACE_METADATA_SOURCE = "";

// Base CDN URLs
var CONTENT_CDN_URL = Script.getExternalPath(Script.ExternalPaths.HF_Content, "/");
var PUBLIC_BUCKET_CDN_URL = Script.getExternalPath(Script.ExternalPaths.HF_Public, "/");

module.exports = {
    INTERFACE_METADATA_SOURCE: INTERFACE_METADATA_SOURCE,
    CONTENT_CDN_URL: CONTENT_CDN_URL,
    PUBLIC_BUCKET_CDN_URL: PUBLIC_BUCKET_CDN_URL
}
