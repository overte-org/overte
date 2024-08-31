(function() {

    HoverContainer = function() {

    }

    HoverContainer.prototype = {
        preload: function(entityID) {
            this.entityID = entityID;

            var data = {
                action: 'add',
                id: this.entityID
            };
            Messages.sendLocalMessage('Hifi-Hand-RayPick-Blocklist', JSON.stringify(data))
        },
        unload: function() {
            var data = {
                action: 'remove',
                id: this.entityID
            };
            Messages.sendLocalMessage('Hifi-Hand-RayPick-Blocklist', JSON.stringify(data))
        }
    }

    return new HoverContainer();

})