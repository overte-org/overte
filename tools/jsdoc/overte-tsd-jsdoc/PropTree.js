"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const logger_1 = require("./logger");
class PropTree {
    constructor(props) {
        this.roots = [];
        this.nodes = {};
        for (let i = 0; i < props.length; ++i) {
            const prop = props[i];
            if (!prop || !prop.name) {
                logger_1.warn('Encountered a property with no name, this is likely due to invalid JSDoc. Skipping.');
                continue;
            }
            const parts = prop.name.split('.');
            this.nodes[prop.name] = {
                prop,
                name: parts[parts.length - 1],
                children: [],
            };
        }
        for (let i = 0; i < props.length; ++i) {
            const prop = props[i];
            if (!prop || !prop.name)
                continue;
            const parts = prop.name.split('.');
            const obj = this.nodes[prop.name];
            if (!obj) {
                logger_1.warn('Failed to find dot-notation property in map. This is likely a bug.');
                continue;
            }
            if (parts.length > 1) {
                parts.pop();
                let parentName = parts.join('.');
                if (parentName.endsWith('[]')) {
                    parentName = parentName.substring(0, parentName.length - '[]'.length);
                }
                const parent = this.nodes[parentName];
                if (!parent) {
                    continue;
                }
                parent.children.push(obj);
            }
            else {
                this.roots.push(obj);
            }
        }
    }
}
exports.PropTree = PropTree;
//# sourceMappingURL=PropTree.js.map