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
            setPixel(document.getElementById("FormAjaxLED"), 0, "RO");
            fnCallback({
                Value1: objData
            });
        },
        // An error handler for the request.
        error: function (xhr, textStatus, errorCode) {
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
    var jAJAX = $("#ajax");
    var objData = null;
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

function PopulateLEDS(cforegdump) {
    $("#cfoVersion").val(cforegdump.Version);
    setPixel(document.getElementById("cfoResetLED"), cforegdump.ResetCFO, "RW");
    setPixel(document.getElementById("serdesResetLED"), cforegdump.ResetSERDESOscillator, "RW");
    setPixel(document.getElementById("SERDESOscillatorClockLED"), cforegdump.SERDESOscillatorClock, "RW");
    setPixel(document.getElementById("SystemClockLED"), cforegdump.SystemClock, "RW");
    setPixel(document.getElementById("TimingEnabledLED"), cforegdump.TimingEnable, "RW");
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
    setPixel(document.getElementById("SERDESOscillatorIICErrorLED"), cforegdump.SERDESOscillatorIICError, "ERR");
    setPixel(document.getElementById("SERDESOscillatorInitCompleteLED"), cforegdump.SERDESOscillatorInitComplete, "RO");
    setPixel(document.getElementById("ROCEmulatorEnabledRing0LED"), cforegdump.Ring0.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing1LED"), cforegdump.Ring1.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing2LED"), cforegdump.Ring2.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing3LED"), cforegdump.Ring3.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing4LED"), cforegdump.Ring4.ROCEmulator, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledRing5LED"), cforegdump.Ring5.ROCEmulator, "RW");
    setPixel(document.getElementById("Ring0TxEnabledLED"), cforegdump.Ring0.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring1TxEnabledLED"), cforegdump.Ring1.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring2TxEnabledLED"), cforegdump.Ring2.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring3TxEnabledLED"), cforegdump.Ring3.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring4TxEnabledLED"), cforegdump.Ring4.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring5TxEnabledLED"), cforegdump.Ring5.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("CFOTxEnabledLED"), cforegdump.CFO.Enabled.TransmitEnable, "RW");
    setPixel(document.getElementById("Ring0RxEnabledLED"), cforegdump.Ring0.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring1RxEnabledLED"), cforegdump.Ring1.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring2RxEnabledLED"), cforegdump.Ring2.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring3RxEnabledLED"), cforegdump.Ring3.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring4RxEnabledLED"), cforegdump.Ring4.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring5RxEnabledLED"), cforegdump.Ring5.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("CFORxEnabledLED"), cforegdump.CFO.Enabled.ReceiveEnable, "RW");
    setPixel(document.getElementById("Ring0TimingEnabledLED"), cforegdump.Ring0.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Ring1TimingEnabledLED"), cforegdump.Ring1.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Ring2TimingEnabledLED"), cforegdump.Ring2.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Ring3TimingEnabledLED"), cforegdump.Ring3.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Ring4TimingEnabledLED"), cforegdump.Ring4.Enabled.TimingEnable, "RW");
    setPixel(document.getElementById("Ring5TimingEnabledLED"), cforegdump.Ring5.Enabled.TimingEnable, "RW");
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
    setPixel(document.getElementById("SERDESResetRing0LED"), cforegdump.Ring0.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing1LED"), cforegdump.Ring1.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing2LED"), cforegdump.Ring2.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing3LED"), cforegdump.Ring3.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing4LED"), cforegdump.Ring4.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing5LED"), cforegdump.Ring5.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetCFOLED"), cforegdump.CFO.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetDoneRing0LED"), cforegdump.Ring0.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing1LED"), cforegdump.Ring1.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing2LED"), cforegdump.Ring2.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing3LED"), cforegdump.Ring3.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing4LED"), cforegdump.Ring4.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing5LED"), cforegdump.Ring5.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneCFOLED"), cforegdump.CFO.ResetDone, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing0LowLED"), cforegdump.Ring0.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing0HighLED"), cforegdump.Ring0.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing1LowLED"), cforegdump.Ring1.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing1HighLED"), cforegdump.Ring1.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing2LowLED"), cforegdump.Ring2.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing2HighLED"), cforegdump.Ring2.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing3LowLED"), cforegdump.Ring3.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing3HighLED"), cforegdump.Ring3.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing4LowLED"), cforegdump.Ring4.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing4HighLED"), cforegdump.Ring4.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing5LowLED"), cforegdump.Ring5.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityRing5HighLED"), cforegdump.Ring5.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityCFOLowLED"), cforegdump.CFO.SERDESRXDisparity.Low, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityCFOHighLED"), cforegdump.CFO.SERDESRXDisparity.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing0LowLED"), cforegdump.Ring0.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing0HighLED"), cforegdump.Ring0.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing1LowLED"), cforegdump.Ring1.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing1HighLED"), cforegdump.Ring1.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing2LowLED"), cforegdump.Ring2.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing2HighLED"), cforegdump.Ring2.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing3LowLED"), cforegdump.Ring3.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing3HighLED"), cforegdump.Ring3.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing4LowLED"), cforegdump.Ring4.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing4HighLED"), cforegdump.Ring4.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing5LowLED"), cforegdump.Ring5.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITRing5HighLED"), cforegdump.Ring5.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESRXCNITCFOLowLED"), cforegdump.CFO.CharacterError.Low, "ERR");
    setPixel(document.getElementById("SERDESRXCNITCFOHighLED"), cforegdump.CFO.CharacterError.High, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing0LED"), cforegdump.Ring0.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing1LED"), cforegdump.Ring1.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing2LED"), cforegdump.Ring2.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing3LED"), cforegdump.Ring3.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing4LED"), cforegdump.Ring4.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockRing5LED"), cforegdump.Ring5.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESUnlockCFOLED"), cforegdump.CFO.UnlockError, "ERR");
    setPixel(document.getElementById("SERDESLockedRing0LED"), cforegdump.Ring0.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing1LED"), cforegdump.Ring1.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing2LED"), cforegdump.Ring2.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing3LED"), cforegdump.Ring3.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing4LED"), cforegdump.Ring4.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing5LED"), cforegdump.Ring5.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedCFOLED"), cforegdump.CFO.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing0LED"), cforegdump.Ring0.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing0LED"), cforegdump.Ring0.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing1LED"), cforegdump.Ring1.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing1LED"), cforegdump.Ring1.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing2LED"), cforegdump.Ring2.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing2LED"), cforegdump.Ring2.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing3LED"), cforegdump.Ring3.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing3LED"), cforegdump.Ring3.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing4LED"), cforegdump.Ring4.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing4LED"), cforegdump.Ring4.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing5LED"), cforegdump.Ring5.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFRing5LED"), cforegdump.Ring5.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowCFOLED"), cforegdump.CFO.OverflowOrUnderflow, "ERR");
    setPixel(document.getElementById("SERDESTXFIFOHFCFOLED"), cforegdump.CFO.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESRXBufferNominalRing0LED"), cforegdump.Ring0.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing0LED"), cforegdump.Ring0.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing0LED"), cforegdump.Ring0.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing0LED"), cforegdump.Ring0.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing0LED"), cforegdump.Ring0.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalRing1LED"), cforegdump.Ring1.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing1LED"), cforegdump.Ring1.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing1LED"), cforegdump.Ring1.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing1LED"), cforegdump.Ring1.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing1LED"), cforegdump.Ring1.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalRing2LED"), cforegdump.Ring2.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing2LED"), cforegdump.Ring2.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing2LED"), cforegdump.Ring2.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing2LED"), cforegdump.Ring2.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing2LED"), cforegdump.Ring2.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalRing3LED"), cforegdump.Ring3.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing3LED"), cforegdump.Ring3.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing3LED"), cforegdump.Ring3.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing3LED"), cforegdump.Ring3.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing3LED"), cforegdump.Ring3.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalRing4LED"), cforegdump.Ring4.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing4LED"), cforegdump.Ring4.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing4LED"), cforegdump.Ring4.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing4LED"), cforegdump.Ring4.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing4LED"), cforegdump.Ring4.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalRing5LED"), cforegdump.Ring5.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing5LED"), cforegdump.Ring5.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing5LED"), cforegdump.Ring5.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing5LED"), cforegdump.Ring5.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFRing5LED"), cforegdump.Ring5.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferNominalCFOLED"), cforegdump.CFO.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyCFOLED"), cforegdump.CFO.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullCFOLED"), cforegdump.CFO.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFCFOLED"), cforegdump.CFO.RXBufferStatus.Underflow, "ERR");
    setPixel(document.getElementById("SERDESRXBufferOFCFOLED"), cforegdump.CFO.RXBufferStatus.Overflow, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing0LED"), cforegdump.Ring0.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing0LED"), cforegdump.Ring0.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing0LED"), cforegdump.Ring0.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing0LED"), cforegdump.Ring0.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing0LED"), cforegdump.Ring0.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing0LED"), cforegdump.Ring0.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing0LED"), cforegdump.Ring0.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing0LED"), cforegdump.Ring0.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing1LED"), cforegdump.Ring1.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing1LED"), cforegdump.Ring1.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing1LED"), cforegdump.Ring1.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing1LED"), cforegdump.Ring1.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing1LED"), cforegdump.Ring1.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing1LED"), cforegdump.Ring1.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing1LED"), cforegdump.Ring1.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing1LED"), cforegdump.Ring1.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing2LED"), cforegdump.Ring2.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing2LED"), cforegdump.Ring2.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing2LED"), cforegdump.Ring2.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing2LED"), cforegdump.Ring2.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing2LED"), cforegdump.Ring2.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing2LED"), cforegdump.Ring2.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing2LED"), cforegdump.Ring2.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing2LED"), cforegdump.Ring2.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing3LED"), cforegdump.Ring3.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing3LED"), cforegdump.Ring3.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing3LED"), cforegdump.Ring3.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing3LED"), cforegdump.Ring3.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing3LED"), cforegdump.Ring3.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing3LED"), cforegdump.Ring3.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing3LED"), cforegdump.Ring3.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing3LED"), cforegdump.Ring3.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing4LED"), cforegdump.Ring4.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing4LED"), cforegdump.Ring4.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing4LED"), cforegdump.Ring4.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing4LED"), cforegdump.Ring4.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing4LED"), cforegdump.Ring4.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing4LED"), cforegdump.Ring4.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing4LED"), cforegdump.Ring4.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing4LED"), cforegdump.Ring4.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKRing5LED"), cforegdump.Ring5.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedRing5LED"), cforegdump.Ring5.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedRing5LED"), cforegdump.Ring5.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedRing5LED"), cforegdump.Ring5.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorRing5LED"), cforegdump.Ring5.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFRing5LED"), cforegdump.Ring5.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFRing5LED"), cforegdump.Ring5.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorRing5LED"), cforegdump.Ring5.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESRXDataOKCFOLED"), cforegdump.CFO.RXStatus.DataOK, "RO");
    setPixel(document.getElementById("SERDESRXSKPAddedCFOLED"), cforegdump.CFO.RXStatus.SKPAdded, "RO");
    setPixel(document.getElementById("SERDESRXSKPRemovedCFOLED"), cforegdump.CFO.RXStatus.SKPRemoved, "RO");
    setPixel(document.getElementById("SERDESRXReceiverDetectedCFOLED"), cforegdump.CFO.RXStatus.ReceiverDetected, "RO");
    setPixel(document.getElementById("SERDESRXDecodeErrorCFOLED"), cforegdump.CFO.RXStatus.DecodeError, "ERR");
    setPixel(document.getElementById("SERDESRXEOFCFOLED"), cforegdump.CFO.RXStatus.EOverflow, "ERR");
    setPixel(document.getElementById("SERDESRXEUFCFOLED"), cforegdump.CFO.RXStatus.EUnderflow, "ERR");
    setPixel(document.getElementById("SERDESRXDisparityErrorCFOLED"), cforegdump.CFO.RXStatus.DisparityError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing0LED"), cforegdump.Ring0.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing1LED"), cforegdump.Ring1.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing2LED"), cforegdump.Ring2.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing3LED"), cforegdump.Ring3.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing4LED"), cforegdump.Ring4.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanRing5LED"), cforegdump.Ring5.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESEyescanCFOLED"), cforegdump.CFO.EyescanError, "ERR");
    setPixel(document.getElementById("SERDESRXCDRLockRing0LED"), cforegdump.Ring0.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockRing1LED"), cforegdump.Ring1.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockRing2LED"), cforegdump.Ring2.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockRing3LED"), cforegdump.Ring3.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockRing4LED"), cforegdump.Ring4.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockRing5LED"), cforegdump.Ring5.RXCDRLock, "RO");
    setPixel(document.getElementById("SERDESRXCDRLockCFOLED"), cforegdump.CFO.RXCDRLock, "RO");
    $("#timestamp").val(cforegdump.Timestamp.toString(16));
    setPixel(document.getElementById("OutputDataFIFOFullRing0LED"), cforegdump.Ring0.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing0LED"), cforegdump.Ring0.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing0LED"), cforegdump.Ring0.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing0LED"), cforegdump.Ring0.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing0LED"), cforegdump.Ring0.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing0LED"), cforegdump.Ring0.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing0LED"), cforegdump.Ring0.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing0LED"), cforegdump.Ring0.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing0LED"), cforegdump.Ring0.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullRing1LED"), cforegdump.Ring1.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing1LED"), cforegdump.Ring1.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing1LED"), cforegdump.Ring1.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing1LED"), cforegdump.Ring1.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing1LED"), cforegdump.Ring1.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing1LED"), cforegdump.Ring1.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing1LED"), cforegdump.Ring1.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing1LED"), cforegdump.Ring1.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing1LED"), cforegdump.Ring1.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullRing2LED"), cforegdump.Ring2.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing2LED"), cforegdump.Ring2.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing2LED"), cforegdump.Ring2.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing2LED"), cforegdump.Ring2.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing2LED"), cforegdump.Ring2.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing2LED"), cforegdump.Ring2.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing2LED"), cforegdump.Ring2.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing2LED"), cforegdump.Ring2.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing2LED"), cforegdump.Ring2.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullRing3LED"), cforegdump.Ring3.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing3LED"), cforegdump.Ring3.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing3LED"), cforegdump.Ring3.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing3LED"), cforegdump.Ring3.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing3LED"), cforegdump.Ring3.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing3LED"), cforegdump.Ring3.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing3LED"), cforegdump.Ring3.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing3LED"), cforegdump.Ring3.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing3LED"), cforegdump.Ring3.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullRing4LED"), cforegdump.Ring4.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing4LED"), cforegdump.Ring4.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing4LED"), cforegdump.Ring4.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing4LED"), cforegdump.Ring4.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing4LED"), cforegdump.Ring4.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing4LED"), cforegdump.Ring4.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing4LED"), cforegdump.Ring4.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing4LED"), cforegdump.Ring4.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing4LED"), cforegdump.Ring4.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OutputDataFIFOFullRing5LED"), cforegdump.Ring5.FIFOFullFlags.OutputData, "ERR");
    setPixel(document.getElementById("CFOLinkFIFOFullRing5LED"), cforegdump.Ring5.FIFOFullFlags.CFOLink, "ERR");
    setPixel(document.getElementById("ReadoutRequestFIFOFullRing5LED"), cforegdump.Ring5.FIFOFullFlags.ReadoutRequest, "ERR");
    setPixel(document.getElementById("DataRequestFIFOFullRing5LED"), cforegdump.Ring5.FIFOFullFlags.DataRequest, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullRing5LED"), cforegdump.Ring5.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullRing5LED"), cforegdump.Ring5.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullRing5LED"), cforegdump.Ring5.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullRing5LED"), cforegdump.Ring5.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullRing5LED"), cforegdump.Ring5.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("OtherOutputFIFOFullCFOLED"), cforegdump.CFO.FIFOFullFlags.OtherOutput, "ERR");
    setPixel(document.getElementById("OutputDCSFIFOFullCFOLED"), cforegdump.CFO.FIFOFullFlags.OutputDCS, "ERR");
    setPixel(document.getElementById("OutputDCS2FIFOFullCFOLED"), cforegdump.CFO.FIFOFullFlags.OutputDCS2, "ERR");
    setPixel(document.getElementById("DataInputFIFOFullCFOLED"), cforegdump.CFO.FIFOFullFlags.DataInput, "ERR");
    setPixel(document.getElementById("DCSInputFIFOFullCFOLED"), cforegdump.CFO.FIFOFullFlags.DCSInput, "ERR");
    setPixel(document.getElementById("fpgapromfifofullLED"), cforegdump.PROMFIFOFull, "RO");
    setPixel(document.getElementById("fpgapromreadyLED"), cforegdump.PROMReady, "RO");
    setPixel(document.getElementById("fpgaCoreFIFOFullLED"), cforegdump.FPGACoreFIFOFull, "RO");
    
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
        url: "/CFO/regDump",
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
        error: function (xhr, textStatus, errorCode) {
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
function SetFIFOFlags(ring, id) {
    var objData = null;
    var post = {};
    post.ring = ring;
    post.id = id;
    AjaxPost("/CFO/SetFIFOFlags", post, function (output) {
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
        send: { data: [{ time: 0, value: 0 }], color: 'black', jsonPath: "/CFO/Send" },
        spayload: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/CFO/SPayload" },
    };
    makeGraph("#send", sendIds);
    var recIds = {
        receive: { data: [{ time: 0, value: 0 }], color: 'black', jsonPath: "/CFO/Receive" },
        rpayload: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/CFO/RPayload" },
    };
    makeGraph("#receive", recIds);
    
    $(window).smartresize(function () {
        $("#send").empty();
        $("#receive").empty();
        makeGraph("#send", sendIds);
        makeGraph("#receive", recIds);
    });
});
