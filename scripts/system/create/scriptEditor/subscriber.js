"use strict";

/* global Script, Entities */

(function (global) {

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

  global.Subscriber = Subscriber;

}(typeof module !== 'undefined' ? module.exports : new Function('return this;')()));
