"use strict";

/* global Entities, Assets, Window, Script, EditorWindowClient,
 EDITOR_HEIGHT, EDITOR_SOURCE_URL, EDITOR_WIDTH, EDITOR_HEIGHT */

(function (global) {

  Script.include([
    './constants.js',
    './editorWindowClient.js'
  ]);

  function EditorWindow(entityId, isClient) {
    this.entityId = entityId;
    this.isClient = isClient;
  }

  EditorWindow.prototype.openEditor = function () {
    if (!Entities.canWriteAssets()) {
      Window.alert("You have no permission to make changes to the asset server's assets");
      return;
    }

    var entityId = this.entityId;
    var isClient = this.isClient;

    var properties = Entities.getEntityProperties(entityId, ['id', 'script', 'serverScripts', 'locked']);
    if (properties.id !== entityId) {
      Window.alert("Entity not found");
      return;
    }
    if (properties.locked) {
      Window.alert("Entity is locked. You must unlock it first.");
      return;
    }

    // Server or Client Script?
    var scriptUrl = isClient ? properties.script : properties.serverScripts;

    // Ensure script is in the Asset Server
    // then spawn editor
    this.prepareScriptToEdit(scriptUrl, function (fileName) {
      var userData = {
        fileName: fileName,
        scriptType: isClient ? 'client' : 'server',
        editingEntityId: entityId,
        grabbableKey: {grabbable: false, triggerable: false}
      };

      var overlayWebWindow = new OverlayWebWindow({
        title: "Script Editor",
        source: EDITOR_SOURCE_URL,
        width: EDITOR_WIDTH * 400,
        height: EDITOR_HEIGHT * 400
      });

      var client = new EditorWindowClient(overlayWebWindow, userData);
      client.preload();

      var onClosed = function () {
        overlayWebWindow.closed.disconnect(onClosed);
        client.unload();
      };

      overlayWebWindow.closed.connect(onClosed);
    });
  };

  EditorWindow.prototype.downloadFileFromUrl = function (url) {
    var request = new XMLHttpRequest();
    request.open('GET', url, false);  // `false` makes the request synchronous
    request.send(null);
    if (request.status === 0 || request.status === 200) {
      return request.responseText;
    }
    return undefined;
  };

  EditorWindow.prototype.getFileNameFromUrl = function (url) {
    var name = url.replace(/.*\//, '');
    if (name === '') {
      name = 'file.js';
    }
    if (!name.match(/\.js$/)) {
      name += '.js';
    }
    return name;
  };

  EditorWindow.prototype.prepareScriptToEdit = function (url, callback) {
    var self = this;
    var fileName = 'file.js';

    // Script already in the Asset Server, just remember its name
    if (url.match(/^atp:\//)) {
      fileName = url.replace('atp:/', '');
      callback(fileName);
      return;
    }

    // Script will be copied to the Asset Server
    var proceed = Window.confirm("The script will be copied to the Asset Server and may overwrite some other files. Say YES if you want to proceed.");
    if (!proceed) {
      return;
    }

    var content = url || ' ';
    if (url.match(/(file|https?):\/\//)) {
      fileName = this.getFileNameFromUrl(url);

      content = this.downloadFileFromUrl(url);
      if (content === undefined) {
        Window.alert("Unable to download the file.");
        return;
      }
    }

    Assets.putAsset({data: String(content), path: '/' + fileName}, function (error) {
      if (error) {
        Window.alert("Cannot save file to Asset Server");
        return;
      }
      Entities.editEntity(self.entityId, self.isClient
              ? {script: 'atp:/' + fileName}
      : {serverScripts: 'atp:/' + fileName});
      callback(fileName);
    });
  };

  global.EditorWindow = EditorWindow;

}(typeof module !== 'undefined' ? module.exports : new Function('return this;')()));
