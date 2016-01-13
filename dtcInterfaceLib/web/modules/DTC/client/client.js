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
        ctx.strokeStyle = "darkred";
        if (bit) {
        ctx.fillStyle = "red";
    } else {
        ctx.fillStyle = 'black';
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
    var jBase = $('input[name=base]:checked');
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
    setPixel(document.getElementById("ROCEmulatorEnabledRing0LED"), dtcregdump.Ring0.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing1LED"), dtcregdump.Ring1.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing2LED"), dtcregdump.Ring2.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing3LED"), dtcregdump.Ring3.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing4LED"), dtcregdump.Ring4.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing5LED"), dtcregdump.Ring5.ROCEmulator, "RW");
    setPixel(document.getElementById("Ring0TxEnabledLED"), dtcregdump.Ring0.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring1TxEnabledLED"), dtcregdump.Ring1.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring2TxEnabledLED"), dtcregdump.Ring2.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring3TxEnabledLED"), dtcregdump.Ring3.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring4TxEnabledLED"), dtcregdump.Ring4.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring5TxEnabledLED"), dtcregdump.Ring5.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("CFOTxEnabledLED"), dtcregdump.CFO.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring0RxEnabledLED"), dtcregdump.Ring0.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring1RxEnabledLED"), dtcregdump.Ring1.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring2RxEnabledLED"), dtcregdump.Ring2.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring3RxEnabledLED"), dtcregdump.Ring3.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring4RxEnabledLED"), dtcregdump.Ring4.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring5RxEnabledLED"), dtcregdump.Ring5.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("CFORxEnabledLED"), dtcregdump.CFO.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring0TimingEnabledLED"), dtcregdump.Ring0.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Ring1TimingEnabledLED"), dtcregdump.Ring1.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Ring2TimingEnabledLED"), dtcregdump.Ring2.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Ring3TimingEnabledLED"), dtcregdump.Ring3.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Ring4TimingEnabledLED"), dtcregdump.Ring4.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Ring5TimingEnabledLED"), dtcregdump.Ring5.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("R0ROC0LED"), dtcregdump.Ring0.ROC0Enabled, "RW");
    setPixel(document.getElementById("R0ROC1LED"), dtcregdump.Ring0.ROC1Enabled, "RW");
    setPixel(document.getElementById("R0ROC2LED"), dtcregdump.Ring0.ROC2Enabled, "RW");
    setPixel(document.getElementById("R0ROC3LED"), dtcregdump.Ring0.ROC3Enabled, "RW");
    setPixel(document.getElementById("R0ROC4LED"), dtcregdump.Ring0.ROC4Enabled, "RW");
    setPixel(document.getElementById("R0ROC5LED"), dtcregdump.Ring0.ROC5Enabled, "RW");
    setPixel(document.getElementById("R1ROC0LED"), dtcregdump.Ring1.ROC0Enabled, "RW");
    setPixel(document.getElementById("R1ROC1LED"), dtcregdump.Ring1.ROC1Enabled, "RW");
    setPixel(document.getElementById("R1ROC2LED"), dtcregdump.Ring1.ROC2Enabled, "RW");
    setPixel(document.getElementById("R1ROC3LED"), dtcregdump.Ring1.ROC3Enabled, "RW");
    setPixel(document.getElementById("R1ROC4LED"), dtcregdump.Ring1.ROC4Enabled, "RW");
    setPixel(document.getElementById("R1ROC5LED"), dtcregdump.Ring1.ROC5Enabled, "RW");
    setPixel(document.getElementById("R2ROC0LED"), dtcregdump.Ring2.ROC0Enabled, "RW");
    setPixel(document.getElementById("R2ROC1LED"), dtcregdump.Ring2.ROC1Enabled, "RW");
    setPixel(document.getElementById("R2ROC2LED"), dtcregdump.Ring2.ROC2Enabled, "RW");
    setPixel(document.getElementById("R2ROC3LED"), dtcregdump.Ring2.ROC3Enabled, "RW");
    setPixel(document.getElementById("R2ROC4LED"), dtcregdump.Ring2.ROC4Enabled, "RW");
    setPixel(document.getElementById("R2ROC5LED"), dtcregdump.Ring2.ROC5Enabled, "RW");
    setPixel(document.getElementById("R3ROC0LED"), dtcregdump.Ring3.ROC0Enabled, "RW");
    setPixel(document.getElementById("R3ROC1LED"), dtcregdump.Ring3.ROC1Enabled, "RW");
    setPixel(document.getElementById("R3ROC2LED"), dtcregdump.Ring3.ROC2Enabled, "RW");
    setPixel(document.getElementById("R3ROC3LED"), dtcregdump.Ring3.ROC3Enabled, "RW");
    setPixel(document.getElementById("R3ROC4LED"), dtcregdump.Ring3.ROC4Enabled, "RW");
    setPixel(document.getElementById("R3ROC5LED"), dtcregdump.Ring3.ROC5Enabled, "RW");
    setPixel(document.getElementById("R4ROC0LED"), dtcregdump.Ring4.ROC0Enabled, "RW");
    setPixel(document.getElementById("R4ROC1LED"), dtcregdump.Ring4.ROC1Enabled, "RW");
    setPixel(document.getElementById("R4ROC2LED"), dtcregdump.Ring4.ROC2Enabled, "RW");
    setPixel(document.getElementById("R4ROC3LED"), dtcregdump.Ring4.ROC3Enabled, "RW");
    setPixel(document.getElementById("R4ROC4LED"), dtcregdump.Ring4.ROC4Enabled, "RW");
    setPixel(document.getElementById("R4ROC5LED"), dtcregdump.Ring4.ROC5Enabled, "RW");
    setPixel(document.getElementById("R5ROC0LED"), dtcregdump.Ring5.ROC0Enabled, "RW");
    setPixel(document.getElementById("R5ROC1LED"), dtcregdump.Ring5.ROC1Enabled, "RW");
    setPixel(document.getElementById("R5ROC2LED"), dtcregdump.Ring5.ROC2Enabled, "RW");
    setPixel(document.getElementById("R5ROC3LED"), dtcregdump.Ring5.ROC3Enabled, "RW");
    setPixel(document.getElementById("R5ROC4LED"), dtcregdump.Ring5.ROC4Enabled, "RW");
    setPixel(document.getElementById("R5ROC5LED"), dtcregdump.Ring5.ROC5Enabled, "RW");
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
    setPixel(document.getElementById("SERDESResetRing0LED"), dtcregdump.Ring0.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing1LED"), dtcregdump.Ring1.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing2LED"), dtcregdump.Ring2.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing3LED"), dtcregdump.Ring3.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing4LED"), dtcregdump.Ring4.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing5LED"), dtcregdump.Ring5.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetCFOLED"), dtcregdump.CFO.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetDoneRing0LED"), dtcregdump.Ring0.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing1LED"), dtcregdump.Ring1.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing2LED"), dtcregdump.Ring2.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing3LED"), dtcregdump.Ring3.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing4LED"), dtcregdump.Ring4.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing5LED"), dtcregdump.Ring5.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneCFOLED"), dtcregdump.CFO.ResetDone, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing0LowLED"), dtcregdump.Ring0.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing0HighLED"), dtcregdump.Ring0.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing1LowLED"), dtcregdump.Ring1.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing1HighLED"), dtcregdump.Ring1.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing2LowLED"), dtcregdump.Ring2.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing2HighLED"), dtcregdump.Ring2.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing3LowLED"), dtcregdump.Ring3.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing3HighLED"), dtcregdump.Ring3.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing4LowLED"), dtcregdump.Ring4.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing4HighLED"), dtcregdump.Ring4.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing5LowLED"), dtcregdump.Ring5.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing5HighLED"), dtcregdump.Ring5.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityCFOLowLED"), dtcregdump.CFO.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityCFOHighLED"), dtcregdump.CFO.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing0LowLED"), dtcregdump.Ring0.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing0HighLED"), dtcregdump.Ring0.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing1LowLED"), dtcregdump.Ring1.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing1HighLED"), dtcregdump.Ring1.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing2LowLED"), dtcregdump.Ring2.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing2HighLED"), dtcregdump.Ring2.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing3LowLED"), dtcregdump.Ring3.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing3HighLED"), dtcregdump.Ring3.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing4LowLED"), dtcregdump.Ring4.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing4HighLED"), dtcregdump.Ring4.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing5LowLED"), dtcregdump.Ring5.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing5HighLED"), dtcregdump.Ring5.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITCFOLowLED"), dtcregdump.CFO.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITCFOHighLED"), dtcregdump.CFO.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing0LED"), dtcregdump.Ring0.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing1LED"), dtcregdump.Ring1.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing2LED"), dtcregdump.Ring2.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing3LED"), dtcregdump.Ring3.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing4LED"), dtcregdump.Ring4.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing5LED"), dtcregdump.Ring5.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockCFOLED"), dtcregdump.CFO.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESLockedRing0LED"), dtcregdump.Ring0.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing1LED"), dtcregdump.Ring1.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing2LED"), dtcregdump.Ring2.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing3LED"), dtcregdump.Ring3.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing4LED"), dtcregdump.Ring4.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing5LED"), dtcregdump.Ring5.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedCFOLED"), dtcregdump.CFO.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing0LED"), dtcregdump.Ring0.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing0LED"), dtcregdump.Ring0.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing1LED"), dtcregdump.Ring1.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing1LED"), dtcregdump.Ring1.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing2LED"), dtcregdump.Ring2.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing2LED"), dtcregdump.Ring2.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing3LED"), dtcregdump.Ring3.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing3LED"), dtcregdump.Ring3.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing4LED"), dtcregdump.Ring4.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing4LED"), dtcregdump.Ring4.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing5LED"), dtcregdump.Ring5.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing5LED"), dtcregdump.Ring5.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowCFOLED"), dtcregdump.CFO.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFCFOLED"), dtcregdump.CFO.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESRXBufferNominalRing0LED"), dtcregdump.Ring0.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing0LED"), dtcregdump.Ring0.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing0LED"), dtcregdump.Ring0.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing0LED"), dtcregdump.Ring0.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing0LED"), dtcregdump.Ring0.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalRing1LED"), dtcregdump.Ring1.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing1LED"), dtcregdump.Ring1.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing1LED"), dtcregdump.Ring1.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing1LED"), dtcregdump.Ring1.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing1LED"), dtcregdump.Ring1.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalRing2LED"), dtcregdump.Ring2.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing2LED"), dtcregdump.Ring2.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing2LED"), dtcregdump.Ring2.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing2LED"), dtcregdump.Ring2.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing2LED"), dtcregdump.Ring2.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalRing3LED"), dtcregdump.Ring3.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing3LED"), dtcregdump.Ring3.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing3LED"), dtcregdump.Ring3.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing3LED"), dtcregdump.Ring3.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing3LED"), dtcregdump.Ring3.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalRing4LED"), dtcregdump.Ring4.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing4LED"), dtcregdump.Ring4.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing4LED"), dtcregdump.Ring4.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing4LED"), dtcregdump.Ring4.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing4LED"), dtcregdump.Ring4.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalRing5LED"), dtcregdump.Ring5.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing5LED"), dtcregdump.Ring5.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing5LED"), dtcregdump.Ring5.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing5LED"), dtcregdump.Ring5.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing5LED"), dtcregdump.Ring5.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalCFOLED"), dtcregdump.CFO.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyCFOLED"), dtcregdump.CFO.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullCFOLED"), dtcregdump.CFO.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFCFOLED"), dtcregdump.CFO.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFCFOLED"), dtcregdump.CFO.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing0LED"), dtcregdump.Ring0.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing0LED"), dtcregdump.Ring0.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing0LED"), dtcregdump.Ring0.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing0LED"), dtcregdump.Ring0.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing0LED"), dtcregdump.Ring0.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing0LED"), dtcregdump.Ring0.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing0LED"), dtcregdump.Ring0.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing0LED"), dtcregdump.Ring0.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing1LED"), dtcregdump.Ring1.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing1LED"), dtcregdump.Ring1.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing1LED"), dtcregdump.Ring1.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing1LED"), dtcregdump.Ring1.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing1LED"), dtcregdump.Ring1.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing1LED"), dtcregdump.Ring1.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing1LED"), dtcregdump.Ring1.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing1LED"), dtcregdump.Ring1.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing2LED"), dtcregdump.Ring2.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing2LED"), dtcregdump.Ring2.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing2LED"), dtcregdump.Ring2.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing2LED"), dtcregdump.Ring2.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing2LED"), dtcregdump.Ring2.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing2LED"), dtcregdump.Ring2.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing2LED"), dtcregdump.Ring2.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing2LED"), dtcregdump.Ring2.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing3LED"), dtcregdump.Ring3.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing3LED"), dtcregdump.Ring3.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing3LED"), dtcregdump.Ring3.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing3LED"), dtcregdump.Ring3.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing3LED"), dtcregdump.Ring3.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing3LED"), dtcregdump.Ring3.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing3LED"), dtcregdump.Ring3.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing3LED"), dtcregdump.Ring3.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing4LED"), dtcregdump.Ring4.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing4LED"), dtcregdump.Ring4.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing4LED"), dtcregdump.Ring4.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing4LED"), dtcregdump.Ring4.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing4LED"), dtcregdump.Ring4.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing4LED"), dtcregdump.Ring4.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing4LED"), dtcregdump.Ring4.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing4LED"), dtcregdump.Ring4.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing5LED"), dtcregdump.Ring5.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing5LED"), dtcregdump.Ring5.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing5LED"), dtcregdump.Ring5.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing5LED"), dtcregdump.Ring5.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing5LED"), dtcregdump.Ring5.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing5LED"), dtcregdump.Ring5.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing5LED"), dtcregdump.Ring5.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing5LED"), dtcregdump.Ring5.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKCFOLED"), dtcregdump.CFO.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedCFOLED"), dtcregdump.CFO.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedCFOLED"), dtcregdump.CFO.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedCFOLED"), dtcregdump.CFO.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorCFOLED"), dtcregdump.CFO.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFCFOLED"), dtcregdump.CFO.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFCFOLED"), dtcregdump.CFO.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorCFOLED"), dtcregdump.CFO.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing0LED"), dtcregdump.Ring0.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing1LED"), dtcregdump.Ring1.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing2LED"), dtcregdump.Ring2.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing3LED"), dtcregdump.Ring3.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing4LED"), dtcregdump.Ring4.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing5LED"), dtcregdump.Ring5.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanCFOLED"), dtcregdump.CFO.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESRXCDRLockRing0LED"), dtcregdump.Ring0.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockRing1LED"), dtcregdump.Ring1.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockRing2LED"), dtcregdump.Ring2.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockRing3LED"), dtcregdump.Ring3.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockRing4LED"), dtcregdump.Ring4.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockRing5LED"), dtcregdump.Ring5.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockCFOLED"), dtcregdump.CFO.RXCDRLock, "RO");
    $("#timestamp").val(dtcregdump.Timestamp.toString(16));
    setPixel(document.getElementById("OutputDataFIFOFullRing0LED"), dtcregdump.Ring0.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing0LED"), dtcregdump.Ring0.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing0LED"), dtcregdump.Ring0.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing0LED"), dtcregdump.Ring0.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing0LED"), dtcregdump.Ring0.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing0LED"), dtcregdump.Ring0.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing0LED"), dtcregdump.Ring0.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing0LED"), dtcregdump.Ring0.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing0LED"), dtcregdump.Ring0.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullRing1LED"), dtcregdump.Ring1.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing1LED"), dtcregdump.Ring1.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing1LED"), dtcregdump.Ring1.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing1LED"), dtcregdump.Ring1.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing1LED"), dtcregdump.Ring1.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing1LED"), dtcregdump.Ring1.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing1LED"), dtcregdump.Ring1.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing1LED"), dtcregdump.Ring1.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing1LED"), dtcregdump.Ring1.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullRing2LED"), dtcregdump.Ring2.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing2LED"), dtcregdump.Ring2.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing2LED"), dtcregdump.Ring2.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing2LED"), dtcregdump.Ring2.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing2LED"), dtcregdump.Ring2.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing2LED"), dtcregdump.Ring2.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing2LED"), dtcregdump.Ring2.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing2LED"), dtcregdump.Ring2.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing2LED"), dtcregdump.Ring2.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullRing3LED"), dtcregdump.Ring3.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing3LED"), dtcregdump.Ring3.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing3LED"), dtcregdump.Ring3.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing3LED"), dtcregdump.Ring3.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing3LED"), dtcregdump.Ring3.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing3LED"), dtcregdump.Ring3.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing3LED"), dtcregdump.Ring3.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing3LED"), dtcregdump.Ring3.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing3LED"), dtcregdump.Ring3.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullRing4LED"), dtcregdump.Ring4.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing4LED"), dtcregdump.Ring4.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing4LED"), dtcregdump.Ring4.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing4LED"), dtcregdump.Ring4.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing4LED"), dtcregdump.Ring4.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing4LED"), dtcregdump.Ring4.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing4LED"), dtcregdump.Ring4.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing4LED"), dtcregdump.Ring4.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing4LED"), dtcregdump.Ring4.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullRing5LED"), dtcregdump.Ring5.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing5LED"), dtcregdump.Ring5.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing5LED"), dtcregdump.Ring5.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing5LED"), dtcregdump.Ring5.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing5LED"), dtcregdump.Ring5.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing5LED"), dtcregdump.Ring5.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing5LED"), dtcregdump.Ring5.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing5LED"), dtcregdump.Ring5.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing5LED"), dtcregdump.Ring5.FIFOFullFlags.DCSInput, "ERR");
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

function LEDAction(url, ring, id) {
    AjaxPost(url, ring, function (output) {
        setPixel(document.getElementById(id), output.Value1, "RW");
    });
}
function LEDExtAction(url, ring, val, id) {
    var post = {};
    post.ring = ring;
    post.val = val;
    AjaxPost(url, post, function (output) {
        setPixel(document.getElementById(id), output.Value1, "RW");
    });
}
function LEDObjAction(url, ring, idlo, idhi) {
    AjaxPost(url, ring, function (output) {
        setPixel(document.getElementById(idlo), output.Value1.Low, "RW");
        setPixel(document.getElementById(idhi), output.Value1.High, "RW");
    });
}
function SetFIFOFlags(ring, id) {
    var post = {};
    post.ring = ring;
    post.id = id;
    AjaxPost("/DTC/SetFIFOFlags", post, function (output) {
        setPixel(document.getElementById("OutputDataFIFOFullRing" + ring + "LED"), output.Value1.OutputData, "RW");
        setPixel(document.getElementById("CFOLinkFIFOFullRing" + ring + "LED"), output.Value1.CFOLink, "RW");
        setPixel(document.getElementById("ReadoutRequestFIFOFullRing" + ring + "LED"), output.Value1.ReadoutRequest, "RW");
        setPixel(document.getElementById("DataRequestFIFOFullRing" + ring + "LED"), output.Value1.DataRequest, "RW");
        setPixel(document.getElementById("OtherOutputFIFOFullRing" + ring + "LED"), output.Value1.OtherOutput, "RW");
        setPixel(document.getElementById("OutputDCSFIFOFullRing" + ring + "LED"), output.Value1.OutputDCS, "RW");
        setPixel(document.getElementById("OutputDCS2FIFOFullRing" + ring + "LED"), output.Value1.OutputDCS2, "RW");
        setPixel(document.getElementById("DataInputFIFOFullRing" + ring + "LED"), output.Value1.DataInput, "RW");
        setPixel(document.getElementById("DCSInputFIFOFullRing" + ring + "LED"), output.Value1.DCSInput, "RW");
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
    })
}

function SetNumericValue(id, url) {
    var value = $(id).val();
    AjaxPost(url, value, function (returnValue) {
        $(id).val(returnValue.Value1);
    });
}

function PostLogMessage() {
    var message = $("#logMessage").val();
    AjaxPost('/DTC/WriteLog', message, function (returnValue) {
        $("#log").val(returnValue.Value1);
    });
}

function ReadLog() {
    AjaxPost('/DTC/ReadLog', null, function (returnValue) {
        $("#log").val(returnValue.Value1);
    });
}

function RunScript() {
    AjaxPost('/DTC/RunScript', $("#script").val(), function (returnValue) {
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
    $('input[name=base]').change(function () {
        var jVal = $("#value");
        var jBase = $('input[name=base]:checked').val();
        var oldval = parseInt(jVal.val(), oldbase);
        jVal.val(oldval.toString(jBase));
        oldbase = jBase;
    });
    $("#logInterval").change(function () {
        if ($("#updateLog").is(':checked')) {
            var newVal = parseInt($("#logInterval").val() * 1000, 10);
            clearInterval(logIntervalHandle);
            logIntervalHandle = setInterval(function () { ReadLog(); }, newVal);
        }
    });
    $("#regInterval").change(function () {
        if ($("#updateReg").is(':checked')) {
            var newVal = parseInt($("#regInterval").val() * 1000, 10);
            clearInterval(regIntervalHandle);
            regIntervalHandle = setInterval(function () { GetRegDump(); }, newVal);
        }
    });
    $("#updateLog").change(function () {
        if ($("#updateLog").is(':checked')) {
            logIntervalHandle = setInterval(function () { ReadLog(); }, parseInt($("#logInterval").val() * 1000, 10));
        }
        else {
            clearInterval(logIntervalHandle);
        }
    });
    logIntervalHandle = setInterval(function () { ReadLog(); }, parseInt($("#logInterval").val() * 1000, 10));
    $("#updateReg").change(function () {
        if ($("#updateReg").is(':checked')) {
            regIntervalHandle = setInterval(function () { GetRegDump(); }, parseInt($("#regInterval").val() * 1000, 10));
        }
        else {
            clearInterval(regIntervalHandle);
        }
    });
    
    var sendIds = {
        send: { data: [{ time: 0, value: 0 }], color: 'black', jsonPath: "/DTC/Send" },
        spayload: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/SPayload" },
    };
    makeGraph("#send", sendIds);
    var recIds = {
        receive: { data: [{ time: 0, value: 0 }], color: 'black', jsonPath: "/DTC/Receive" },
        rpayload: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/RPayload" },
    };
    makeGraph("#receive", recIds);
    
    $(window).smartresize(function () {
        $("#send").empty();
        $("#receive").empty();
        makeGraph("#send", sendIds);
        makeGraph("#receive", recIds);
    });
});
