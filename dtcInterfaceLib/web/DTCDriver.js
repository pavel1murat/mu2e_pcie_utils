// DTCDriver.js
// Author: Eric Flumerfelt, FNAL/RSI
// Last Modified: October 30, 2014
// Modified By: Eric Flumerfelt
//

// Node.js "includes"
var emitter = require('events').EventEmitter;

var dtc = require('./DTC');
var DTC;
// So that we can send events back to serverbase
var dtcem = new emitter();

dtcem.init = function() {
    DTC = new dtc.DTC();
}

dtcem.read = function(address) {
    var addr = parseInt(address,16);
    DTC.ReadRegister(addr);
    return DTC.dataWord;
}

dtcem.write = function(address, value) {
    var addr = parseInt(address,16);
    var val = parseInt(value,16);
    console.log("Writing " + val + " to " + addr);
    DTC.WriteRegister(val, addr);
    console.log("Reading " + address);
    return dtcem.read(address);
}

dtcem.readResetDTC = function() {
    DTC.ReadResetDTC();
    return DTC.booleanValue;
}

dtcem.resetDTC = function() {
    DTC.ResetDTC();
    return dtcem.readResetDTC();
}

dtcem.readClearLatchedErrors = function() {
    DTC.ReadClearLatchedErrors();
    return DTC.booleanValue;
}

dtcem.clearLatchedErrors = function(){
    DTC.ClearLatchedErrors();
    return dtcem.readClearLatchedErrors();
}

dtcem.readSERDESLoopback = function(ring) {
    DTC.ReadSERDESLoopback(ring);
    return DTC.booleanValue;
}

dtcem.toggleSERDESLoopback = function(ring) {
    var val = dtcem.ReadSERDESLoopback(ring);
    if(val) {
	DTC.DisableSERDESLoopback(ring);
    } else {
	DTC.EnableSERDESLoopback(ring);
    }
    return dtcem.ReadSERDESLoopback(ring);
}

dtcem.readROCEmulator = function() {
    DTC.ReadROCEmulatorEnabled();
    return DTC.booleanValue;
}

dtcem.toggleROCEmulator = function() {
    var val = dtcem.readROCEmulator();
    if(val) {
	DTC.DisableROCEmulator();
    } else {
	DTC.EnableROCEmulator();
    }
    return dtcem.readROCEmulator();
}

dtcem.readRingEnabled = function(ring) {
    DTC.ReadRingEnabled(ring);
    return DTC.booleanValue;
}

dtcem.toggleRingEnabled = function(ring) {
    var val = dtcem.readRingEnabled(ring);
    if(val) {
	DTC.DisableRing(ring);
    } else {
        DTC.EnableRing(ring);
    }
    return dtcem.readRingEnabled(ring);
}

dtcem.readResetSERDES = function(ring) {
    DTC.ReadResetSERDES(ring);
    return DTC.booleanValue;
}

dtcem.resetSERDES = function(ring) {
    DTC.ResetSERDES(ring);
    return dtcem.readResetSERDES(ring);
}

dtcem.readSERDESRXDisparity = function(ring) {
    DTC.ReadSERDESRXDisparityError(ring);
    return DTC.SERDESRXDisparityError.GetData(true);
}

dtcem.readSERDESRXCharacterError = function(ring) {
    DTC.ReadSERDESRXCharacterNotInTableError(ring);
    return DTC.CharacterNotInTableError.GetData(true);
}

dtcem.readSERDESUnlockError = function(ring) {
    DTC.ReadSERDESUnlockError(ring);
    return DTC.booleanValue;
}

dtcem.readSERDESPLLLocked = function(ring) {
    DTC.ReadSERDESPLLLocked(ring);
    return DTC.booleanValue;
}

dtcem.readSERDESOverflowOrUnderflow = function(ring) {
    DTC.ReadSERDESOverflowOrUnderflow(ring);
    return DTC.booleanValue;
}

dtcem.readSERDESBufferFIFOHalfFull = function(ring) {
    DTC.ReadSERDESBufferFIFOHalfFull(ring);
    return DTC.booleanValue;
}

dtcem.readSERDESRXBufferStatus = function(ring) {
    DTC.ReadSERDESRXBufferStatus(ring);
    return DTC.SERDESRXBufferStatus.GetStatus();
}

dtcem.readSERDESResetDone = function(ring) {
    DTC.ReadSERDESResetDone(ring);
    return DTC.booleanValue;
}

dtcem.readTimestampPreset = function() {
    DTC.ReadTimestampPreset();
    return DTC.timestampPreset.GetTimestamp(true);
}

dtcem.setTimestampPreset = function(preset) {
    DTC.WriteTimestampPreset(preset);
    return dtcem.readTimestampPreset();
}

dtcem.readFPGAPROMProgramFIFOFull = function() {
    DTC.ReadFPGAPROMProgramFIFOFull();
    return DTC.booleanValue;
}

dtcem.readFPGAPROMReady = function() {
    DTC.ReadFPGAPROMReady();
    return DTC.booleanValue;
}

module.exports = dtcem;
