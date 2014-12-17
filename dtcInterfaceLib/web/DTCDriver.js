// dtcem.js
// Author: Eric Flumerfelt, FNAL/RSI
// Last Modified: October 30, 2014
// Modified By: Eric Flumerfelt
//

// Node.js "includes"
var emitter = require('events').EventEmitter;

var dtc = require('./DTC');
var DTC = new dtc.DTC();
// So that we can send events back to serverbase
var dtcem = new emitter();
var date = new Date();

dtcem.read = function (address) {
    var addr = parseInt(address, 16);
    //console.log("Reading " + address);
    dtcem.Err = DTC.ReadRegister(addr);
    return DTC.ReadDataWord();
};

dtcem.write = function (address, value) {
    var addr = parseInt(address, 16);
    var val = parseInt(value, 16);
    console.log("Writing " + val + " to " + addr);
    dtcem.Err = DTC.WriteRegister(val, addr);
    return dtcem.read(address);
};

dtcem.readResetDTC = function () {
    dtcem.Err = DTC.ReadResetDTC();
    return DTC.ReadBooleanValue();
};

dtcem.resetDTC = function () {
    dtcem.Err = DTC.ResetDTC();
    return dtcem.readResetDTC();
};

dtcem.readClearLatchedErrors = function () {
    dtcem.Err = DTC.ReadClearLatchedErrors();
    return DTC.ReadBooleanValue();
};

dtcem.toggleClearLatchedErrors = function () {
    var val = dtcem.readClearLatchedErrors();
    if (val) {
        dtcem.Err = DTC.ClearClearLatchedErrors();
    }
    else {
        dtcem.Err = DTC.ClearLatchedErrors();
    }
    return dtcem.readClearLatchedErrors();
};

dtcem.readSERDESLoopback = function (ring) {
    dtcem.Err = DTC.ReadSERDESLoopback(ring);
    return DTC.ReadBooleanValue();
};

dtcem.toggleSERDESLoopback = function (ring) {
    var val = dtcem.readSERDESLoopback(ring);
    if (val) {
        dtcem.Err = DTC.DisableSERDESLoopback(ring);
    } else {
        dtcem.Err = DTC.EnableSERDESLoopback(ring);
    }
    return dtcem.readSERDESLoopback(ring);
};

dtcem.readROCEmulator = function () {
    dtcem.Err = DTC.ReadROCEmulatorEnabled();
    return DTC.ReadBooleanValue();
}

dtcem.toggleROCEmulator = function () {
    var val = dtcem.readROCEmulator();
    if (val) {
        dtcem.Err = DTC.DisableROCEmulator();
    } else {
        dtcem.Err = DTC.EnableROCEmulator();
    }
    return dtcem.readROCEmulator();
}

dtcem.readRingEnabled = function (ring) {
    dtcem.Err = DTC.ReadRingEnabled(ring);
    return DTC.ReadBooleanValue();
}

dtcem.toggleRingEnabled = function (ring) {
    var val = dtcem.readRingEnabled(ring);
    if (val) {
        dtcem.Err = DTC.DisableRing(ring);
    } else {
        dtcem.Err = DTC.EnableRing(ring);
    }
    return dtcem.readRingEnabled(ring);
}

dtcem.readResetSERDES = function (ring) {
    dtcem.Err = DTC.ReadResetSERDES(ring);
    return DTC.ReadBooleanValue();
}

dtcem.resetSERDES = function (ring) {
    var val = dtcem.readResetSERDES(ring);
    
    dtcem.Err = DTC.ResetSERDES(ring, 100);
    return dtcem.readResetSERDES(ring);
}

dtcem.readSERDESRXDisparity = function (ring) {
    dtcem.Err = DTC.ReadSERDESRXDisparityError(ring);
    switch (DTC.ReadRXDisparityError().GetData(true)) {
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
    dtcem.Err = DTC.ReadSERDESRXCharacterNotInTableError(ring);
    switch (DTC.ReadCNITError().GetData(true)) {
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
    dtcem.Err = DTC.ReadSERDESUnlockError(ring);
    return DTC.ReadBooleanValue();
}

dtcem.readSERDESPLLLocked = function (ring) {
    dtcem.Err = DTC.ReadSERDESPLLLocked(ring);
    return DTC.ReadBooleanValue();
}

dtcem.readSERDESOverflowOrUnderflow = function (ring) {
    dtcem.Err = DTC.ReadSERDESOverflowOrUnderflow(ring);
    return DTC.ReadBooleanValue();
}

dtcem.readSERDESBufferFIFOHalfFull = function (ring) {
    dtcem.Err = DTC.ReadSERDESBufferFIFOHalfFull(ring);
    return DTC.ReadBooleanValue();
}

dtcem.readSERDESRXBufferStatus = function (ring) {
    dtcem.Err = DTC.ReadSERDESRXBufferStatus(ring);
    var output = { Nominal: 0, Empty: 0, Full: 0, Underflow: 0, Overflow: 0 };
    switch (DTC.ReadRXBufferStatus().GetStatus()) {
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
    dtcem.Err = DTC.ReadSERDESResetDone(ring);
    return DTC.ReadBooleanValue();
}

dtcem.readTimestampPreset = function () {
    dtcem.Err = DTC.ReadTimestampPreset();
    return DTC.ReadTimestamp().GetTimestamp(true);
}

dtcem.setTimestampPreset = function (preset) {
    dtcem.Err = DTC.WriteTimestampPreset(new dtc.DTC_Timestamp(parseInt(preset, 16)));
    return dtcem.readTimestampPreset();
}

dtcem.readFPGAPROMProgramFIFOFull = function () {
    dtcem.Err = DTC.ReadFPGAPROMProgramFIFOFull();
    return DTC.ReadBooleanValue();
}

dtcem.readFPGAPROMReady = function () {
    dtcem.Err = DTC.ReadFPGAPROMReady();
    return DTC.ReadBooleanValue();
}

var getRegDump = function () {
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

var regDump = {};
var rdTime = date;
rdTime.setTime(date.getTime() - 1000);
dtcem.regDump = function () {
    if (rdTime.getTime() + 1000 < new Date().getTime()) {
        console.log("Getting new regDump");
        regDump = getRegDump();
        rdTime = new Date();
    }
    
    return regDump;
};

var send = 0;
var sTime = date;
sTime.setTime(date.getTime() - 1000);
dtcem.getSendStatistics = function () {
    if (sTime.getTime() + 800 < new Date().getTime()) {
        send = dtcem.read("0x900C") / 4;
        sTime = new Date();
    }
    
    return { value: send, time: new Date() };
}

var receive = 0;
var rTime = date;
rTime.setTime(date.getTime() - 1000);
dtcem.getReceiveStatistics = function () {
    if (rTime.getTime() + 800 < new Date().getTime()) {
        receive = dtcem.read("0x9010") / 4;
        rTime = new Date();
    }
    
    return { value: receive, time: new Date() };
}

var sendP = 0;
var spTime = date;
spTime.setTime(date.getTime() - 1000);
dtcem.getSendPayloadStatistics = function () {
    if (spTime.getTime() + 800 < new Date().getTime()) {
        sendP = dtcem.read("0x9014") / 4;
        spTime = new Date();
    }
    
    return { value: sendP, time: new Date() };
}

var receiveP = 0;
var rpTime = date;
rpTime.setTime(date.getTime() - 1000);
dtcem.getReceivePayloadStatistics = function () {
    if (rpTime.getTime() + 800 < new Date().getTime()) {
        receiveP = dtcem.read("0x9018") / 4;
        rpTime = new Date();
    }
    
    return { value: receiveP, time: new Date() };
}

dtcem.startTest = function (dma, packetSize, loopback, txChecker, rxGenerator) {
    DTC.StartTest(dma, packetSize, loopback, txChecker, rxGenerator);
    return DTC.ReadBooleanValue();
};

dtcem.stopTest = function (dma) {
    DTC.StopTest(dma);
    return DTC.ReadBooleanValue();
};

dtcem.getDMAStats = function (dma, dir) {
    DTC.ReadDMAStatsData();
    var stats = DTC.ReadDMAStats(dma, dir);
    var output = {};
    console.log(stats);
    output.Throughput = stats[0].LBR;
    output.DMAActive = stats[0].LAT;
    output.DMAWait = stats[0].LWT;
    return output;
}

dtcem.getDMAState = function (dma, dir) {
    DTC.ReadDMAStateData(dma, dir);
    var state = DTC.ReadDMAState();
    var output = {};
    console.log(state);
    output.BDErrors = state.BDerrs;
    output.BDSErrors = state.BDSerrs;
    output.SWBDs = state.BDs;
    output.SWBuffs = state.Buffers;
    output.Interrupts = state.IntEnab;
    output.stats = dtcem.getDMAStats(dma, dir);
    return output;
}

dtcem.getPCIeStats = function () {
    DTC.ReadPCIeStatsData(1);
    var stats = DTC.ReadPCIeStats();
    var output = {};
    output.WritesRate = stats.Stats[0].LTX * 8 / 1e9;
    output.ReadsRate = stats.Stats[0].LRX * 8 / 1e9;
    return output;
}

dtcem.getPCIeState = function () {
    DTC.ReadPCIeStateData();
    var stats = DTC.ReadPCIeState();
    var output = {};
    switch (stats.LinkState) {
        case 0:
            output.LinkStatus = "Down";
            break;
        case 1:
            output.LinkStatus = "Up";
            break;
    }
    output.LinkWidth = stats.LinkWidth + "x";
    switch (stats.LinkSpeed) {
        case 1:
            output.LinkSpeed = "2.5";
            break;
        case 2:
            output.LinkSpeed = "5";
            break;
    }
    output.VendorID = stats.VendorId;
    output.DeviceID = stats.DeviceId;
    output.MPS = stats.MPS;
    output.MRRS = stats.MRRS;
    switch (stats.IntMode) {
        case 0:
            output.Interrupts = "None"
            break;
        case 1:
            output.Interrupts = "Legacy";
            break;
        case 2:
            output.Interrupts = "MSI";
            break;
        case 3:
            output.Interrupts = "MSI-X"
            break;

    }
    output.credits = {};
    output.credits.ph = stats.InitFCPH;
    output.credits.pd = stats.InitFCPD;
    output.credits.nph = stats.InitFCNPH;
    output.credits.npd = stats.InitFCNPD;
    output.credits.ch = stats.InitFCCplH;
    output.credits.cd = stats.InitFCCplD;
    output.stats = dtcem.getPCIeStats();
    return output;
}


function getSystemStatus() {
    var status = {};
    status.ring0 = {};
    status.ring1 = {};
    status.ring0.TX = dtcem.getDMAState(dtc.DTC_DMA_Engine_DAQ, dtc.DTC_DMA_Direction_C2S);
    status.ring0.RX = dtcem.getDMAState(dtc.DTC_DMA_Engine_DAQ, dtc.DTC_DMA_Direction_S2C);
    status.ring1.TX = dtcem.getDMAState(dtc.DTC_DMA_Engine_DCS, dtc.DTC_DMA_Direction_C2S);
    status.ring1.RX = dtcem.getDMAState(dtc.DTC_DMA_Engine_DCS, dtc.DTC_DMA_Direction_S2C);
    status.pcie = dtcem.getPCIeState();
    
    return status;
};

var systemStatus = {};
var ssTime = date;
ssTime.setTime(date.getTime() - 1000);
dtcem.systemStatus = function () {
    if (ssTime.getTime() + 1000 < new Date().getTime()) {
        console.log("Getting new SystemStatus");
        systemStatus = getSystemStatus();
        ssTime = new Date();
    }
    
    return systemStatus;
};

dtcem.getPCIeWriteRate = function () {
    dtcem.systemStatus();
    return { value: systemStatus.pcie.stats.WritesRate, time: new Date() };
};

dtcem.getPCIeReadRate = function () {
    dtcem.systemStatus();
    return {value: systemStatus.pcie.stats.ReadsRate, time: new Date()};
};

dtcem.getDMAReadRate = function (channel) {
    dtcem.systemStatus();
    var dmastats = systemStatus.ring0.RX.stats;
    if (channel === 1) {
        dmastats = systemStatus.ring1.RX.stats;
    }
    return { value: dmastats.Throughput, time: new Date() };
};

dtcem.getDMAWriteRate = function (channel) {
    dtcem.systemStatus();
    var dmastats = systemStatus.ring0.TX.stats;
    if (channel === 1) {
        dmastats = systemStatus.ring1.TX.stats;
    }
    return { value: dmastats.Throughput, time: new Date() };
};

module.exports = dtcem;
