// DTCDriver.js
// Author: Eric Flumerfelt, FNAL/RSI
// Last Modified: October 30, 2014
// Modified By: Eric Flumerfelt
//

// Node.js "includes"
var emitter = require('events').EventEmitter;

var dtc = require('./DTC');

// So that we can send events back to serverbase
var dtcem = new emitter();
// Is the command complete?
dtcem.value = 0;
dtcem.address = 0x9000;

dtcem.init = function() {
    dtcem.DTC = new dtc.DTC();
}

dtcem.read = function(address) {
    dtcem.address = parseInt(address,16);
    dtcem.DTC.ReadRegister(dtcem.address);
    dtcem.value = dtcem.DTC.dataWord;
}

dtcem.write = function(address, value) {
    dtcem.address = parseInt(address,16);
    dtcem.value = parseInt(value,16);
    console.log("Writing " + dtcem.value + " to " + dtcem.address);
    dtcem.DTC.WriteRegister(dtcem.value, dtcem.address);
    console.log("Reading " + address);
    dtcem.read(address);
}


module.exports = dtcem;
