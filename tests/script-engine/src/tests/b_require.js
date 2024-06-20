
// b.js
var a = require('./a.js');
a.value += 1;
console.log('message from b');
module.exports = a.value;