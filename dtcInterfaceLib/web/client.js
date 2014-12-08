/*var socket = io.connect('http://localhost');
socket.on('txt_change', function (data) {
    console.log(data);
    $("#txt").val(data.txt);
});
$(document).ready(function () {
    $("#txt").keyup(function () {
        socket.emit('txt_change', { "txt" : $(this).val() });
    });
});*/

function setPixel(led, bit, modestring) {
    ctx = led.getContext("2d");
    
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
    }
    else {
        ctx.strokeStype = "darkred";
        ctx.fillStyle = "red";
    }
    ctx.beginPath();
    ctx.moveTo(led.width / 4, ctx.lineWidth); // Create a starting point
    ctx.lineTo(led.width * 3 / 4, ctx.lineWidth); // Create a horizontal line
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
        url: "./dtc_register_io",
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
        error: function () {
            alert("An error occurred");
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

function PopulateLEDS(dtcregdump) {
    $("#dtcVersion").val(dtcregdump.Version.toString(16));
    setPixel(document.getElementById("dtcResetLED"), dtcregdump.ResetDTC, "RW");
    setPixel(document.getElementById("clearErrorsLED"), dtcregdump.ClearLatchedErrors, "RW");
    setPixel(document.getElementById("ROCEmulatorEnabledLED"), dtcregdump.ROCEmulator, "RW");
    setPixel(document.getElementById("Ring0EnabledLED"), dtcregdump.Ring0.Enabled, "RW");
    setPixel(document.getElementById("Ring1EnabledLED"), dtcregdump.Ring1.Enabled, "RW");
    setPixel(document.getElementById("Ring2EnabledLED"), dtcregdump.Ring2.Enabled, "RW");
    setPixel(document.getElementById("Ring3EnabledLED"), dtcregdump.Ring3.Enabled, "RW");
    setPixel(document.getElementById("Ring4EnabledLED"), dtcregdump.Ring4.Enabled, "RW");
    setPixel(document.getElementById("Ring5EnabledLED"), dtcregdump.Ring5.Enabled, "RW");
    setPixel(document.getElementById("SERDESLoopbackRing0LED"), dtcregdump.Ring0.SERDESLoopback, "RW");
    setPixel(document.getElementById("SERDESLoopbackRing1LED"), dtcregdump.Ring1.SERDESLoopback, "RW");
    setPixel(document.getElementById("SERDESLoopbackRing2LED"), dtcregdump.Ring2.SERDESLoopback, "RW");
    setPixel(document.getElementById("SERDESLoopbackRing3LED"), dtcregdump.Ring3.SERDESLoopback, "RW");
    setPixel(document.getElementById("SERDESLoopbackRing4LED"), dtcregdump.Ring4.SERDESLoopback, "RW");
    setPixel(document.getElementById("SERDESLoopbackRing5LED"), dtcregdump.Ring5.SERDESLoopback, "RW");
    setPixel(document.getElementById("SERDESResetRing0LED"), dtcregdump.Ring0.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing1LED"), dtcregdump.Ring1.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing2LED"), dtcregdump.Ring2.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing3LED"), dtcregdump.Ring3.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing4LED"), dtcregdump.Ring4.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESResetRing5LED"), dtcregdump.Ring5.ResetSERDES, "RW");
    setPixel(document.getElementById("SERDESRXDisparityRing0LowLED"), dtcregdump.Ring0.SERDESRXDisparity.Low, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing0HighLED"), dtcregdump.Ring0.SERDESRXDisparity.High, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing1LowLED"), dtcregdump.Ring1.SERDESRXDisparity.Low, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing1HighLED"), dtcregdump.Ring1.SERDESRXDisparity.High, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing2LowLED"), dtcregdump.Ring2.SERDESRXDisparity.Low, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing2HighLED"), dtcregdump.Ring2.SERDESRXDisparity.High, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing3LowLED"), dtcregdump.Ring3.SERDESRXDisparity.Low, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing3HighLED"), dtcregdump.Ring3.SERDESRXDisparity.High, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing4LowLED"), dtcregdump.Ring4.SERDESRXDisparity.Low, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing4HighLED"), dtcregdump.Ring4.SERDESRXDisparity.High, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing5LowLED"), dtcregdump.Ring5.SERDESRXDisparity.Low, "RO");
    setPixel(document.getElementById("SERDESRXDisparityRing5HighLED"), dtcregdump.Ring5.SERDESRXDisparity.High, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing0LowLED"), dtcregdump.Ring0.CharacterError.Low, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing0HighLED"), dtcregdump.Ring0.CharacterError.High, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing1LowLED"), dtcregdump.Ring1.CharacterError.Low, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing1HighLED"), dtcregdump.Ring1.CharacterError.High, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing2LowLED"), dtcregdump.Ring2.CharacterError.Low, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing2HighLED"), dtcregdump.Ring2.CharacterError.High, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing3LowLED"), dtcregdump.Ring3.CharacterError.Low, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing3HighLED"), dtcregdump.Ring3.CharacterError.High, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing4LowLED"), dtcregdump.Ring4.CharacterError.Low, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing4HighLED"), dtcregdump.Ring4.CharacterError.High, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing5LowLED"), dtcregdump.Ring5.CharacterError.Low, "RO");
    setPixel(document.getElementById("SERDESRXCNITRing5HighLED"), dtcregdump.Ring5.CharacterError.High, "RO");
    setPixel(document.getElementById("SERDESUnlockRing0LED"), dtcregdump.Ring0.UnlockError, "RO");
    setPixel(document.getElementById("SERDESUnlockRing1LED"), dtcregdump.Ring1.UnlockError, "RO");
    setPixel(document.getElementById("SERDESUnlockRing2LED"), dtcregdump.Ring2.UnlockError, "RO");
    setPixel(document.getElementById("SERDESUnlockRing3LED"), dtcregdump.Ring3.UnlockError, "RO");
    setPixel(document.getElementById("SERDESUnlockRing4LED"), dtcregdump.Ring4.UnlockError, "RO");
    setPixel(document.getElementById("SERDESUnlockRing5LED"), dtcregdump.Ring5.UnlockError, "RO");
    setPixel(document.getElementById("SERDESLockedRing0LED"), dtcregdump.Ring0.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing1LED"), dtcregdump.Ring1.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing2LED"), dtcregdump.Ring2.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing3LED"), dtcregdump.Ring3.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing4LED"), dtcregdump.Ring4.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESLockedRing5LED"), dtcregdump.Ring5.PLLLocked, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing0LED"), dtcregdump.Ring0.OverflowOrUnderflow, "RO");
    setPixel(document.getElementById("SERDESTXFIFOHFRing0LED"), dtcregdump.Ring0.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing1LED"), dtcregdump.Ring1.OverflowOrUnderflow, "RO");
    setPixel(document.getElementById("SERDESTXFIFOHFRing1LED"), dtcregdump.Ring1.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing2LED"), dtcregdump.Ring2.OverflowOrUnderflow, "RO");
    setPixel(document.getElementById("SERDESTXFIFOHFRing2LED"), dtcregdump.Ring2.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing3LED"), dtcregdump.Ring3.OverflowOrUnderflow, "RO");
    setPixel(document.getElementById("SERDESTXFIFOHFRing3LED"), dtcregdump.Ring3.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing4LED"), dtcregdump.Ring4.OverflowOrUnderflow, "RO");
    setPixel(document.getElementById("SERDESTXFIFOHFRing4LED"), dtcregdump.Ring4.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESTXUOFlowRing5LED"), dtcregdump.Ring5.OverflowOrUnderflow, "RO");
    setPixel(document.getElementById("SERDESTXFIFOHFRing5LED"), dtcregdump.Ring5.FIFOHalfFull, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing0LED"), dtcregdump.Ring0.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing1LED"), dtcregdump.Ring1.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing2LED"), dtcregdump.Ring2.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing3LED"), dtcregdump.Ring3.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing4LED"), dtcregdump.Ring4.ResetDone, "RO");
    setPixel(document.getElementById("SERDESResetDoneRing5LED"), dtcregdump.Ring5.ResetDone, "RO");
    $("#timestamp").val(dtcregdump.Timestamp.toString(16));
    setPixel(document.getElementById("fpgapromfifofullLED"), dtcregdump.PROMFIFOFull, "RO");
    setPixel(document.getElementById("fpgapromreadyLED"), dtcregdump.PROMReady, "RO");
    
    setPixel(document.getElementById("SERDESRXBufferNominalRing0LED"), dtcregdump.Ring0.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing0LED"), dtcregdump.Ring0.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing0LED"), dtcregdump.Ring0.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing0LED"), dtcregdump.Ring0.RXBufferStatus.Underflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferOFRing0LED"), dtcregdump.Ring0.RXBufferStatus.Overflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferNominalRing1LED"), dtcregdump.Ring1.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing1LED"), dtcregdump.Ring1.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing1LED"), dtcregdump.Ring1.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing1LED"), dtcregdump.Ring1.RXBufferStatus.Underflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferOFRing1LED"), dtcregdump.Ring1.RXBufferStatus.Overflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferNominalRing2LED"), dtcregdump.Ring2.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing2LED"), dtcregdump.Ring2.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing2LED"), dtcregdump.Ring2.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing2LED"), dtcregdump.Ring2.RXBufferStatus.Underflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferOFRing2LED"), dtcregdump.Ring2.RXBufferStatus.Overflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferNominalRing3LED"), dtcregdump.Ring3.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing3LED"), dtcregdump.Ring3.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing3LED"), dtcregdump.Ring3.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing3LED"), dtcregdump.Ring3.RXBufferStatus.Underflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferOFRing3LED"), dtcregdump.Ring3.RXBufferStatus.Overflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferNominalRing4LED"), dtcregdump.Ring4.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing4LED"), dtcregdump.Ring4.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing4LED"), dtcregdump.Ring4.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing4LED"), dtcregdump.Ring4.RXBufferStatus.Underflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferOFRing4LED"), dtcregdump.Ring4.RXBufferStatus.Overflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferNominalRing5LED"), dtcregdump.Ring5.RXBufferStatus.Nominal , "RO");
    setPixel(document.getElementById("SERDESRXBufferEmptyRing5LED"), dtcregdump.Ring5.RXBufferStatus.Empty, "RO");
    setPixel(document.getElementById("SERDESRXBufferFullRing5LED"), dtcregdump.Ring5.RXBufferStatus.Full, "RO");
    setPixel(document.getElementById("SERDESRXBufferUFRing5LED"), dtcregdump.Ring5.RXBufferStatus.Underflow, "RO");
    setPixel(document.getElementById("SERDESRXBufferOFRing5LED"), dtcregdump.Ring5.RXBufferStatus.Overflow, "RO");
  
}

function AjaxPost(urlString, ringNum) {
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
        data: { ring: ringNum },
        dataType: "json",
        // Our success handler.
        success: function (objData) {
            // At this point, we have data coming back
            // from the server.
            PopulateLEDS(objData);
        },
        // An error handler for the request.
        error: function () {
            alert("An error occurred");
        },
        // I get called no matter what.
        complete: function () {
            // Remove completed request object.
            AjaxPost.Xhr = null;
        }
    });
}

function SetTimestamp() {
    var value = $("#timestamp").val();
    AjaxPost('./DTC/setTimestampPreset', value);
}

// When the DOM is ready to be interacted with, init.
$(function () {
    AjaxPost("./dtc_reg_dump", null);
    $("#post").click(function () {
        UpdateFormFields();
        AjaxPost("./dtc_reg_dump", null);
    });
    $('input[name=base]').change(function () {
        var jVal = $("#value");
        var jBase = $('input[name=base]:checked').val();
        var oldval = parseInt(jVal.val(), oldbase);
        jVal.val(oldval.toString(jBase));
        oldbase = jBase;
    });
});
