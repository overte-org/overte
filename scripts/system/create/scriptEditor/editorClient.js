"use strict";

/* global Messages, Script, Entities, MyAvatar, ConsoleMonitor,
 * CHANNEL_NAME, INIT_ENTITIES_DELAY, Editor, Subscriber,
 * StatusMonitor
 */

((typeof module !== 'undefined' ? module : {}).exports = function () {

  Script.include([
    './consoleMonitor.js',
    './constants.js',
    './editor.js',
    './statusMonitor.js',
    './subscriber.js'
  ]);

  function EditorClient() {
    this.subscriber = new Subscriber(this);
    this.remotelyCallable = [
      'emitWebEvent'
    ];
  }

  EditorClient.prototype.preload = function (entityId) {
    var self = this;

    this.entityId = entityId;
    this.channelName = CHANNEL_NAME.replace('{id}', entityId);

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
    }, INIT_ENTITIES_DELAY);
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

  return new EditorClient();
});
