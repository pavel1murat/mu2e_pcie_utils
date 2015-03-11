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
    AjaxPost('/DTC/WriteLog', message, function (returnValue) {
        $("#log").val(returnValue.Value1);
    });
}

function ReadLog() {
    var objData = null;
    AjaxPost('/DTC/ReadLog', null, function (returnValue) {
        $("#log").val(returnValue.Value1);
    });
}

function RunScript() {
    var objData = null;
    AjaxPost('/DTC/RunScript', $("#script").val(), function (returnValue) {
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


function PopulateLEDS(dtcregdump) {
    $("#dtcVersion").val(dtcregdump.Version.toString(16));
    setPixel(document.getElementById("dtcResetLED"), dtcregdump.ResetDTC, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLED"), dtcregdump.ROCEmulator, "RW");
    setPixel(document.getElementById("CFOEmulatorEnabledLED"), dtcregdump.CFOEmulator, "RW");
    $("#triggerDMALength").val(dtcregdump.TriggerDMALength);
    $("#minDMALength").val(dtcregdump.MinDMALength);
    $("#dmaTimeoutPreset").val(dtcregdump.DMATimeout);
    $("#dataPendingTimer").val(dtcregdump.DataPendingTimer);
    $("#packetSize").val(dtcregdump.PacketSize);
    setPixel(document.getElementById("Ring0EnabledLED"), dtcregdump.Ring0.Enabled, "RW");
    setPixel(document.getElementById("Ring1EnabledLED"), dtcregdump.Ring1.Enabled, "RW");
    setPixel(document.getElementById("Ring2EnabledLED"), dtcregdump.Ring2.Enabled, "RW");
    setPixel(document.getElementById("Ring3EnabledLED"), dtcregdump.Ring3.Enabled, "RW");
    setPixel(document.getElementById("Ring4EnabledLED"), dtcregdump.Ring4.Enabled, "RW");
    setPixel(document.getElementById("Ring5EnabledLED"), dtcregdump.Ring5.Enabled, "RW");
    setPixel(document.getElementById("CFOEnabledLED"), dtcregdump.CFO.Enabled, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing0LED"), dtcregdump.Ring0.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing0LED"), dtcregdump.Ring0.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing0LED"), dtcregdump.Ring0.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing0LED"), dtcregdump.Ring0.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing1LED"), dtcregdump.Ring1.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing1LED"), dtcregdump.Ring1.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing1LED"), dtcregdump.Ring1.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing1LED"), dtcregdump.Ring1.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing2LED"), dtcregdump.Ring2.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing2LED"), dtcregdump.Ring2.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing2LED"), dtcregdump.Ring2.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing2LED"), dtcregdump.Ring2.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing3LED"), dtcregdump.Ring3.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing3LED"), dtcregdump.Ring3.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing3LED"), dtcregdump.Ring3.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing3LED"), dtcregdump.Ring3.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing4LED"), dtcregdump.Ring4.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing4LED"), dtcregdump.Ring4.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing4LED"), dtcregdump.Ring4.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing4LED"), dtcregdump.Ring4.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSRing5LED"), dtcregdump.Ring5.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMARing5LED"), dtcregdump.Ring5.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMARing5LED"), dtcregdump.Ring5.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSRing5LED"), dtcregdump.Ring5.SERDESLoopback.FEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPCSCFOLED"), dtcregdump.CFO.SERDESLoopback.NEPCS, "RW");
    setPixel(document.getElementById("SERDESLoopbackNEPMACFOLED"), dtcregdump.CFO.SERDESLoopback.NEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPMACFOLED"), dtcregdump.CFO.SERDESLoopback.FEPMA, "RW");
    setPixel(document.getElementById("SERDESLoopbackFEPCSCFOLED"), dtcregdump.CFO.SERDESLoopback.FEPCS, "RW");
}


$(function () {
    GetRegDump();
    //setInterval(function () { GetRegDump(); }, 1500);
    
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
                case 2:
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
        dma0TX: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/DMA0Transmit" },
        dma0RX: { data: [{ time: 0, value: 0 }], color: 'blue', jsonPath: "/DTC/DMA0Receive" },
    };
    //makeGraph("#dma0", dma0);
    var dma1 = {
        dma1TX: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/DMA1Transmit" },
        dma1RX: { data: [{ time: 0, value: 0 }], color: 'blue', jsonPath: "/DTC/DMA1Receive" },
    };
    //makeGraph("#dma1", dma1);
    
    $(window).smartresize(function () {
        $("#dma0").empty();
        $("#dma1").empty();
        //makeGraph("#dma0", dma0);
        //makeGraph("#dma1", dma1);
    });
});
