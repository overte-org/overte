"use strict";

/* global Entities, Window, EditorWindow, Script */

(function (global) {

  var scriptEditorFactory = Script.require('./scriptEditor.js');

  function EditorSpawner() { }

  EditorSpawner.prototype.spawn = function (entityId, scriptType) {
    if (!entityId) {
      return;
    }

    // Check if entity exists
    var properties = Entities.getEntityProperties(entityId, ['id', 'script', 'serverScripts', 'locked']);
    if (properties.id !== entityId) {
      Window.alert("Entity not found");
      return;
    }

    if (properties.locked) {
      Window.alert("Entity is locked. You must unlock it first.");
      return;
    }

    var isClient = scriptType === 'client';
    var entityScriptBody = '"use strict"; (' + String(scriptEditorFactory) + ');';

    var editorWindow = scriptEditorFactory({
      entityId: entityId,
      isClient: isClient,
      scriptBody: entityScriptBody
    });
    editorWindow.openEditor();
  };

  global.editorSpawner = new EditorSpawner();

}(typeof module !== 'undefined' ? module.exports : new Function('return this;')()));
