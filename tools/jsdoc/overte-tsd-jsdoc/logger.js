"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const header = '[TSD-JSDoc]';
let isVerbose = false;
function setVerbose(value) {
    isVerbose = value;
}
exports.setVerbose = setVerbose;
function warn(msg, data) {
    if (typeof (console) === 'undefined')
        return;
    let prefix = header;
    if (data && data.meta) {
        const meta = data.meta;
        prefix = `${prefix} ${meta.filename}:${meta.lineno}:${meta.columnno}`;
    }
    console.warn(`${prefix} ${msg}`);
    if (isVerbose && arguments.length > 1) {
        console.warn(data);
    }
    if (isDebug) {
        console.log(`${header} WARN: ${msg}`);
        if (arguments.length > 1) {
            console.log(data);
        }
    }
}
exports.warn = warn;
let isDebug = false;
function setDebug(value) {
    isDebug = value;
}
exports.setDebug = setDebug;
function debug(msg, data) {
    if (typeof (console) === 'undefined')
        return;
    if (isDebug) {
        console.log(`${header} DEBUG: ${msg}`);
        if (arguments.length > 1) {
            console.log(data);
        }
    }
}
exports.debug = debug;
function docletDebugInfo(doclet) {
    let debugInfo = `{longname='${doclet.longname}', kind='${doclet.kind}'`;
    if ((doclet.kind !== 'package') && doclet.meta) {
        if (doclet.meta.code.id)
            debugInfo += `, id='${doclet.meta.code.id}'`;
        if (doclet.meta.range)
            debugInfo += `, range=[${doclet.meta.range[0]}-${doclet.meta.range[1]}]`;
        else if (doclet.meta.lineno)
            debugInfo += `, lineno=${doclet.meta.lineno}`;
    }
    debugInfo += `}`;
    return debugInfo;
}
exports.docletDebugInfo = docletDebugInfo;
//# sourceMappingURL=logger.js.map