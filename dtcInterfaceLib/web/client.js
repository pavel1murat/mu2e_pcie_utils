var socket = io.connect('http://localhost');
socket.on('txt_change', function (data) {
    console.log(data);
    $("#txt").val(data.txt);
});
$(document).ready(function () {
    $("#txt").keyup(function () {
        socket.emit('txt_change', { "txt" : $(this).val() });
    });
});

function setPixel(led, bit) {
    ctx = led.getContext("2d");

    ctx.lineWidth = 2;
    ctx.strokeStyle = "darkgreen";
    if (bit) {
        ctx.fillStyle = "lightgreen";
    } else {
        ctx.fillStyle = 'black';
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
function PopulateValueBox( value, base ){
  $( "#value" ).val( parseInt(value,16).toString(base) );
}

// I take the given option selection and return the
// associated data using a remote method call.
function GetAJAXValues( strOption, address, value, fnCallback ){
  // Check to see if there is currently an AJAX
  // request on this method.
  if (GetAJAXValues.Xhr){
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
    success: function( objData ){
      // At this point, we have data coming back
      // from the server.
      fnCallback({
        Value1: objData
      });
    },
    // An error handler for the request.
    error: function(){
      alert( "An error occurred" );
    },
    // I get called no matter what.
    complete: function(){
      // Remove completed request object.
      GetAJAXValues.Xhr = null;
    }
  });
}

// I handle the updating of the form fields based on the
// selected option of the combo box.
function UpdateFormFields(){
var jSelect = $( "#mode" );
var jAddr = $( "#address" );
var jVal = $( "#value" );
var jBase = $( 'input[name=base]:checked' );
var jAJAX = $( "#ajax" );
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
  function( objRemoteData ){
   PopulateValueBox(
    objRemoteData.Value1,
    jBase.val()
   );
  }
 );
}

function PopulateLEDS(dtcregdump) {
    $( "#dtcVersion" ).val(dtcregdump.Version.toString(16));
    setPixel(document.getElementById("dtcResetLED"), dtcregdump.ResetDTC);
    setPixel(document.getElementById("clearErrorsLED"), dtcregdump.ClearLatchedErrors);
    setPixel(document.getElementById("ROCEmulatorEnabledLED"), dtcregdump.ROCEmulator);
    setPixel(document.getElementById("Ring0EnabledLED"), dtcregdump.Ring0.Enabled);
    setPixel(document.getElementById("Ring1EnabledLED"), dtcregdump.Ring1.Enabled);
    setPixel(document.getElementById("Ring2EnabledLED"), dtcregdump.Ring2.Enabled);
    setPixel(document.getElementById("Ring3EnabledLED"), dtcregdump.Ring3.Enabled);
    setPixel(document.getElementById("Ring4EnabledLED"), dtcregdump.Ring4.Enabled);
    setPixel(document.getElementById("Ring5EnabledLED"), dtcregdump.Ring5.Enabled);
    setPixel(document.getElementById("SERDESLoopbackRing0LED"), dtcregdump.Ring0.SERDESLoopback);
    setPixel(document.getElementById("SERDESLoopbackRing1LED"), dtcregdump.Ring1.SERDESLoopback);
    setPixel(document.getElementById("SERDESLoopbackRing2LED"), dtcregdump.Ring2.SERDESLoopback);
    setPixel(document.getElementById("SERDESLoopbackRing3LED"), dtcregdump.Ring3.SERDESLoopback);
    setPixel(document.getElementById("SERDESLoopbackRing4LED"), dtcregdump.Ring4.SERDESLoopback);
    setPixel(document.getElementById("SERDESLoopbackRing5LED"), dtcregdump.Ring5.SERDESLoopback);
    setPixel(document.getElementById("SERDESResetRing0LED"), dtcregdump.Ring0.ResetSERDES);
    setPixel(document.getElementById("SERDESResetRing1LED"), dtcregdump.Ring1.ResetSERDES);
    setPixel(document.getElementById("SERDESResetRing2LED"), dtcregdump.Ring2.ResetSERDES);
    setPixel(document.getElementById("SERDESResetRing3LED"), dtcregdump.Ring3.ResetSERDES);
    setPixel(document.getElementById("SERDESResetRing4LED"), dtcregdump.Ring4.ResetSERDES);
    setPixel(document.getElementById("SERDESResetRing5LED"), dtcregdump.Ring5.ResetSERDES);
    switch(dtcregdump.Ring0.SERDESRXDisparity) {
    case 0:
        setPixel(document.getElementById("SERDESRXDisparityRing0LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing0HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXDisparityRing0LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing0HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXDisparityRing0LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing0HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXDisparityRing0LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing0HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring1.SERDESRXDisparity) {
    case 0:
        setPixel(document.getElementById("SERDESRXDisparityRing1LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing1HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXDisparityRing1LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing1HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXDisparityRing1LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing1HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXDisparityRing1LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing1HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring2.SERDESRXDisparity) {
    case 0:
        setPixel(document.getElementById("SERDESRXDisparityRing2LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing2HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXDisparityRing2LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing2HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXDisparityRing2LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing2HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXDisparityRing2LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing2HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring3.SERDESRXDisparity) {
    case 0:
        setPixel(document.getElementById("SERDESRXDisparityRing3LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing3HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXDisparityRing3LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing3HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXDisparityRing3LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing3HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXDisparityRing3LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing3HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring4.SERDESRXDisparity) {
    case 0:
        setPixel(document.getElementById("SERDESRXDisparityRing4LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing4HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXDisparityRing4LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing4HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXDisparityRing4LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing4HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXDisparityRing4LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing4HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring5.SERDESRXDisparity) {
    case 0:
        setPixel(document.getElementById("SERDESRXDisparityRing5LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing5HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXDisparityRing5LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing5HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXDisparityRing5LowLED"),0);
		 setPixel(document.getElementById("SERDESRXDisparityRing5HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXDisparityRing5LowLED"),1);
		 setPixel(document.getElementById("SERDESRXDisparityRing5HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring0.CharacterError) {
    case 0:
        setPixel(document.getElementById("SERDESRXCNITRing0LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing0HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXCNITRing0LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing0HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXCNITRing0LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing0HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXCNITRing0LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing0HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring1.CharacterError) {
    case 0:
        setPixel(document.getElementById("SERDESRXCNITRing1LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing1HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXCNITRing1LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing1HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXCNITRing1LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing1HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXCNITRing1LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing1HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring2.CharacterError) {
    case 0:
        setPixel(document.getElementById("SERDESRXCNITRing2LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing2HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXCNITRing2LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing2HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXCNITRing2LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing2HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXCNITRing2LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing2HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring3.CharacterError) {
    case 0:
        setPixel(document.getElementById("SERDESRXCNITRing3LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing3HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXCNITRing3LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing3HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXCNITRing3LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing3HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXCNITRing3LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing3HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring4.CharacterError) {
    case 0:
        setPixel(document.getElementById("SERDESRXCNITRing4LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing4HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXCNITRing4LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing4HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXCNITRing4LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing4HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXCNITRing4LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing4HighLED"),1);
	break;
    }
    switch(dtcregdump.Ring5.CharacterError) {
    case 0:
        setPixel(document.getElementById("SERDESRXCNITRing5LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing5HighLED"),0);
	break;
    case 1:
		 setPixel(document.getElementById("SERDESRXCNITRing5LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing5HighLED"),0);
	break;
    case 2:
		 setPixel(document.getElementById("SERDESRXCNITRing5LowLED"),0);
		 setPixel(document.getElementById("SERDESRXCNITRing5HighLED"),1);
	break;
    case 3:
		 setPixel(document.getElementById("SERDESRXCNITRing5LowLED"),1);
		 setPixel(document.getElementById("SERDESRXCNITRing5HighLED"),1);
	break;
    }
    setPixel(document.getElementById("SERDESUnlockRing0LED"), dtcregdump.Ring0.UnlockError);
    setPixel(document.getElementById("SERDESUnlockRing1LED"), dtcregdump.Ring1.UnlockError);
    setPixel(document.getElementById("SERDESUnlockRing2LED"), dtcregdump.Ring2.UnlockError);
    setPixel(document.getElementById("SERDESUnlockRing3LED"), dtcregdump.Ring3.UnlockError);
    setPixel(document.getElementById("SERDESUnlockRing4LED"), dtcregdump.Ring4.UnlockError);
    setPixel(document.getElementById("SERDESUnlockRing5LED"), dtcregdump.Ring5.UnlockError);
    setPixel(document.getElementById("SERDESLockedRing0LED"), dtcregdump.Ring0.PLLLocked);
    setPixel(document.getElementById("SERDESLockedRing1LED"), dtcregdump.Ring1.PLLLocked);
    setPixel(document.getElementById("SERDESLockedRing2LED"), dtcregdump.Ring2.PLLLocked);
    setPixel(document.getElementById("SERDESLockedRing3LED"), dtcregdump.Ring3.PLLLocked);
    setPixel(document.getElementById("SERDESLockedRing4LED"), dtcregdump.Ring4.PLLLocked);
    setPixel(document.getElementById("SERDESLockedRing5LED"), dtcregdump.Ring5.PLLLocked);
    setPixel(document.getElementById("SERDESTXUOFlowRing0LED"), dtcregdump.Ring0.OverflowOrUnderflow);
    setPixel(document.getElementById("SERDESTXFIFOHFRing0LED"), dtcregdump.Ring0.FIFOHalfFull);
    setPixel(document.getElementById("SERDESTXUOFlowRing1LED"), dtcregdump.Ring1.OverflowOrUnderflow);
    setPixel(document.getElementById("SERDESTXFIFOHFRing1LED"), dtcregdump.Ring1.FIFOHalfFull);
    setPixel(document.getElementById("SERDESTXUOFlowRing2LED"), dtcregdump.Ring2.OverflowOrUnderflow);
    setPixel(document.getElementById("SERDESTXFIFOHFRing2LED"), dtcregdump.Ring2.FIFOHalfFull);
    setPixel(document.getElementById("SERDESTXUOFlowRing3LED"), dtcregdump.Ring3.OverflowOrUnderflow);
    setPixel(document.getElementById("SERDESTXFIFOHFRing3LED"), dtcregdump.Ring3.FIFOHalfFull);
    setPixel(document.getElementById("SERDESTXUOFlowRing4LED"), dtcregdump.Ring4.OverflowOrUnderflow);
    setPixel(document.getElementById("SERDESTXFIFOHFRing4LED"), dtcregdump.Ring4.FIFOHalfFull);
    setPixel(document.getElementById("SERDESTXUOFlowRing5LED"), dtcregdump.Ring5.OverflowOrUnderflow);
    setPixel(document.getElementById("SERDESTXFIFOHFRing5LED"), dtcregdump.Ring5.FIFOHalfFull);
    setPixel(document.getElementById("SERDESResetDoneRing0LED"), dtcregdump.Ring0.ResetDone);
    setPixel(document.getElementById("SERDESResetDoneRing1LED"), dtcregdump.Ring1.ResetDone);
    setPixel(document.getElementById("SERDESResetDoneRing2LED"), dtcregdump.Ring2.ResetDone);
    setPixel(document.getElementById("SERDESResetDoneRing3LED"), dtcregdump.Ring3.ResetDone);
    setPixel(document.getElementById("SERDESResetDoneRing4LED"), dtcregdump.Ring4.ResetDone);
    setPixel(document.getElementById("SERDESResetDoneRing5LED"), dtcregdump.Ring5.ResetDone);
    $( "#timestamp" ).val(dtcregdump.Timestamp.toString(16));
    setPixel(document.getElementById("fpgapromfifofullLED"), dtcregdump.PROMFIFOFull);
    setPixel(document.getElementById("fpgapromreadyLED"), dtcregdump.PROMReady);
    switch(dtcregdump.Ring0.RXBufferStatus) {
    case 0:
	setPixel(document.getElementById("SERDESRXBufferNominalRing0LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing0LED"), 0);
        break;
    case 1:
	setPixel(document.getElementById("SERDESRXBufferNominalRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing0LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferFullRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing0LED"), 0);
        break;
    case 2:
	setPixel(document.getElementById("SERDESRXBufferNominalRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing0LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferUFRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing0LED"), 0);
        break;
    case 5:
	setPixel(document.getElementById("SERDESRXBufferNominalRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing0LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferOFRing0LED"), 0);
        break;
    case 6:
	setPixel(document.getElementById("SERDESRXBufferNominalRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing0LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing0LED"), 1);
        break;
    }
    switch(dtcregdump.Ring1.RXBufferStatus) {
    case 0:
	setPixel(document.getElementById("SERDESRXBufferNominalRing1LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing1LED"), 0);
        break;
    case 1:
	setPixel(document.getElementById("SERDESRXBufferNominalRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing1LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferFullRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing1LED"), 0);
        break;
    case 2:
	setPixel(document.getElementById("SERDESRXBufferNominalRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing1LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferUFRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing1LED"), 0);
        break;
    case 5:
	setPixel(document.getElementById("SERDESRXBufferNominalRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing1LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferOFRing1LED"), 0);
        break;
    case 6:
	setPixel(document.getElementById("SERDESRXBufferNominalRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing1LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing1LED"), 1);
        break;
    }
    switch(dtcregdump.Ring2.RXBufferStatus) {
    case 0:
	setPixel(document.getElementById("SERDESRXBufferNominalRing2LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing2LED"), 0);
        break;
    case 1:
	setPixel(document.getElementById("SERDESRXBufferNominalRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing2LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferFullRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing2LED"), 0);
        break;
    case 2:
	setPixel(document.getElementById("SERDESRXBufferNominalRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing2LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferUFRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing2LED"), 0);
        break;
    case 5:
	setPixel(document.getElementById("SERDESRXBufferNominalRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing2LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferOFRing2LED"), 0);
        break;
    case 6:
	setPixel(document.getElementById("SERDESRXBufferNominalRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing2LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing2LED"), 1);
        break;
    }
    switch(dtcregdump.Ring3.RXBufferStatus) {
    case 0:
	setPixel(document.getElementById("SERDESRXBufferNominalRing3LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing3LED"), 0);
        break;
    case 1:
	setPixel(document.getElementById("SERDESRXBufferNominalRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing3LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferFullRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing3LED"), 0);
        break;
    case 2:
	setPixel(document.getElementById("SERDESRXBufferNominalRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing3LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferUFRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing3LED"), 0);
        break;
    case 5:
	setPixel(document.getElementById("SERDESRXBufferNominalRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing3LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferOFRing3LED"), 0);
        break;
    case 6:
	setPixel(document.getElementById("SERDESRXBufferNominalRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing3LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing3LED"), 1);
        break;
    }
    switch(dtcregdump.Ring4.RXBufferStatus) {
    case 0:
	setPixel(document.getElementById("SERDESRXBufferNominalRing4LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing4LED"), 0);
        break;
    case 1:
	setPixel(document.getElementById("SERDESRXBufferNominalRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing4LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferFullRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing4LED"), 0);
        break;
    case 2:
	setPixel(document.getElementById("SERDESRXBufferNominalRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing4LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferUFRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing4LED"), 0);
        break;
    case 5:
	setPixel(document.getElementById("SERDESRXBufferNominalRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing4LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferOFRing4LED"), 0);
        break;
    case 6:
	setPixel(document.getElementById("SERDESRXBufferNominalRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing4LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing4LED"), 1);
        break;
    }
    switch(dtcregdump.Ring5.RXBufferStatus) {
    case 0:
	setPixel(document.getElementById("SERDESRXBufferNominalRing5LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing5LED"), 0);
        break;
    case 1:
	setPixel(document.getElementById("SERDESRXBufferNominalRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing5LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferFullRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing5LED"), 0);
        break;
    case 2:
	setPixel(document.getElementById("SERDESRXBufferNominalRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing5LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferUFRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing5LED"), 0);
        break;
    case 5:
	setPixel(document.getElementById("SERDESRXBufferNominalRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing5LED"), 1);
        setPixel(document.getElementById("SERDESRXBufferOFRing5LED"), 0);
        break;
    case 6:
	setPixel(document.getElementById("SERDESRXBufferNominalRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferEmptyRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferFullRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferUFRing5LED"), 0);
        setPixel(document.getElementById("SERDESRXBufferOFRing5LED"), 1);
        break;
    }
}

function GetRegDump(fnCallback) {
  // Check to see if there is currently an AJAX
  // request on this method.
  if (GetRegDump.Xhr){
    // Abort the current request.
    GetRegDump.Xhr.abort();
  }
  // Get data via AJAX. Store the XHR (AJAX request
  // object in the method in case we need to abort
  // it on subsequent requests.
  GetRegDump.Xhr = $.ajax({
    type: "post",
    url: "./dtc_reg_dump",
    data: {option: "read"},
    dataType: "json",
    // Our success handler.
    success: function( objData ){
      // At this point, we have data coming back
      // from the server.
      fnCallback({
        Value1: objData
      });
    },
    // An error handler for the request.
    error: function(){
      alert( "An error occurred" );
    },
    // I get called no matter what.
    complete: function(){
      // Remove completed request object.
      GetRegDump.Xhr = null;
    }
  });
}

function ReadRegisters(){
    var objData = null;
    GetRegDump(function(regdump) {
	    PopulateLEDS(regdump.Value1);
	});
}

// When the DOM is ready to be interacted with, init.
$(function(){
  ReadRegisters();
  $( "#post").click(function(){
    UpdateFormFields();
  });
  $( 'input[name=base]' ).change(function(){
    var jVal = $( "#value" );
    var jBase = $( 'input[name=base]:checked' ).val();
    var oldval = parseInt(jVal.val(),oldbase);
    jVal.val(oldval.toString(jBase));
    oldbase = jBase;
  });
});
