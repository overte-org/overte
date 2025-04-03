"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
function defineTags(dictionary) {
    dictionary.defineTag("template", {
        onTagged: function (doclet, tag) {
            doclet.tags = doclet.tags || [];
            doclet.tags.push(tag);
        }
    });
}
exports.defineTags = defineTags;
;
const regexTypeOf = /typeof\s+([^\|\}]+)/g;
exports.handlers = {
    jsdocCommentFound: function (event) {
        let oldResult = "";
        let newResult = event.comment || "";
        while (newResult !== oldResult) {
            oldResult = newResult;
            newResult = newResult.replace(regexTypeOf, (typeExpression, className) => "Class<" + className + ">");
        }
        event.comment = newResult;
    }
};
//# sourceMappingURL=plugin.js.map