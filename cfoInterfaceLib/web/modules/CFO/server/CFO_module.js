// CFODriver.js
// Author: Eric Flumerfelt, FNAL/RSI
// Last Modified: March 11, 2015
// 
// This module for serverbase.js performs register I/O on the mu2e CFO board

var cfo = require('./CFO');
var cfolt = require('./CFOLibTest');
//var gmetric = require('./gmetric');
var fs = require('fs');
var emitter = require('events').EventEmitter;

var CFO = new cfo.CFO();
// So that we can send events back to serverbase
var cfoem = new emitter();
// Variables used to store persistent data
var date = new Date();
date.setTime(0);

var sTime = date,
    rTime = date,
    spTime = date,
    rpTime = date,
    rdTime = date,
    ssTime = date;
var send = 0,
    receive = 0,
    sendP = 0,
    receiveP = 0;
var regDump = {};
var systemStatus = {};
var dmatest = new cfolt.CFOLibTest();

//
// Local Functions
// Cannot be called directly by outside code
//
function logMessage(message, method, name) {
    message = "\n" + new Date().toLocaleString() + " " + name + " " + method + " " + message;
    console.log(message);
    fs.appendFileSync("/tmp/CFO.log", message);
    console.log("Done logging message");
}

function SendStatistics() {
    cfoem.GET_Receive();
    cfoem.GET_Send();
    //gmetric.send_gmetric("/etc/ganglia/gmond.conf", "PCIe Send Rate", send.toString(), "double", "B/s", "both", 15, 0, "CFO_PCIe", "mu2e DAQ", "PCIe Send Rate", "PCIe Send Rate");
    //gmetric.send_gmetric("/etc/ganglia/gmond.conf", "PCIe Receive Rate", receive.toString(), "double", "B/s", "both", 15, 0, "CFO_PCIe", "mu2e DAQ", "PCIe Receive Rate", "PCIe Receive Rate");
};

function readMaxRocs(input, ring) {
    input.ROC0Enabled = false;
    input.ROC1Enabled = false;
    input.ROC2Enabled = false;
    input.ROC3Enabled = false;
    input.ROC4Enabled = false;
    input.ROC5Enabled = false;
    
    var rocs = CFO.ReadRingROCCount(ring);
    switch (rocs) {
        case 5:
            input.ROC5Enabled = true;
        case 4:
            input.ROC4Enabled = true;
        case 3:
            input.ROC3Enabled = true;
        case 2:
            input.ROC2Enabled = true;
        case 1:
            input.ROC1Enabled = true;
        case 0:
            input.ROC0Enabled = true;
    }
    return input;
}

function getRegDump() {
    rdTime = new Date();
    regDump = JSON.parse(CFO.RegDump());
    console.log("Done with RegDump");
}

function startTest(dma, packetSize, loopback, txChecker, rxGenerator) {
    var test = CFO.StartTest(dma, packetSize, loopback, txChecker, rxGenerator);
    return test.GetState();
};

function stopTest(dma) {
    var test = CFO.StopTest(dma);
    return test.GetState();
};

function getDMAStats(dma, dir) {
    var stats = CFO.ReadDMAStats(dma, dir);
    var output = {};
    //console.log(stats.at(0));
    output.Throughput = stats.at(0).LBR;
    output.DMAActive = stats.at(0).LAT;
    output.DMAWait = stats.at(0).LWT;
    return output;
}

function getDMAState(dma, dir) {
    var state = CFO.ReadDMAState(dma, dir);
    var output = {};
    //console.log(state);
    output.BDErrors = state.BDerrs;
    output.BDSErrors = state.BDSerrs;
    output.SWBDs = state.BDs;
    output.SWBuffs = state.Buffers;
    output.Interrupts = state.IntEnab;
    output.stats = getDMAStats(dma, dir);
    return output;
}

function getPCIeStats() {
    var stats = CFO.ReadPCIeStats();
    var output = {};
    output.WritesRate = stats.LTX;
    output.ReadsRate = stats.LRX;
    return output;
}

function getPCIeState() {
    var stats = CFO.ReadPCIeState();
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
    output.stats = getPCIeStats();
    return output;
}

function getSystemStatus() {
    var status = {};
    status.path0 = {};
    status.path1 = {};
    status.path0.TX = getDMAState(cfo.CFO_DMA_Engine_DAQ, cfo.DTC_DMA_Direction_C2S);
    status.path0.RX = getDMAState(cfo.CFO_DMA_Engine_DAQ, cfo.DTC_DMA_Direction_S2C);
    status.path1.TX = getDMAState(cfo.CFO_DMA_Engine_DCS, cfo.DTC_DMA_Direction_C2S);
    status.path1.RX = getDMAState(cfo.CFO_DMA_Engine_DCS, cfo.DTC_DMA_Direction_S2C);
    status.pcie = getPCIeState();
    
    systemStatus = status;
};

function getDMAReadRate(channel) {
    cfoem.RO_SystemStatus();
    var dmastats = systemStatus.path0.RX.stats;
    if (channel === 1) {
        dmastats = systemStatus.path1.RX.stats;
    }
    return dmastats.Throughput;
};

function getDMAWriteRate(channel) {
    cfoem.RO_SystemStatus();
    var dmastats = systemStatus.path0.TX.stats;
    if (channel === 1) {
        dmastats = systemStatus.path1.TX.stats;
    }
    return dmastats.Throughput;
};

function getDMATestStatus(testStatus) {
    if (dmatest !== null) {
        testStatus.testRunning = dmatest.isRunning();
        testStatus.regPassed += dmatest.regPassed();
        testStatus.regFailed += dmatest.regFailed();
        testStatus.pciePassed += dmatest.pciePassed();
        testStatus.pcieFailed += dmatest.pcieFailed();
        testStatus.dmaPassed += dmatest.dmaStatePassed();
        testStatus.dmaFailed += dmatest.dmaStateFailed();
        testStatus.daqPassed += dmatest.daqPassed();
        testStatus.daqFailed += dmatest.daqFailed();
        testStatus.dcsPassed += dmatest.dcsPassed();
        testStatus.dcsFailed += dmatest.dcsFailed();
        testStatus.loopbackPassed += dmatest.loopbackPassed();
        testStatus.loopbackFailed += dmatest.loopbackFailed();
    } else {
        testStatus.testRunning = false;
        dmatest = new cfolt.CFOLibTest();
    }
    cfoem.emit('message', JSON.stringify(testStatus));
    return testStatus;
}

function read(address) {
    var addr = parseInt(address, 16);
    //console.log("Reading " + address);
    return CFO.RegisterRead(addr);
};

/* Register Writes have been removed as of v1.0, sorry
function write(address, value) {
    var addr = parseInt(address, 16);
    var val = parseInt(value, 16);
    console.log("Writing " + val + " to " + addr);
    cfoem.Err = CFO.WriteRegister(val, addr);
    return read(address);
};
*/

//
// Master Init Function
// Runs once on the node.js cluster master
//
cfoem.MasterInitFunction = function (workerData) {
    //Only the master should log Ganglia metrics
    setInterval(SendStatistics, 15000);
    
    console.log("Setting up log file");
    if (!fs.existsSync("/tmp/CFO.log")) {
        console.log("Creating new log file");
        fs.writeFileSync("/tmp/CFO.log", "Log file created at " + new Date().toLocaleString());
    }
    console.log("Done setting up log");
    var testStatus = {};
    testStatus.regPassed = 0;
    testStatus.regFailed = 0;
    testStatus.pciePassed = 0;
    testStatus.pcieFailed = 0;
    testStatus.dmaPassed = 0;
    testStatus.dmaFailed = 0;
    testStatus.daqPassed = 0;
    testStatus.daqFailed = 0;
    testStatus.dcsPassed = 0;
    testStatus.dcsFailed = 0;
    testStatus.loopbackPassed = 0;
    testStatus.loopbackFailed = 0;
    testStatus.testRunning = false;
    
    workerData['CFO'] = testStatus;
}


//
// libCFOInterface Wrapper Functions
// RO_: Read-only, any authenticated user can run these
// RW_: Read-write, only authorized users can run these
// 2/16/2015: Reordered functions to mimic CFO.h
//
cfoem.RO_readDesignVersion = function () {
    return CFO.ReadDesignVersion();
}

cfoem.RW_resetCFO = function (POST) {
    CFO.ResetCFO();
    logMessage("the CFO", "reset", POST.who);
    return cfoem.RO_readResetCFO();
};

cfoem.RO_readResetCFO = function () {
    return CFO.ReadResetCFO();
};

cfoem.RW_resetSERDESOscillator = function () {
    CFO.ResetSERDESOscillator();
    logMessage("the SERDES Oscillator", "reset", POST.who);
    return cfoem.RO_readResetSERDESOscillator();
}

cfoem.RO_readResetSERDESOscillator = function () {
    return CFO.ReadResetSERDESOscillator();
}

cfoem.RW_toggleSERDESOscillatorClock = function () {
    CFO.ToggleSERDESOscillatorClock();
    logMessage("the SERDES Oscillator Clock Speed", "toggled", POST.who);
    return cfoem.RO_readSERDESOscillatorClock();
}

cfoem.RO_readSERDESOscillatorClock = function () {
    return CFO.ReadSERDESOscillatorClock();
}

cfoem.RW_toggleSystemClock = function () {
    CFO.ToggleSystemClockEnable();
    logMessage("the System Clock", "toggled", POST.who);
    return cfoem.RO_readSystemClock();
}

cfoem.RO_readSystemClock = function () {
    return CFO.ReadSystemClock();
}

cfoem.RW_toggleTimingEnable = function () {
    CFO.ToggleTimingEnable();
    logMessage("internal timing", "toggled", POST.who);
    return cfoem.RO_readTimingEnable();
}

cfoem.RO_readTimingEnable = function () {
    return CFO.ReadTimingEnable();
}

cfoem.RW_setTriggerDMATransferLength = function (POST) {
    var val = CFO.SetTriggerDMATransferLength(POST.ring);
    logMessage("Trigger DMA Transfer Length to " + POST.ring, "set", POST.who);
    return val;
}

cfoem.RO_readTriggerDMATransferLength = function () {
    return CFO.ReadTriggerDMATransferLength();
}

cfoem.RW_setMinDMATransferLength = function (POST) {
    var val = CFO.SetMinDMATransferLength(POST.ring);
    logMessage("Minimum DMA Transfer Length to " + POST.ring, "set", POST.who);
    return val;
}

cfoem.RO_readMinDMATransferLength = function () {
    return CFO.ReadMinDMATransferLength();
}

cfoem.RW_setSERDESLoopback = function (POST) {
    var val = CFO.SetSERDESLoopbackMode(parseInt(POST.ring), parseInt(POST.val));
    logMessage("SERDES Loopback on ring " + POST.ring + " to " + val, "set", POST.who);
    return val;
};

cfoem.RO_readSERDESLoopback = function (POST) {
    var output = { NEPCS: 0, NEPMA: 0, FEPMA: 0, FEPCS: 0 };
    switch (CFO.ReadSERDESLoopback(parseInt(POST.ring))) {
        case 1:
            output.NEPCS = 1;
            break;
        case 2:
            output.NEPMA = 1;
            break;
        case 4:
            output.FEPMA = 1;
            break;
        case 6:
            output.FEPCS = 1;
            break;
    }
    return output;
};

cfoem.RO_readSERDESOscillatorICCError = function () {
    return CFO.ReadSERDESOscillatorIICError();
}

cfoem.RO_readSERDESOScillatorInitializationComplete = function () {
    return CFO.ReadSERDESOscillatorInitializationComplete();
}

cfoem.RW_toggleROCEmulator = function (POST) {
    var val = CFO.ToggleROCEmulator(POST.ring);
    logMessage("ROC Emulator on ring " + POST.ring + "(" + val + ")", "toggled", POST.who);
    return val;
}

cfoem.RO_readROCEmulator = function (POST) {
    return CFO.ReadROCEmulator(POST.ring);
}

cfoem.RW_toggleRingEnabled = function (POST) {
    var id = POST.val;
    var current = new cfo.CFO_RingEnableMode(false, false, false);
    switch (id) {
        case 0:
            current.TransmitEnable = true;
            break;
        case 1:
            current.ReceiveEnable = true;
            break;
        case 2:
            current.TimingEnable = true;
            break;
    }
    var val = CFO.ToggleRingEnabled(POST.ring, current);
    logMessage("Ring Enable bit " + POST.val + " on ring " + POST.ring + " (" + val + ")", "toggled", POST.who);
    return val;
}

cfoem.RO_readRingEnabled = function (POST) {
    var output = { TransmitEnable: false, ReceiveEnable: false, TimingEnable: false };
    var enable = CFO.ReadRingEnabled(parseInt(POST.ring));
    output.TransmitEnable = enable.TransmitEnable;
    output.ReceiveEnable = enable.ReceiveEnable;
    output.TimingEnable = enable.TimingEnable;
    return output;
}

cfoem.RW_resetSERDES = function (POST) {
    logMessage("the SERDES on ring " + POST.ring, "is resetting", POST.who);
    return CFO.ResetSERDES(parseInt(POST.ring));
}

cfoem.RO_readResetSERDES = function (POST) {
    return CFO.ReadResetSERDES(parseInt(POST.ring));
}

cfoem.RO_readSERDESResetDone = function (POST) {
    return CFO.ReadResetSERDESDone(parseInt(POST.ring));
}

cfoem.RO_readSERDESRXDisparity = function (POST) {
    switch (CFO.ReadSERDESRXDisparityError(parseInt(POST.ring)).GetData(true)) {
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

cfoem.RW_clearSERDESRXDisparity = function (POST) {
    CFO.ClearSERDESRXDisparityError(parseInt(POST.ring))
    return RO_readSERDESRXDisparity(POST);
}

cfoem.RO_readSERDESRXCharacterError = function (POST) {
    switch (CFO.ReadSERDESRXCharacterNotInTableError(parseInt(POST.ring)).GetData(true)) {
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

cfoem.RW_clearSERDESRXCharacterError = function (POST) {
    CFO.ClearSERDESRXCharacterNotInTableError(parseInt(POST.ring));
    return RO_readSREDESRXCharacterError(POST);
}

cfoem.RO_readSERDESUnlockError = function (POST) {
    return CFO.ReadSERDESUnlockError(parseInt(POST.ring));
}

cfoem.RW_clearSERDESUnlockError = function (POST) {
    return CFO.ClearSERDESUnlockError(parseInt(POST.ring));
}

cfoem.RO_readSERDESPLLLocked = function (POST) {
    return CFO.ReadSERDESPLLLocked(parseInt(POST.ring));
}

cfoem.RO_readSERDESOverflowOrUnderflow = function (POST) {
    return CFO.ReadSERDESOverflowOrUnderflow(parseInt(POST.ring));
}

cfoem.RO_readSERDESBufferFIFOHalfFull = function (POST) {
    return CFO.ReadSERDESBufferFIFOHalfFull(parseInt(POST.ring));
}

cfoem.RO_readSERDESRXBufferStatus = function (POST) {
    var output = { Nominal: 0, Empty: 0, Full: 0, Underflow: 0, Overflow: 0 };
    switch (CFO.ReadSERDESRXBufferStatus(parseInt(POST.ring))) {
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

cfoem.RO_readSERDESRXStatus = function (POST) {
    var output = {
        DataOK: 0, SKPAdded: 0, SKPRemoved: 0, ReceiverDetected: 0, 
        DecodeError: 0, EOverflow: 0, EUnderflow: 0, DisparityError: 0
    };
    switch (CFO.ReadSERDESRXStatus(parseInt(POST.ring))) {
        case 0:
            output.DataOK = 1;
            break;
        case 1:
            output.SKPAdded = 1;
            break;
        case 2:
            output.SKPRemoved = 1;
            break;
        case 3:
            output.ReceiverDetected = 1;
            break;
        case 4:
            output.DecodeError = 1;
            break;
        case 5:
            output.EOverflow = 1;
            break;
        case 6:
            output.EUnderflow = 1;
            break;
        case 7:
            output.DisparityError = 1;
            break;
            
    }
    return output;
}

cfoem.RO_readSERDESEyescanError = function (POST) {
    return CFO.ReadSERDESEyescanError(parseInt(POST.ring));
}

cfoem.RW_clearSERDESEyescanError = function (POST) {
    return CFO.ClearSERDESEyescanError(parseInt(POST.ring));
}

cfoem.RO_readSERDESRXCDRLock = function (POST) {
    return CFO.ReadSERDESRXCDRLock(parseInt(POST.ring));
}

cfoem.RW_writeDMATimeoutPreset = function (POST) {
    var val = CFO.WriteDMATimeoutPreset(POST.ring);
    logMessage("DMA Timeout Preset to " + POST.ring, "set", POST.who);
    return val;
}

cfoem.RO_readDMATimeoutPreset = function () {
    return CFO.ReadDMATimeoutPreset();
}

cfoem.RW_writeDataPendingTimer = function (POST) {
    var val = CFO.WriteDataPendingTimer(POST.ring);
    logMessage("DMA Data Pending Timeout to " + POST.ring, "set", POST.who);
    return val;
}

cfoem.RO_readDataPendingTimer = function () {
    return CFO.ReadDataPendingTimer();
}

cfoem.RW_setPacketSize = function (POST) {
    var val = CFO.SetPacketSize(POST.ring);
    logMessage("DMA Packet Size to " + POST.ring, "set", POST.who);
    return val;
}

cfoem.RO_readPacketSize = function () {
    return CFO.ReadPacketSize();
}

cfoem.RO_readFIFOFlags = function (POST) {
    return CFO.ReadFIFOFullErrorFlags(parseInt(POST.ring));
}

cfoem.RW_SetFIFOFlags = function (POST) {
    var activeFlag = POST.id;
    var flags = new cfo.CFO_FIFOFullErrorFlags();
    flags[activeFlag] = true;
    return CFO.ToggleFIFOFullErrorFlags(parseInt(POST.ring), flags);
}

cfoem.RW_setTimestampPreset = function (POST) {
    var ts = CFO.WriteTimestampPreset(new cfo.CFO_Timestamp(parseInt(parseInt(POST.ring), 16)));
    logMessage("the timestamp preset (" + ts.GetTimestamp(true) + ")", "set", POST.who);
    return ts.GetTimestamp(true);
}

cfoem.RO_readTimestampPreset = function () {
    return CFO.ReadTimestampPreset().GetTimestamp(true);
}

cfoem.RO_readFPGAPROMProgramFIFOFull = function () {
    return CFO.ReadFPGAPROMProgramFIFOFull();
}

cfoem.RO_readFPGAPROMReady = function () {
    return CFO.ReadFPGAPROMReady();
}

cfoem.RO_readFPGACoreAccessFIFOFull = function () {
    return CFO.ReadFPGACoreAccessFIFOFull();
}

cfoem.RW_setSimMode = function (POST) {
    var mode = POST.id;
    var output = {Disabled:0,Tracker:0,Calorimeter:0,CosmicVeto:0,Hardware:0};
    var modeOut = CFO.SetSimMode(mode);
    if(modeOut == 0) { output.Disabled = 1;}
    if(modeOut == 1) { output.Tracker = 1; }
    if (modeOut == 2) { output.Calorimeter = 1; }
    if (modeOut == 3) { output.CosmicVeto = 1; }
    if (modeOut == 4) { output.Hardware = 1; }
    return output;
}


//
// Other Functions
//
cfoem.RO_regDump = function () {
    if (rdTime.getTime() + 1000 < new Date().getTime()) {
        console.log("Getting new regDump");
        getRegDump();
    }
    
    return regDump;
};
cfoem.RW_regDump = function () { return cfoem.RO_regDump(); }

cfoem.RO_SystemStatus = function () {
    if (ssTime.getTime() + 1000 < new Date().getTime()) {
        console.log("Getting new SystemStatus");
        getSystemStatus();
        ssTime = new Date();
    }
    
    return systemStatus;
}

cfoem.RO_ReadLog = function () {
    console.log("Reading CFO log file");
    var logContent = "" + fs.readFileSync("/tmp/CFO.log");
    return logContent;
};

cfoem.RO_RegIO = function (POST) {
    console.log("In RO Register IO handler");
    if (POST.option === "write") {
        throw new TypeError("Unauthorized Access Attempt!");
    }
    
    return read(POST.address).toString(16);
}

cfoem.RW_WriteLog = function (POST) {
    logMessage(parseInt(POST.ring), "says", POST.who);
    res.end(readLog());
    console.log("Done sending log message reply");
}

cfoem.RW_StartDMATest = function (POST, testStatus) {
    console.log("Starting DMA I/O Test");
    console.log(POST);
    var regEnabled = POST.reg === 'true' ? true : false;
    var pcieEnabled = POST.pcie === 'true' ? true : false;
    var dmaEnabled = POST.dma === 'true' ? true : false;
    var daqEnabled = POST.daq === 'true' ? true : false;
    var dcsEnabled = POST.dcs === 'true' ? true : false;
    var loopbackEnabled = POST.loopback === 'true' ? true : false;
    var numTests = parseInt(POST.n);
    if (!testStatus.testRunning) {
        dmatest.startTest(regEnabled, pcieEnabled, dmaEnabled, daqEnabled, dcsEnabled, loopbackEnabled, numTests, false);
    }
    return getDMATestStatus(testStatus);
}

cfoem.RW_ResetTestStatus = function (POST, testStatus) {
    if (dmatest !== null && dmatest.isRunning()) {
        dmatest.stopTests();
        while (testStatus.testRunning) {
            getDMATestStatus(testStatus);
        }
    }
    testStatus.regPassed = 0;
    testStatus.regFailed = 0;
    testStatus.pciePassed = 0;
    testStatus.pcieFailed = 0;
    testStatus.dmaPassed = 0;
    testStatus.dmaFailed = 0;
    testStatus.daqPassed = 0;
    testStatus.daqFailed = 0;
    testStatus.dcsPassed = 0;
    testStatus.dcsFailed = 0;
    testStatus.loopbackPassed = 0;
    testStatus.loopbackFailed = 0;
    
    return getDMATestStatus(testStatus);
}

cfoem.RW_StopDMATest = function (POST, testStatus) {
    if (dmatest !== null) {
        dmatest.stopTests();
    }
    
    getDMATestStatus(testStatus);
    
    return testStatus;
}

cfoem.RW_DMAIO = function (POST, testStatus) {
    //console.log(POST);
    var packets = JSON.parse(POST.packets);
    var readDaq = false;
    var readDcs = false;
    var timestamp = cfo.new_u8array(6);
    
    console.log("packets.length: " + packets.length);
    for (var i = 0; i < packets.length; i++) {
        switch (packets[i].type) {
            case 0://DCS Request
                console.log("Packet type DCS");
                var data = cfo.new_u8array(12);
                for (var j = 0; j < 12; j++) { cfo.u8array_setitem(data, j, packets[i].data[j]); }
                var packet = new cfo.CFO_DCSRequestPacket(packets[i].ringID, packets[i].hopCount, data);
                CFO.WriteDMADCSPacket(packet);
                readDcs = true;
                console.log("Done");
                break;
            case 1:// Readout Request
                console.log("Packet type Readout Request");
                for (var j = 0; j < 6; j++) { cfo.u8array_setitem(timestamp, j, packets[i].timestamp[j]); }
                var packet = new cfo.CFO_ReadoutRequestPacket(packets[i].ringID, new cfo.CFO_Timestamp(timestamp), packets[i].hopCount);
                CFO.WriteDMADAQPacket(packet);
                readDaq = true;
                console.log("Done");
                break;
            case 2:// Data Request
                console.log("Packet type Data Request");
                for (var j = 0; j < 6; j++) { cfo.u8array_setitem(timestamp, j, packets[i].timestamp[j]); }
                var packet = new cfo.CFO_DataRequestPacket(packets[i].ringID, packets[i].hopCount, new cfo.CFO_Timestamp(timestamp));
                CFO.WriteDMADAQPacket(packet);
                readDaq = true;
                console.log("Done");
                break;
        }
    }
    
    var res = "";
    if (readDaq) {
        console.log("Reading DAQ Response");
        var daqResponse = CFO.GetJSONData(new cfo.CFO_Timestamp(timestamp));
        res += daqResponse + "\n";
    }
    if (readDcs) {
        console.log("Reading DCS Response");
        res += CFO.ReadNextDCSPacket().toJSON();
    }
    console.log("Result is " + res);
    return res;

}

cfoem.RW_setMaxROC = function (POST) {
    CFO.SetMaxROCNumber(parseInt(POST.ring), parseInt(POST.val));
    logMessage("Max ROC on ring " + POST.ring + " to " + POST.val, "set", POST.who);
    return 1;
}

/* Script handler removed as of v1.0, sorry...
cfoem.RW_RunScript = function (POST) {
    console.log("In Script handler");
    
    var success = true;
    var status = "Success!";
    var text = parseInt(POST.ring);
    var lines = text.split("\n");
    for (var i = 0; i < lines.length; i++) {
        var thisLine = lines[i];
        var thisLineSplit = thisLine.split(" ");
        var address = thisLineSplit[0];
        var val = thisLineSplit[1];
        logMessage("a write: " + val + " to " + address, "scripted", POST.who);
        var value = write(address, val);
        if (value !== val) { success = false; }
    }
    
    if (success) {
        logMessage("the script was run successfully.", "noticed that", POST.who);
    }
    else {
        logMessage("the script had an error!", "noticed that", POST.who);
        status = "Script did not run successfully!!!";
        console.log("CFO Error Status: " + cfoem.Err);
    }
    
    return status;
}
*/

cfoem.RW_TestControl = function (POST) {
    console.log("Starting or Stopping a DMA test");
    var dma = POST.dma;
    var started = POST.started;
    var packetSize = POST.size;
    var enableLoopback = POST.loopbackEnabled;
    var enableTXChecker = POST.txChecker;
    var enableRXGenerator = POST.rxGenerator;
    if (started) {
        logMessage("a DMA test on channel " + dma, "started", POST.who);
        started = startTest(dma, packetSize, enableLoopback, enableTXChecker, enableRXGenerator);
    } else {
        logMessage("a DMA test on channel " + dma, "stopped", POST.who);
        started = stopTest(dma);
    }
    
    return started;
}

/* Register writing removed as of v1.0, sorry
cfoem.RW_RegIO = function (POST) {
    console.log("In RO Register IO handler");
    if (POST.option === "read") {
        return cfoem.RO_RegIO(POST);
    }
    
    logMessage(POST.value.toString(16) + " to " + POST.address.toString(16), "wrote", POST.who);
    value = write(POST.address, POST.value);
    
    console.log("Replying with value " + value.toString(16));
    return value.toString(16);
}
*/

//
// GET_ Functions
// Get data in json form for d3 graphs
// Format: { value:, time:, name: }
//
cfoem.GET_Send = function () {
    if (sTime.getTime() + 800 < new Date().getTime()) {
        send = (read("0x900C") & 0xFFFFFFFC) >>> 0;
        sTime = new Date();
    }
    
    return { name: "send", value: send, time: new Date() };
}

cfoem.GET_Receive = function () {
    if (rTime.getTime() + 800 < new Date().getTime()) {
        receive = (read("0x9010") & 0xFFFFFFFC) >>> 0;
        rTime = new Date();
    }
    
    return { name: "receive", value: receive, time: new Date() };
}

cfoem.GET_SPayload = function () {
    if (spTime.getTime() + 800 < new Date().getTime()) {
        sendP = (read("0x9014") & 0xFFFFFFFC) >>> 0;
        
        spTime = new Date();
    }
    
    return { name: "spayload", value: sendP, time: new Date() };
}

cfoem.GET_RPayload = function () {
    if (rpTime.getTime() + 800 < new Date().getTime()) {
        receiveP = (read("0x9018") & 0xFFFFFFFC) >>> 0;
        rpTime = new Date();
    }
    
    return { name: "rpayload", value: receiveP, time: new Date() };
}

cfoem.GET_PCIeTransmit = function () {
    cfoem.RO_SystemStatus();
    return { name: "pcieTX", value: systemStatus.pcie.stats.WritesRate, time: new Date() };
};

cfoem.GET_PCIeReceive = function () {
    cfoem.RO_SystemStatus();
    return { name: "pcieRX", value: systemStatus.pcie.stats.ReadsRate, time: new Date() };
};

cfoem.GET_DMA0Transmit = function () {
    return { name: "dma0TX", value: getDMAWriteRate(0), time: new Date() };
};

cfoem.GET_DMA0Receive = function () {
    return { name: "dma0RX", value: getDMAReadRate(0), time: new Date() };
};

cfoem.GET_DMA1Transmit = function () {
    return { name: "dma1TX", value: getDMAWriteRate(1), time: new Date() };
};

cfoem.GET_DMA1Receive = function () {
    return { name: "dma1RX", value: getDMAReadRate(1), time: new Date() };
};

cfoem.GET_DMATestStatistics = function (testStatus) {
    getDMATestStatus(testStatus);
    //console.log("In GET_DMATestStatistics");
    //console.log(testStatus);
    return {
        daqC2S: getDMAReadRate(0), daqS2C: getDMAWriteRate(0), daqPassed: testStatus.daqPassed, daqFailed: testStatus.daqFailed,
        dcsC2S: getDMAReadRate(1), dcsS2C: getDMAWriteRate(1), dcsPassed: testStatus.dcsPassed, dcsFailed: testStatus.dcsFailed,
        regPassed: testStatus.regPassed, regFailed: testStatus.regFailed, 
        pciePassed: testStatus.pciePassed, pcieFailed: testStatus.pcieFailed,
        dmaPassed: testStatus.dmaPassed, dmaFailed: testStatus.dmaFailed,
        loopbackPassed: testStatus.loopbackPassed, loopbackFailed: testStatus.loopbackFailed,
        testRunning: testStatus.testRunning
    };
}
//
// module.exports
// A function that puts the emitter into the given array
//
module.exports = function (module_holder) {
    module_holder["CFO"] = cfoem;
};
