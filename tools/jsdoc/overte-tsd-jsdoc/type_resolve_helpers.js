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
const PropTree_1 = require("./PropTree");
const rgxObjectTokenize = /(<|>|,|\(|\)|\||\{|\}|:)/;
const rgxCommaAll = /,/g;
const rgxParensAll = /\(|\)/g;
const anyTypeNode = ts.createKeywordTypeNode(ts.SyntaxKind.AnyKeyword);
const voidTypeNode = ts.createKeywordTypeNode(ts.SyntaxKind.VoidKeyword);
const strTypeNode = ts.createKeywordTypeNode(ts.SyntaxKind.StringKeyword);
var ENodeType;
(function (ENodeType) {
    ENodeType[ENodeType["GENERIC"] = 0] = "GENERIC";
    ENodeType[ENodeType["UNION"] = 1] = "UNION";
    ENodeType[ENodeType["FUNCTION"] = 2] = "FUNCTION";
    ENodeType[ENodeType["TUPLE"] = 3] = "TUPLE";
    ENodeType[ENodeType["TYPE"] = 4] = "TYPE";
    ENodeType[ENodeType["OBJECT"] = 5] = "OBJECT";
})(ENodeType || (ENodeType = {}));
class StringTreeNode {
    constructor(name, type, parent) {
        this.name = name;
        this.type = type;
        this.parent = parent;
        this.children = [];
    }
    dump(output, indent = 0) {
        output(`${'  '.repeat(indent)}name: ${this.name}, type:${this.typeToString()}`);
        this.children.forEach((child) => {
            child.dump(output, indent + 1);
        });
    }
    walkTypes(callback) {
        for (let i = 0; i < this.children.length; ++i) {
            if (this.type === ENodeType.OBJECT && (i % 2 === 0))
                continue;
            this.children[i].walkTypes(callback);
        }
        callback(this);
    }
    typeToString() {
        switch (this.type) {
            case ENodeType.GENERIC:
                return 'GENERIC';
            case ENodeType.UNION:
                return 'UNION';
            case ENodeType.FUNCTION:
                return 'FUNCTION';
            case ENodeType.TUPLE:
                return 'TUPLE';
            case ENodeType.TYPE:
                return 'TYPE';
            case ENodeType.OBJECT:
                return 'OBJECT';
            default:
                return 'UNKNOWN';
        }
    }
}
exports.StringTreeNode = StringTreeNode;
function resolveComplexTypeName(name, doclet) {
    const root = generateTree(name);
    if (!root) {
        logger_1.warn(`failed to generate tree for ${name}, defaulting to any`);
        return anyTypeNode;
    }
    return resolveTree(root);
}
exports.resolveComplexTypeName = resolveComplexTypeName;
function generateTree(name, parent = null) {
    const anyNode = new StringTreeNode('any', ENodeType.TYPE, parent);
    const parts = name.split(rgxObjectTokenize).filter(function (e) {
        return e.trim() !== '';
    });
    for (let i = 0; i < parts.length; ++i) {
        const part = parts[i].trim();
        const partUpper = part.toUpperCase();
        if (part.endsWith('.')) {
            const matchingIndex = findMatchingBracket(parts, i + 1, '<', '>');
            if (matchingIndex === -1) {
                logger_1.warn(`Unable to find matching '<', '>' brackets in '${part}', defaulting to \`any\``, name);
                return anyNode;
            }
            const node = new StringTreeNode(part.substring(0, part.length - 1), ENodeType.GENERIC, parent);
            generateTree(parts.slice(i + 2, matchingIndex).join(''), node);
            if (!parent)
                return node;
            parent.children.push(node);
            i = matchingIndex + 1;
            continue;
        }
        if (part === '(') {
            const matchingIndex = findMatchingBracket(parts, i, '(', ')');
            if (matchingIndex === -1) {
                logger_1.warn(`Unable to find matching '(', ')' brackets in '${part}', defaulting to \`any\``, name);
                return anyNode;
            }
            const node = new StringTreeNode('Union', ENodeType.UNION, parent);
            generateTree(parts.slice(i + 1, matchingIndex).join(''), node);
            if (!parent)
                return node;
            parent.children.push(node);
            i = matchingIndex + 1;
            continue;
        }
        if (part === '{') {
            const matchingIndex = findMatchingBracket(parts, i, '{', '}');
            if (matchingIndex === -1) {
                logger_1.warn(`Unable to find matching '{', '}' brackets in '${part}', defaulting to \`any\``, name);
                return anyNode;
            }
            const node = new StringTreeNode('Object', ENodeType.OBJECT, parent);
            generateTree(parts.slice(i + 1, matchingIndex).join(''), node);
            if (!parent)
                return node;
            parent.children.push(node);
            i = matchingIndex + 1;
            continue;
        }
        if (partUpper === 'FUNCTION') {
            const node = new StringTreeNode(part, ENodeType.FUNCTION, parent);
            let matchingIndex = findMatchingBracket(parts, i + 1, '(', ')');
            if (matchingIndex === -1) {
                logger_1.warn(`Unable to find matching '(', ')' brackets in '${part}', defaulting to \`any\``, name);
                return anyNode;
            }
            if (matchingIndex > i + 2)
                generateTree(parts.slice(i + 2, matchingIndex).join(''), node);
            if (parts.length > matchingIndex + 2 && parts[matchingIndex + 1] === ':') {
                generateTree(parts[matchingIndex + 2], node);
                matchingIndex += 2;
            }
            else {
                node.children.push(new StringTreeNode('void', ENodeType.TYPE, node));
            }
            if (!parent)
                return node;
            parent.children.push(node);
            i = matchingIndex + 1;
            continue;
        }
        if (part === '|' || part === ',' || part === ':') {
            continue;
        }
        const node = new StringTreeNode(part, ENodeType.TYPE, parent);
        if (part === '*')
            node.name = 'any';
        else if (partUpper === 'OBJECT')
            node.name = 'object';
        else if (partUpper === 'ARRAY')
            node.name = 'any[]';
        if (!parent)
            return node;
        parent.children.push(node);
    }
    return anyNode;
}
exports.generateTree = generateTree;
function findMatchingBracket(parts, startIndex, openBracket, closeBracket) {
    let count = 0;
    for (let i = startIndex; i < parts.length; ++i) {
        if (parts[i] === openBracket) {
            ++count;
        }
        else if (parts[i] === closeBracket) {
            if (--count === 0) {
                return i;
            }
        }
    }
    return -1;
}
function resolveTree(node, parentTypes = null) {
    const childTypes = [];
    node.children.forEach((child) => resolveTree(child, childTypes));
    const upperName = node.name.toUpperCase();
    switch (node.type) {
        case ENodeType.OBJECT:
            const objectProperties = [];
            for (var i = 0; i < node.children.length; i = i + 2) {
                let valType = childTypes[i + 1];
                if (!valType) {
                    logger_1.warn('Unable to resolve object value type, this is likely due to invalid JSDoc. Defaulting to \`any\`.', node);
                    valType = anyTypeNode;
                }
                const property = ts.createPropertySignature(undefined, ts.createIdentifier(node.children[i].name), undefined, valType, undefined);
                objectProperties.push(property);
            }
            const objectNode = ts.createTypeLiteralNode(objectProperties);
            ts.setEmitFlags(objectNode, ts.EmitFlags.SingleLine);
            if (!parentTypes)
                return objectNode;
            parentTypes.push(objectNode);
            break;
        case ENodeType.GENERIC:
            let genericNode;
            if (upperName === 'OBJECT') {
                let keyType = childTypes[0];
                if (!keyType) {
                    logger_1.warn(`Unable to resolve object key type, this is likely due to invalid JSDoc. Defaulting to \`string\`.`);
                    keyType = strTypeNode;
                }
                else if (node.children[0].type !== ENodeType.TYPE || (node.children[0].name !== 'string' && node.children[0].name !== 'number')) {
                    logger_1.warn(`Invalid object key type. It must be \`string\` or \`number\`, but got: ${node.children[0].name}. Defaulting to \`string\`.`);
                    keyType = strTypeNode;
                }
                let valType = childTypes[1];
                if (!valType) {
                    logger_1.warn('Unable to resolve object value type, this is likely due to invalid JSDoc. Defaulting to \`any\`.', node);
                    valType = anyTypeNode;
                }
                const indexParam = ts.createParameter(undefined, undefined, undefined, 'key', undefined, keyType, undefined);
                const indexSignature = ts.createIndexSignature(undefined, undefined, [indexParam], valType);
                genericNode = ts.createTypeLiteralNode([indexSignature]);
            }
            else if (upperName === 'ARRAY') {
                let valType = childTypes[0];
                if (!valType) {
                    logger_1.warn('Unable to resolve array value type, defaulting to \`any\`.', node);
                    valType = anyTypeNode;
                }
                genericNode = ts.createArrayTypeNode(valType);
            }
            else if (upperName === 'CLASS') {
                let valType = childTypes[0];
                if (!valType) {
                    logger_1.warn('Unable to resolve class value type, defaulting to \`any\`.', node);
                    valType = anyTypeNode;
                }
                genericNode = ts.createTypeQueryNode(ts.createIdentifier(node.children[0].name));
            }
            else {
                if (childTypes.length === 0) {
                    logger_1.warn('Unable to resolve generic type, defaulting to \`any\`.', node);
                    childTypes.push(anyTypeNode);
                }
                if (upperName === 'PROMISE') {
                    while (childTypes.length > 1)
                        childTypes.pop();
                }
                genericNode = ts.createTypeReferenceNode(node.name, childTypes);
            }
            if (!parentTypes)
                return genericNode;
            parentTypes.push(genericNode);
            break;
        case ENodeType.UNION:
            if (childTypes.length === 0) {
                logger_1.warn('Unable to resolve any types for union, defaulting to \`any\`.', node);
                childTypes.push(anyTypeNode);
            }
            const unionNode = ts.createUnionTypeNode(childTypes);
            if (!parentTypes)
                return unionNode;
            parentTypes.push(unionNode);
            break;
        case ENodeType.FUNCTION:
            const funcParameters = [];
            if (childTypes.length === 0 || childTypes.length === 1) {
                const anyArray = ts.createArrayTypeNode(anyTypeNode);
                const dotDotDot = ts.createToken(ts.SyntaxKind.DotDotDotToken);
                funcParameters.push(ts.createParameter(undefined, undefined, dotDotDot, 'params', undefined, anyArray, undefined));
                if (childTypes.length === 0)
                    childTypes.push(voidTypeNode);
            }
            for (var i = 0; i < node.children.length - 1; ++i) {
                const param = ts.createParameter(undefined, undefined, undefined, 'arg' + i, undefined, childTypes[i], undefined);
                funcParameters.push(param);
            }
            const functionNode = ts.createFunctionTypeNode(undefined, funcParameters, childTypes[childTypes.length - 1]);
            if (!parentTypes)
                return functionNode;
            parentTypes.push(functionNode);
            break;
        case ENodeType.TYPE:
            const typeNode = ts.createTypeReferenceNode(node.name, undefined);
            if (!parentTypes)
                return typeNode;
            parentTypes.push(typeNode);
            break;
    }
    return anyTypeNode;
}
function toKeywordTypeKind(k) {
    if (!k || k.length === 0)
        return null;
    k = k.toUpperCase();
    switch (k) {
        case 'ANY': return ts.SyntaxKind.AnyKeyword;
        case 'UNKNOWN': return ts.SyntaxKind.UnknownKeyword;
        case 'NUMBER': return ts.SyntaxKind.NumberKeyword;
        case 'BIGINT': return ts.SyntaxKind.BigIntKeyword;
        case 'OBJECT': return ts.SyntaxKind.ObjectKeyword;
        case 'BOOLEAN': return ts.SyntaxKind.BooleanKeyword;
        case 'BOOL': return ts.SyntaxKind.BooleanKeyword;
        case 'STRING': return ts.SyntaxKind.StringKeyword;
        case 'SYMBOL': return ts.SyntaxKind.SymbolKeyword;
        case 'THIS': return ts.SyntaxKind.ThisKeyword;
        case 'VOID': return ts.SyntaxKind.VoidKeyword;
        case 'UNDEFINED': return ts.SyntaxKind.UndefinedKeyword;
        case 'NULL': return ts.SyntaxKind.NullKeyword;
        case 'NEVER': return ts.SyntaxKind.NeverKeyword;
        default:
            return null;
    }
}
exports.toKeywordTypeKind = toKeywordTypeKind;
function resolveOptionalParameter(doclet) {
    if (doclet.defaultvalue || doclet.optional)
        return ts.createToken(ts.SyntaxKind.QuestionToken);
    return undefined;
}
exports.resolveOptionalParameter = resolveOptionalParameter;
function resolveVariableParameter(doclet) {
    if (doclet.variable)
        return ts.createToken(ts.SyntaxKind.DotDotDotToken);
    return undefined;
}
exports.resolveVariableParameter = resolveVariableParameter;
function resolveOptionalFromName(doclet) {
    let name = doclet.name;
    if (name.startsWith('[') && name.endsWith(']')) {
        name = name.substring(1, name.length - 1);
        return [name, ts.createToken(ts.SyntaxKind.QuestionToken)];
    }
    if (doclet.optional) {
        return [name, ts.createToken(ts.SyntaxKind.QuestionToken)];
    }
    return [name, undefined];
}
exports.resolveOptionalFromName = resolveOptionalFromName;
function getExprWithTypeArgs(identifier) {
    const expr = ts.createIdentifier(identifier);
    return ts.createExpressionWithTypeArguments(undefined, expr);
}
function resolveHeritageClauses(doclet, onlyExtend) {
    const clauses = [];
    let extensions = doclet.augments || [];
    if (onlyExtend) {
        extensions = extensions.concat(doclet.implements || []);
        extensions = extensions.concat(doclet.mixes || []);
    }
    if (extensions.length) {
        clauses.push(ts.createHeritageClause(ts.SyntaxKind.ExtendsKeyword, extensions.map(getExprWithTypeArgs)));
    }
    if (onlyExtend)
        return clauses;
    let implementations = (doclet.implements || []).concat(doclet.mixes || []);
    if (implementations.length) {
        clauses.push(ts.createHeritageClause(ts.SyntaxKind.ImplementsKeyword, implementations.map(getExprWithTypeArgs)));
    }
    return clauses;
}
exports.resolveHeritageClauses = resolveHeritageClauses;
function resolveTypeParameters(doclet) {
    const typeParams = [];
    if (doclet.tags) {
        for (let i = 0; i < doclet.tags.length; ++i) {
            const tag = doclet.tags[i];
            if (tag.title === 'template') {
                onTemplateTag(tag.text);
            }
        }
    }
    else if (doclet.comment && doclet.comment.includes('@template')) {
        logger_1.debug(`resolveTypeParameters(): jsdoc@3.6.x @template handling directly in the comment text for ${logger_1.docletDebugInfo(doclet)}`);
        for (let line of doclet.comment.split(/\r?\n/)) {
            line = line.trim();
            if (line.startsWith('*'))
                line = line.slice(1).trim();
            if (line.startsWith('@template')) {
                line = line.slice('@template'.length).trim();
                onTemplateTag(line);
            }
        }
    }
    function onTemplateTag(tagText) {
        const types = (tagText || 'T').split(',');
        for (let x = 0; x < types.length; ++x) {
            const name = types[x].trim();
            if (!name)
                continue;
            typeParams.push(ts.createTypeParameterDeclaration(name, undefined, undefined));
        }
    }
    return typeParams;
}
exports.resolveTypeParameters = resolveTypeParameters;
function resolveType(t, doclet) {
    if (!t || !t.names || t.names.length === 0) {
        if (doclet && doclet.properties)
            return resolveTypeName('object', doclet);
        if (doclet) {
            logger_1.warn(`Unable to resolve type for ${doclet.longname || doclet.name}, none specified in JSDoc. Defaulting to \`any\`.`, doclet);
        }
        else {
            logger_1.warn(`Unable to resolve type for an unnamed item, this is likely due to invalid JSDoc.` +
                ` Often this is caused by invalid JSDoc on a parameter. Defaulting to \`any\`.`, doclet);
        }
        return anyTypeNode;
    }
    if (t.names.length === 1) {
        return resolveTypeName(t.names[0], doclet);
    }
    else {
        const types = [];
        for (let i = 0; i < t.names.length; ++i) {
            types.push(resolveTypeName(t.names[i], doclet));
        }
        return ts.createUnionTypeNode(types);
    }
}
exports.resolveType = resolveType;
function resolveTypeName(name, doclet) {
    if (!name) {
        logger_1.warn('Unable to resolve type name, it is null, undefined, or empty. Defaulting to \`any\`.', doclet);
        return anyTypeNode;
    }
    if (name === '*')
        return anyTypeNode;
    const keyword = toKeywordTypeKind(name);
    if (keyword !== null) {
        if (keyword === ts.SyntaxKind.ThisKeyword)
            return ts.createThisTypeNode();
        if (keyword === ts.SyntaxKind.ObjectKeyword) {
            if (!doclet || !doclet.properties)
                return anyTypeNode;
            else
                return resolveTypeLiteral(doclet.properties);
        }
        return ts.createKeywordTypeNode(keyword);
    }
    const upperName = name.toUpperCase();
    if (upperName === 'FUNCTION' || upperName === 'FUNCTION()') {
        if (doclet && doclet.kind === 'typedef') {
            const params = createFunctionParams(doclet);
            const type = createFunctionReturnType(doclet);
            return ts.createFunctionTypeNode(undefined, params, type);
        }
        else {
            const anyArray = ts.createArrayTypeNode(anyTypeNode);
            const dotDotDot = ts.createToken(ts.SyntaxKind.DotDotDotToken);
            const param = ts.createParameter(undefined, undefined, dotDotDot, 'params', undefined, anyArray, undefined);
            return ts.createFunctionTypeNode(undefined, [param], anyTypeNode);
        }
    }
    return resolveComplexTypeName(name);
}
exports.resolveTypeName = resolveTypeName;
function resolveTypeLiteral(props) {
    if (!props)
        return ts.createTypeLiteralNode([]);
    const tree = new PropTree_1.PropTree(props);
    return createTypeLiteral(tree.roots);
}
exports.resolveTypeLiteral = resolveTypeLiteral;
function createTypeLiteral(children, parent) {
    const members = [];
    for (let i = 0; i < children.length; ++i) {
        const node = children[i];
        const opt = node.prop.optional ? ts.createToken(ts.SyntaxKind.QuestionToken) : undefined;
        const t = node.children.length ? createTypeLiteral(node.children, node) : resolveType(node.prop.type);
        const property = ts.createPropertySignature(undefined, node.name, opt, t, undefined);
        if (!parent && (node.prop.description || node.prop.defaultvalue)) {
            let comment = `*\n `;
            if (node.prop.description)
                comment += `* ${node.prop.description.split(/\r\s*/).join("\n * ")}\n `;
            if (node.prop.defaultvalue)
                comment += `* @defaultValue ${node.prop.defaultvalue}\n `;
            ts.addSyntheticLeadingComment(property, ts.SyntaxKind.MultiLineCommentTrivia, comment, true);
        }
        members.push(property);
    }
    let node = ts.createTypeLiteralNode(members);
    if (parent && parent.prop.type) {
        const names = parent.prop.type.names;
        if (names.length === 1 && names[0].toLowerCase() === 'array.<object>') {
            node = ts.createArrayTypeNode(node);
        }
    }
    return node;
}
exports.createTypeLiteral = createTypeLiteral;
function createFunctionParams(doclet) {
    const params = [];
    if ((doclet.kind === 'function' || doclet.kind === 'typedef') && doclet.this) {
        const type = resolveType({ names: [doclet.this] }, doclet);
        params.push(ts.createParameter(undefined, undefined, undefined, 'this', undefined, type, undefined));
    }
    if (!doclet.params || !doclet.params.length)
        return params;
    const tree = new PropTree_1.PropTree(doclet.params);
    for (let i = 0; i < tree.roots.length; ++i) {
        const node = tree.roots[i];
        const opt = resolveOptionalParameter(node.prop);
        const dots = resolveVariableParameter(node.prop);
        let type = node.children.length ? createTypeLiteral(node.children, node) : resolveType(node.prop.type);
        if (dots) {
            type = ts.createArrayTypeNode(type);
        }
        params.push(ts.createParameter(undefined, undefined, dots, node.name, opt, type, undefined));
    }
    return params;
}
exports.createFunctionParams = createFunctionParams;
function createFunctionReturnType(doclet) {
    if (doclet.returns && doclet.returns.length === 1) {
        return resolveType(doclet.returns[0].type, doclet);
    }
    else {
        return ts.createKeywordTypeNode(ts.SyntaxKind.VoidKeyword);
    }
}
exports.createFunctionReturnType = createFunctionReturnType;
//# sourceMappingURL=type_resolve_helpers.js.map