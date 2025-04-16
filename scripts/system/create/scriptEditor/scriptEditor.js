"use strict";

/* global Script, Entities, Assets, MyAvatar, Vec3, Window, Messages */

((typeof module !== 'undefined' ? module : {}).exports = function (params) {

  "use strict";

  var global = {};
  global.CHANNEL_NAME = 'OVERTE_EDITOR_CHANNEL_{id}';
  global.EDITOR_SOURCE_URL = 'qrc:///html/overte-editor-app/index.html';
  global.EDITOR_SCRIPT_FILE = 'scriptEditor-v0.1.js';
  global.EDITOR_SCRIPT_BODY = params && params.scriptBody ? params.scriptBody : ' ';
  global.EDITOR_WIDTH = 1.92;
  global.EDITOR_HEIGHT = 1.08;
  global.INIT_ENTITIES_DELAY = 500;

  /* ------------------------------------------------ */

  function ConsoleMonitor(callbackFn) {
    this.infoFn = function (message, scriptName) {
      callbackFn('INFO', message, scriptName);
    };
    this.warnFn = function (message, scriptName) {
      callbackFn('WARNING', message, scriptName);
    };
    this.errorFn = function (message, scriptName) {
      callbackFn('ERROR', message, scriptName);
    };
    this.exceptionFn = function (error) {
      callbackFn('ERROR', JSON.stringify(error));
    };
  }

  ConsoleMonitor.prototype.subscribe = function () {
    Script.printedMessage.connect(this.infoFn);
    Script.infoMessage.connect(this.infoFn);
    Script.warningMessage.connect(this.warnFn);
    Script.errorMessage.connect(this.errorFn);
    Script.unhandledException.connect(this.exceptionFn);
  };

  ConsoleMonitor.prototype.unsubscribe = function () {
    Script.printedMessage.disconnect(this.infoFn);
    Script.infoMessage.disconnect(this.infoFn);
    Script.warningMessage.disconnect(this.warnFn);
    Script.errorMessage.disconnect(this.errorFn);
    Script.unhandledException.disconnect(this.exceptionFn);
  };

  /* ------------------------------------------------ */

  function StatusMonitor(entityId, statusCallback) {
    var self = this;

    this.entityID = entityId;
    this.active = true;
    this.sendRequestTimerID = null;
    this.status = 'UNLOADED';

    var onStatusReceived = function (success, isRunning, status, errorInfo) {
      if (self.active) {
        statusCallback({
          statusRetrieved: success,
          isRunning: isRunning,
          status: status,
          errorInfo: errorInfo
        });
        self.status = isRunning ? 'RUNNING' : 'UNLOADED';
        self.sendRequestTimerID = Script.setTimeout(function () {
          if (self.active) {
            Entities.getServerScriptStatus(entityId, onStatusReceived);
          }
        }, 1000);
      }
    };
    Entities.getServerScriptStatus(entityId, onStatusReceived);
  }

  StatusMonitor.prototype.stop = function () {
    this.active = false;
  };

  /* ------------------------------------------------ */

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

  /* ------------------------------------------------ */

  function Subscriber(parent) {
    this.parent = parent;
    this.subscriptions = [];
  }

  Subscriber.prototype.subscribe = function (signal, method) {
    var self = this;
    var subscription = {
      signal: signal,
      handler: function () {
        self.parent[method].apply(self.parent, arguments);
      }
    };

    signal.connect(subscription.handler);
    this.subscriptions.push(subscription);
  };

  Subscriber.prototype.unsubscribe = function () {
    var i;
    for (i = 0; i < this.subscriptions.length; i++) {
      var s = this.subscriptions[i];
      s.signal.disconnect(s.handler);
    }
    this.subscriptions.length = 0;
  };

  /* ------------------------------------------------ */

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
    var fileName = global.EDITOR_SCRIPT_FILE;

    Assets.putAsset({data: String(global.EDITOR_SCRIPT_BODY), path: '/' + fileName}, function (error) {
      if (error) {
        Window.alert("Cannot save file to Asset Server");
        return;
      }

      Entities.addEntity({
        type: "Web",
        dpi: 20,
        position: position,
        rotation: MyAvatar.orientation,
        sourceUrl: global.EDITOR_SOURCE_URL,
        script: 'atp:/' + fileName,
        serverScripts: 'atp:/' + fileName,
        dimensions: {x: global.EDITOR_WIDTH, y: global.EDITOR_HEIGHT, z: 0.01},
        userData: JSON.stringify(userData)
      });
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

  /* ------------------------------------------------ */

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
        source: global.EDITOR_SOURCE_URL,
        width: global.EDITOR_WIDTH * 400,
        height: global.EDITOR_HEIGHT * 400
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

    if (url.match(/(file|https?):\/\//)) {
      fileName = this.getFileNameFromUrl(url);
    }

    // Script will be copied to the Asset Server
    var answer = Window.prompt(
        "The script will be copied to the Asset Server "
        + "and may overwrite some other files. Enter file name.", fileName);
    if (!answer) {
      return;
    }
    fileName = answer;

    var content = url || ' ';
    if (url.match(/(file|https?):\/\//)) {
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

  /* ------------------------------------------------ */

  function EditorClient() {
    this.subscriber = new Subscriber(this);
    this.remotelyCallable = [
      'emitWebEvent'
    ];
  }

  EditorClient.prototype.preload = function (entityId) {
    var self = this;

    this.entityId = entityId;
    this.channelName = global.CHANNEL_NAME.replace('{id}', entityId);

    Messages.subscribe(this.channelName);
    this.subscriber.subscribe(Entities.webEventReceived, 'onWebEventReceived');
    this.subscriber.subscribe(Messages.messageReceived, 'onMessageReceived');

    Script.setTimeout(function () {
      var userData = Editor.parseUserData(entityId);

      self.statusMonitor = new StatusMonitor(userData.editingEntityId, function (status) {
        self.updateScriptStatus(status);
      });

      self.consoleMonitor = new ConsoleMonitor(function (type, message, scriptName) {
        self.appendLog(type, message, scriptName);
      });

      self.consoleMonitor.subscribe();
    }, global.INIT_ENTITIES_DELAY);
  };

  EditorClient.prototype.unload = function () {
    Messages.unsubscribe(this.channelName);
    this.subscriber.unsubscribe();

    if (this.statusMonitor) {
      this.statusMonitor.stop();
    }
    if (this.consoleMonitor) {
      this.consoleMonitor.unsubscribe();
    }
  };

  EditorClient.prototype.emitWebEvent = function (_id, params) {
    var message = params[0];
    Entities.emitScriptEvent(this.entityId, message);
  };

  EditorClient.prototype.onWebEventReceived = function (id, message) {
    var action;
    if (id !== this.entityId) {
      return;
    }

    try {
      action = JSON.parse(message);
    } catch (e) {
      return;
    }

    switch (action.type) {
      case 'INITIALIZE':
        this.emitToWebView({type: 'SET_TOOLBAR_BUTTONS', showClose: true, showOpenInEntity: false});
        this.callServer('initialize');
        break;
      case 'UPDATE':
        this.callServer('update', [JSON.stringify(action)]);
        break;
      case 'SET_SCROLL':
        this.sendToAll(action);
        break;
      case 'SAVE':
        this.callServer('save');
        break;
      case 'RELOAD':
        this.callServer('load');
        break;
      case 'RUN':
        this.callServer('runScript');
        break;
      case 'STOP':
        this.callServer('stopScript');
        break;
      case 'CLOSE':
        Entities.deleteEntity(this.entityId);
        break;
    }
  };

  EditorClient.prototype.onMessageReceived = function (channel, message, senderId, localOnly) {
    if (channel !== this.channelName) {
      return;
    }
    Entities.emitScriptEvent(this.entityId, message);
  };

  EditorClient.prototype.sendToAll = function (action) {
    Messages.sendMessage(this.channelName, JSON.stringify(action));

  };

  EditorClient.prototype.callServer = function (method, params) {
    if (params === undefined) {
      params = [];
    }
    params.unshift(MyAvatar.sessionUUID);
    Entities.callEntityServerMethod(this.entityId, method, params);
  };

  EditorClient.prototype.emitToWebView = function (action) {
    var message = JSON.stringify(action);
    Entities.emitScriptEvent(this.entityId, message);
  };

  EditorClient.prototype.updateScriptStatus = function (status) {
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

  EditorClient.prototype.appendLog = function (severity, message, scriptName) {
    if (severity === 'ERROR') {
      this.emitToWebView({type: 'LOG_ERROR', error: message});
      return;
    }
    this.emitToWebView({type: 'LOG_INFO', items: [message]});
  };

  /* ------------------------------------------------ */

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
    this.channelName = global.CHANNEL_NAME.replace('{id}', entityId);

    Script.setTimeout(function () {
      var userData = Editor.parseUserData(entityId);
      self.editor = new Editor(userData);
      self.load(entityId, [undefined]);
    }, global.INIT_ENTITIES_DELAY);
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

  /* ------------------------------------------------ */

  return (function () {
    if (Script.context === 'entity_server') {
      return new EditorServer();
    }

    if (params !== undefined) {
      return new EditorWindow(params.entityId, params.isClient);
    }

    return new EditorClient();
  }());

});
