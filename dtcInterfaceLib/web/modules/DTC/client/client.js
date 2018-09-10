var logIntervalHandle;
var regIntervalHandle;

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
        ctx.strokeStyle = "darkred";
        if (bit) {
            ctx.fillStyle = "red";
        } else {
            ctx.fillStyle = "black";
        }
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
    setPixel(document.getElementById("FormAjaxLED"), 1, "RO");
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
            setPixel(document.getElementById("FormAjaxLED"), 0, "RO");
            fnCallback({
                Value1: objData
            });
        },
        // An error handler for the request.
        error: function () {
            //alert("An error occurred:\n" + textStatus + "\n" + errorCode);
            setPixel(document.getElementById("FormAjaxLED"), 1, "ERR");
        },
        // I get called no matter what.
        complete: function () {
            // Remove completed request object.
            GetAJAXValues.Xhr = null;
        }
    });
}

// I handle the updating of the form fields based on the
// selected option of the combo box.
function UpdateFormFields() {
    var jSelect = $("#mode");
    var jAddr = $("#address");
    var jVal = $("#value");
    var jBase = $("input[name=base]:checked");
    // Check to see if we are using AJAX or static data
    // to re-populate the form.

    // Make a remote call to get the remote data.
    // Because we have to do this asynchronously,
    // we have to provide a callback method that
    // will hook the results up to the populate
    // fields method.
    GetAJAXValues(
        jSelect.val(),
        jAddr.val(),
        jVal.val(),
        // Callback method for results.
        function (objRemoteData) {
            PopulateValueBox(
                objRemoteData.Value1,
                jBase.val()
            );
        }
    );
}

function PopulateLEDS(dtcregdump) {
    $("#dtcVersion").val(dtcregdump.Version);
    setPixel(document.getElementById("dtcResetLED"), dtcregdump.ResetDTC, "RW");
    setPixel(document.getElementById("serdesResetLED"), dtcregdump.ResetSERDESOscillator, "RW");
    setPixel(document.getElementById("SERDESOscillatorClockLED"), dtcregdump.SERDESOscillatorClock, "RW");
    setPixel(document.getElementById("SystemClockLED"), dtcregdump.SystemClock, "RW");
    setPixel(document.getElementById("TimingEnabledLED"), dtcregdump.TimingEnable, "RW");
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
    setPixel(document.getElementById("SERDESOscillatorIICErrorLED"), dtcregdump.SERDESOscillatorIICError, "ERR");
    setPixel(document.getElementById("SERDESOscillatorInitCompleteLED"), dtcregdump.SERDESOscillatorInitComplete, "RO");
    setPixel(document.getElementById("ROCEmulatorEnabledLink0LED"), dtcregdump.Link0.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink1LED"), dtcregdump.Link1.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink2LED"), dtcregdump.Link2.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink3LED"), dtcregdump.Link3.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink4LED"), dtcregdump.Link4.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLink5LED"), dtcregdump.Link5.ROCEmulator, "RW");
    setPixel(document.getElementById("Link0TxEnabledLED"), dtcregdump.Link0.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Link1TxEnabledLED"), dtcregdump.Link1.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Link2TxEnabledLED"), dtcregdump.Link2.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Link3TxEnabledLED"), dtcregdump.Link3.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Link4TxEnabledLED"), dtcregdump.Link4.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Link5TxEnabledLED"), dtcregdump.Link5.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("CFOTxEnabledLED"), dtcregdump.CFO.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Link0RxEnabledLED"), dtcregdump.Link0.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Link1RxEnabledLED"), dtcregdump.Link1.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Link2RxEnabledLED"), dtcregdump.Link2.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Link3RxEnabledLED"), dtcregdump.Link3.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Link4RxEnabledLED"), dtcregdump.Link4.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Link5RxEnabledLED"), dtcregdump.Link5.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("CFORxEnabledLED"), dtcregdump.CFO.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Link0TimingEnabledLED"), dtcregdump.Link0.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Link1TimingEnabledLED"), dtcregdump.Link1.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Link2TimingEnabledLED"), dtcregdump.Link2.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Link3TimingEnabledLED"), dtcregdump.Link3.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Link4TimingEnabledLED"), dtcregdump.Link4.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Link5TimingEnabledLED"), dtcregdump.Link5.Enabled.TimingEnable, "RW");
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
    setPixel(document.getElementById("SERDESResetRing0LED"), dtcregdump.Link0.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing1LED"), dtcregdump.Link1.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing2LED"), dtcregdump.Link2.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing3LED"), dtcregdump.Link3.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing4LED"), dtcregdump.Link4.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing5LED"), dtcregdump.Link5.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetCFOLED"), dtcregdump.CFO.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetDoneLink0LED"), dtcregdump.Link0.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneLink1LED"), dtcregdump.Link1.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneLink2LED"), dtcregdump.Link2.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneLink3LED"), dtcregdump.Link3.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneLink4LED"), dtcregdump.Link4.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneLink5LED"), dtcregdump.Link5.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneCFOLED"), dtcregdump.CFO.ResetDone, "RO");
    setPixel(document.getElementById("SERDESRXDisparityLink0LowLED"), dtcregdump.Link0.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink0HighLED"), dtcregdump.Link0.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink1LowLED"), dtcregdump.Link1.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink1HighLED"), dtcregdump.Link1.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink2LowLED"), dtcregdump.Link2.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink2HighLED"), dtcregdump.Link2.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink3LowLED"), dtcregdump.Link3.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink3HighLED"), dtcregdump.Link3.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink4LowLED"), dtcregdump.Link4.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink4HighLED"), dtcregdump.Link4.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink5LowLED"), dtcregdump.Link5.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityLink5HighLED"), dtcregdump.Link5.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityCFOLowLED"), dtcregdump.CFO.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityCFOHighLED"), dtcregdump.CFO.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing0LowLED"), dtcregdump.Link0.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing0HighLED"), dtcregdump.Link0.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing1LowLED"), dtcregdump.Link1.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing1HighLED"), dtcregdump.Link1.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing2LowLED"), dtcregdump.Link2.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing2HighLED"), dtcregdump.Link2.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing3LowLED"), dtcregdump.Link3.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing3HighLED"), dtcregdump.Link3.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing4LowLED"), dtcregdump.Link4.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing4HighLED"), dtcregdump.Link4.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing5LowLED"), dtcregdump.Link5.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing5HighLED"), dtcregdump.Link5.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITCFOLowLED"), dtcregdump.CFO.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITCFOHighLED"), dtcregdump.CFO.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESUnlockLink0LED"), dtcregdump.Link0.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockLink1LED"), dtcregdump.Link1.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockLink2LED"), dtcregdump.Link2.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockLink3LED"), dtcregdump.Link3.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockLink4LED"), dtcregdump.Link4.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockLink5LED"), dtcregdump.Link5.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockCFOLED"), dtcregdump.CFO.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESLockedLink0LED"), dtcregdump.Link0.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedLink1LED"), dtcregdump.Link1.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedLink2LED"), dtcregdump.Link2.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedLink3LED"), dtcregdump.Link3.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedLink4LED"), dtcregdump.Link4.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedLink5LED"), dtcregdump.Link5.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedCFOLED"), dtcregdump.CFO.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowLink0LED"), dtcregdump.Link0.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFLink0LED"), dtcregdump.Link0.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowLink1LED"), dtcregdump.Link1.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFLink1LED"), dtcregdump.Link1.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowLink2LED"), dtcregdump.Link2.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFLink2LED"), dtcregdump.Link2.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowLink3LED"), dtcregdump.Link3.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFLink3LED"), dtcregdump.Link3.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowLink4LED"), dtcregdump.Link4.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFLink4LED"), dtcregdump.Link4.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowLink5LED"), dtcregdump.Link5.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFLink5LED"), dtcregdump.Link5.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowCFOLED"), dtcregdump.CFO.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFCFOLED"), dtcregdump.CFO.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESRXBufferNominalLink0LED"), dtcregdump.Link0.RXBufferStatus.Nominal, "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyLink0LED"), dtcregdump.Link0.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullLink0LED"), dtcregdump.Link0.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFLink0LED"), dtcregdump.Link0.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFLink0LED"), dtcregdump.Link0.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalLink1LED"), dtcregdump.Link1.RXBufferStatus.Nominal, "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyLink1LED"), dtcregdump.Link1.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullLink1LED"), dtcregdump.Link1.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFLink1LED"), dtcregdump.Link1.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFLink1LED"), dtcregdump.Link1.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalLink2LED"), dtcregdump.Link2.RXBufferStatus.Nominal, "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyLink2LED"), dtcregdump.Link2.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullLink2LED"), dtcregdump.Link2.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFLink2LED"), dtcregdump.Link2.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFLink2LED"), dtcregdump.Link2.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalLink3LED"), dtcregdump.Link3.RXBufferStatus.Nominal, "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyLink3LED"), dtcregdump.Link3.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullLink3LED"), dtcregdump.Link3.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFLink3LED"), dtcregdump.Link3.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFLink3LED"), dtcregdump.Link3.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalLink4LED"), dtcregdump.Link4.RXBufferStatus.Nominal, "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyLink4LED"), dtcregdump.Link4.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullLink4LED"), dtcregdump.Link4.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFLink4LED"), dtcregdump.Link4.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFLink4LED"), dtcregdump.Link4.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalLink5LED"), dtcregdump.Link5.RXBufferStatus.Nominal, "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyLink5LED"), dtcregdump.Link5.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullLink5LED"), dtcregdump.Link5.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFLink5LED"), dtcregdump.Link5.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFLink5LED"), dtcregdump.Link5.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalCFOLED"), dtcregdump.CFO.RXBufferStatus.Nominal, "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyCFOLED"), dtcregdump.CFO.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullCFOLED"), dtcregdump.CFO.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFCFOLED"), dtcregdump.CFO.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFCFOLED"), dtcregdump.CFO.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKLink0LED"), dtcregdump.Link0.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedLink0LED"), dtcregdump.Link0.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedLink0LED"), dtcregdump.Link0.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedLink0LED"), dtcregdump.Link0.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorLink0LED"), dtcregdump.Link0.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFLink0LED"), dtcregdump.Link0.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFLink0LED"), dtcregdump.Link0.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorLink0LED"), dtcregdump.Link0.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKLink1LED"), dtcregdump.Link1.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedLink1LED"), dtcregdump.Link1.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedLink1LED"), dtcregdump.Link1.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedLink1LED"), dtcregdump.Link1.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorLink1LED"), dtcregdump.Link1.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFLink1LED"), dtcregdump.Link1.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFLink1LED"), dtcregdump.Link1.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorLink1LED"), dtcregdump.Link1.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKLink2LED"), dtcregdump.Link2.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedLink2LED"), dtcregdump.Link2.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedLink2LED"), dtcregdump.Link2.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedLink2LED"), dtcregdump.Link2.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorLink2LED"), dtcregdump.Link2.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFLink2LED"), dtcregdump.Link2.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFLink2LED"), dtcregdump.Link2.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorLink2LED"), dtcregdump.Link2.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKLink3LED"), dtcregdump.Link3.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedLink3LED"), dtcregdump.Link3.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedLink3LED"), dtcregdump.Link3.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedLink3LED"), dtcregdump.Link3.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorLink3LED"), dtcregdump.Link3.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFLink3LED"), dtcregdump.Link3.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFLink3LED"), dtcregdump.Link3.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorLink3LED"), dtcregdump.Link3.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKLink4LED"), dtcregdump.Link4.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedLink4LED"), dtcregdump.Link4.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedLink4LED"), dtcregdump.Link4.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedLink4LED"), dtcregdump.Link4.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorLink4LED"), dtcregdump.Link4.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFLink4LED"), dtcregdump.Link4.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFLink4LED"), dtcregdump.Link4.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorLink4LED"), dtcregdump.Link4.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKLink5LED"), dtcregdump.Link5.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedLink5LED"), dtcregdump.Link5.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedLink5LED"), dtcregdump.Link5.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedLink5LED"), dtcregdump.Link5.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorLink5LED"), dtcregdump.Link5.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFLink5LED"), dtcregdump.Link5.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFLink5LED"), dtcregdump.Link5.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorLink5LED"), dtcregdump.Link5.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKCFOLED"), dtcregdump.CFO.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedCFOLED"), dtcregdump.CFO.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedCFOLED"), dtcregdump.CFO.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedCFOLED"), dtcregdump.CFO.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorCFOLED"), dtcregdump.CFO.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFCFOLED"), dtcregdump.CFO.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFCFOLED"), dtcregdump.CFO.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorCFOLED"), dtcregdump.CFO.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESEyescanLink0LED"), dtcregdump.Link0.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanLink1LED"), dtcregdump.Link1.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanLink2LED"), dtcregdump.Link2.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanLink3LED"), dtcregdump.Link3.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanLink4LED"), dtcregdump.Link4.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanLink5LED"), dtcregdump.Link5.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanCFOLED"), dtcregdump.CFO.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESRXCDRLockLink0LED"), dtcregdump.Link0.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockLink1LED"), dtcregdump.Link1.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockLink2LED"), dtcregdump.Link2.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockLink3LED"), dtcregdump.Link3.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockLink4LED"), dtcregdump.Link4.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockLink5LED"), dtcregdump.Link5.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockCFOLED"), dtcregdump.CFO.RXCDRLock, "RO");
    $("#timestamp").val(dtcregdump.Timestamp.toString(16));
    setPixel(document.getElementById("OutputDataFIFOFullLink0LED"), dtcregdump.Link0.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullLink0LED"), dtcregdump.Link0.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullLink0LED"), dtcregdump.Link0.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullLink0LED"), dtcregdump.Link0.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullLink0LED"), dtcregdump.Link0.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullLink0LED"), dtcregdump.Link0.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullLink0LED"), dtcregdump.Link0.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullLink0LED"), dtcregdump.Link0.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullLink0LED"), dtcregdump.Link0.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullLink1LED"), dtcregdump.Link1.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullLink1LED"), dtcregdump.Link1.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullLink1LED"), dtcregdump.Link1.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullLink1LED"), dtcregdump.Link1.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullLink1LED"), dtcregdump.Link1.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullLink1LED"), dtcregdump.Link1.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullLink1LED"), dtcregdump.Link1.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullLink1LED"), dtcregdump.Link1.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullLink1LED"), dtcregdump.Link1.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullLink2LED"), dtcregdump.Link2.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullLink2LED"), dtcregdump.Link2.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullLink2LED"), dtcregdump.Link2.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullLink2LED"), dtcregdump.Link2.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullLink2LED"), dtcregdump.Link2.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullLink2LED"), dtcregdump.Link2.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullLink2LED"), dtcregdump.Link2.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullLink2LED"), dtcregdump.Link2.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullLink2LED"), dtcregdump.Link2.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullLink3LED"), dtcregdump.Link3.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullLink3LED"), dtcregdump.Link3.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullLink3LED"), dtcregdump.Link3.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullLink3LED"), dtcregdump.Link3.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullLink3LED"), dtcregdump.Link3.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullLink3LED"), dtcregdump.Link3.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullLink3LED"), dtcregdump.Link3.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullLink3LED"), dtcregdump.Link3.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullLink3LED"), dtcregdump.Link3.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullLink4LED"), dtcregdump.Link4.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullLink4LED"), dtcregdump.Link4.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullLink4LED"), dtcregdump.Link4.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullLink4LED"), dtcregdump.Link4.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullLink4LED"), dtcregdump.Link4.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullLink4LED"), dtcregdump.Link4.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullLink4LED"), dtcregdump.Link4.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullLink4LED"), dtcregdump.Link4.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullLink4LED"), dtcregdump.Link4.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullLink5LED"), dtcregdump.Link5.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullLink5LED"), dtcregdump.Link5.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullLink5LED"), dtcregdump.Link5.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullLink5LED"), dtcregdump.Link5.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullLink5LED"), dtcregdump.Link5.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullLink5LED"), dtcregdump.Link5.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullLink5LED"), dtcregdump.Link5.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullLink5LED"), dtcregdump.Link5.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullLink5LED"), dtcregdump.Link5.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullCFOLED"), dtcregdump.CFO.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullCFOLED"), dtcregdump.CFO.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullCFOLED"), dtcregdump.CFO.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullCFOLED"), dtcregdump.CFO.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullCFOLED"), dtcregdump.CFO.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("fpgapromfifofullLED"), dtcregdump.PROMFIFOFull, "RO");
    setPixel(document.getElementById("fpgapromreadyLED"), dtcregdump.PROMReady, "RO");
    setPixel(document.getElementById("fpgaCoreFIFOFullLED"), dtcregdump.FPGACoreFIFOFull, "RO");

}

function GetRegDumpAjax(fnCallback) {
    // Check to see if there is currently an AJAX
    // request on this method.
    if (GetRegDumpAjax.Xhr) {
        // Abort the current request.
        GetRegDumpAjax.Xhr.abort();
    }
    setPixel(document.getElementById("RegDumpAjaxLED"), 1, "RO");
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
            setPixel(document.getElementById("RegDumpAjaxLED"), 0, "RO");
            fnCallback({
                Value1: objData
            });
        },
        // An error handler for the request.
        error: function () {
            setPixel(document.getElementById("RegDumpAjaxLED"), 0, "ERR");
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

function SetFIFOFlags(link, id) {
    var post = {};
    post.link = link;
    post.id = id;
    AjaxPost("/DTC/SetFIFOFlags", post, function (output) {
        setPixel(document.getElementById("OutputDataFIFOFullLink" + link + "LED"), output.Value1.OutputData, "RW");
        setPixel(document.getElementById("CFOLinkFIFOFullLink" + link + "LED"), output.Value1.CFOLink, "RW");
        setPixel(document.getElementById("ReadoutRequestFIFOFullLink" + link + "LED"), output.Value1.ReadoutRequest, "RW");
        setPixel(document.getElementById("DataRequestFIFOFullLink" + link + "LED"), output.Value1.DataRequest, "RW");
        setPixel(document.getElementById("OtherOutputFIFOFullLink" + link + "LED"), output.Value1.OtherOutput, "RW");
        setPixel(document.getElementById("OutputDCSFIFOFullLink" + link + "LED"), output.Value1.OutputDCS, "RW");
        setPixel(document.getElementById("OutputDCS2FIFOFullLink" + link + "LED"), output.Value1.OutputDCS2, "RW");
        setPixel(document.getElementById("DataInputFIFOFullLink" + link + "LED"), output.Value1.DataInput, "RW");
        setPixel(document.getElementById("DCSInputFIFOFullLink" + link + "LED"), output.Value1.DCSInput, "RW");
    });
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


// When the DOM is ready to be interacted with, init.
$(function () {
    setPixel(document.getElementById("RegDumpAjaxLED"), 0, "RO");
    setPixel(document.getElementById("AjaxRequestLED"), 0, "RO");
    setPixel(document.getElementById("FormAjaxLED"), 0, "RO");
    GetRegDump();
    ReadLog();
    $("#post").click(function () {
        UpdateFormFields();
    });
    $("input[name=base]").change(function () {
        var jVal = $("#value");
        var jBase = $("input[name=base]:checked").val();
        var oldval = parseInt(jVal.val(), oldbase);
        jVal.val(oldval.toString(jBase));
        oldbase = jBase;
    });
    $("#logInterval").change(function () {
        if ($("#updateLog").is(":checked")) {
            var newVal = parseInt($("#logInterval").val() * 1000, 10);
            clearInterval(logIntervalHandle);
            logIntervalHandle = setInterval(function () { ReadLog(); }, newVal);
        }
    });
    $("#regInterval").change(function () {
        if ($("#updateReg").is(":checked")) {
            var newVal = parseInt($("#regInterval").val() * 1000, 10);
            clearInterval(regIntervalHandle);
            regIntervalHandle = setInterval(function () { GetRegDump(); }, newVal);
        }
    });
    $("#updateLog").change(function () {
        if ($("#updateLog").is(":checked")) {
            logIntervalHandle = setInterval(function () { ReadLog(); }, parseInt($("#logInterval").val() * 1000, 10));
        } else {
            clearInterval(logIntervalHandle);
        }
    });
    logIntervalHandle = setInterval(function () { ReadLog(); }, parseInt($("#logInterval").val() * 1000, 10));
    $("#updateReg").change(function () {
        if ($("#updateReg").is(":checked")) {
            regIntervalHandle = setInterval(function () { GetRegDump(); }, parseInt($("#regInterval").val() * 1000, 10));
        } else {
            clearInterval(regIntervalHandle);
        }
    });

    var sendIds = {
        send: { data: [{ time: 0, value: 0 }], color: "black", jsonPath: "/DTC/Send" },
        spayload: { data: [{ time: 0, value: 0 }], color: "red", jsonPath: "/DTC/SPayload" },
    };
    makeGraph("#send", sendIds);
    var recIds = {
        receive: { data: [{ time: 0, value: 0 }], color: "black", jsonPath: "/DTC/Receive" },
        rpayload: { data: [{ time: 0, value: 0 }], color: "red", jsonPath: "/DTC/RPayload" },
    };
    makeGraph("#receive", recIds);

    $(window).smartresize(function () {
        $("#send").empty();
        $("#receive").empty();
        makeGraph("#send", sendIds);
        makeGraph("#receive", recIds);
    });
});
