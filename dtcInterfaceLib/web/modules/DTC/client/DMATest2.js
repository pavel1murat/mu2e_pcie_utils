var packets = {};
var numPackets = 0;

function addPacket(type, id, packet) {
    packets[id] = packet;
    $.get("/DTC/" + type, function (data) {
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
            ctx.fillStyle = "black";
        }
    } else if (modestring === "RW") {
        ctx.strokeStyle = "darkblue";
        if (bit) {
            ctx.fillStyle = "lightblue";
        } else {
            ctx.fillStyle = "black";
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
        url: "/DTC/RegIO",
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
        error: function () { // (xhr, textStatus, errorCode) {
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
        url: "/DTC/regDump",
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
        error: function () { //(xhr, textStatus, errorCode) {
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
    GetRegDumpAjax(function (regDump) {
        PopulateLEDS(regDump.Value1);
    });
}

function LEDAction(url, link, id) {
    AjaxPost(url, link, function (output) {
        setPixel(document.getElementById(id), output.Value1, "RW");
    });
}

function LEDExtAction(url, link, val, id) {
    var post = {};
    post.link = link;
    post.val = val;
    AjaxPost(url, post, function (output) {
        setPixel(document.getElementById(id), output.Value1, "RW");
    });
}

function LEDObjAction(url, link, idlo, idhi) {
    AjaxPost(url, link, function (output) {
        setPixel(document.getElementById(idlo), output.Value1.Low, "RW");
        setPixel(document.getElementById(idhi), output.Value1.High, "RW");
    });
}

function SetNumericValue(id, url) {
    var value = $(id).val();
    AjaxPost(url, value, function (returnValue) {
        $(id).val(returnValue.Value1);
    });
}

function PostLogMessage() {
    var message = $("#logMessage").val();
    AjaxPost("/DTC/WriteLog", message, function (returnValue) {
        $("#log").val(returnValue.Value1);
    });
}

function ReadLog() {
    AjaxPost("/DTC/ReadLog", null, function (returnValue) {
        $("#log").val(returnValue.Value1);
    });
}

function RunScript() {
    AjaxPost("/DTC/RunScript", $("#script").val(), function (returnValue) {
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
            };

            if (timeout)
                clearTimeout(timeout);
            else if (execAsap)
                func.apply(obj, args);

            timeout = setTimeout(delayed, threshold || 100);
        };
    };
    // smartresize
    jQuery.fn[sr] = function (fn) { return fn ? this.bind("resize", debounce(fn)) : this.trigger(sr); };

})(jQuery, "smartresize");


function PopulateLEDS(dtcregdump) {
    $("#dtcVersion").val(dtcregdump.Version);
    setPixel(document.getElementById("dtcResetLED"), dtcregdump.ResetDTC, "RW");
    setPixel(document.getElementById("serdesResetLED"), dtcregdump.ResetSERDESOscillator, "RW");
    setPixel(document.getElementById("SERDESOscillatorClockLED"), dtcregdump.SERDESOscillatorClock, "RW");
    setPixel(document.getElementById("SystemClockLED"), dtcregdump.SystemClock, "RW");
    setPixel(document.getElementById("TimingEnabledLED"), dtcregdump.TimingEnable, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink0LED"), dtcregdump.Link0.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink1LED"), dtcregdump.Link1.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink2LED"), dtcregdump.Link2.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink3LED"), dtcregdump.Link3.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink4LED"), dtcregdump.Link4.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink5LED"), dtcregdump.Link5.ROCEmulator, "RW");
    $("#triggerDMALength").val(dtcregdump.TriggerDMALength);
    $("#minDMALength").val(dtcregdump.MinDMALength);
    $("#dmaTimeoutPreset").val(dtcregdump.DMATimeout);
    $("#dataPendingTimer").val(dtcregdump.DataPendingTimer);
    $("#packetSize").val(dtcregdump.PacketSize);
    setPixel(document.getElementById("simModeDisabledLED"), dtcregdump.SimMode.Disabled, "RW");
    setPixel(document.getElementById("simModeTrackerLED"), dtcregdump.SimMode.Tracker, "RW");
    setPixel(document.getElementById("simModeCalorimeterLED"), dtcregdump.SimMode.Calorimeter, "RW");
    setPixel(document.getElementById("simModeCRVLED"), dtcregdump.SimMode.CosmicVeto, "RW");
    setPixel(document.getElementById("simModeHardwareLED"), dtcregdump.SimMode.Hardware, "RW");
    setPixel(document.getElementById("Link0TxEnabledLED"), dtcregdump.Link0.TxEnabled, "RW");
    setPixel(document.getElementById("Link1TxEnabledLED"), dtcregdump.Link1.TxEnabled, "RW");
    setPixel(document.getElementById("Link2TxEnabledLED"), dtcregdump.Link2.TxEnabled, "RW");
    setPixel(document.getElementById("Link3TxEnabledLED"), dtcregdump.Link3.TxEnabled, "RW");
    setPixel(document.getElementById("Link4TxEnabledLED"), dtcregdump.Link4.TxEnabled, "RW");
    setPixel(document.getElementById("Link5TxEnabledLED"), dtcregdump.Link5.TxEnabled, "RW");
    setPixel(document.getElementById("CFOTxEnabledLED"), dtcregdump.CFO.TxEnabled, "RW");
    setPixel(document.getElementById("Link0RxEnabledLED"), dtcregdump.Link0.RxEnabled, "RW");
    setPixel(document.getElementById("Link1RxEnabledLED"), dtcregdump.Link1.RxEnabled, "RW");
    setPixel(document.getElementById("Link2RxEnabledLED"), dtcregdump.Link2.RxEnabled, "RW");
    setPixel(document.getElementById("Link3RxEnabledLED"), dtcregdump.Link3.RxEnabled, "RW");
    setPixel(document.getElementById("Link4RxEnabledLED"), dtcregdump.Link4.RxEnabled, "RW");
    setPixel(document.getElementById("Link5RxEnabledLED"), dtcregdump.Link5.RxEnabled, "RW");
    setPixel(document.getElementById("CFORxEnabledLED"), dtcregdump.CFO.RxEnabled, "RW");
    setPixel(document.getElementById("Link0TimingEnabledLED"), dtcregdump.Link0.TimingEnabled, "RW");
    setPixel(document.getElementById("Link1TimingEnabledLED"), dtcregdump.Link1.TimingEnabled, "RW");
    setPixel(document.getElementById("Link2TimingEnabledLED"), dtcregdump.Link2.TimingEnabled, "RW");
    setPixel(document.getElementById("Link3TimingEnabledLED"), dtcregdump.Link3.TimingEnabled, "RW");
    setPixel(document.getElementById("Link4TimingEnabledLED"), dtcregdump.Link4.TimingEnabled, "RW");
    setPixel(document.getElementById("Link5TimingEnabledLED"), dtcregdump.Link5.TimingEnabled, "RW");
    setPixel(document.getElementById("R0ROC0LED"), dtcregdump.Link0.ROC0Enabled, "RW");
    setPixel(document.getElementById("R0ROC1LED"), dtcregdump.Link0.ROC1Enabled, "RW");
    setPixel(document.getElementById("R0ROC2LED"), dtcregdump.Link0.ROC2Enabled, "RW");
    setPixel(document.getElementById("R0ROC3LED"), dtcregdump.Link0.ROC3Enabled, "RW");
    setPixel(document.getElementById("R0ROC4LED"), dtcregdump.Link0.ROC4Enabled, "RW");
    setPixel(document.getElementById("R0ROC5LED"), dtcregdump.Link0.ROC5Enabled, "RW");
    setPixel(document.getElementById("R1ROC0LED"), dtcregdump.Link1.ROC0Enabled, "RW");
    setPixel(document.getElementById("R1ROC1LED"), dtcregdump.Link1.ROC1Enabled, "RW");
    setPixel(document.getElementById("R1ROC2LED"), dtcregdump.Link1.ROC2Enabled, "RW");
    setPixel(document.getElementById("R1ROC3LED"), dtcregdump.Link1.ROC3Enabled, "RW");
    setPixel(document.getElementById("R1ROC4LED"), dtcregdump.Link1.ROC4Enabled, "RW");
    setPixel(document.getElementById("R1ROC5LED"), dtcregdump.Link1.ROC5Enabled, "RW");
    setPixel(document.getElementById("R2ROC0LED"), dtcregdump.Link2.ROC0Enabled, "RW");
    setPixel(document.getElementById("R2ROC1LED"), dtcregdump.Link2.ROC1Enabled, "RW");
    setPixel(document.getElementById("R2ROC2LED"), dtcregdump.Link2.ROC2Enabled, "RW");
    setPixel(document.getElementById("R2ROC3LED"), dtcregdump.Link2.ROC3Enabled, "RW");
    setPixel(document.getElementById("R2ROC4LED"), dtcregdump.Link2.ROC4Enabled, "RW");
    setPixel(document.getElementById("R2ROC5LED"), dtcregdump.Link2.ROC5Enabled, "RW");
    setPixel(document.getElementById("R3ROC0LED"), dtcregdump.Link3.ROC0Enabled, "RW");
    setPixel(document.getElementById("R3ROC1LED"), dtcregdump.Link3.ROC1Enabled, "RW");
    setPixel(document.getElementById("R3ROC2LED"), dtcregdump.Link3.ROC2Enabled, "RW");
    setPixel(document.getElementById("R3ROC3LED"), dtcregdump.Link3.ROC3Enabled, "RW");
    setPixel(document.getElementById("R3ROC4LED"), dtcregdump.Link3.ROC4Enabled, "RW");
    setPixel(document.getElementById("R3ROC5LED"), dtcregdump.Link3.ROC5Enabled, "RW");
    setPixel(document.getElementById("R4ROC0LED"), dtcregdump.Link4.ROC0Enabled, "RW");
    setPixel(document.getElementById("R4ROC1LED"), dtcregdump.Link4.ROC1Enabled, "RW");
    setPixel(document.getElementById("R4ROC2LED"), dtcregdump.Link4.ROC2Enabled, "RW");
    setPixel(document.getElementById("R4ROC3LED"), dtcregdump.Link4.ROC3Enabled, "RW");
    setPixel(document.getElementById("R4ROC4LED"), dtcregdump.Link4.ROC4Enabled, "RW");
    setPixel(document.getElementById("R4ROC5LED"), dtcregdump.Link4.ROC5Enabled, "RW");
    setPixel(document.getElementById("R5ROC0LED"), dtcregdump.Link5.ROC0Enabled, "RW");
    setPixel(document.getElementById("R5ROC1LED"), dtcregdump.Link5.ROC1Enabled, "RW");
    setPixel(document.getElementById("R5ROC2LED"), dtcregdump.Link5.ROC2Enabled, "RW");
    setPixel(document.getElementById("R5ROC3LED"), dtcregdump.Link5.ROC3Enabled, "RW");
    setPixel(document.getElementById("R5ROC4LED"), dtcregdump.Link5.ROC4Enabled, "RW");
    setPixel(document.getElementById("R5ROC5LED"), dtcregdump.Link5.ROC5Enabled, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSLink0LED"), dtcregdump.Link0.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMALink0LED"), dtcregdump.Link0.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMALink0LED"), dtcregdump.Link0.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSLink0LED"), dtcregdump.Link0.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSLink1LED"), dtcregdump.Link1.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMALink1LED"), dtcregdump.Link1.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMALink1LED"), dtcregdump.Link1.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSLink1LED"), dtcregdump.Link1.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSLink2LED"), dtcregdump.Link2.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMALink2LED"), dtcregdump.Link2.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMALink2LED"), dtcregdump.Link2.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSLink2LED"), dtcregdump.Link2.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSLink3LED"), dtcregdump.Link3.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMALink3LED"), dtcregdump.Link3.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMALink3LED"), dtcregdump.Link3.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSLink3LED"), dtcregdump.Link3.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSLink4LED"), dtcregdump.Link4.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMALink4LED"), dtcregdump.Link4.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMALink4LED"), dtcregdump.Link4.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSLink4LED"), dtcregdump.Link4.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSLink5LED"), dtcregdump.Link5.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMALink5LED"), dtcregdump.Link5.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMALink5LED"), dtcregdump.Link5.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSLink5LED"), dtcregdump.Link5.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSCFOLED"), dtcregdump.CFO.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMACFOLED"), dtcregdump.CFO.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMACFOLED"), dtcregdump.CFO.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSCFOLED"), dtcregdump.CFO.SERDESLoopback.FEPCS, "RW");
}

function SetSimMode(id) {
    var post = {};
    post.id = id;
    AjaxPost("/DTC/SetSimMode", post, function (output) {
        setPixel(document.getElementById("simModeDisabledLED"), output.Value1.Disabled, "RW");
        setPixel(document.getElementById("simModeTrackerLED"), output.Value1.Tracker, "RW");
        setPixel(document.getElementById("simModeCalorimeterLED"), output.Value1.Calorimeter, "RW");
        setPixel(document.getElementById("simModeCRVLED"), output.Value1.CosmicVeto, "RW");
        setPixel(document.getElementById("simModeHardwareLED"), output.Value1.Hardware, "RW");
    });
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
            packet.linkID = parseInt($("#linkID", this).val());
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

        AjaxPost("/DTC/DMAIO", { 'packets': JSON.stringify(packets) }, function (data) {
            $("#response").val(data);
        });
    });

    var dma0 = {
        dma0TX: { data: [{ time: 0, value: 0 }], color: "red", jsonPath: "/DTC/DMA0Transmit" },
        dma0RX: { data: [{ time: 0, value: 0 }], color: "blue", jsonPath: "/DTC/DMA0Receive" },
    };
    //makeGraph("#dma0", dma0);
    var dma1 = {
        dma1TX: { data: [{ time: 0, value: 0 }], color: "red", jsonPath: "/DTC/DMA1Transmit" },
        dma1RX: { data: [{ time: 0, value: 0 }], color: "blue", jsonPath: "/DTC/DMA1Receive" },
    };
    //makeGraph("#dma1", dma1);

    $(window).smartresize(function () {
        $("#dma0").empty();
        $("#dma1").empty();
        //makeGraph("#dma0", dma0);
        //makeGraph("#dma1", dma1);
    });
});
