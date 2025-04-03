"use strict";
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (Object.hasOwnProperty.call(mod, k)) result[k] = mod[k];
    result["default"] = mod;
    return result;
};
Object.defineProperty(exports, "__esModule", { value: true });
const fs = __importStar(require("fs"));
const path = __importStar(require("path"));
const logger_1 = require("./logger");
function isDocumentedDoclet(doclet) {
    if (doclet.undocumented)
        if (doclet.comment && doclet.comment.length > 0)
            return true;
        else
            return false;
    else
        return true;
}
exports.isDocumentedDoclet = isDocumentedDoclet;
function hasParamsDoclet(doclet) {
    if (doclet.kind === 'class'
        || doclet.kind === 'interface'
        || doclet.kind === 'mixin'
        || doclet.kind === 'file'
        || doclet.kind === 'event'
        || doclet.kind === 'function'
        || doclet.kind === 'callback'
        || doclet.kind === 'typedef') {
        if (doclet.params && doclet.params.length > 0)
            return true;
    }
    return false;
}
exports.hasParamsDoclet = hasParamsDoclet;
function isClassDoclet(doclet) {
    return doclet.kind === 'class' || doclet.kind === 'interface' || doclet.kind === 'mixin';
}
exports.isClassDoclet = isClassDoclet;
function isClassDeclarationDoclet(doclet) {
    return !!(doclet.kind === 'class'
        && doclet.meta
        && (!doclet.meta.code.type
            || doclet.meta.code.type === 'ClassDeclaration'
            || doclet.meta.code.type === 'ClassExpression'));
}
exports.isClassDeclarationDoclet = isClassDeclarationDoclet;
function isConstructorDoclet(doclet) {
    return !!(doclet.kind === 'class'
        && doclet.meta
        && doclet.meta.code.type === 'MethodDefinition');
}
exports.isConstructorDoclet = isConstructorDoclet;
function isFileDoclet(doclet) {
    return doclet.kind === 'file';
}
exports.isFileDoclet = isFileDoclet;
function isEventDoclet(doclet) {
    return doclet.kind === 'event';
}
exports.isEventDoclet = isEventDoclet;
function isFunctionDoclet(doclet) {
    return doclet.kind === 'function' || doclet.kind === 'callback';
}
exports.isFunctionDoclet = isFunctionDoclet;
function isMemberDoclet(doclet) {
    return doclet.kind === 'member' || doclet.kind === 'constant';
}
exports.isMemberDoclet = isMemberDoclet;
function isNamespaceDoclet(doclet) {
    return doclet.kind === 'module' || doclet.kind === 'namespace';
}
exports.isNamespaceDoclet = isNamespaceDoclet;
function isTypedefDoclet(doclet) {
    return doclet.kind === 'typedef';
}
exports.isTypedefDoclet = isTypedefDoclet;
function isPackageDoclet(doclet) {
    return doclet.kind === 'package';
}
exports.isPackageDoclet = isPackageDoclet;
function isEnumDoclet(doclet) {
    return isMemberDoclet(doclet) && (doclet.isEnum === true);
}
exports.isEnumDoclet = isEnumDoclet;
function isDefaultExportDoclet(doclet, treeNodes) {
    if (doclet.kind !== 'module'
        && doclet.meta
        && doclet.meta.code.name === 'module.exports'
        && doclet.longname.startsWith('module:')) {
        const moduleName = doclet.memberof ? doclet.memberof : doclet.longname;
        const node = treeNodes[moduleName];
        if (node && node.doclet.kind === 'module') {
            if (!doclet.memberof) {
                doclet.memberof = node.doclet.longname;
                logger_1.debug(`isDefaultExport(): ${logger_1.docletDebugInfo(doclet)}.memberof fixed to '${doclet.memberof}'`);
            }
            if (!doclet.meta.code.value) {
                const sourcePath = path.join(doclet.meta.path, doclet.meta.filename);
                const fd = fs.openSync(sourcePath, 'r');
                if (fd < 0) {
                    logger_1.warn(`Could not read from '${sourcePath}'`);
                    return true;
                }
                const begin = doclet.meta.range[0];
                const end = doclet.meta.range[1];
                const length = end - begin;
                const buffer = Buffer.alloc(length);
                if (fs.readSync(fd, buffer, 0, length, begin) !== length) {
                    logger_1.warn(`Could not read from '${sourcePath}'`);
                    return true;
                }
                doclet.meta.code.value = buffer.toString().trim();
                if (doclet.meta.code.value.endsWith(";"))
                    doclet.meta.code.value = doclet.meta.code.value.slice(0, -1).trimRight();
                if (doclet.meta.code.value.match(/^export +default +/))
                    doclet.meta.code.value = doclet.meta.code.value.replace(/^export +default +/, "");
                logger_1.debug(`isDefaultExport(): ${logger_1.docletDebugInfo(doclet)}.meta.code.value fixed to '${doclet.meta.code.value}'`);
            }
            return true;
        }
    }
    return false;
}
exports.isDefaultExportDoclet = isDefaultExportDoclet;
function isNamedExportDoclet(doclet, treeNodes) {
    const node = treeNodes[doclet.longname];
    if (node && node.isNamedExport) {
        return true;
    }
    if (doclet.kind !== 'module'
        && doclet.meta
        && doclet.meta.code.name
        && (doclet.meta.code.name.startsWith('module.exports.') || doclet.meta.code.name.startsWith('exports.'))
        && doclet.longname.startsWith('module:')
        && doclet.memberof) {
        const parent = treeNodes[doclet.memberof];
        if (parent && parent.doclet.kind === 'module') {
            if (node) {
                node.isNamedExport = true;
            }
            return true;
        }
    }
    return false;
}
exports.isNamedExportDoclet = isNamedExportDoclet;
function isExportsAssignmentDoclet(doclet, treeNodes) {
    if (doclet.kind === 'member'
        && doclet.meta
        && doclet.meta.code.name
        && doclet.meta.code.name === 'exports'
        && doclet.longname.startsWith('module:')
        && doclet.memberof) {
        const node = treeNodes[doclet.memberof];
        if (node && node.doclet.kind === 'module')
            return true;
    }
    return false;
}
exports.isExportsAssignmentDoclet = isExportsAssignmentDoclet;
//# sourceMappingURL=doclet_utils.js.map