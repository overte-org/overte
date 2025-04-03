"use strict";
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (Object.hasOwnProperty.call(mod, k)) result[k] = mod[k];
    result["default"] = mod;
    return result;
};
Object.defineProperty(exports, "__esModule", { value: true });
const ts = __importStar(require("typescript"));
const logger_1 = require("./logger");
const assert_never_1 = require("./assert_never");
const doclet_utils_1 = require("./doclet_utils");
const create_helpers_1 = require("./create_helpers");
const type_resolve_helpers_1 = require("./type_resolve_helpers");
function shouldMoveOutOfClass(doclet) {
    return doclet_utils_1.isClassDoclet(doclet)
        || doclet_utils_1.isNamespaceDoclet(doclet)
        || doclet_utils_1.isEnumDoclet(doclet)
        || doclet.kind === 'typedef';
}
class Emitter {
    constructor(options) {
        this.options = options;
        this.results = [];
        this._treeRoots = [];
        this._treeNodes = {};
    }
    parse(docs) {
        logger_1.debug(`Emitter.parse()`);
        this.results = [];
        this._treeRoots = [];
        this._treeNodes = {};
        if (!docs)
            return;
        this._createTreeNodes(docs);
        this._buildTree(docs);
        if (this.options.generationStrategy === 'exported')
            this._markExported();
        this._parseTree();
    }
    emit() {
        console.log(`----------------------------------------------------------------`);
        console.log(`Emitter.emit()`);
        const resultFile = ts.createSourceFile('types.d.ts', '', ts.ScriptTarget.Latest, false, ts.ScriptKind.TS);
        const printer = ts.createPrinter({
            removeComments: false,
            newLine: ts.NewLineKind.LineFeed,
        });
        let out2 = '';
        for (let i = 0; i < this.results.length; ++i) {
            out2 += printer.printNode(ts.EmitHint.Unspecified, this.results[i], resultFile);
            out2 += '\n\n';
        }
        return out2;
    }
    _createTreeNodes(docs) {
        logger_1.debug(`----------------------------------------------------------------`);
        logger_1.debug(`Emitter._createTreeNodes()`);
        for (let i = 0; i < docs.length; ++i) {
            const doclet = docs[i];
            if (doclet.kind === 'package') {
                logger_1.debug(`Emitter._createTreeNodes(): skipping ${logger_1.docletDebugInfo(doclet)} (package)`);
                continue;
            }
            const node = this._treeNodes[doclet.longname];
            if (!node) {
                logger_1.debug(`Emitter._createTreeNodes(): adding ${logger_1.docletDebugInfo(doclet)} to this._treeNodes`);
                this._treeNodes[doclet.longname] = { doclet, children: [] };
            }
            else {
                logger_1.debug(`Emitter._createTreeNodes(): skipping ${logger_1.docletDebugInfo(doclet)} (doclet name already known)`);
            }
        }
    }
    _buildTree(docs) {
        logger_1.debug(`----------------------------------------------------------------`);
        logger_1.debug(`Emitter._buildTree()`);
        for (let i = 0; i < docs.length; ++i) {
            const doclet = docs[i];
            this._buildTreeNode(docs, doclet);
        }
    }
    _buildTreeNode(docs, doclet) {
        if (doclet.kind === 'package') {
            logger_1.debug(`Emitter._buildTreeNode(): skipping ${logger_1.docletDebugInfo(doclet)} (package)`);
            return;
        }
        if (doclet_utils_1.isClassDoclet(doclet) && doclet_utils_1.isConstructorDoclet(doclet)) {
            const ownerClass = this._getNodeFromLongname(doclet.longname, (node) => doclet_utils_1.isClassDeclarationDoclet(node.doclet));
            if (!ownerClass) {
                logger_1.warn(`Failed to find owner class of constructor '${doclet.longname}'.`, doclet);
                return;
            }
            if (this._checkDuplicateChild(doclet, ownerClass, (child) => doclet_utils_1.isConstructorDoclet(child.doclet)))
                return;
            logger_1.debug(`Emitter._buildTreeNode(): adding constructor ${logger_1.docletDebugInfo(doclet)} to class declaration ${logger_1.docletDebugInfo(ownerClass.doclet)}`);
            ownerClass.children.push({ doclet: doclet, children: [] });
            if (!doclet_utils_1.hasParamsDoclet(doclet) && doclet_utils_1.isClassDoclet(ownerClass.doclet) && doclet_utils_1.hasParamsDoclet(ownerClass.doclet)) {
                logger_1.debug(`Emitter._buildTreeNode(): inheriting 'params' from owner class ${logger_1.docletDebugInfo(ownerClass.doclet)} for undocumented constructor ${logger_1.docletDebugInfo(doclet)}`);
                doclet.params = ownerClass.doclet.params;
            }
            return;
        }
        let interfaceMerge = null;
        if (doclet.kind === 'class') {
            const impls = doclet.implements || [];
            const mixes = doclet.mixes || [];
            const extras = impls.concat(mixes);
            if (extras.length) {
                const longname = this._getInterfaceKey(doclet.longname);
                interfaceMerge = this._treeNodes[longname] = {
                    doclet: {
                        kind: 'interface',
                        name: doclet.name,
                        scope: doclet.scope,
                        longname: longname,
                        augments: extras,
                        memberof: doclet.memberof,
                    },
                    children: [],
                };
                logger_1.debug(`Emitter._buildTreeNode(): merge interface ${logger_1.docletDebugInfo(interfaceMerge.doclet)} created for ${logger_1.docletDebugInfo(doclet)}`);
            }
        }
        let namespaceMerge = null;
        if (doclet.kind === 'interface' || doclet.kind === 'mixin') {
            const staticChildren = docs.filter(d => d.memberof === doclet.longname && d.scope === 'static');
            if (staticChildren.length) {
                const longname = this._getNamespaceKey(doclet.longname);
                namespaceMerge = this._treeNodes[longname] = {
                    doclet: {
                        kind: 'namespace',
                        name: doclet.name,
                        scope: doclet.scope,
                        longname: longname,
                        memberof: doclet.memberof,
                    },
                    children: [],
                };
                logger_1.debug(`Emitter._buildTreeNode(): merge namespace ${logger_1.docletDebugInfo(namespaceMerge.doclet)} created for ${logger_1.docletDebugInfo(doclet)}`);
                staticChildren.forEach(c => c.memberof = longname);
            }
        }
        doclet_utils_1.isDefaultExportDoclet(doclet, this._treeNodes);
        if (doclet.memberof) {
            const parent = this._getNodeFromLongname(doclet.memberof, function (node) {
                if (doclet.scope === 'instance')
                    return doclet_utils_1.isClassDoclet(node.doclet);
                return true;
            });
            if (!parent) {
                logger_1.warn(`Failed to find parent of doclet '${doclet.longname}' using memberof '${doclet.memberof}', this is likely due to invalid JSDoc.`, doclet);
                return;
            }
            if (doclet_utils_1.isDefaultExportDoclet(doclet, this._treeNodes)) {
                if (doclet.meta && doclet.meta.code.value && doclet.meta.code.value.startsWith('{')) {
                    logger_1.debug(`Emitter._buildTreeNode(): 'module.exports = {name: ... }' named export pattern doclet ${logger_1.docletDebugInfo(doclet)}: skipping doclet but scan the object members`);
                    const value = JSON.parse(doclet.meta.code.value);
                    for (const name in value) {
                        this._resolveDocletType(name, parent, function (namedExportNode) {
                            logger_1.debug(`Emitter._buildTreeNode(): tagging ${logger_1.docletDebugInfo(namedExportNode.doclet)} as a named export`);
                            namedExportNode.isNamedExport = true;
                        });
                    }
                    return;
                }
                else {
                    const thisEmitter = this;
                    if (this._checkDuplicateChild(doclet, parent, (child) => doclet_utils_1.isDefaultExportDoclet(child.doclet, thisEmitter._treeNodes)))
                        return;
                    logger_1.debug(`Emitter._buildTreeNode(): adding default export ${logger_1.docletDebugInfo(doclet)} to module ${logger_1.docletDebugInfo(parent.doclet)}`);
                    parent.children.push({ doclet: doclet, children: [] });
                    return;
                }
            }
            if (doclet_utils_1.isExportsAssignmentDoclet(doclet, this._treeNodes)) {
                logger_1.debug(`Emitter._buildTreeNode(): adding 'exports =' assignment ${logger_1.docletDebugInfo(doclet)} to module ${logger_1.docletDebugInfo(parent.doclet)}`);
                parent.children.push({ doclet: doclet, children: [] });
                return;
            }
            const obj = this._treeNodes[doclet.longname];
            if (!obj) {
                logger_1.warn('Failed to find doclet node when building tree, this is likely a bug.', doclet);
                return;
            }
            const isParentClassLike = doclet_utils_1.isClassDoclet(parent.doclet);
            if (isParentClassLike && shouldMoveOutOfClass(doclet)) {
                logger_1.debug(`Emitter._buildTreeNode(): move out of class!`);
                const mod = this._getOrCreateClassNamespace(parent);
                if (interfaceMerge) {
                    logger_1.debug(`Emitter._buildTreeNode(): adding ${logger_1.docletDebugInfo(interfaceMerge.doclet)} to ${logger_1.docletDebugInfo(mod.doclet)}`);
                    mod.children.push(interfaceMerge);
                }
                if (namespaceMerge) {
                    logger_1.debug(`Emitter._buildTreeNode(): adding ${logger_1.docletDebugInfo(namespaceMerge.doclet)} to ${logger_1.docletDebugInfo(mod.doclet)}`);
                    mod.children.push(namespaceMerge);
                }
                logger_1.debug(`Emitter._buildTreeNode(): adding ${logger_1.docletDebugInfo(obj.doclet)} to ${logger_1.docletDebugInfo(mod.doclet)}`);
                mod.children.push(obj);
            }
            else {
                if (this._checkDuplicateChild(doclet, parent, function (child) {
                    if (child.doclet.kind !== doclet.kind)
                        return false;
                    if (child.doclet.longname === doclet.longname)
                        return true;
                    const shortname = doclet.name || '';
                    const optionalLongname = doclet.longname.slice(0, doclet.longname.length - shortname.length) + `[${shortname}]`;
                    if (child.doclet.longname === optionalLongname)
                        return true;
                    return false;
                }))
                    return;
                const isObjModuleLike = doclet_utils_1.isNamespaceDoclet(doclet);
                const isParentModuleLike = doclet_utils_1.isNamespaceDoclet(parent.doclet);
                if (isObjModuleLike && isParentModuleLike) {
                    logger_1.debug(`Emitter._buildTreeNode(): nested modules / namespaces!`);
                    obj.isNested = true;
                }
                const isParentEnum = doclet_utils_1.isEnumDoclet(parent.doclet);
                if (!isParentEnum) {
                    if (interfaceMerge) {
                        logger_1.debug(`Emitter._buildTreeNode(): adding ${logger_1.docletDebugInfo(interfaceMerge.doclet)} to ${logger_1.docletDebugInfo(parent.doclet)}`);
                        parent.children.push(interfaceMerge);
                    }
                    if (namespaceMerge) {
                        logger_1.debug(`Emitter._buildTreeNode(): adding ${logger_1.docletDebugInfo(namespaceMerge.doclet)} to ${logger_1.docletDebugInfo(parent.doclet)}`);
                        parent.children.push(namespaceMerge);
                    }
                    logger_1.debug(`Emitter._buildTreeNode(): adding ${logger_1.docletDebugInfo(obj.doclet)} to ${logger_1.docletDebugInfo(parent.doclet)}`);
                    parent.children.push(obj);
                }
            }
        }
        else {
            const obj = this._treeNodes[doclet.longname];
            if (!obj) {
                logger_1.warn('Failed to find doclet node when building tree, this is likely a bug.', doclet);
                return;
            }
            if (interfaceMerge) {
                logger_1.debug(`Emitter._buildTreeNode(): ${logger_1.docletDebugInfo(interfaceMerge.doclet)} detected as a root`);
                this._treeRoots.push(interfaceMerge);
            }
            if (namespaceMerge) {
                logger_1.debug(`Emitter._buildTreeNode(): ${logger_1.docletDebugInfo(namespaceMerge.doclet)} detected as a root`);
                this._treeRoots.push(namespaceMerge);
            }
            logger_1.debug(`Emitter._buildTreeNode(): ${logger_1.docletDebugInfo(obj.doclet)} detected as a root`);
            this._treeRoots.push(obj);
        }
    }
    _checkDuplicateChild(doclet, parent, match) {
        for (const child of parent.children) {
            if (match(child)) {
                if (!doclet_utils_1.isDocumentedDoclet(doclet)) {
                    logger_1.debug(`Emitter._checkConcurrentChild(): skipping undocumented ${logger_1.docletDebugInfo(doclet)} because ${logger_1.docletDebugInfo(child.doclet)} is already known in parent ${logger_1.docletDebugInfo(parent.doclet)}`);
                }
                else {
                    logger_1.debug(`Emitter._buildTreeNode(): replacing ${logger_1.docletDebugInfo(child.doclet)} with ${logger_1.docletDebugInfo(doclet)} in ${logger_1.docletDebugInfo(parent.doclet)}`);
                    child.doclet = doclet;
                }
                return true;
            }
        }
        return false;
    }
    _markExported() {
        logger_1.debug(`----------------------------------------------------------------`);
        logger_1.debug(`Emitter._markExported()`);
        this._markExportedNode = this._markExportedNode.bind(this);
        for (let i = 0; i < this._treeRoots.length; i++) {
            const node = this._treeRoots[i];
            if (node.doclet.kind === 'module')
                this._markExportedNode(node);
        }
    }
    _markExportedNode(node, markThisNode = true) {
        logger_1.debug(`Emitter._markExportedNode(${logger_1.docletDebugInfo(node.doclet)}, markThisNode=${markThisNode})`);
        const doProcessNode = (node.isExported === undefined);
        if (markThisNode)
            node.isExported = true;
        else if (!node.isExported)
            node.isExported = false;
        if (!doProcessNode)
            return;
        switch (node.doclet.kind) {
            case 'class':
            case 'interface':
            case 'mixin':
                this._markExportedParams(node, node.doclet.params);
                if (node.doclet.augments)
                    for (const augment of node.doclet.augments)
                        this._resolveDocletType(augment, node, this._markExportedNode);
                if (node.doclet.implements)
                    for (const implement of node.doclet.implements)
                        this._resolveDocletType(implement, node, this._markExportedNode);
                if (node.doclet.mixes)
                    for (const mix of node.doclet.mixes)
                        this._resolveDocletType(mix, node, this._markExportedNode);
                this._markExportedChildren(node);
                break;
            case 'file':
                this._markExportedParams(node, node.doclet.params);
                break;
            case 'event':
                this._markExportedParams(node, node.doclet.params);
                break;
            case 'callback':
            case 'function':
                if (node.doclet.this)
                    this._resolveDocletType(node.doclet.this, node, this._markExportedNode);
                this._markExportedParams(node, node.doclet.params);
                this._markExportedReturns(node, node.doclet.returns);
                break;
            case 'member':
            case 'constant':
                if (doclet_utils_1.isDefaultExportDoclet(node.doclet, this._treeNodes)) {
                    if (node.doclet.meta && node.doclet.meta.code.value) {
                        this._resolveDocletType(node.doclet.meta.code.value, node, this._markExportedNode);
                    }
                }
                else if (doclet_utils_1.isNamedExportDoclet(node.doclet, this._treeNodes)
                    && node.doclet.meta && node.doclet.meta.code.value
                    && (!doclet_utils_1.isEnumDoclet(node.doclet))) {
                    const thisEmitter = this;
                    this._resolveDocletType(node.doclet.meta.code.value, node, function (refNode) {
                        const markThisNode = node.doclet.meta && (node.doclet.meta.code.value === node.doclet.name);
                        thisEmitter._markExportedNode(refNode, markThisNode);
                    }, function (target) {
                        return (target !== node);
                    });
                }
                else {
                    this._markExportedTypes(node, node.doclet.type);
                }
                break;
            case 'module':
                for (const child of node.children) {
                    if (doclet_utils_1.isDefaultExportDoclet(child.doclet, this._treeNodes)
                        || doclet_utils_1.isNamedExportDoclet(child.doclet, this._treeNodes)) {
                        this._markExportedNode(child);
                    }
                }
                break;
            case 'namespace':
                this._markExportedChildren(node);
                break;
            case 'typedef':
                this._markExportedTypes(node, node.doclet.type);
                this._markExportedParams(node, node.doclet.params);
                this._markExportedReturns(node, node.doclet.returns);
                break;
            case 'signal':
                break;
            default:
                return assert_never_1.assertNever(node.doclet);
        }
    }
    _markExportedTypes(node, types) {
        if (types) {
            for (const typeName of types.names) {
                this._resolveDocletType(typeName, node, this._markExportedNode);
            }
        }
    }
    _markExportedParams(node, params) {
        if (params) {
            for (const param of params) {
                if (param.type) {
                    for (const paramType of param.type.names) {
                        this._resolveDocletType(paramType, node, this._markExportedNode);
                    }
                }
            }
        }
    }
    _markExportedReturns(node, returns) {
        if (returns) {
            for (const ret of returns) {
                for (const retType of ret.type.names) {
                    this._resolveDocletType(retType, node, this._markExportedNode);
                }
            }
        }
    }
    _markExportedChildren(node) {
        for (const child of node.children) {
            this._markExportedNode(child);
        }
    }
    _parseTree() {
        logger_1.debug(`----------------------------------------------------------------`);
        logger_1.debug(`Emitter._parseTree()`);
        for (let i = 0; i < this._treeRoots.length; ++i) {
            const node = this._parseTreeNode(this._treeRoots[i]);
            if (node)
                this.results.push(node);
        }
    }
    _parseTreeNode(node, parent) {
        if (this.options.generationStrategy === 'exported' && !node.isExported) {
            logger_1.debug(`Emitter._parseTreeNode(${logger_1.docletDebugInfo(node.doclet)}): skipping doclet, not exported`);
            return null;
        }
        if (!node.exportName
            && this._ignoreDoclet(node.doclet)) {
            logger_1.debug(`Emitter._parseTreeNode(${logger_1.docletDebugInfo(node.doclet)}): skipping ignored doclet`);
            return null;
        }
        logger_1.debug(`Emitter._parseTreeNode(${logger_1.docletDebugInfo(node.doclet)}, parent=${parent ? logger_1.docletDebugInfo(parent.doclet) : parent})`);
        const children = [];
        if (children) {
            for (let i = 0; i < node.children.length; ++i) {
                const childNode = this._parseTreeNode(node.children[i], node);
                if (childNode)
                    children.push(childNode);
            }
        }
        switch (node.doclet.kind) {
            case 'class':
                if (doclet_utils_1.isConstructorDoclet(node.doclet)) {
                    return create_helpers_1.createConstructor(node.doclet);
                }
                else {
                    return create_helpers_1.createClass(node.doclet, children, node.exportName);
                }
            case 'constant':
            case 'member':
                if (doclet_utils_1.isDefaultExportDoclet(node.doclet, this._treeNodes)
                    && node.doclet.meta
                    && node.doclet.meta.code.value) {
                    return create_helpers_1.createExportDefault(node.doclet, node.doclet.meta.code.value);
                }
                if (doclet_utils_1.isNamedExportDoclet(node.doclet, this._treeNodes)
                    && node.doclet.meta
                    && node.doclet.meta.code.value
                    && !doclet_utils_1.isEnumDoclet(node.doclet)) {
                    if (node.doclet.meta.code.value !== node.doclet.name) {
                        const thisEmitter = this;
                        let tsRes = null;
                        this._resolveDocletType(node.doclet.meta.code.value, node, function (refNode) {
                            const namedRefNode = {
                                doclet: refNode.doclet,
                                children: refNode.children,
                                isNested: refNode.isNested,
                                isExported: true,
                                exportName: node.doclet.name
                            };
                            tsRes = thisEmitter._parseTreeNode(namedRefNode, parent);
                        });
                        return tsRes;
                    }
                    else {
                        logger_1.debug(`Emitter._parseTreeNode(): skipping named export with reference of the same name`);
                        return null;
                    }
                }
                if (doclet_utils_1.isExportsAssignmentDoclet(node.doclet, this._treeNodes)) {
                    logger_1.debug(`Emitter._parseTreeNode(): skipping 'exports =' assignment`);
                    return null;
                }
                if (node.doclet.isEnum)
                    return create_helpers_1.createEnum(node.doclet, node.exportName);
                else if (parent && parent.doclet.kind === 'class')
                    return create_helpers_1.createClassMember(node.doclet);
                else if (parent && parent.doclet.kind === 'interface')
                    return create_helpers_1.createInterfaceMember(node.doclet);
                else
                    return create_helpers_1.createNamespaceMember(node.doclet, node.exportName);
            case 'callback':
            case 'function':
                if (node.doclet.memberof) {
                    const parent = this._treeNodes[node.doclet.memberof];
                    if (parent && parent.doclet.kind === 'class')
                        return create_helpers_1.createClassMethod(node.doclet);
                    else if (parent && parent.doclet.kind === 'interface')
                        return create_helpers_1.createInterfaceMethod(node.doclet);
                }
                return create_helpers_1.createFunction(node.doclet, node.exportName);
            case 'interface':
                return create_helpers_1.createInterface(node.doclet, children, node.exportName);
            case 'mixin':
                return create_helpers_1.createInterface(node.doclet, children, node.exportName);
            case 'module':
                return create_helpers_1.createModule(node.doclet, !!node.isNested, children);
            case 'namespace':
                return create_helpers_1.createNamespace(node.doclet, !!node.isNested, children, node.exportName);
            case 'typedef':
                return create_helpers_1.createTypedef(node.doclet, children, node.exportName);
            case 'file':
                return null;
            case 'event':
                return null;
            case 'signal':
                return null;
            default:
                return assert_never_1.assertNever(node.doclet);
        }
    }
    _ignoreDoclet(doclet) {
        if (doclet.kind !== 'package' && doclet_utils_1.isConstructorDoclet(doclet)) {
            return false;
        }
        let reason = undefined;
        if (doclet.kind === 'package')
            reason = 'package doclet';
        else if (!!doclet.ignore)
            reason = 'doclet with an ignore flag';
        else if (!this.options.private && doclet.access === 'private')
            reason = 'private access disabled';
        else if (doclet.kind === 'function' && (doclet.override || doclet.overrides))
            reason = 'overriding doclet';
        if (reason
            || doclet.kind === 'package') {
            logger_1.debug(`Emitter._ignoreDoclet(doclet=${logger_1.docletDebugInfo(doclet)}) => true (${reason})`);
            return true;
        }
        if (doclet.access === undefined) {
            return false;
        }
        const accessLevels = ["private", "package", "protected", "public"];
        const ignored = accessLevels.indexOf(doclet.access.toString()) < accessLevels.indexOf(this.options.access || "package");
        if (ignored) {
            logger_1.debug(`Emitter._ignoreDoclet(doclet=${logger_1.docletDebugInfo(doclet)}) => true (low access level)`);
        }
        return ignored;
    }
    _getInterfaceKey(longname) {
        return longname ? longname + '$$interface$helper' : '';
    }
    _getNamespaceKey(longname) {
        return longname ? longname + '$$namespace$helper' : '';
    }
    _getOrCreateClassNamespace(obj) {
        if (obj.doclet.kind === 'module' || obj.doclet.kind === 'namespace') {
            return obj;
        }
        const namespaceKey = this._getNamespaceKey(obj.doclet.longname);
        let mod = this._treeNodes[namespaceKey];
        if (mod) {
            return mod;
        }
        mod = this._treeNodes[namespaceKey] = {
            doclet: {
                kind: 'namespace',
                name: obj.doclet.name,
                scope: 'static',
                longname: namespaceKey,
            },
            children: [],
        };
        if (obj.doclet.memberof) {
            const parent = this._treeNodes[obj.doclet.memberof];
            if (!parent) {
                logger_1.warn(`Failed to find parent of doclet '${obj.doclet.longname}' using memberof '${obj.doclet.memberof}', this is likely due to invalid JSDoc.`, obj.doclet);
                return mod;
            }
            let parentMod = this._getOrCreateClassNamespace(parent);
            logger_1.debug(`Emitter._getOrCreateClassNamespace(): pushing ${logger_1.docletDebugInfo(mod.doclet)} as a child of ${logger_1.docletDebugInfo(parentMod.doclet)}`);
            mod.doclet.memberof = parentMod.doclet.longname;
            parentMod.children.push(mod);
        }
        else {
            logger_1.debug(`Emitter._getOrCreateClassNamespace(): no memberof, pushing ${logger_1.docletDebugInfo(mod.doclet)} as a root`);
            this._treeRoots.push(mod);
        }
        return mod;
    }
    _getNodeFromLongname(longname, filter) {
        function _debug(msg) { }
        const node = this._treeNodes[longname];
        if (!node) {
            _debug(`Emitter._getNodeFromLongname('${longname}') => null`);
            logger_1.warn(`No such doclet '${longname}'`);
            return null;
        }
        if (!filter || filter(node)) {
            _debug(`Emitter._getNodeFromLongname('${longname}') => ${logger_1.docletDebugInfo(node.doclet)}`);
            return node;
        }
        if (node.doclet.kind === 'module') {
            for (const child of node.children) {
                if (child.doclet.longname === longname && filter(child)) {
                    _debug(`Emitter._getNodeFromLongname('${longname}') => ${logger_1.docletDebugInfo(child.doclet)}`);
                    return child;
                }
            }
            _debug(`Emitter._getNodeFromLongname('${longname}') => null`);
            logger_1.warn(`No such doclet '${longname}' in module`);
            return null;
        }
        else {
            _debug(`Emitter._getNodeFromLongname('${longname}') => null`);
            logger_1.warn(`Unexpected doclet for longname '${longname}`, node.doclet);
            return null;
        }
    }
    _resolveDocletType(typeName, currentNode, callback, filter) {
        function _debug(msg) { }
        _debug(`Emitter._resolveDocletType(typeName='${typeName}', currentNode=${logger_1.docletDebugInfo(currentNode.doclet)})`);
        const tokens = type_resolve_helpers_1.generateTree(typeName);
        if (!tokens) {
            logger_1.warn(`Could not resolve type '${typeName}' in current node:`, currentNode.doclet);
            return;
        }
        tokens.dump((msg) => _debug(`Emitter._resolveDocletType(): tokens = ${msg}`));
        const thisEmitter = this;
        tokens.walkTypes(function (token) {
            _debug(`Emitter._resolveDocletType(): token = {name:${token.name}, type:${token.typeToString()}}`);
            typeName = token.name;
            if (typeName.match(/.*\[ *\].*/))
                typeName = typeName.slice(0, typeName.indexOf('['));
            _debug(`Emitter._resolveDocletType(): typeName = ${typeName}`);
            switch (typeName) {
                case 'void':
                case 'null':
                case 'string':
                case 'String':
                case 'boolean':
                case 'Boolean':
                case 'number':
                case 'Number':
                case 'function':
                case 'Function':
                case 'any':
                case 'object':
                case 'Object':
                case '*':
                case 'Array':
                case 'Union':
                case 'Promise':
                case 'HTMLElement':
                    return;
            }
            let scope = currentNode.doclet.longname;
            while (scope) {
                _debug(`Emitter._resolveDocletType(): scope='${scope}'`);
                const longnames = [
                    `${scope}.${typeName}`,
                    `${scope}~${typeName}`,
                    `${scope}~${typeName}$$interface$helper`,
                    `${scope}~${typeName}$$namespace$helper`,
                ];
                let targetFound = false;
                for (const longname of longnames) {
                    _debug(`Emitter._resolveDocletType(): trying longname '${longname}'...`);
                    const target = thisEmitter._treeNodes[longname];
                    if (target) {
                        if (filter && (!filter(target))) {
                            _debug(`Emitter._resolveDocletType(): filtered out ${logger_1.docletDebugInfo(target.doclet)}`);
                        }
                        else {
                            _debug(`Emitter._resolveDocletType(): found! ${logger_1.docletDebugInfo(target.doclet)}`);
                            callback(target);
                            targetFound = true;
                        }
                    }
                }
                if (targetFound) {
                    _debug(`Emitter._resolveDocletType(): done`);
                    return;
                }
                const scopeNode = thisEmitter._treeNodes[scope];
                if (!scopeNode)
                    break;
                for (const tsTypeParameterDeclaration of type_resolve_helpers_1.resolveTypeParameters(scopeNode.doclet)) {
                    if (tsTypeParameterDeclaration.name.text === typeName) {
                        _debug(`Emitter._resolveDocletType(): template found! in ${logger_1.docletDebugInfo(scopeNode.doclet)}`);
                        return;
                    }
                }
                scope = scopeNode.doclet.memberof;
            }
            logger_1.warn(`Could not resolve type '${typeName}' in current node:`, currentNode.doclet);
        });
    }
}
exports.Emitter = Emitter;
//# sourceMappingURL=Emitter.js.map