"use strict";

/* global CHANNEL_NAME, INIT_ENTITIES_DELAY, Editor,
 * Script, Messages, Entities
 */

((typeof module !== 'undefined' ? module : {}).exports = function () {

  Script.include([
    './constants.js',
    './editor.js'
  ]);

  function EditorServer() {
    this.remotelyCallable = [
      'initialize',
      'update',
      'save',
      'load',
      'runScript',
      'stopScript'
    ];
  }

  EditorServer.prototype.preload = function (entityId) {
    var self = this;

    this.entityId = entityId;
    this.channelName = CHANNEL_NAME.replace('{id}', entityId);

    Script.setTimeout(function () {
      var userData = Editor.parseUserData(entityId);
      self.editor = new Editor(userData);
      self.load(entityId, [undefined]);
    }, INIT_ENTITIES_DELAY);
  };

  EditorServer.prototype.unload = function () { };

  EditorServer.prototype.initialize = function (_id, params) {
    var clientId = params[0];
    var content = this.editor.content;
    var fileName = this.editor.fileName;
    this.sendToClient(clientId, {type: 'SET_STATE', content: content, fileName: fileName, status: 'UNLOADED'});
  };

  EditorServer.prototype.update = function (_id, params) {
    var action;
    try {
      action = JSON.parse(params[1]);
    } catch (e) {
      return;
    }
    if (this.editor.applyUpdate(action)) {
      this.sendToAll(action);
    }
  };

  EditorServer.prototype.save = function (_id, params) {
    var self = this;
    var clientId = params[0];

    this.editor.saveFile(function (error) {
      if (error) {
        self.sendToClient(clientId, {type: 'SHOW_MESSAGE', message: 'Error: ' + error});
      } else {
        self.sendToClient(clientId, {type: 'SHOW_MESSAGE', message: 'File saved'});
      }
    });
  };

  EditorServer.prototype.load = function (_id, params) {
    var self = this;
    var clientId = params[0];

    this.editor.loadFile(function (content, fileName) {
      if (!clientId) {
        return;
      }
      self.sendToAll({type: 'SET_STATE', content: content, fileName: fileName});
      self.sendToClient(clientId, {type: 'SHOW_MESSAGE', message: 'File loaded'});
    });
  };

  EditorServer.prototype.runScript = function (_id, params) {
    var self = this;

    this.editor.saveFile(function (error) {
      if (!error) {
        self.editor.runScript();
      }
    });
  };

  EditorServer.prototype.stopScript = function (_id, params) {
    this.editor.stopScript();
  };

  EditorServer.prototype.callClient = function (clientId, methodName, params) {
    if (this.client) {
      this.client[methodName](this.entityId, params);
      return;
    }
    Entities.callEntityClientMethod(clientId, this.entityId, methodName, params);
  };

  EditorServer.prototype.sendToClient = function (clientId, action) {
    this.callClient(clientId, 'emitWebEvent', [JSON.stringify(action)]);
  };

  EditorServer.prototype.sendToAll = function (action) {
    Messages.sendMessage(this.channelName, JSON.stringify(action));
  };

  return new EditorServer();
});
