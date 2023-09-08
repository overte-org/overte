"use strict";

/* global Script */

(function (global) {

  global.CHANNEL_NAME = 'OVERTE_EDITOR_CHANNEL_{id}';
  global.EDITOR_SOURCE_URL = 'qrc:///html/overte-editor-app/index.html';
  global.EDITOR_CLIENT_SCRIPT_URL = 'https://raw.githubusercontent.com/keeshii/overte/master/scripts/system/create/scriptEditor/editorClient.js';
  global.EDITOR_SERVER_SCRIPT_URL = 'https://raw.githubusercontent.com/keeshii/overte/master/scripts/system/create/scriptEditor/editorServer.js';
  global.EDITOR_WIDTH = 1.92;
  global.EDITOR_HEIGHT = 1.08;
  global.INIT_ENTITIES_DELAY = 500;

}(typeof module !== 'undefined' ? module.exports : new Function('return this;')()));
