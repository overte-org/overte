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
const doclet_utils_1 = require("./doclet_utils");
const PropTree_1 = require("./PropTree");
const type_resolve_helpers_1 = require("./type_resolve_helpers");
const declareModifier = ts.createModifier(ts.SyntaxKind.DeclareKeyword);
const constModifier = ts.createModifier(ts.SyntaxKind.ConstKeyword);
const readonlyModifier = ts.createModifier(ts.SyntaxKind.ReadonlyKeyword);
const exportModifier = ts.createModifier(ts.SyntaxKind.ExportKeyword);
const defaultModifier = ts.createModifier(ts.SyntaxKind.DefaultKeyword);
function validateClassLikeChildren(children, validate, msg) {
    if (children) {
        for (let i = children.length - 1; i >= 0; --i) {
            const child = children[i];
            if (!validate(child)) {
                logger_1.warn(`Encountered child that is not a ${msg}, this is likely due to invalid JSDoc.`, child);
                children.splice(i, 1);
            }
        }
    }
}
function validateClassChildren(children) {
    validateClassLikeChildren(children, ts.isClassElement, 'ClassElement');
}
function validateInterfaceChildren(children) {
    validateClassLikeChildren(children, ts.isTypeElement, 'TypeElement');
}
function validateModuleChildren(children) {
    if (children) {
        for (let i = children.length - 1; i >= 0; --i) {
            const child = children[i];
            if (!ts.isClassDeclaration(child)
                && !ts.isInterfaceDeclaration(child)
                && !ts.isFunctionDeclaration(child)
                && !ts.isEnumDeclaration(child)
                && !ts.isModuleDeclaration(child)
                && !ts.isTypeAliasDeclaration(child)
                && !ts.isVariableStatement(child)
                && !ts.isExportAssignment(child)) {
                logger_1.warn('Encountered child that is not a supported declaration, this is likely due to invalid JSDoc.', child);
                children.splice(i, 1);
            }
        }
    }
}
function buildName(doclet, altName) {
    if (altName)
        return ts.createIdentifier(altName);
    if (doclet.name.startsWith('exports.'))
        return ts.createIdentifier(doclet.name.replace('exports.', ''));
    return ts.createIdentifier(doclet.name);
}
function buildOptionalName(doclet, altName) {
    if (altName)
        return ts.createIdentifier(altName);
    if (doclet.meta && (doclet.meta.code.name === 'module.exports'))
        return undefined;
    if (doclet.name.startsWith('exports.'))
        return ts.createIdentifier(doclet.name.replace('exports.', ''));
    return ts.createIdentifier(doclet.name);
}
function formatMultilineComment(comment) {
    return comment.split('\n').join('\n * ');
}
function handlePropsComment(props, jsdocTagName) {
    return props.map((prop) => {
        if (prop.description) {
            let name;
            if (prop.optional) {
                if (prop.defaultvalue !== undefined) {
                    name = `[${prop.name} = ${prop.defaultvalue}]`;
                }
                else {
                    name = `[${prop.name}]`;
                }
            }
            else {
                name = prop.name;
            }
            const description = ` - ${formatMultilineComment(prop.description)}`;
            return `\n * @${jsdocTagName} ${name}${description}`;
        }
        return '';
    }).filter((value) => value !== '').join('');
}
function handleReturnsComment(doclet) {
    if (doclet_utils_1.isFunctionDoclet(doclet) && doclet.returns) {
        return doclet.returns.map((ret) => {
            if (ret.description)
                return `\n * @returns ${formatMultilineComment(ret.description)}`;
            return '';
        })
            .filter((value) => value !== '').join('');
    }
    return '';
}
function handleExamplesComment(doclet) {
    if (doclet.examples !== undefined) {
        return doclet.examples.map((example) => {
            return `\n * @example
 * ${formatMultilineComment(example)}`;
        })
            .join('');
    }
    return '';
}
function handleParamsComment(doclet) {
    if ((doclet_utils_1.isClassDoclet(doclet)
        || doclet_utils_1.isFileDoclet(doclet)
        || doclet_utils_1.isEventDoclet(doclet)
        || doclet_utils_1.isFunctionDoclet(doclet)
        || doclet_utils_1.isTypedefDoclet(doclet))
        && doclet.params) {
        return handlePropsComment(doclet.params, 'param');
    }
    return '';
}
function handlePropertiesComment(doclet) {
    if (!doclet_utils_1.isEnumDoclet(doclet) && doclet.properties) {
        return handlePropsComment(doclet.properties, 'property');
    }
    return '';
}
function handleComment(doclet, node) {
    if (doclet.comment && doclet.comment.length > 4) {
        let description = '';
        if (doclet.description) {
            description = `\n * ${formatMultilineComment(doclet.description)}`;
        }
        else if (doclet_utils_1.isClassDoclet(doclet) && doclet.classdesc) {
            if (!ts.isConstructorDeclaration(node)) {
                description = `\n * ${formatMultilineComment(doclet.classdesc)}`;
            }
        }
        const examples = handleExamplesComment(doclet);
        const properties = handlePropertiesComment(doclet);
        const params = handleParamsComment(doclet);
        const returns = handleReturnsComment(doclet);
        if (doclet_utils_1.isEnumDoclet(doclet)) {
            if (!ts.isEnumDeclaration(node)) {
                logger_1.warn(`Node is not an enum declaration, even though the doclet is. This is likely a tsd-jsdoc bug.`);
                return node;
            }
            if (doclet.properties) {
                const enumProperties = doclet.properties;
                const enumMembers = node.members;
                for (let index = 0; index < enumProperties.length; index++) {
                    const enumProperty = enumProperties[index];
                    const enumMember = enumMembers[index];
                    handleComment(enumProperty, enumMember);
                }
            }
        }
        if (description || examples || properties || params || returns) {
            let comment = `*${description}${examples}${properties}${params}${returns}
 `;
            if (doclet.kind === 'typedef') {
                comment = `*${description}${examples}${params}${returns}
 `;
            }
            const kind = ts.SyntaxKind.MultiLineCommentTrivia;
            ts.addSyntheticLeadingComment(node, kind, comment, true);
        }
    }
    return node;
}
function createClass(doclet, children, altName) {
    logger_1.debug(`createClass(${logger_1.docletDebugInfo(doclet)}, altName=${altName})`);
    validateClassChildren(children);
    const mods = [];
    if (!doclet.memberof)
        mods.push(declareModifier);
    if (doclet.meta && doclet.meta.code.name === 'module.exports')
        mods.push(exportModifier, defaultModifier);
    const members = children || [];
    const typeParams = type_resolve_helpers_1.resolveTypeParameters(doclet);
    const heritageClauses = type_resolve_helpers_1.resolveHeritageClauses(doclet, false);
    if (doclet.params) {
        if (members.filter(member => ts.isConstructorDeclaration(member)).length === 0) {
            logger_1.debug(`createClass(): no constructor set yet, adding one automatically`);
            members.unshift(ts.createConstructor(undefined, undefined, type_resolve_helpers_1.createFunctionParams(doclet), undefined));
        }
    }
    if (doclet.properties) {
        const tree = new PropTree_1.PropTree(doclet.properties);
        nextProperty: for (let i = 0; i < tree.roots.length; ++i) {
            const node = tree.roots[i];
            for (const tsProp of members.filter(member => ts.isPropertyDeclaration(member))) {
                if (tsProp.name) {
                    const propName = tsProp.name.text;
                    if (propName === node.name) {
                        logger_1.debug(`createClass(): skipping property already declared '${node.name}'`);
                        continue nextProperty;
                    }
                }
            }
            const opt = node.prop.optional ? ts.createToken(ts.SyntaxKind.QuestionToken) : undefined;
            const t = node.children.length ? type_resolve_helpers_1.createTypeLiteral(node.children, node) : type_resolve_helpers_1.resolveType(node.prop.type);
            const property = ts.createProperty(undefined, undefined, node.name, opt, t, undefined);
            if (node.prop.description) {
                let comment = `*\n * ${node.prop.description.split(/\r\s*/).join("\n * ")}\n`;
                ts.addSyntheticLeadingComment(property, ts.SyntaxKind.MultiLineCommentTrivia, comment, true);
            }
            members.push(property);
        }
    }
    return handleComment(doclet, ts.createClassDeclaration(undefined, mods, buildOptionalName(doclet, altName), typeParams, heritageClauses, members));
}
exports.createClass = createClass;
function createInterface(doclet, children, altName) {
    logger_1.debug(`createInterface(${logger_1.docletDebugInfo(doclet)}, altName=${altName})`);
    validateInterfaceChildren(children);
    const mods = doclet.memberof ? undefined : [declareModifier];
    const members = children;
    const typeParams = type_resolve_helpers_1.resolveTypeParameters(doclet);
    const heritageClauses = type_resolve_helpers_1.resolveHeritageClauses(doclet, true);
    return handleComment(doclet, ts.createInterfaceDeclaration(undefined, mods, buildName(doclet, altName), typeParams, heritageClauses, members));
}
exports.createInterface = createInterface;
function createFunction(doclet, altName) {
    logger_1.debug(`createFunction(${logger_1.docletDebugInfo(doclet)}, altName=${altName})`);
    const mods = [];
    if (!doclet.memberof)
        mods.push(declareModifier);
    if (doclet.meta && (doclet.meta.code.name === 'module.exports'))
        mods.push(exportModifier, defaultModifier);
    const params = type_resolve_helpers_1.createFunctionParams(doclet);
    const type = type_resolve_helpers_1.createFunctionReturnType(doclet);
    const typeParams = type_resolve_helpers_1.resolveTypeParameters(doclet);
    return handleComment(doclet, ts.createFunctionDeclaration(undefined, mods, undefined, buildOptionalName(doclet, altName), typeParams, params, type, undefined));
}
exports.createFunction = createFunction;
function createClassMethod(doclet) {
    logger_1.debug(`createClassMethod(${logger_1.docletDebugInfo(doclet)})`);
    const mods = [];
    const params = type_resolve_helpers_1.createFunctionParams(doclet);
    const type = type_resolve_helpers_1.createFunctionReturnType(doclet);
    const typeParams = type_resolve_helpers_1.resolveTypeParameters(doclet);
    if (!doclet.memberof)
        mods.push(declareModifier);
    if (doclet.access === 'private')
        mods.push(ts.createModifier(ts.SyntaxKind.PrivateKeyword));
    else if (doclet.access === 'protected')
        mods.push(ts.createModifier(ts.SyntaxKind.ProtectedKeyword));
    else if (doclet.access === 'public')
        mods.push(ts.createModifier(ts.SyntaxKind.PublicKeyword));
    if (doclet.scope === 'static')
        mods.push(ts.createModifier(ts.SyntaxKind.StaticKeyword));
    const [name, questionToken] = type_resolve_helpers_1.resolveOptionalFromName(doclet);
    return handleComment(doclet, ts.createMethod(undefined, mods, undefined, name, questionToken, typeParams, params, type, undefined));
}
exports.createClassMethod = createClassMethod;
function createInterfaceMethod(doclet) {
    logger_1.debug(`createInterfaceMethod(${logger_1.docletDebugInfo(doclet)})`);
    const mods = [];
    const params = type_resolve_helpers_1.createFunctionParams(doclet);
    const type = type_resolve_helpers_1.createFunctionReturnType(doclet);
    const typeParams = type_resolve_helpers_1.resolveTypeParameters(doclet);
    const [name, questionToken] = type_resolve_helpers_1.resolveOptionalFromName(doclet);
    return handleComment(doclet, ts.createMethodSignature(typeParams, params, type, name, questionToken));
}
exports.createInterfaceMethod = createInterfaceMethod;
function createEnum(doclet, altName) {
    logger_1.debug(`createEnum(${logger_1.docletDebugInfo(doclet)}, altName=${altName})`);
    const mods = [];
    const props = [];
    if (!doclet.memberof)
        mods.push(declareModifier);
    if (doclet.kind === 'constant')
        mods.push(constModifier);
    if (doclet.properties && doclet.properties.length) {
        for (let i = 0; i < doclet.properties.length; ++i) {
            const p = doclet.properties[i];
            const l = p.defaultvalue !== undefined ? ts.createLiteral(p.defaultvalue) : undefined;
            props.push(ts.createEnumMember(p.name, l));
        }
    }
    return handleComment(doclet, ts.createEnumDeclaration(undefined, mods, buildName(doclet, altName), props));
}
exports.createEnum = createEnum;
function createClassMember(doclet) {
    logger_1.debug(`createClassMember(${logger_1.docletDebugInfo(doclet)})`);
    const type = type_resolve_helpers_1.resolveType(doclet.type, doclet);
    const mods = getAccessModifiers(doclet);
    if (doclet.scope === 'static')
        mods.push(ts.createModifier(ts.SyntaxKind.StaticKeyword));
    if (doclet.kind === 'constant' || doclet.readonly)
        mods.push(readonlyModifier);
    const [name, questionToken] = type_resolve_helpers_1.resolveOptionalFromName(doclet);
    return handleComment(doclet, ts.createProperty(undefined, mods, name, questionToken, type, undefined));
}
exports.createClassMember = createClassMember;
function getAccessModifiers(doclet) {
    const mods = [];
    if (doclet.access === 'private' || doclet.access === 'package')
        mods.push(ts.createModifier(ts.SyntaxKind.PrivateKeyword));
    else if (doclet.access === 'protected')
        mods.push(ts.createModifier(ts.SyntaxKind.ProtectedKeyword));
    else if (doclet.access === 'public')
        mods.push(ts.createModifier(ts.SyntaxKind.PublicKeyword));
    return mods;
}
function createConstructor(doclet) {
    logger_1.debug(`createConstructor(${logger_1.docletDebugInfo(doclet)})`);
    return handleComment(doclet, ts.createConstructor(undefined, getAccessModifiers(doclet), type_resolve_helpers_1.createFunctionParams(doclet), undefined));
}
exports.createConstructor = createConstructor;
function createInterfaceMember(doclet) {
    logger_1.debug(`createInterfaceMember(${logger_1.docletDebugInfo(doclet)})`);
    const mods = [];
    const type = type_resolve_helpers_1.resolveType(doclet.type, doclet);
    if (doclet.kind === 'constant')
        mods.push(readonlyModifier);
    if (doclet.scope === 'static')
        mods.push(ts.createModifier(ts.SyntaxKind.StaticKeyword));
    const [name, questionToken] = type_resolve_helpers_1.resolveOptionalFromName(doclet);
    return handleComment(doclet, ts.createPropertySignature(mods, name, questionToken, type, undefined));
}
exports.createInterfaceMember = createInterfaceMember;
function createNamespaceMember(doclet, altName) {
    logger_1.debug(`createNamespaceMember(${logger_1.docletDebugInfo(doclet)})`);
    const mods = doclet.memberof ? undefined : [declareModifier];
    const flags = (doclet.kind === 'constant' || doclet.readonly) ? ts.NodeFlags.Const : undefined;
    const literalValue = doclet.defaultvalue !== undefined ? doclet.defaultvalue
        : doclet.meta && doclet.meta.code.type === 'Literal' ? doclet.meta.code.value
            : undefined;
    const initializer = (flags === ts.NodeFlags.Const && literalValue !== undefined) ? ts.createLiteral(literalValue) : undefined;
    const type = initializer ? undefined : type_resolve_helpers_1.resolveType(doclet.type, doclet);
    return handleComment(doclet, ts.createVariableStatement(mods, ts.createVariableDeclarationList([
        ts.createVariableDeclaration(buildName(doclet, altName), type, initializer)
    ], flags)));
}
exports.createNamespaceMember = createNamespaceMember;
function createExportDefault(doclet, value) {
    logger_1.debug(`createExportDefault(${logger_1.docletDebugInfo(doclet)}, '${value}')`);
    const expression = ts.createIdentifier(value);
    return handleComment(doclet, ts.createExportDefault(expression));
}
exports.createExportDefault = createExportDefault;
function createModule(doclet, nested, children) {
    logger_1.debug(`createModule(${logger_1.docletDebugInfo(doclet)})`);
    validateModuleChildren(children);
    const mods = doclet.memberof ? undefined : [declareModifier];
    let body = undefined;
    let flags = ts.NodeFlags.None;
    if (nested)
        flags |= ts.NodeFlags.NestedNamespace;
    if (children)
        body = ts.createModuleBlock(children);
    let nameStr = doclet.name;
    if ((nameStr[0] === '"' && nameStr[nameStr.length - 1] === '"')
        || (nameStr[0] === '\'' && nameStr[nameStr.length - 1] === '\'')) {
        nameStr = nameStr.substr(1, nameStr.length - 2);
    }
    const name = ts.createStringLiteral(nameStr);
    return handleComment(doclet, ts.createModuleDeclaration(undefined, mods, name, body, flags));
}
exports.createModule = createModule;
function createNamespace(doclet, nested, children, altName) {
    logger_1.debug(`createNamespace(${logger_1.docletDebugInfo(doclet)}, altName=${altName})`);
    validateModuleChildren(children);
    const mods = doclet.memberof ? undefined : [declareModifier];
    let body = undefined;
    let flags = ts.NodeFlags.Namespace;
    if (nested)
        flags |= ts.NodeFlags.NestedNamespace;
    if (children) {
        body = ts.createModuleBlock(children);
    }
    return handleComment(doclet, ts.createModuleDeclaration(undefined, mods, buildName(doclet, altName), body, flags));
}
exports.createNamespace = createNamespace;
function createTypedef(doclet, children, altName) {
    logger_1.debug(`createTypedef(${logger_1.docletDebugInfo(doclet)}, altName=${altName})`);
    const mods = doclet.memberof ? undefined : [declareModifier];
    const type = type_resolve_helpers_1.resolveType(doclet.type, doclet);
    const typeParams = type_resolve_helpers_1.resolveTypeParameters(doclet);
    return handleComment(doclet, ts.createTypeAliasDeclaration(undefined, mods, buildName(doclet, altName), typeParams, type));
}
exports.createTypedef = createTypedef;
//# sourceMappingURL=create_helpers.js.map