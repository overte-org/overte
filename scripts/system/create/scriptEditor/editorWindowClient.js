"use strict";

/* global Messages, Script, Entities, MyAvatar, Vec3, ConsoleMonitor,
 * EDITOR_CLIENT_SCRIPT_URL, EDITOR_HEIGHT, EDITOR_SERVER_SCRIPT_URL,
 * EDITOR_SOURCE_URL, EDITOR_WIDTH, Editor, Subscriber,
 * StatusMonitor */

(function (global) {

  Script.include([
    './consoleMonitor.js',
    './constants.js',
    './editor.js',
    './statusMonitor.js',
    './subscriber.js'
  ]);

  function EditorWindowClient(window, userData) {
    this.window = window;
    this.userData = userData;
    this.subscriber = new Subscriber(this);
  }

  EditorWindowClient.prototype.preload = function () {
    var self = this;

    this.editor = new Editor(this.userData);
    this.subscriber.subscribe(this.window.webEventReceived, 'onWebEventReceived');

    this.statusMonitor = new StatusMonitor(this.userData.editingEntityId, function (status) {
      self.updateScriptStatus(status);
    });

    this.consoleMonitor = new ConsoleMonitor(function (type, message, scriptName) {
      self.appendLog(type, message, scriptName);
    });
    this.consoleMonitor.subscribe();
  };

  EditorWindowClient.prototype.unload = function () {
    this.subscriber.unsubscribe();
    this.statusMonitor.stop();
    this.consoleMonitor.unsubscribe();
  };

  EditorWindowClient.prototype.onWebEventReceived = function (message) {
    var self = this;
    var action;

    try {
      action = JSON.parse(message);
    } catch (e) {
      return;
    }

    switch (action.type) {
      case 'INITIALIZE':
        this.emitToWebView({type: 'SET_TOOLBAR_BUTTONS', showClose: false, showOpenInEntity: true});
        this.editor.loadFile(function (content, fileName) {
          self.emitToWebView({type: 'SET_STATE', content: content, fileName: fileName});
        });
        break;
      case 'UPDATE':
        this.editor.applyUpdate(action);
        this.emitToWebView(action);
        break;
      case 'SET_SCROLL':
        break;
      case 'SAVE':
        this.editor.saveFile(function (error) {
          if (error) {
            self.emitToWebView({type: 'SHOW_MESSAGE', message: 'Error: ' + error});
            return;
          }
          self.emitToWebView({type: 'SHOW_MESSAGE', message: 'File saved'});
        });
        break;
      case 'RELOAD':
        this.editor.loadFile(function (content, fileName) {
          self.emitToWebView({type: 'SET_STATE', content: content, fileName: fileName});
          self.emitToWebView({type: 'SHOW_MESSAGE', message: 'File loaded'});
        });
        break;
      case 'RUN':
        this.editor.saveFile(function (error) {
          if (!error) {
            self.editor.runScript();
          }
        });
        break;
      case 'STOP':
        this.editor.stopScript();
        break;
      case 'OPEN_IN_ENTITY':
        this.spawnEditorEntity();
        this.window.close();
        break;
    }
  };

  EditorWindowClient.prototype.spawnEditorEntity = function () {
    var userData = this.userData;
    var translation = Vec3.multiplyQbyV(MyAvatar.orientation, {x: 0, y: 0.5, z: -3});
    var position = Vec3.sum(MyAvatar.position, translation);

    Entities.addEntity({
      type: "Web",
      dpi: 20,
      position: position,
      rotation: MyAvatar.orientation,
      sourceUrl: EDITOR_SOURCE_URL,
      script: EDITOR_CLIENT_SCRIPT_URL,
      serverScripts: EDITOR_SERVER_SCRIPT_URL,
      dimensions: {x: EDITOR_WIDTH, y: EDITOR_HEIGHT, z: 0.01},
      userData: JSON.stringify(userData)
    });
  };

  EditorWindowClient.prototype.emitToWebView = function (action) {
    var message = JSON.stringify(action);
    this.window.emitScriptEvent(message);
  };

  EditorWindowClient.prototype.updateScriptStatus = function (status) {
    if (status.isRunning && status.status === 'running') {
      this.emitToWebView({type: 'SET_STATUS', status: 'RUNNING'});
      this.wasRunning = true;
      return;
    }

    if (status.status === 'error_running_script' && this.wasRunning) {
      this.emitToWebView({type: 'LOG_ERROR', error: status.errorInfo});
      this.emitToWebView({type: 'SET_STATUS', status: 'UNLOADED'});
      this.wasRunning = false;
    }
  };

  EditorWindowClient.prototype.appendLog = function (severity, message, scriptName) {
    if (severity === 'ERROR') {
      this.emitToWebView({type: 'LOG_ERROR', error: message});
      return;
    }
    this.emitToWebView({type: 'LOG_INFO', items: [message]});
  };

  global.EditorWindowClient = EditorWindowClient;
  
}(typeof module !== 'undefined' ? module.exports : new Function('return this;')()));
