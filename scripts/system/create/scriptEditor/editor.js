"use strict";

/* global Entities, Assets */

(function (global) {

  function Editor(params) {
    this.fileName = params.fileName.replace(/^atp:\/+/, '');
    this.content = '';
    this.scriptType = params.scriptType;
    this.editingEntityId = params.editingEntityId;
  }

  Editor.parseUserData = function (entityId) {
    var properties = Entities.getEntityProperties(entityId, ['userData']);
    var userData;
    try {
      userData = JSON.parse(properties.userData);
    } catch (e) {
      return;
    }
    return {
      fileName: userData.fileName,
      scriptType: userData.scriptType,
      editingEntityId: userData.editingEntityId
    };
  };

  Editor.prototype.applyUpdate = function (action) {
    var text = this.content;
    if (action.remove) {
      text = text.substring(0, action.position)
              + text.substring(action.position + action.remove);
    }

    if (action.insert) {
      text = text.substring(0, action.position)
              + String(action.insert)
              + text.substring(action.position);
    }

    if (text === this.content) {
      return false;
    }

    this.content = text;
    return true;
  };


  Editor.prototype.saveFile = function (callback) {
    if (!this.fileName) {
      return;
    }
    var content = this.content;
    var fileName = this.fileName;

    Assets.putAsset({
      data: String(content),
      path: '/' + fileName
    }, function (error) {
      callback(error);
    });
  };

  Editor.prototype.loadFile = function (callback) {
    var self = this;
    Assets.getAsset({
      url: '/' + this.fileName,
      responseType: "text"
    }, function (error, result) {
      if (error) {
        return;
      }
      self.content = result.response;
      callback(self.content, 'atp:/' + self.fileName);
    });
  };

  Editor.prototype.runScript = function () {
    var timestamp;
    switch (this.scriptType) {
      case 'client':
        timestamp = Date.now();
        Entities.editEntity(this.editingEntityId, {scriptTimestamp: timestamp});
        break;
      case 'server':
        this.stopScript();
        break;
    }
  };

  Editor.prototype.stopScript = function () {
    Entities.reloadServerScripts(this.editingEntityId);
  };

  global.Editor = Editor;

}(typeof module !== 'undefined' ? module.exports : new Function('return this;')()));
