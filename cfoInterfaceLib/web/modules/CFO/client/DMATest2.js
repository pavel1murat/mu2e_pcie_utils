var packets = {};
var numPackets = 0;
function addPacket(type, id, packet) {
    packets[id] = packet;
    $.get("/CFO/" + type, function (data) {
        var datatemp = data.replace(/%id%/g, id);
        $("#packets").append(datatemp);
        $("#packet" + id + " #delete").click(function () {
            $("#packet" + id).remove();
        });
    });
    ++numPackets;
}

function setPixel(led, bit, modestring) {
    var ctx = led.getContext("2d");
    
    ctx.lineWidth = 2;
    
    if (modestring === "RO") {
        ctx.strokeStyle = "darkgreen";
        if (bit) {
            ctx.fillStyle = "lightgreen";
        } else {
            ctx.fillStyle = 'black';
        }
    }
    else if (modestring === "RW") {
        ctx.strokeStyle = "darkblue";
        if (bit) {
            ctx.fillStyle = "lightblue";
        } else {
            ctx.fillStyle = 'black';
        }
    } else {
        ctx.strokeStype = "darkred";
        ctx.fillStyle = "red";
    }
    ctx.beginPath();
    ctx.moveTo(led.width / 4, ctx.lineWidth); //Create a starting point
    ctx.lineTo(led.width * 3 / 4, ctx.lineWidth); //Create a horizontal line
    ctx.arcTo(led.width - ctx.lineWidth, ctx.lineWidth, led.width - ctx.lineWidth, led.height / 4, led.width / 8); // Create an arc
    ctx.lineTo(led.width - ctx.lineWidth, led.height * 3 / 4); // Continue with vertical line
    ctx.arcTo(led.width - ctx.lineWidth, led.height - ctx.lineWidth, led.width * 3 / 4, led.height - ctx.lineWidth, led.width / 8);
    ctx.lineTo(led.width / 4, led.height - ctx.lineWidth);
    ctx.arcTo(ctx.lineWidth, led.height - ctx.lineWidth, ctx.lineWidth, led.height * 3 / 4, led.width / 8);
    ctx.lineTo(ctx.lineWidth, led.height / 4);
    ctx.arcTo(ctx.lineWidth, ctx.lineWidth, led.width / 4, ctx.lineWidth, led.width / 8);
    ctx.closePath();
    ctx.stroke();
    ctx.fill();
}

var oldbase = 16;
// Define a method that handles nothing but the actual
// form population. This helps us decouple the data
// insertion from the data retrieval.
function PopulateValueBox(value, base) {
    $("#value").val(parseInt(value, 16).toString(base));
}

// I take the given option selection and return the
// associated data using a remote method call.
function GetAJAXValues(strOption, address, value, fnCallback) {
    // Check to see if there is currently an AJAX
    // request on this method.
    if (GetAJAXValues.Xhr) {
        // Abort the current request.
        GetAJAXValues.Xhr.abort();
    }
    // Get data via AJAX. Store the XHR (AJAX request
    // object in the method in case we need to abort
    // it on subsequent requests.
    GetAJAXValues.Xhr = $.ajax({
        type: "post",
        url: "/CFO/RegIO",
        data: {
            option: strOption,
            address: address,
            value: value
        },
        dataType: "json",
        // Our success handler.
        success: function (objData) {
            // At this point, we have data coming back
            // from the server.
            fnCallback({
                Value1: objData
            });
        },
        // An error handler for the request.
        error: function (xhr, textStatus, errorCode) {
            //alert("An error occurred:\n" + textStatus + "\n" + errorC
        },
        // I get called no matter what.
        complete: function () {
            // Remove completed request object.
            GetAJAXValues.Xhr = null;
        }
    });
}

function GetRegDumpAjax(fnCallback) {
    // Check to see if there is currently an AJAX
    // request on this method.
    if (GetRegDumpAjax.Xhr) {
        // Abort the current request.
        GetRegDumpAjax.Xhr.abort();
    }
    // Get data via AJAX. Store the XHR (AJAX request
    // object in the method in case we need to abort
    // it on subsequent requests.
    GetRegDumpAjax.Xhr = $.ajax({
        type: "post",
        url: "/CFO/regDump",
        data: { data: "nullData" },
        dataType: "json",
        // Our success handler.
        success: function (objData) {
            // At this point, we have data coming back
            // from the server.
            fnCallback({
                Value1: objData
            });
        },
        // An error handler for the request.
        error: function (xhr, textStatus, errorCode) {
            //alert("An error occurred:\n" + textStatus + "\n" + errorCode);
        },
        // I get called no matter what.
        complete: function () {
            // Remove completed request object.
            GetRegDumpAjax.Xhr = null;
        }
    });
}

function GetRegDump() {
    var objData = null;
    GetRegDumpAjax(function (regDump) {
        PopulateLEDS(regDump.Value1);
    });
}

function LEDAction(url, ring, id) {
    var objData = null;
    AjaxPost(url, ring, function (output) {
        setPixel(document.getElementById(id), output.Value1, "RW");
    });
}
function LEDExtAction(url, ring, val, id) {
    var objData = null;
    var post = {};
    post.ring = ring;
    post.val = val;
    AjaxPost(url, post, function (output) {
        setPixel(document.getElementById(id), output.Value1, "RW");
    });
}
function LEDObjAction(url, ring, idlo, idhi) {
    var objData = null;
    AjaxPost(url, ring, function (output) {
        setPixel(document.getElementById(idlo), output.Value1.Low, "RW");
        setPixel(document.getElementById(idhi), output.Value1.High, "RW");
    });
}

function SetNumericValue(id, url) {
    var objData = null;
    var value = $(id).val();
    AjaxPost(url, value, function (returnValue) {
        $(id).val(returnValue.Value1);
    });
}

function PostLogMessage() {
    var objData = null;
    var message = $("#logMessage").val();
    AjaxPost('/CFO/WriteLog', message, function (returnValue) {
        $("#log").val(returnValue.Value1);
    });
}

function ReadLog() {
    var objData = null;
    AjaxPost('/CFO/ReadLog', null, function (returnValue) {
        $("#log").val(returnValue.Value1);
    });
}

function RunScript() {
    var objData = null;
    AjaxPost('/CFO/RunScript', $("#script").val(), function (returnValue) {
        $("#script").val(returnValue);
    });
}

(function ($, sr) {
    
    // debouncing function from John Hann
    // http://unscriptable.com/index.php/2009/03/20/debouncing-javascript-methods/
    var debounce = function (func, threshold, execAsap) {
        var timeout;
        
        return function debounced() {
            var obj = this, args = arguments;
            function delayed() {
                if (!execAsap)
                    func.apply(obj, args);
                timeout = null;
            }            ;
            
            if (timeout)
                clearTimeout(timeout);
            else if (execAsap)
                func.apply(obj, args);
            
            timeout = setTimeout(delayed, threshold || 100);
        };
    }
    // smartresize 
    jQuery.fn[sr] = function (fn) { return fn ? this.bind('resize', debounce(fn)) : this.trigger(sr); };

})(jQuery, 'smartresize');


function PopulateLEDS(cforegdump) {
    $("#cfoVersion").val(cforegdump.Version);
    setPixel(document.getElementById("cfoResetLED"), cforegdump.ResetCFO, "RW");
    setPixel(document.getElementById("serdesResetLED"), cforegdump.ResetSERDESOscillator, "RW");
    setPixel(document.getElementById("SERDESOscillatorClockLED"), cforegdump.SERDESOscillatorClock, "RW");
    setPixel(document.getElementById("SystemClockLED"), cforegdump.SystemClock, "RW");
    setPixel(document.getElementById("TimingEnabledLED"), cforegdump.TimingEnable, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing0LED"), cforegdump.Ring0.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing1LED"), cforegdump.Ring1.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing2LED"), cforegdump.Ring2.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing3LED"), cforegdump.Ring3.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing4LED"), cforegdump.Ring4.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing5LED"), cforegdump.Ring5.ROCEmulator, "RW");
    $("#triggerDMALength").val(cforegdump.TriggerDMALength);
    $("#minDMALength").val(cforegdump.MinDMALength);
    $("#dmaTimeoutPreset").val(cforegdump.DMATimeout);
    $("#dataPendingTimer").val(cforegdump.DataPendingTimer);
    $("#packetSize").val(cforegdump.PacketSize);
    setPixel(document.getElementById("simModeDisabledLED"), cforegdump.SimMode.Disabled, "RW");
    setPixel(document.getElementById("simModeTrackerLED"), cforegdump.SimMode.Tracker, "RW");
    setPixel(document.getElementById("simModeCalorimeterLED"), cforegdump.SimMode.Calorimeter, "RW");
    setPixel(document.getElementById("simModeCRVLED"), cforegdump.SimMode.CosmicVeto, "RW");
    setPixel(document.getElementById("simModeHardwareLED"), cforegdump.SimMode.Hardware, "RW");
    setPixel(document.getElementById("Ring0TxEnabledLED"), cforegdump.Ring0.TxEnabled, "RW");
    setPixel(document.getElementById("Ring1TxEnabledLED"), cforegdump.Ring1.TxEnabled, "RW");
    setPixel(document.getElementById("Ring2TxEnabledLED"), cforegdump.Ring2.TxEnabled, "RW");
    setPixel(document.getElementById("Ring3TxEnabledLED"), cforegdump.Ring3.TxEnabled, "RW");
    setPixel(document.getElementById("Ring4TxEnabledLED"), cforegdump.Ring4.TxEnabled, "RW");
    setPixel(document.getElementById("Ring5TxEnabledLED"), cforegdump.Ring5.TxEnabled, "RW");
    setPixel(document.getElementById("CFOTxEnabledLED"), cforegdump.CFO.TxEnabled, "RW");
    setPixel(document.getElementById("Ring0RxEnabledLED"), cforegdump.Ring0.RxEnabled, "RW");
    setPixel(document.getElementById("Ring1RxEnabledLED"), cforegdump.Ring1.RxEnabled, "RW");
    setPixel(document.getElementById("Ring2RxEnabledLED"), cforegdump.Ring2.RxEnabled, "RW");
    setPixel(document.getElementById("Ring3RxEnabledLED"), cforegdump.Ring3.RxEnabled, "RW");
    setPixel(document.getElementById("Ring4RxEnabledLED"), cforegdump.Ring4.RxEnabled, "RW");
    setPixel(document.getElementById("Ring5RxEnabledLED"), cforegdump.Ring5.RxEnabled, "RW");
    setPixel(document.getElementById("CFORxEnabledLED"), cforegdump.CFO.RxEnabled, "RW");
    setPixel(document.getElementById("Ring0TimingEnabledLED"), cforegdump.Ring0.TimingEnabled, "RW");
    setPixel(document.getElementById("Ring1TimingEnabledLED"), cforegdump.Ring1.TimingEnabled, "RW");
    setPixel(document.getElementById("Ring2TimingEnabledLED"), cforegdump.Ring2.TimingEnabled, "RW");
    setPixel(document.getElementById("Ring3TimingEnabledLED"), cforegdump.Ring3.TimingEnabled, "RW");
    setPixel(document.getElementById("Ring4TimingEnabledLED"), cforegdump.Ring4.TimingEnabled, "RW");
    setPixel(document.getElementById("Ring5TimingEnabledLED"), cforegdump.Ring5.TimingEnabled, "RW");
    setPixel(document.getElementById("R0ROC0LED"), cforegdump.Ring0.ROC0Enabled, "RW");
    setPixel(document.getElementById("R0ROC1LED"), cforegdump.Ring0.ROC1Enabled, "RW");
    setPixel(document.getElementById("R0ROC2LED"), cforegdump.Ring0.ROC2Enabled, "RW");
    setPixel(document.getElementById("R0ROC3LED"), cforegdump.Ring0.ROC3Enabled, "RW");
    setPixel(document.getElementById("R0ROC4LED"), cforegdump.Ring0.ROC4Enabled, "RW");
    setPixel(document.getElementById("R0ROC5LED"), cforegdump.Ring0.ROC5Enabled, "RW");
    setPixel(document.getElementById("R1ROC0LED"), cforegdump.Ring1.ROC0Enabled, "RW");
    setPixel(document.getElementById("R1ROC1LED"), cforegdump.Ring1.ROC1Enabled, "RW");
    setPixel(document.getElementById("R1ROC2LED"), cforegdump.Ring1.ROC2Enabled, "RW");
    setPixel(document.getElementById("R1ROC3LED"), cforegdump.Ring1.ROC3Enabled, "RW");
    setPixel(document.getElementById("R1ROC4LED"), cforegdump.Ring1.ROC4Enabled, "RW");
    setPixel(document.getElementById("R1ROC5LED"), cforegdump.Ring1.ROC5Enabled, "RW");
    setPixel(document.getElementById("R2ROC0LED"), cforegdump.Ring2.ROC0Enabled, "RW");
    setPixel(document.getElementById("R2ROC1LED"), cforegdump.Ring2.ROC1Enabled, "RW");
    setPixel(document.getElementById("R2ROC2LED"), cforegdump.Ring2.ROC2Enabled, "RW");
    setPixel(document.getElementById("R2ROC3LED"), cforegdump.Ring2.ROC3Enabled, "RW");
    setPixel(document.getElementById("R2ROC4LED"), cforegdump.Ring2.ROC4Enabled, "RW");
    setPixel(document.getElementById("R2ROC5LED"), cforegdump.Ring2.ROC5Enabled, "RW");
    setPixel(document.getElementById("R3ROC0LED"), cforegdump.Ring3.ROC0Enabled, "RW");
    setPixel(document.getElementById("R3ROC1LED"), cforegdump.Ring3.ROC1Enabled, "RW");
    setPixel(document.getElementById("R3ROC2LED"), cforegdump.Ring3.ROC2Enabled, "RW");
    setPixel(document.getElementById("R3ROC3LED"), cforegdump.Ring3.ROC3Enabled, "RW");
    setPixel(document.getElementById("R3ROC4LED"), cforegdump.Ring3.ROC4Enabled, "RW");
    setPixel(document.getElementById("R3ROC5LED"), cforegdump.Ring3.ROC5Enabled, "RW");
    setPixel(document.getElementById("R4ROC0LED"), cforegdump.Ring4.ROC0Enabled, "RW");
    setPixel(document.getElementById("R4ROC1LED"), cforegdump.Ring4.ROC1Enabled, "RW");
    setPixel(document.getElementById("R4ROC2LED"), cforegdump.Ring4.ROC2Enabled, "RW");
    setPixel(document.getElementById("R4ROC3LED"), cforegdump.Ring4.ROC3Enabled, "RW");
    setPixel(document.getElementById("R4ROC4LED"), cforegdump.Ring4.ROC4Enabled, "RW");
    setPixel(document.getElementById("R4ROC5LED"), cforegdump.Ring4.ROC5Enabled, "RW");
    setPixel(document.getElementById("R5ROC0LED"), cforegdump.Ring5.ROC0Enabled, "RW");
    setPixel(document.getElementById("R5ROC1LED"), cforegdump.Ring5.ROC1Enabled, "RW");
    setPixel(document.getElementById("R5ROC2LED"), cforegdump.Ring5.ROC2Enabled, "RW");
    setPixel(document.getElementById("R5ROC3LED"), cforegdump.Ring5.ROC3Enabled, "RW");
    setPixel(document.getElementById("R5ROC4LED"), cforegdump.Ring5.ROC4Enabled, "RW");
    setPixel(document.getElementById("R5ROC5LED"), cforegdump.Ring5.ROC5Enabled, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing0LED"), cforegdump.Ring0.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing0LED"), cforegdump.Ring0.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing0LED"), cforegdump.Ring0.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing0LED"), cforegdump.Ring0.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing1LED"), cforegdump.Ring1.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing1LED"), cforegdump.Ring1.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing1LED"), cforegdump.Ring1.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing1LED"), cforegdump.Ring1.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing2LED"), cforegdump.Ring2.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing2LED"), cforegdump.Ring2.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing2LED"), cforegdump.Ring2.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing2LED"), cforegdump.Ring2.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing3LED"), cforegdump.Ring3.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing3LED"), cforegdump.Ring3.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing3LED"), cforegdump.Ring3.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing3LED"), cforegdump.Ring3.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing4LED"), cforegdump.Ring4.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing4LED"), cforegdump.Ring4.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing4LED"), cforegdump.Ring4.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing4LED"), cforegdump.Ring4.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing5LED"), cforegdump.Ring5.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing5LED"), cforegdump.Ring5.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing5LED"), cforegdump.Ring5.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing5LED"), cforegdump.Ring5.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSCFOLED"), cforegdump.CFO.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMACFOLED"), cforegdump.CFO.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMACFOLED"), cforegdump.CFO.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSCFOLED"), cforegdump.CFO.SERDESLoopback.FEPCS, "RW");
}

function SetSimMode(id) {
    var objData = null;
    var post = {};
    post.id = id;
    AjaxPost("/CFO/SetSimMode", post, function (output) {
        setPixel(document.getElementById("simModeDisabledLED"), output.Value1.Disabled, "RW");
        setPixel(document.getElementById("simModeTrackerLED"), output.Value1.Tracker, "RW");
        setPixel(document.getElementById("simModeCalorimeterLED"), output.Value1.Calorimeter, "RW");
        setPixel(document.getElementById("simModeCRVLED"), output.Value1.CosmicVeto, "RW");
        setPixel(document.getElementById("simModeHardwareLED"), output.Value1.Hardware, "RW");
    })
}

$(function () {
    GetRegDump();
    setInterval(function () { GetRegDump(); }, 1500);
    
    $("#addDCSPacket").click(function () {
        addPacket("DCSRequestPacket.html", numPackets);
    });
    
    $("#addDataRequestPacket").click(function () {
        addPacket("DataRequestPacket.html", numPackets);
    });
    
    $("#addReadoutRequestPacket").click(function () {
        addPacket("ReadoutRequestPacket.html", numPackets);
    });
    
    $("#sendPackets").click(function () {
        var packets = [];
        
        $("#packets").children().each(function (index, element) {
            var packet = {};
            packet.transferCountHigh = parseInt($("#transferCountHigh", this).val());
            packet.transferCountLow = parseInt($("#transferCountLow", this).val());
            packet.isValid = parseInt($("#validBit", this).val());
            packet.ringID = parseInt($("#ringID", this).val());
            packet.type = parseInt($("#packetType", this).val());
            packet.hopCount = parseInt($("#hopCount", this).val());
            switch (packet.type) {
                case 0:
                    packet.data = [];
                    packet.data.push(parseInt($("#data0", this).val()));
                    packet.data.push(parseInt($("#data1", this).val()));
                    packet.data.push(parseInt($("#data2", this).val()));
                    packet.data.push(parseInt($("#data3", this).val()));
                    packet.data.push(parseInt($("#data4", this).val()));
                    packet.data.push(parseInt($("#data5", this).val()));
                    packet.data.push(parseInt($("#data6", this).val()));
                    packet.data.push(parseInt($("#data7", this).val()));
                    packet.data.push(parseInt($("#data8", this).val()));
                    packet.data.push(parseInt($("#data9", this).val()));
                    packet.data.push(parseInt($("#data10", this).val()));
                    packet.data.push(parseInt($("#data11", this).val()));
                    break;
                case 1:
                    packet.request = [];
                    packet.request.push(parseInt($("#request0", this).val()));
                    packet.request.push(parseInt($("#request1", this).val()));
                    packet.request.push(parseInt($("#request2", this).val()));
                    packet.request.push(parseInt($("#request3", this).val()));
                case 2:
                    packet.debug = $("#debug", this).is(":checked");
                    packet.timestamp = [];
                    packet.timestamp.push(parseInt($("#timestamp0", this).val()));
                    packet.timestamp.push(parseInt($("#timestamp1", this).val()));
                    packet.timestamp.push(parseInt($("#timestamp2", this).val()));
                    packet.timestamp.push(parseInt($("#timestamp3", this).val()));
                    packet.timestamp.push(parseInt($("#timestamp4", this).val()));
                    packet.timestamp.push(parseInt($("#timestamp5", this).val()));
                    break;
            }
            
            packets.push(packet);
        });
        
        AjaxPost("/CFO/DMAIO", { 'packets': JSON.stringify(packets) }, function (data) {
            $("#response").val(data);
        });
    });
    
    var dma0 = {
        dma0TX: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/CFO/DMA0Transmit" },
        dma0RX: { data: [{ time: 0, value: 0 }], color: 'blue', jsonPath: "/CFO/DMA0Receive" },
    };
    //makeGraph("#dma0", dma0);
    var dma1 = {
        dma1TX: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/CFO/DMA1Transmit" },
        dma1RX: { data: [{ time: 0, value: 0 }], color: 'blue', jsonPath: "/CFO/DMA1Receive" },
    };
    //makeGraph("#dma1", dma1);
    
    $(window).smartresize(function () {
        $("#dma0").empty();
        $("#dma1").empty();
        //makeGraph("#dma0", dma0);
        //makeGraph("#dma1", dma1);
    });
});
