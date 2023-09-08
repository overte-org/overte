"use strict";

/* global Script */

(function (global) {

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

  global.ConsoleMonitor = ConsoleMonitor;

}(typeof module !== 'undefined' ? module.exports : new Function('return this;')()));
