// dtcem.js
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

dtcem.init = function () {
    DTC = new dtc.DTC();
}

dtcem.read = function (address) {
    var addr = parseInt(address, 16);
    DTC.ReadRegister(addr);
    return DTC.dataWord;
}

dtcem.write = function (address, value) {
    var addr = parseInt(address, 16);
    var val = parseInt(value, 16);
    console.log("Writing " + val + " to " + addr);
    DTC.WriteRegister(val, addr);
    console.log("Reading " + address);
    return dtcem.read(address);
}

dtcem.readResetDTC = function () {
    DTC.ReadResetDTC();
    return DTC.booleanValue;
}

dtcem.resetDTC = function () {
    DTC.ResetDTC();
    return dtcem.readResetDTC();
}

dtcem.readClearLatchedErrors = function () {
    DTC.ReadClearLatchedErrors();
    return DTC.booleanValue;
}

dtcem.clearLatchedErrors = function () {
    DTC.ClearLatchedErrors();
    return dtcem.readClearLatchedErrors();
}

dtcem.readSERDESLoopback = function (ring) {
    DTC.ReadSERDESLoopback(ring);
    return DTC.booleanValue;
}

dtcem.toggleSERDESLoopback = function (ring) {
    var val = dtcem.ReadSERDESLoopback(ring);
    if (val) {
        DTC.DisableSERDESLoopback(ring);
    } else {
        DTC.EnableSERDESLoopback(ring);
    }
    return dtcem.ReadSERDESLoopback(ring);
}

dtcem.readROCEmulator = function () {
    DTC.ReadROCEmulatorEnabled();
    return DTC.booleanValue;
}

dtcem.toggleROCEmulator = function () {
    var val = dtcem.readROCEmulator();
    if (val) {
        DTC.DisableROCEmulator();
    } else {
        DTC.EnableROCEmulator();
    }
    return dtcem.readROCEmulator();
}

dtcem.readRingEnabled = function (ring) {
    DTC.ReadRingEnabled(ring);
    return DTC.booleanValue;
}

dtcem.toggleRingEnabled = function (ring) {
    var val = dtcem.readRingEnabled(ring);
    if (val) {
        DTC.DisableRing(ring);
    } else {
        DTC.EnableRing(ring);
    }
    return dtcem.readRingEnabled(ring);
}

dtcem.readResetSERDES = function (ring) {
    DTC.ReadResetSERDES(ring);
    return DTC.booleanValue;
}

dtcem.resetSERDES = function (ring) {
    DTC.ResetSERDES(ring);
    return dtcem.readResetSERDES(ring);
}

dtcem.readSERDESRXDisparity = function (ring) {
    DTC.ReadSERDESRXDisparityError(ring);
    switch (DTC.SERDESRXDisparityError.GetData(true)) {
        case 0:
            return { low: 0, high: 0 };
        case 1:
            return { low: 1, high: 0 };
        case 2:
            return { low: 0, high: 1 };
        case 3:
            return { low: 1, high: 1 };
    }
}

dtcem.readSERDESRXCharacterError = function (ring) {
    DTC.ReadSERDESRXCharacterNotInTableError(ring);
    switch (DTC.CharacterNotInTableError.GetData(true)) {
        case 0:
            return { low: 0, high: 0 };
        case 1:
            return { low: 1, high: 0 };
        case 2:
            return { low: 0, high: 1 };
        case 3:
            return { low: 1, high: 1 };
    }
}

dtcem.readSERDESUnlockError = function (ring) {
    DTC.ReadSERDESUnlockError(ring);
    return DTC.booleanValue;
}

dtcem.readSERDESPLLLocked = function (ring) {
    DTC.ReadSERDESPLLLocked(ring);
    return DTC.booleanValue;
}

dtcem.readSERDESOverflowOrUnderflow = function (ring) {
    DTC.ReadSERDESOverflowOrUnderflow(ring);
    return DTC.booleanValue;
}

dtcem.readSERDESBufferFIFOHalfFull = function (ring) {
    DTC.ReadSERDESBufferFIFOHalfFull(ring);
    return DTC.booleanValue;
}

dtcem.readSERDESRXBufferStatus = function (ring) {
    DTC.ReadSERDESRXBufferStatus(ring);
    var output = { Nominal: 0, Empty: 0, Full: 0, Underflow: 0, Overflow: 0 };
    switch (DTC.SERDESRXBufferStatus.GetStatus()) {
        case 0:
            output.Nominal = 1;
            break;
        case 1:
            output.Empty = 1;
            break;
        case 2:
            output.Full = 1;
            break;
        case 5:
            output.Underflow = 1;
            break;
        case 6:
            output.Overflow = 1;
            break;
    }
    return output;
}

dtcem.readSERDESResetDone = function (ring) {
    DTC.ReadSERDESResetDone(ring);
    return DTC.booleanValue;
}

dtcem.readTimestampPreset = function () {
    DTC.ReadTimestampPreset();
    return DTC.timestampPreset.GetTimestamp(true);
}

dtcem.setTimestampPreset = function (preset) {
    DTC.WriteTimestampPreset(parseInt(preset,16));
    return dtcem.readTimestampPreset();
}

dtcem.readFPGAPROMProgramFIFOFull = function () {
    DTC.ReadFPGAPROMProgramFIFOFull();
    return DTC.booleanValue;
}

dtcem.readFPGAPROMReady = function () {
    DTC.ReadFPGAPROMReady();
    return DTC.booleanValue;
}

dtcem.regDump = function () {
    var dtcRegisters = {};
    dtcRegisters.Ring0 = {};
    dtcRegisters.Ring1 = {};
    dtcRegisters.Ring2 = {};
    dtcRegisters.Ring3 = {};
    dtcRegisters.Ring4 = {};
    dtcRegisters.Ring5 = {};
    dtcRegisters.Version = dtcem.read("0x9000");
    dtcRegisters.ResetDTC = dtcem.readResetDTC;
    dtcRegisters.ClearLatchedErrors = dtcem.readClearLatchedErrors();
    dtcRegisters.Ring0.SERDESLoopback = dtcem.readSERDESLoopback(0);
    dtcRegisters.Ring1.SERDESLoopback = dtcem.readSERDESLoopback(1);
    dtcRegisters.Ring2.SERDESLoopback = dtcem.readSERDESLoopback(2);
    dtcRegisters.Ring3.SERDESLoopback = dtcem.readSERDESLoopback(3);
    dtcRegisters.Ring4.SERDESLoopback = dtcem.readSERDESLoopback(4);
    dtcRegisters.Ring5.SERDESLoopback = dtcem.readSERDESLoopback(5);
    dtcRegisters.ROCEmulator = dtcem.readROCEmulator();
    dtcRegisters.Ring0.Enabled = dtcem.readRingEnabled(0);
    dtcRegisters.Ring1.Enabled = dtcem.readRingEnabled(1);
    dtcRegisters.Ring2.Enabled = dtcem.readRingEnabled(2);
    dtcRegisters.Ring3.Enabled = dtcem.readRingEnabled(3);
    dtcRegisters.Ring4.Enabled = dtcem.readRingEnabled(4);
    dtcRegisters.Ring5.Enabled = dtcem.readRingEnabled(5);
    dtcRegisters.Ring0.ResetSERDES = dtcem.readResetSERDES(0);
    dtcRegisters.Ring1.ResetSERDES = dtcem.readResetSERDES(1);
    dtcRegisters.Ring2.ResetSERDES = dtcem.readResetSERDES(2);
    dtcRegisters.Ring3.ResetSERDES = dtcem.readResetSERDES(3);
    dtcRegisters.Ring4.ResetSERDES = dtcem.readResetSERDES(4);
    dtcRegisters.Ring5.ResetSERDES = dtcem.readResetSERDES(5);
    dtcRegisters.Ring0.SERDESRXDisparity = dtcem.readSERDESRXDisparity(0);
    dtcRegisters.Ring1.SERDESRXDisparity = dtcem.readSERDESRXDisparity(1);
    dtcRegisters.Ring2.SERDESRXDisparity = dtcem.readSERDESRXDisparity(2);
    dtcRegisters.Ring3.SERDESRXDisparity = dtcem.readSERDESRXDisparity(3);
    dtcRegisters.Ring4.SERDESRXDisparity = dtcem.readSERDESRXDisparity(4);
    dtcRegisters.Ring5.SERDESRXDisparity = dtcem.readSERDESRXDisparity(5);
    dtcRegisters.Ring0.CharacterError = dtcem.readSERDESRXCharacterError(0);
    dtcRegisters.Ring1.CharacterError = dtcem.readSERDESRXCharacterError(1);
    dtcRegisters.Ring2.CharacterError = dtcem.readSERDESRXCharacterError(2);
    dtcRegisters.Ring3.CharacterError = dtcem.readSERDESRXCharacterError(3);
    dtcRegisters.Ring4.CharacterError = dtcem.readSERDESRXCharacterError(4);
    dtcRegisters.Ring5.CharacterError = dtcem.readSERDESRXCharacterError(5);
    dtcRegisters.Ring0.UnlockError = dtcem.readSERDESUnlockError(0);
    dtcRegisters.Ring1.UnlockError = dtcem.readSERDESUnlockError(1);
    dtcRegisters.Ring2.UnlockError = dtcem.readSERDESUnlockError(2);
    dtcRegisters.Ring3.UnlockError = dtcem.readSERDESUnlockError(3);
    dtcRegisters.Ring4.UnlockError = dtcem.readSERDESUnlockError(4);
    dtcRegisters.Ring5.UnlockError = dtcem.readSERDESUnlockError(5);
    dtcRegisters.Ring0.PLLLocked = dtcem.readSERDESPLLLocked(0);
    dtcRegisters.Ring1.PLLLocked = dtcem.readSERDESPLLLocked(1);
    dtcRegisters.Ring2.PLLLocked = dtcem.readSERDESPLLLocked(2);
    dtcRegisters.Ring3.PLLLocked = dtcem.readSERDESPLLLocked(3);
    dtcRegisters.Ring4.PLLLocked = dtcem.readSERDESPLLLocked(4);
    dtcRegisters.Ring5.PLLLocked = dtcem.readSERDESPLLLocked(5);
    dtcRegisters.Ring0.OverflowOrUnderflow = dtcem.readSERDESOverflowOrUnderflow(0);
    dtcRegisters.Ring1.OverflowOrUnderflow = dtcem.readSERDESOverflowOrUnderflow(1);
    dtcRegisters.Ring2.OverflowOrUnderflow = dtcem.readSERDESOverflowOrUnderflow(2);
    dtcRegisters.Ring3.OverflowOrUnderflow = dtcem.readSERDESOverflowOrUnderflow(3);
    dtcRegisters.Ring4.OverflowOrUnderflow = dtcem.readSERDESOverflowOrUnderflow(4);
    dtcRegisters.Ring5.OverflowOrUnderflow = dtcem.readSERDESOverflowOrUnderflow(5);
    dtcRegisters.Ring0.FIFOHalfFull = dtcem.readSERDESBufferFIFOHalfFull(0);
    dtcRegisters.Ring1.FIFOHalfFull = dtcem.readSERDESBufferFIFOHalfFull(1);
    dtcRegisters.Ring2.FIFOHalfFull = dtcem.readSERDESBufferFIFOHalfFull(2);
    dtcRegisters.Ring3.FIFOHalfFull = dtcem.readSERDESBufferFIFOHalfFull(3);
    dtcRegisters.Ring4.FIFOHalfFull = dtcem.readSERDESBufferFIFOHalfFull(4);
    dtcRegisters.Ring5.FIFOHalfFull = dtcem.readSERDESBufferFIFOHalfFull(5);
    dtcRegisters.Ring0.RXBufferStatus = dtcem.readSERDESRXBufferStatus(0);
    dtcRegisters.Ring1.RXBufferStatus = dtcem.readSERDESRXBufferStatus(1);
    dtcRegisters.Ring2.RXBufferStatus = dtcem.readSERDESRXBufferStatus(2);
    dtcRegisters.Ring3.RXBufferStatus = dtcem.readSERDESRXBufferStatus(3);
    dtcRegisters.Ring4.RXBufferStatus = dtcem.readSERDESRXBufferStatus(4);
    dtcRegisters.Ring5.RXBufferStatus = dtcem.readSERDESRXBufferStatus(5);
    dtcRegisters.Ring0.ResetDone = dtcem.readSERDESResetDone(0);
    dtcRegisters.Ring1.ResetDone = dtcem.readSERDESResetDone(1);
    dtcRegisters.Ring2.ResetDone = dtcem.readSERDESResetDone(2);
    dtcRegisters.Ring3.ResetDone = dtcem.readSERDESResetDone(3);
    dtcRegisters.Ring4.ResetDone = dtcem.readSERDESResetDone(4);
    dtcRegisters.Ring5.ResetDone = dtcem.readSERDESResetDone(5);
    dtcRegisters.Timestamp = dtcem.readTimestampPreset();
    dtcRegisters.PROMFIFOFull = dtcem.readFPGAPROMProgramFIFOFull();
    dtcRegisters.PROMReady = dtcem.readFPGAPROMReady();
    return dtcRegisters;
}

module.exports = dtcem;
