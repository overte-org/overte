# Armored Chat

1. What is Armored Chat
2. User manual
    - Installation
    - Settings
    - Usability tips

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

---

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
