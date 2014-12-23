var path0Test = false,
    path1Test = false;


function AjaxPost(urlString, dataIn, fnCallback) {
    // Check to see if there is currently an AJAX
    // request on this method.
    if (AjaxPost.Xhr) {
        // Abort the current request.
        AjaxPost.Xhr.abort();
    }
    // Get data via AJAX. Store the XHR (AJAX request
    // object in the method in case we need to abort
    // it on subsequent requests.
    AjaxPost.Xhr = $.ajax({
        type: "post",
        url: urlString,
        data: dataIn,
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
            AjaxPost.Xhr = null;
        }
    });
}

function readSystemState() {
    AjaxPost("/DTC/SystemStatus", null, function (returnValue) {
        var status = returnValue.Value1;
        $('#path0TXThroughput').val(status.path0.TX.stats.Throughput);
        $('#path0RXThroughput').val(status.path0.RX.stats.Throughput);
        $("#path0TXDMAActive").val(status.path0.TX.stats.DMAActive);
        $("#path0RXDMAActive").val(status.path0.RX.stats.DMAActive);
        $("#path0TXDMAWait").val(status.path0.TX.stats.DMAWait);
        $("#path0RXDMAWait").val(status.path0.RX.stats.DMAWait);
        $("#path0TXBDErrors").val(status.path0.TX.BDErrors);
        $("#path0RXBDErrors").val(status.path0.RX.BDErrors);
        $("#path0TXBDSErrors").val(status.path0.TX.BDSErrors);
        $("#path0TXSWBDs").val(status.path0.TX.SWBDs);
        $("#path0RXSWBDs").val(status.path0.RX.SWBDs);
        $("#path0TXSWBuffs").val(status.path0.TX.SWBuffs);
        $("#path0RXSWBuffs").val(status.path0.RX.SWBuffs);
        $("#path0InterruptsEnabled").prop('checked', status.path0.TX.Interrupts);
        $("#path1TXThroughput").val(status.path1.TX.stats.Throughput);
        $("#path1RXThroughput").val(status.path1.RX.stats.Throughput);
        $("#path1TXDMAActive").val(status.path1.TX.stats.DMAActive);
        $("#path1RXDMAActive").val(status.path1.RX.stats.DMAActive);
        $("#path1TXDMAWait").val(status.path1.TX.stats.DMAWait);
        $("#path1RXDMAWait").val(status.path1.RX.stats.DMAWait);
        $("#path1TXBDErrors").val(status.path1.TX.BDErrors);
        $("#path1RXBDErrors").val(status.path1.RX.BDErrors);
        $("#path1TXBDSErrors").val(status.path1.TX.BDSErrors);
        $("#path1TXSWBDs").val(status.path1.TX.SWBDs);
        $("#path1RXSWBDs").val(status.path1.RX.SWBDs);
        $("#path1TXSWBuffs").val(status.path1.TX.SWBuffs);
        $("#path1RXSWBuffs").val(status.path1.RX.SWBuffs);
        $("#path1InterruptsEnabled").prop('checked', status.path1.TX.Interrupts);
        $("#pcieWritesRate").val(status.pcie.stats.WritesRate);
        $("#pcieReadsRate").val(status.pcie.stats.ReadsRate);
        $("#pcieLinkStatus").val(status.pcie.LinkStatus);
        $("#pcieVendorID").val(status.pcie.VendorID);
        $("#pcieLinkSpeed").val(status.pcie.LinkSpeed);
        $("#pcieDeviceID").val(status.pcie.DeviceID);
        $("#pcieLinkWidth").val(status.pcie.LinkWidth);
        $("#pcieMPSBytes").val(status.pcie.MPS);
        $("#pcieInterrupts").val(status.pcie.Interrupts);
        $("#pcieMRRSBytes").val(status.pcie.MRRS);
        $("#phCredits").val(status.pcie.credits.ph);
        $("#pdCredits").val(status.pcie.credits.pd);
        $("#nphCredits").val(status.pcie.credits.nph);
        $("#npdCredits").val(status.pcie.credits.npd);
        $("#chCredits").val(status.pcie.credits.ch);
        $("#cdCredits").val(status.pcie.credits.cd);
    });
};

function path0StartStopClicked() {
    var data = {
        dma: 0,
        started: !path0Test,
        size: $('path0PacketSize').val(),
        loopbackEnabled: $('#path0Loopback').is(":checked"),
        txChecker: $('#path0TXChecker').is(":checked"),  
        rxGenerator: $('#path0RXGenerator').is(":checked"),
    };
    AjaxPost("DTC/TestControl", data, function (returnValue) {
        if (returnValue.Value1) {
            $("#path0StartStopButton").val("Stop Test");
            path0Test = true;
        } else {
            $("#path0StartStopButton").val("Start Test");
            path0Test = false;
        }
    });
};

function path1StartStopClicked() {
    var data = {
        dma: 1,
        started: !path1Test,
        size: $('path1PacketSize').val(),
        loopbackEnabled: $('#path1Loopback').is(":checked"),
        txChecker: $('#path1TXChecker').is(":checked"),  
        rxGenerator: $('#path1RXGenerator').is(":checked"),
    };
    AjaxPost("DTC/TestControl", data, function (returnValue) {
        if (returnValue.Value1) {
            $("#path1StartStopButton").val("Stop Test");
            path1Test = true;
        } else {
            $("#path1StartStopButton").val("Start Test");
            path1Test = false;
        }
    });
};

$(document).ready(function () {
    $('.tabs .tab-links a').on('click', function (e) {
        var currentAttrValue = $(this).attr('href');
        
        // Show/Hide Tabs
        $('.tabs ' + currentAttrValue).show().siblings().hide();
        
        // Change/remove current tab to active
        $(this).parent('li').addClass('active').siblings().removeClass('active');
        
        e.preventDefault();
    });
    
    $('#ring0Loopback').change(function () {
        if ($('#ring0Loopback').is(":checked")) {
            $('#ring0TXChecker').prop('checked', false);
            $('#ring0RXGenerator').prop('checked', false);
            $('#ring0TXChecker').prop('disabled', true);
            $('#ring0RXGenerator').prop('disabled', true);
        } else {
            $('#ring0TXChecker').prop('disabled', false);
            $('#ring0RXGenerator').prop('disabled', false);
        }
    });
    $('#ring1Loopback').change(function () {
        if ($('#ring1Loopback').is(":checked")) {
            $('#ring1TXChecker').prop('checked', false);
            $('#ring1RXGenerator').prop('checked', false);
            $('#ring1TXChecker').prop('disabled', true);
            $('#ring1RXGenerator').prop('disabled', true);
        } else {
            $('#ring1TXChecker').prop('disabled', false);
            $('#ring1RXGenerator').prop('disabled', false);
        }
    });
    $('#ring0TXChecker').change(function () {
        if ($('#ring0TXChecker').is(":checked")) {
            $('#ring0Loopback').prop('checked', false);
            $('#ring0Loopback').prop('disabled', true);
        } else if (!$('#ring0RXGenerator').is(":checked")) {
            $('#ring0Loopback').prop('disabled', false);
        }
    });
    $('#ring0RXGenerator').change(function () {
        if ($('#ring0RXGenerator').is(":checked")) {
            $('#ring0Loopback').prop('checked', false);
            $('#ring0Loopback').prop('disabled', true);
        } else if (!$('#ring0TXChecker').is(":checked")) {
            $('#ring0Loopback').prop('disabled', false);
        }
    });
    $('#ring1TXChecker').change(function () {
        if ($('#ring1TXChecker').is(":checked")) {
            $('#ring1Loopback').prop('checked', false);
            $('#ring1Loopback').prop('disabled', true);
        } else if (!$('#ring1RXGenerator').is(":checked")) {
            $('#ring1Loopback').prop('disabled', false);
        }
    });
    $('#ring1RXGenerator').change(function () {
        if ($('#ring1RXGenerator').is(":checked")) {
            $('#ring1Loopback').prop('checked', false);
            $('#ring1Loopback').prop('disabled', true);
        } else if (!$('#ring1TXChecker').is(":checked")) {
            $('#ring1Loopback').prop('disabled', false);
        }
    });
    setInterval(readSystemState(), 1000);
    makeGraph("#dma0TX", { dma0TX: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/DMA0Transmit" } });
    makeGraph("#dma0RX", { dma0RX: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/DMA0Receive" } });
    makeGraph("#dma1TX", { dma1TX: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/DMA1Transmit" } });
    makeGraph("#dma1RX", { dma1RX: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/DMA1Receive" } });
    $('.tabs #tab3').show().siblings().hide();
    makeGraph("#pcieTX", { pcieTX: { data: [{ time: 0, value: 0 }], color: 'red' }, jsonPath: "/DTC/PCIeTransmit" });
    makeGraph("#pcieRX", { pcieRX: { data: [{ time: 0, value: 0 }], color: 'red' }, jsonPath: "/DTC/PCIeReceive" });
    $('.tabs #tab1').show().siblings().hide();
});