print("Script.require: " + JSON.stringify(Script.require));
print("Script.require.cache: " + JSON.stringify(Script.require.cache));
print(JSON.stringify("Script.require.test_prop before defining: " + Script.require.test_prop));
Script.require.test_prop = "test property";
print(JSON.stringify("Script.require.test_prop after defining: " + Script.require.test_prop));
print("Before require");
// TODO: find correct local path for the test module
Script.require("http://oaktown.pl/scripts/001_test_print.js");
print("After require");
