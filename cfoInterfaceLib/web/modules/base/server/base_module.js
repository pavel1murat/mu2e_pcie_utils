// Base_module.js : Server-side utilities module
// Author: Eric Flumerfelt, FNAL RSI
// Modified: December 23, 2014
//
// Currently Contains:
//  -- GET_ReadLog: Read the serverbase.js server.log


// Node.js "includes"
var fs = require("fs");
var Emitter = require("events").EventEmitter;

var base = new Emitter();

base.MasterInitFunction = function() {

};

base.GET_ReadLog = function() {
    console.log("Reading " + ("/tmp/serverbase.log"));
    fs.readFile("/tmp/serverbase.log", function(err, data) {
        if (err) throw err;
        base.emit("end", data);
    });
};
module.exports = function(moduleHolder) {
    moduleHolder["base"] = base;
};