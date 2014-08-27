var modulename = require('./build/Release/modulename');

console.log(modulename);
/*var obj = new modulename.MyObject(42);
console.warn(obj);
console.warn(obj.value());
*/
var x = new modulename.X();
console.log(x.foo(10,20));