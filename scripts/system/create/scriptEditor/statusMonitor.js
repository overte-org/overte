"use strict";

/* global Script, Entities */

(function (global) {

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

  global.StatusMonitor = StatusMonitor;

}(typeof module !== 'undefined' ? module.exports : new Function('return this;')()));
