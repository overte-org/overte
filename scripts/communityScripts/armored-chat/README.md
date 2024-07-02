# Armored Chat

1. What is Armored Chat
2. User manual
    - Installation
    - Settings
    - Usability tips
3. Development

## What is Armored Chat

Armored Chat is a chat application strictly made to communicate between players in the same domain. It is made using QML and to be as light weight as reasonably possible.

### Dependencies

AC uses the Overte [Messages](https://apidocs.overte.org/Messages.html) API to communicate.

For notifications, AC uses [notificationCore.js](https://github.com/overte-org/overte/blob/bb8bac43eadd3b20956a2ff7b0b21c28844b0f77/scripts/communityScripts/notificationCore/notificationCore.js).

## User manual

### Installation

Armored Chat is preinstalled courtesy of [defaultScripts.js](https://github.com/overte-org/overte/blob/8661e8a858663b48e8485c2cd7120dc3e2d7b87e/scripts/defaultScripts.js).

If AC is not preinstalled, or for some other reason it can not be automatically installed, you can install it manually by following [these instructions](https://github.com/overte-org/overte/blob/8661e8a858663b48e8485c2cd7120dc3e2d7b87e/scripts/defaultScripts.js) to open your script management application, and loading the script url:

```
https://raw.githubusercontent.com/overte-org/overte/master/scripts/communityScripts/armored-chat/armored_chat.js
```

---

### Settings

Armored Chat comes with basic settings for managing itself.

#### External window

This boolean setting toggles whether AC will be a in-game overlay window, or whether AC will be a external floating window.

Default is `false`.

#### Maximum saved messages

This integer represents the amount of messages to save in the AC history. More messages may be present if AC is left on long enough. This setting only sets the number of saved messages and not the maximum amount of messages that can be viewed at any time.

This means if you set the value to `5`, your history will save a maximum of 5 messages, however you will still be able to see a longer history in the session should you receive more. Once AC completely closes and fetches your message history as it initializes, you will only see the last 5 messages.

Default value is `200`

#### Erase chat history

This action immediately clears the AC history and the session. Functionally this will set the message list to a empty Array.

### Usage

AC has two chat modes: Local, and Domain. Local chat displays all other local chat messages that are within 20 units of you. Domain chat will display all other Domain messages sent though that channel regardless of distance.

AC also handles link embedding. When you send an HTTP(S) link, it will automatically parse it using Qt RichText and allow everyone to click on the message. Next to the link you will also see a "⮺" symbol. Clicking on this symbol will open the link in an external window.

### Usability tips

#### Navigation

You can scroll quickly using kinetic scrolling! Try "grabbing" the right side of messages, where the timestamp is, and flinging yourself in a direction.

#### Formatting

You can format messages using basic HTML elements. Try `<div style="color: red"> Red text! </div>` to color your text red.
Find the full list of Qt rich text tags [here](https://doc.qt.io/qt-6/richtext-html-subset.html). Please note that some of these tags may be intentionally restricted.

#### Media embedding

Images can be embedded when linked directly.

Try it out by linking to the Overte logo! `https://github.com/overte-org/overte/raw/master/interface/resources/images/brand-banner.svg`

In order for images to be embedded, URLs must end in a image filetype.
Supported filetypes are:

-   `.png`
-   `.jpg`
-   `.jpeg`
-   `.gif`
-   `.bmp`
-   `.svg`
-   `.webp`

## Development

### To QML communication

Here are the signals needed to communicate from the JavaScript core to the QML interface.

AC calls a `_emitEvent()` function that also includes a `type` key in the object. This `type` tells the QML and/or the JS core what the packet is for.
When you call the `_emitEvent()` function be sure to include the following signals as a `type`. In the examples below, the `type` is being excluded for brevity.

Example:

```json
{ type: "show_message", displayName: "username", ...}
```

#### "show_message"

This signal tells the QML to add a new message to the ListView element list.

Supply a `JSON` object.

```json
{
    "displayName": "username",
    "message": "chat message",
    "channel": "domain", // Channel to send message on. By default it should only be "domain" or "local".
    "date": "[ time and date string ]" // Optional, defaults to current time and date.
}
```

#### "clear_messages"

Clear all messages displayed in the ListView elements. Note this does not clear the history and this is only a visual erasure.

No payload required.

#### "notification"

Renders a notification to the domain channel.
The intended use is to provide updates about the domain and make the notifications accessible.

Supply a `JSON` object.

```json
{
    "message": "notification message" // Notification to render
}
```

#### "initial_settings"

Visually set the settings in the QML interface based on the supplied object.

Supply a `JSON` object.

```json
{
    "settings": {
        // JSON object of current AC settings
        "external_window": false,
        "maximum_messages": 200
    }
}
```

### To JS communication

Here are the signals needed to communicate from the QML interface to the JavaScript core. AC is developed in a way that all actions that are not style related are preformed though the JavaScript core.
This means that what ever action you want to preform must go though the JavaScript core for processing.

This is formatted the same was as the communication packets to the QML interface. Supply the following entries as "type"s in your packet.

#### "send_message"

Tell AC to broadcast a message to the domain.

Supply a `JSON` object.

```json
{
    "message": "message content", // The contents of the message to send.
    "channel": "domain" // Channel to emit the message to.
}
```

#### "setting_change"

Tell AC to change a setting. Exercise caution when using this as you can add new settings unintentionally if you are not careful.

Supply a `JSON` object

```json
{
    "setting": "external_window", // The name of the setting to change
    "value": true // The value to change the setting to
}
```

#### "action"

Tell AC to preform a generic action. This is normally reserved for functions that would get called on a button onClicked event in the QML.

Supply a `JSON` object

```json
{
    "action": "erase_history" // The action to preform
}
```

#### "initialized"

Tell AC the QML overlay has loaded successfully.
This is called to hide the overlay on creation.

No payload required.
