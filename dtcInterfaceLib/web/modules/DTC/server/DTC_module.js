// DTCDriver.js
// Author: Eric Flumerfelt, FNAL/RSI
// Last Modified: March 11, 2015
//
// This module for serverbase.js performs register I/O on the mu2e DTC board

var dtc = require("./DTC");
var dtclt = require("./DTCLibTest");
//var gmetric = require('./gmetric');
var fs = require("fs");
var emitter = require("events").EventEmitter;

var DTC = new dtc.DTC();
// So that we can send events back to serverbase
var dtcem = new emitter();
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
var dmatest = new dtclt.DTCLibTest();

//
// Local Functions
// Cannot be called directly by outside code
//
function logMessage(message, method, name) {
    message = "\n" + new Date().toLocaleString() + " " + name + " " + method + " " + message;
    console.log(message);
    fs.appendFileSync("/tmp/DTC.log", message);
    console.log("Done logging message");
}

function SendStatistics() {
    dtcem.GET_Receive();
    dtcem.GET_Send();
    //gmetric.send_gmetric("/etc/ganglia/gmond.conf", "PCIe Send Rate", send.toString(), "double", "B/s", "both", 15, 0, "DTC_PCIe", "mu2e DAQ", "PCIe Send Rate", "PCIe Send Rate");
    //gmetric.send_gmetric("/etc/ganglia/gmond.conf", "PCIe Receive Rate", receive.toString(), "double", "B/s", "both", 15, 0, "DTC_PCIe", "mu2e DAQ", "PCIe Receive Rate", "PCIe Receive Rate");
};

function readMaxRocs(input, link) {
    input.ROC0Enabled = false;
    input.ROC1Enabled = false;
    input.ROC2Enabled = false;
    input.ROC3Enabled = false;
    input.ROC4Enabled = false;
    input.ROC5Enabled = false;

    var rocs = DTC.ReadRingROCCount(link);
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
    regDump = JSON.parse(DTC.RegDump());
    console.log("Done with RegDump");
}

function getDMATestStatus(testStatus) {
    if (dmatest !== null) {
        testStatus.testRunning = dmatest.isRunning();
        testStatus.regPassed += dmatest.regPassed();
        testStatus.regFailed += dmatest.regFailed();
        testStatus.daqPassed += dmatest.daqPassed();
        testStatus.daqFailed += dmatest.daqFailed();
        testStatus.dcsPassed += dmatest.dcsPassed();
        testStatus.dcsFailed += dmatest.dcsFailed();
        testStatus.loopbackPassed += dmatest.loopbackPassed();
        testStatus.loopbackFailed += dmatest.loopbackFailed();
    } else {
        testStatus.testRunning = false;
        dmatest = new dtclt.DTCLibTest();
    }
    dtcem.emit("message", JSON.stringify(testStatus));
    return testStatus;
}

function read(address) {
    var addr = parseInt(address, 16);
    //console.log("Reading " + address);
    return DTC.RegisterRead(addr);
};

/* Register Writes have been removed as of v1.0, sorry
function write(address, value) {
    var addr = parseInt(address, 16);
    var val = parseInt(value, 16);
    console.log("Writing " + val + " to " + addr);
    dtcem.Err = DTC.WriteRegister(val, addr);
    return read(address);
};
*/

//
// Master Init Function
// Runs once on the node.js cluster master
//
dtcem.MasterInitFunction = function(workerData) {
    //Only the master should log Ganglia metrics
    setInterval(SendStatistics, 15000);

    console.log("Setting up log file");
    if (!fs.existsSync("/tmp/DTC.log")) {
        console.log("Creating new log file");
        fs.writeFileSync("/tmp/DTC.log", "Log file created at " + new Date().toLocaleString());
    }
    console.log("Done setting up log");
    var testStatus = {};
    testStatus.regPassed = 0;
    testStatus.regFailed = 0;
    testStatus.daqPassed = 0;
    testStatus.daqFailed = 0;
    testStatus.dcsPassed = 0;
    testStatus.dcsFailed = 0;
    testStatus.loopbackPassed = 0;
    testStatus.loopbackFailed = 0;
    testStatus.testRunning = false;

    workerData["DTC"] = testStatus;
};


//
// libDTCInterface Wrapper Functions
// RO_: Read-only, any authenticated user can run these
// RW_: Read-write, only authorized users can run these
// 2/16/2015: Reordered functions to mimic DTC.h
//
dtcem.RO_readDesignVersion = function() {
    return DTC.ReadDesignVersion();
};
dtcem.RW_resetDTC = function(POST) {
    DTC.ResetDTC();
    logMessage("the DTC", "reset", POST.who);
    return dtcem.RO_readResetDTC();
};

dtcem.RO_readResetDTC = function() {
    return DTC.ReadResetDTC();
};

dtcem.RW_resetSERDESOscillator = function() {
    DTC.ResetSERDESOscillator();
    logMessage("the SERDES Oscillator", "reset", POST.who);
    return dtcem.RO_readResetSERDESOscillator();
};
dtcem.RO_readResetSERDESOscillator = function() {
    return DTC.ReadResetSERDESOscillator();
};
dtcem.RW_toggleSERDESOscillatorClock = function() {
    DTC.ToggleSERDESOscillatorClock();
    logMessage("the SERDES Oscillator Clock Speed", "toggled", POST.who);
    return dtcem.RO_readSERDESOscillatorClock();
};
dtcem.RO_readSERDESOscillatorClock = function() {
    return DTC.ReadSERDESOscillatorClock();
};
dtcem.RW_toggleSystemClock = function() {
    DTC.ToggleSystemClockEnable();
    logMessage("the System Clock", "toggled", POST.who);
    return dtcem.RO_readSystemClock();
};
dtcem.RO_readSystemClock = function() {
    return DTC.ReadSystemClock();
};
dtcem.RW_toggleTimingEnable = function() {
    DTC.ToggleTimingEnable();
    logMessage("internal timing", "toggled", POST.who);
    return dtcem.RO_readTimingEnable();
};
dtcem.RO_readTimingEnable = function() {
    return DTC.ReadTimingEnable();
};
dtcem.RW_setTriggerDMATransferLength = function(POST) {
    var val = DTC.SetTriggerDMATransferLength(POST.link);
    logMessage("Trigger DMA Transfer Length to " + POST.link, "set", POST.who);
    return val;
};
dtcem.RO_readTriggerDMATransferLength = function() {
    return DTC.ReadTriggerDMATransferLength();
};
dtcem.RW_setMinDMATransferLength = function(POST) {
    var val = DTC.SetMinDMATransferLength(POST.link);
    logMessage("Minimum DMA Transfer Length to " + POST.link, "set", POST.who);
    return val;
};
dtcem.RO_readMinDMATransferLength = function() {
    return DTC.ReadMinDMATransferLength();
};
dtcem.RW_setSERDESLoopback = function(POST) {
    var val = DTC.SetSERDESLoopbackMode(parseInt(POST.link), parseInt(POST.val));
    logMessage("SERDES Loopback on link " + POST.link + " to " + val, "set", POST.who);
    return val;
};

dtcem.RO_readSERDESLoopback = function(POST) {
    var output = { NEPCS: 0, NEPMA: 0, FEPMA: 0, FEPCS: 0 };
    switch (DTC.ReadSERDESLoopback(parseInt(POST.link))) {
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

dtcem.RO_readSERDESOscillatorICCError = function() {
    return DTC.ReadSERDESOscillatorIICError();
};
dtcem.RO_readSERDESOScillatorInitializationComplete = function() {
    return DTC.ReadSERDESOscillatorInitializationComplete();
};
dtcem.RW_toggleROCEmulator = function(POST) {
    var val = DTC.ToggleROCEmulator(POST.link);
    logMessage("ROC Emulator on link " + POST.link + "(" + val + ")", "toggled", POST.who);
    return val;
};
dtcem.RO_readROCEmulator = function(POST) {
    return DTC.ReadROCEmulator(POST.link);
};
dtcem.RW_toggleRingEnabled = function(POST) {
    var id = POST.val;
    var current = new dtc.DTC_RingEnableMode(false, false, false);
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
    var val = DTC.ToggleRingEnabled(POST.link, current);
    logMessage("Ring Enable bit " + POST.val + " on link " + POST.link + " (" + val + ")", "toggled", POST.who);
    return val;
};
dtcem.RO_readRingEnabled = function(POST) {
    var output = { TransmitEnable: false, ReceiveEnable: false, TimingEnable: false };
    var enable = DTC.ReadRingEnabled(parseInt(POST.link));
    output.TransmitEnable = enable.TransmitEnable;
    output.ReceiveEnable = enable.ReceiveEnable;
    output.TimingEnable = enable.TimingEnable;
    return output;
};
dtcem.RW_resetSERDES = function(POST) {
    logMessage("the SERDES on link " + POST.link, "is resetting", POST.who);
    return DTC.ResetSERDES(parseInt(POST.link));
};
dtcem.RO_readResetSERDES = function(POST) {
    return DTC.ReadResetSERDES(parseInt(POST.link));
};
dtcem.RO_readSERDESResetDone = function(POST) {
    return DTC.ReadResetSERDESDone(parseInt(POST.link));
};
dtcem.RO_readSERDESRXDisparity = function(POST) {
    switch (DTC.ReadSERDESRXDisparityError(parseInt(POST.link)).GetData(true)) {
    case 0:
        return { low: 0, high: 0 };
    case 1:
        return { low: 1, high: 0 };
    case 2:
        return { low: 0, high: 1 };
    case 3:
        return { low: 1, high: 1 };
    }
};
dtcem.RW_clearSERDESRXDisparity = function(POST) {
    DTC.ClearSERDESRXDisparityError(parseInt(POST.link));
    return RO_readSERDESRXDisparity(POST);
};
dtcem.RO_readSERDESRXCharacterError = function(POST) {
    switch (DTC.ReadSERDESRXCharacterNotInTableError(parseInt(POST.link)).GetData(true)) {
    case 0:
        return { low: 0, high: 0 };
    case 1:
        return { low: 1, high: 0 };
    case 2:
        return { low: 0, high: 1 };
    case 3:
        return { low: 1, high: 1 };
    }
};
dtcem.RW_clearSERDESRXCharacterError = function(POST) {
    DTC.ClearSERDESRXCharacterNotInTableError(parseInt(POST.link));
    return RO_readSREDESRXCharacterError(POST);
};
dtcem.RO_readSERDESUnlockError = function(POST) {
    return DTC.ReadSERDESUnlockError(parseInt(POST.link));
};
dtcem.RW_clearSERDESUnlockError = function(POST) {
    return DTC.ClearSERDESUnlockError(parseInt(POST.link));
};
dtcem.RO_readSERDESPLLLocked = function(POST) {
    return DTC.ReadSERDESPLLLocked(parseInt(POST.link));
};
dtcem.RO_readSERDESOverflowOrUnderflow = function(POST) {
    return DTC.ReadSERDESOverflowOrUnderflow(parseInt(POST.link));
};
dtcem.RO_readSERDESBufferFIFOHalfFull = function(POST) {
    return DTC.ReadSERDESBufferFIFOHalfFull(parseInt(POST.link));
};
dtcem.RO_readSERDESRXBufferStatus = function(POST) {
    var output = { Nominal: 0, Empty: 0, Full: 0, Underflow: 0, Overflow: 0 };
    switch (DTC.ReadSERDESRXBufferStatus(parseInt(POST.link))) {
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
};
dtcem.RO_readSERDESRXStatus = function(POST) {
    var output = {
        DataOK: 0,
        SKPAdded: 0,
        SKPRemoved: 0,
        ReceiverDetected: 0,
        DecodeError: 0,
        EOverflow: 0,
        EUnderflow: 0,
        DisparityError: 0
    };
    switch (DTC.ReadSERDESRXStatus(parseInt(POST.link))) {
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
};
dtcem.RO_readSERDESEyescanError = function(POST) {
    return DTC.ReadSERDESEyescanError(parseInt(POST.link));
};
dtcem.RW_clearSERDESEyescanError = function(POST) {
    return DTC.ClearSERDESEyescanError(parseInt(POST.link));
};
dtcem.RO_readSERDESRXCDRLock = function(POST) {
    return DTC.ReadSERDESRXCDRLock(parseInt(POST.link));
};
dtcem.RW_writeDMATimeoutPreset = function(POST) {
    var val = DTC.WriteDMATimeoutPreset(POST.link);
    logMessage("DMA Timeout Preset to " + POST.link, "set", POST.who);
    return val;
};
dtcem.RO_readDMATimeoutPreset = function() {
    return DTC.ReadDMATimeoutPreset();
};
dtcem.RW_writeDataPendingTimer = function(POST) {
    var val = DTC.WriteDataPendingTimer(POST.link);
    logMessage("DMA Data Pending Timeout to " + POST.link, "set", POST.who);
    return val;
};
dtcem.RO_readDataPendingTimer = function() {
    return DTC.ReadDataPendingTimer();
};
dtcem.RW_setPacketSize = function(POST) {
    var val = DTC.SetPacketSize(POST.link);
    logMessage("DMA Packet Size to " + POST.link, "set", POST.who);
    return val;
};
dtcem.RO_readPacketSize = function() {
    return DTC.ReadPacketSize();
};
dtcem.RO_readFIFOFlags = function(POST) {
    return DTC.ReadFIFOFullErrorFlags(parseInt(POST.link));
};
dtcem.RW_SetFIFOFlags = function(POST) {
    var activeFlag = POST.id;
    var flags = new dtc.DTC_FIFOFullErrorFlags();
    flags[activeFlag] = true;
    return DTC.ToggleFIFOFullErrorFlags(parseInt(POST.link), flags);
};
dtcem.RW_setTimestampPreset = function(POST) {
    var ts = DTC.WriteTimestampPreset(new dtc.DTC_Timestamp(parseInt(parseInt(POST.link), 16)));
    logMessage("the timestamp preset (" + ts.GetTimestamp(true) + ")", "set", POST.who);
    return ts.GetTimestamp(true);
};
dtcem.RO_readTimestampPreset = function() {
    return DTC.ReadTimestampPreset().GetTimestamp(true);
};
dtcem.RO_readFPGAPROMProgramFIFOFull = function() {
    return DTC.ReadFPGAPROMProgramFIFOFull();
};
dtcem.RO_readFPGAPROMReady = function() {
    return DTC.ReadFPGAPROMReady();
};
dtcem.RO_readFPGACoreAccessFIFOFull = function() {
    return DTC.ReadFPGACoreAccessFIFOFull();
};
dtcem.RW_setSimMode = function(POST) {
    var mode = POST.id;
    var output = { Disabled: 0, Tracker: 0, Calorimeter: 0, CosmicVeto: 0, Hardware: 0 };
    var modeOut = DTC.SetSimMode(mode);
    if (modeOut == 0) {
        output.Disabled = 1;
    }
    if (modeOut == 1) {
        output.Tracker = 1;
    }
    if (modeOut == 2) {
        output.Calorimeter = 1;
    }
    if (modeOut == 3) {
        output.CosmicVeto = 1;
    }
    if (modeOut == 4) {
        output.Hardware = 1;
    }
    return output;
};


//
// Other Functions
//
dtcem.RO_regDump = function() {
    if (rdTime.getTime() + 1000 < new Date().getTime()) {
        console.log("Getting new regDump");
        getRegDump();
    }

    return regDump;
};
dtcem.RW_regDump = function() { return dtcem.RO_regDump(); };
dtcem.RO_SystemStatus = function() {
    if (ssTime.getTime() + 1000 < new Date().getTime()) {
        console.log("Getting new SystemStatus");
        getSystemStatus();
        ssTime = new Date();
    }

    return systemStatus;
};
dtcem.RO_ReadLog = function() {
    console.log("Reading DTC log file");
    var logContent = "" + fs.readFileSync("/tmp/DTC.log");
    return logContent;
};

dtcem.RO_RegIO = function(POST) {
    console.log("In RO Register IO handler");
    if (POST.option === "write") {
        throw new TypeError("Unauthorized Access Attempt!");
    }

    return read(POST.address).toString(16);
};
dtcem.RW_WriteLog = function(POST) {
    logMessage(parseInt(POST.link), "says", POST.who);
    res.end(readLog());
    console.log("Done sending log message reply");
};
dtcem.RW_StartDMATest = function(POST, testStatus) {
    console.log("Starting DMA I/O Test");
    console.log(POST);
    var regEnabled = POST.reg === "true" ? true : false;
    var daqEnabled = POST.daq === "true" ? true : false;
    var dcsEnabled = POST.dcs === "true" ? true : false;
    var loopbackEnabled = POST.loopback === "true" ? true : false;
    var numTests = parseInt(POST.n);
    if (!testStatus.testRunning) {
        dmatest.startTest(regEnabled, daqEnabled, dcsEnabled, loopbackEnabled, numTests, false);
    }
    return getDMATestStatus(testStatus);
};
dtcem.RW_ResetTestStatus = function(POST, testStatus) {
    if (dmatest !== null && dmatest.isRunning()) {
        dmatest.stopTests();
        while (testStatus.testRunning) {
            getDMATestStatus(testStatus);
        }
    }
    testStatus.regPassed = 0;
    testStatus.regFailed = 0;
    testStatus.daqPassed = 0;
    testStatus.daqFailed = 0;
    testStatus.dcsPassed = 0;
    testStatus.dcsFailed = 0;
    testStatus.loopbackPassed = 0;
    testStatus.loopbackFailed = 0;

    return getDMATestStatus(testStatus);
};
dtcem.RW_StopDMATest = function(POST, testStatus) {
    if (dmatest !== null) {
        dmatest.stopTests();
    }

    getDMATestStatus(testStatus);

    return testStatus;
};
dtcem.RW_DMAIO = function(POST) {
    //console.log(POST);
    var packets = JSON.parse(POST.packets);
    var readDaq = false;
    var readDcs = false;
    var timestamp = dtc.new_u8array(6);
    var packet;

    console.log("packets.length: " + packets.length);
    for (var i = 0; i < packets.length; i++) {
        switch (packets[i].type) {
        case 0: //DCS Request
            console.log("Packet type DCS");
            var data = dtc.new_u8array(12);
            for (var j = 0; j < 12; j++) {
                dtc.u8array_setitem(data, j, packets[i].data[j]);
            }
            packet = new dtc.DTC_DCSRequestPacket(packets[i].linkID, packets[i].hopCount, data);
            DTC.WriteDMADCSPacket(packet);
            readDcs = true;
            console.log("Done");
            break;
        case 1: // Readout Request
            console.log("Packet type Readout Request");
            for (var j = 0; j < 6; j++) {
                dtc.u8array_setitem(timestamp, j, packets[i].timestamp[j]);
            }
            packet = new dtc.DTC_ReadoutRequestPacket(packets[i].linkID, new dtc.DTC_Timestamp(timestamp), packets[i].hopCount);
            DTC.WriteDMADAQPacket(packet);
            readDaq = true;
            console.log("Done");
            break;
        case 2: // Data Request
            console.log("Packet type Data Request");
            for (var j = 0; j < 6; j++) {
                dtc.u8array_setitem(timestamp, j, packets[i].timestamp[j]);
            }
            packet = new dtc.DTC_DataRequestPacket(packets[i].linkID, packets[i].hopCount, new dtc.DTC_Timestamp(timestamp));
            DTC.WriteDMADAQPacket(packet);
            readDaq = true;
            console.log("Done");
            break;
        }
    }

    var res = "";
    if (readDaq) {
        console.log("Reading DAQ Response");
        var daqResponse = DTC.GetJSONData(new dtc.DTC_Timestamp(timestamp));
        res += daqResponse + "\n";
    }
    if (readDcs) {
        console.log("Reading DCS Response");
        res += DTC.ReadNextDCSPacket().toJSON();
    }
    console.log("Result is " + res);
    return res;

};
dtcem.RW_setMaxROC = function(POST) {
    DTC.SetMaxROCNumber(parseInt(POST.link), parseInt(POST.val));
    logMessage("Max ROC on link " + POST.link + " to " + POST.val, "set", POST.who);
    return 1;
};

/* Script handler removed as of v1.0, sorry...
dtcem.RW_RunScript = function (POST) {
    console.log("In Script handler");

    var success = true;
    var status = "Success!";
    var text = parseInt(POST.link);
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
        console.log("DTC Error Status: " + dtcem.Err);
    }

    return status;
}
*/

/* Register writing removed as of v1.0, sorry
dtcem.RW_RegIO = function (POST) {
    console.log("In RO Register IO handler");
    if (POST.option === "read") {
        return dtcem.RO_RegIO(POST);
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
dtcem.GET_Send = function() {
    if (sTime.getTime() + 800 < new Date().getTime()) {
        send = (read("0x900C") & 0xFFFFFFFC) >>> 0;
        sTime = new Date();
    }

    return { name: "send", value: send, time: new Date() };
};
dtcem.GET_Receive = function() {
    if (rTime.getTime() + 800 < new Date().getTime()) {
        receive = (read("0x9010") & 0xFFFFFFFC) >>> 0;
        rTime = new Date();
    }

    return { name: "receive", value: receive, time: new Date() };
};
dtcem.GET_SPayload = function() {
    if (spTime.getTime() + 800 < new Date().getTime()) {
        sendP = (read("0x9014") & 0xFFFFFFFC) >>> 0;

        spTime = new Date();
    }

    return { name: "spayload", value: sendP, time: new Date() };
};
dtcem.GET_RPayload = function() {
    if (rpTime.getTime() + 800 < new Date().getTime()) {
        receiveP = (read("0x9018") & 0xFFFFFFFC) >>> 0;
        rpTime = new Date();
    }

    return { name: "rpayload", value: receiveP, time: new Date() };
};
dtcem.GET_DMA0Transmit = function() {
    return { name: "dma0TX", value: getDMAWriteRate(0), time: new Date() };
};

dtcem.GET_DMA0Receive = function() {
    return { name: "dma0RX", value: getDMAReadRate(0), time: new Date() };
};

dtcem.GET_DMA1Transmit = function() {
    return { name: "dma1TX", value: getDMAWriteRate(1), time: new Date() };
};

dtcem.GET_DMA1Receive = function() {
    return { name: "dma1RX", value: getDMAReadRate(1), time: new Date() };
};

dtcem.GET_DMATestStatistics = function(testStatus) {
    getDMATestStatus(testStatus);
    //console.log("In GET_DMATestStatistics");
    //console.log(testStatus);
    return {
        daqC2S: getDMAReadRate(0),
        daqS2C: getDMAWriteRate(0),
        daqPassed: testStatus.daqPassed,
        daqFailed: testStatus.daqFailed,
        dcsC2S: getDMAReadRate(1),
        dcsS2C: getDMAWriteRate(1),
        dcsPassed: testStatus.dcsPassed,
        dcsFailed: testStatus.dcsFailed,
        regPassed: testStatus.regPassed,
        regFailed: testStatus.regFailed,
        loopbackPassed: testStatus.loopbackPassed,
        loopbackFailed: testStatus.loopbackFailed,
        testRunning: testStatus.testRunning
    };
};
//
// module.exports
// A function that puts the emitter into the given array
//
module.exports = function(module_holder) {
    module_holder["DTC"] = dtcem;
};
