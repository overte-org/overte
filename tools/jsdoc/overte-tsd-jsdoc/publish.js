"use strict";
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (Object.hasOwnProperty.call(mod, k)) result[k] = mod[k];
    result["default"] = mod;
    return result;
};
Object.defineProperty(exports, "__esModule", { value: true });
const path = __importStar(require("path"));
const fs = __importStar(require("fs"));
const helper = __importStar(require("jsdoc/util/templateHelper"));
const Emitter_1 = require("./Emitter");
const logger_1 = require("./logger");
function publish(data, opts) {
    logger_1.setVerbose(!!opts.verbose);
    logger_1.setDebug(!!opts.debug);
    if (!opts.generationStrategy) {
        opts.generationStrategy = 'documented';
    }
    logger_1.debug(`publish(): Generation strategy: '${opts.generationStrategy}'`);
    if (opts.generationStrategy === 'documented') {
        data(function () {
            if (this.undocumented) {
                if ((!this.comment) || (this.comment === '')) {
                    logger_1.debug(`publish(): ${logger_1.docletDebugInfo(this)} removed`);
                    return true;
                }
                else {
                    logger_1.debug(`publish(): ${logger_1.docletDebugInfo(this)} saved from removal`);
                }
            }
            return false;
        }).remove();
    }
    else if (opts.generationStrategy === 'exported') {
        logger_1.warn(`Note: The 'exported' generation strategy is still an experimental feature for the moment, thank you for your comprehension. `
            + `Feel free to contribute in case you find a bug.`);
    }
    const docs = data().get().filter(d => !d.inherited || d.overrides);
    const emitter = new Emitter_1.Emitter(opts);
    emitter.parse(docs);
    if (opts.destination === 'console') {
        console.log(emitter.emit());
    }
    else {
        try {
            fs.mkdirSync(opts.destination);
        }
        catch (e) {
            if (e.code !== 'EEXIST') {
                throw e;
            }
        }
        let filedata = emitter.emit().toString();
        console.log('stage1 starting preprocesing');
        filedata = filedata.replace(/(: void)/gmi, '');
        filedata = filedata.replace(/(: int)/gmi, ': number');
        filedata = filedata.replace(/(numberer)/gmi, 'Inter');
        console.log('stage2 replacing Void and int');
        filedata = filedata.replace(/([a-zA-Z])~([a-zA-Z])/g, '$1.$2');
        filedata = filedata.replace(/\b(([A-Z][a-z]+)+)-(([A-Z][a-z]+)+)\b/g, '$1$3');
        console.log('stage3 removing "-" from type names');
        filedata = filedata.replace(/\b(Vec[2-4]|Mat4|Quat)\b/g, 'T$1');
        console.log('stage4 add prefix "T" to the types');
        filedata = filedata.replace(/\bdeclare namespace T(Vec[2-4]|Mat4|Quat)/g, 'declare namespace $1');
        filedata = filedata.replace(/\bT(Vec[2-4]|Mat4|Quat)\./g, '$1.');
        console.log('stage5 Restore previous names for namespace');
        filedata = filedata.replace(/function: \(/g, 'fn: (');
        console.log('stage6 "function" is really wrong name for na variable');
        const pkgArray = helper.find(data, { kind: 'package' }) || [];
        const pkg = pkgArray[0];
        let definitionName = 'types';
        if (pkg && pkg.name) {
            definitionName = pkg.name.split('/').pop() || definitionName;
        }
        const out = path.join(opts.destination, opts.outFile || `${definitionName}.d.ts`);
        fs.writeFileSync(out, filedata);
    }
}
exports.publish = publish;
//# sourceMappingURL=publish.js.map