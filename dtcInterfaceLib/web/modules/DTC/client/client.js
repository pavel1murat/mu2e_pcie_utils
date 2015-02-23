var logIntervalHandle;
var regIntervalHandle;

function setPixel( led,bit,modestring ) {
    var ctx = led.getContext( "2d" );
    
    ctx.lineWidth = 2;
    
    if ( modestring === "RO" ) {
        ctx.strokeStyle = "darkgreen";
        if ( bit ) {
            ctx.fillStyle = "lightgreen";
        } else {
            ctx.fillStyle = 'black';
        }
    }
    else if ( modestring === "RW" ) {
        ctx.strokeStyle = "darkblue";
        if ( bit ) {
            ctx.fillStyle = "lightblue";
        } else {
            ctx.fillStyle = 'black';
        }
    } else {
        ctx.strokeStype = "darkred";
        ctx.fillStyle = "red";
    }
    ctx.beginPath( );
    ctx.moveTo( led.width / 4,ctx.lineWidth ); //Create a starting point
    ctx.lineTo( led.width * 3 / 4,ctx.lineWidth ); //Create a horizontal line
    ctx.arcTo( led.width - ctx.lineWidth,ctx.lineWidth,led.width - ctx.lineWidth,led.height / 4,led.width / 8 ); // Create an arc
    ctx.lineTo( led.width - ctx.lineWidth,led.height * 3 / 4 ); // Continue with vertical line
    ctx.arcTo( led.width - ctx.lineWidth,led.height - ctx.lineWidth,led.width * 3 / 4,led.height - ctx.lineWidth,led.width / 8 );
    ctx.lineTo( led.width / 4,led.height - ctx.lineWidth );
    ctx.arcTo( ctx.lineWidth,led.height - ctx.lineWidth,ctx.lineWidth,led.height * 3 / 4,led.width / 8 );
    ctx.lineTo( ctx.lineWidth,led.height / 4 );
    ctx.arcTo( ctx.lineWidth,ctx.lineWidth,led.width / 4,ctx.lineWidth,led.width / 8 );
    ctx.closePath( );
    ctx.stroke( );
    ctx.fill( );
}

var oldbase = 16;
// Define a method that handles nothing but the actual
// form population. This helps us decouple the data
// insertion from the data retrieval.
function PopulateValueBox( value,base ) {
    $( "#value" ).val( parseInt( value,16 ).toString( base ) );
}

// I take the given option selection and return the
// associated data using a remote method call.
function GetAJAXValues( strOption,address,value,fnCallback ) {
    // Check to see if there is currently an AJAX
    // request on this method.
    if ( GetAJAXValues.Xhr ) {
        // Abort the current request.
        GetAJAXValues.Xhr.abort( );
    }
    setPixel( document.getElementById( "FormAjaxLED" ),1,"RO" );
    // Get data via AJAX. Store the XHR (AJAX request
    // object in the method in case we need to abort
    // it on subsequent requests.
    GetAJAXValues.Xhr = $.ajax( {
        type: "post",
        url: "/DTC/RegIO",
        data: {
            option: strOption,
            address: address,
            value: value
        },
        dataType: "json",
        // Our success handler.
        success: function ( objData ) {
            // At this point, we have data coming back
            // from the server.
            setPixel( document.getElementById( "FormAjaxLED" ),0,"RO" );
            fnCallback( {
                Value1: objData
            } );
        },
        // An error handler for the request.
        error: function ( xhr,textStatus,errorCode ) {
            //alert("An error occurred:\n" + textStatus + "\n" + errorCode);
            setPixel( document.getElementById( "FormAjaxLED" ),1,"ERR" );
        },
        // I get called no matter what.
        complete: function () {
            // Remove completed request object.
            GetAJAXValues.Xhr = null;
        }
    } );
}

// I handle the updating of the form fields based on the
// selected option of the combo box.
function UpdateFormFields() {
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
        jSelect.val( ),
  jAddr.val( ),
  jVal.val( ),
  // Callback method for results.
  function ( objRemoteData ) {
            PopulateValueBox(
                objRemoteData.Value1,
    jBase.val( )
            );
        }
    );
}

function PopulateLEDS( dtcregdump ) {
    $( "#dtcVersion" ).val( dtcregdump.Version.toString( 16 ) );
    setPixel( document.getElementById( "dtcResetLED" ),dtcregdump.ResetDTC,"RW" );
    setPixel( document.getElementById( "ROCEmulatorEnabledLED" ),dtcregdump.ROCEmulator,"RW" );
    setPixel( document.getElementById( "CFOEmulatorEnabledLED" ),dtcregdump.CFOEmulator,"RW" );
    $( "#triggerDMALength" ).val( dtcregdump.TriggerDMALength );
    $( "#minDMALength" ).val( dtcregdump.MinDMALength );
    $( "#dmaTimeoutPreset" ).val( dtcregdump.DMATimeout );
    $( "#dataPendingTimer" ).val( dtcregdump.DataPendingTimer );
    $( "#packetSize" ).val( dtcregdump.PacketSize );
    setPixel( document.getElementById( "Ring0EnabledLED" ),dtcregdump.Ring0.Enabled,"RW" );
    setPixel( document.getElementById( "Ring1EnabledLED" ),dtcregdump.Ring1.Enabled,"RW" );
    setPixel( document.getElementById( "Ring2EnabledLED" ),dtcregdump.Ring2.Enabled,"RW" );
    setPixel( document.getElementById( "Ring3EnabledLED" ),dtcregdump.Ring3.Enabled,"RW" );
    setPixel( document.getElementById( "Ring4EnabledLED" ),dtcregdump.Ring4.Enabled,"RW" );
    setPixel( document.getElementById( "Ring5EnabledLED" ),dtcregdump.Ring5.Enabled,"RW" );
    setPixel( document.getElementById( "CFOEnabledLED" ),dtcregdump.CFO.Enabled,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPCSRing0LED" ),dtcregdump.Ring0.SERDESLoopback.NEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPMARing0LED" ),dtcregdump.Ring0.SERDESLoopback.NEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPMARing0LED" ),dtcregdump.Ring0.SERDESLoopback.FEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPCSRing0LED" ),dtcregdump.Ring0.SERDESLoopback.FEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPCSRing1LED" ),dtcregdump.Ring1.SERDESLoopback.NEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPMARing1LED" ),dtcregdump.Ring1.SERDESLoopback.NEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPMARing1LED" ),dtcregdump.Ring1.SERDESLoopback.FEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPCSRing1LED" ),dtcregdump.Ring1.SERDESLoopback.FEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPCSRing2LED" ),dtcregdump.Ring2.SERDESLoopback.NEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPMARing2LED" ),dtcregdump.Ring2.SERDESLoopback.NEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPMARing2LED" ),dtcregdump.Ring2.SERDESLoopback.FEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPCSRing2LED" ),dtcregdump.Ring2.SERDESLoopback.FEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPCSRing3LED" ),dtcregdump.Ring3.SERDESLoopback.NEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPMARing3LED" ),dtcregdump.Ring3.SERDESLoopback.NEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPMARing3LED" ),dtcregdump.Ring3.SERDESLoopback.FEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPCSRing3LED" ),dtcregdump.Ring3.SERDESLoopback.FEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPCSRing4LED" ),dtcregdump.Ring4.SERDESLoopback.NEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPMARing4LED" ),dtcregdump.Ring4.SERDESLoopback.NEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPMARing4LED" ),dtcregdump.Ring4.SERDESLoopback.FEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPCSRing4LED" ),dtcregdump.Ring4.SERDESLoopback.FEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPCSRing5LED" ),dtcregdump.Ring5.SERDESLoopback.NEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPMARing5LED" ),dtcregdump.Ring5.SERDESLoopback.NEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPMARing5LED" ),dtcregdump.Ring5.SERDESLoopback.FEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPCSRing5LED" ),dtcregdump.Ring5.SERDESLoopback.FEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPCSCFOLED" ),dtcregdump.CFO.SERDESLoopback.NEPCS,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackNEPMACFOLED" ),dtcregdump.CFO.SERDESLoopback.NEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPMACFOLED" ),dtcregdump.CFO.SERDESLoopback.FEPMA,"RW" );
    setPixel( document.getElementById( "SERDESLoopbackFEPCSCFOLED" ),dtcregdump.CFO.SERDESLoopback.FEPCS,"RW" );
    setPixel( document.getElementById( "SERDESResetRing0LED" ),dtcregdump.Ring0.ResetSERDES,"RW" );
    setPixel( document.getElementById( "SERDESResetRing1LED" ),dtcregdump.Ring1.ResetSERDES,"RW" );
    setPixel( document.getElementById( "SERDESResetRing2LED" ),dtcregdump.Ring2.ResetSERDES,"RW" );
    setPixel( document.getElementById( "SERDESResetRing3LED" ),dtcregdump.Ring3.ResetSERDES,"RW" );
    setPixel( document.getElementById( "SERDESResetRing4LED" ),dtcregdump.Ring4.ResetSERDES,"RW" );
    setPixel( document.getElementById( "SERDESResetRing5LED" ),dtcregdump.Ring5.ResetSERDES,"RW" );
    setPixel( document.getElementById( "SERDESResetCFOLED" ),dtcregdump.CFO.ResetSERDES,"RW" );
    setPixel( document.getElementById( "SERDESResetDoneRing0LED" ),dtcregdump.Ring0.ResetDone,"RO" );
    setPixel( document.getElementById( "SERDESResetDoneRing1LED" ),dtcregdump.Ring1.ResetDone,"RO" );
    setPixel( document.getElementById( "SERDESResetDoneRing2LED" ),dtcregdump.Ring2.ResetDone,"RO" );
    setPixel( document.getElementById( "SERDESResetDoneRing3LED" ),dtcregdump.Ring3.ResetDone,"RO" );
    setPixel( document.getElementById( "SERDESResetDoneRing4LED" ),dtcregdump.Ring4.ResetDone,"RO" );
    setPixel( document.getElementById( "SERDESResetDoneRing5LED" ),dtcregdump.Ring5.ResetDone,"RO" );
    setPixel( document.getElementById( "SERDESResetDoneCFOLED" ),dtcregdump.CFO.ResetDone,"RO" );
    setPixel( document.getElementById( "SERDESRXDisparityRing0LowLED" ),dtcregdump.Ring0.SERDESRXDisparity.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing0HighLED" ),dtcregdump.Ring0.SERDESRXDisparity.High,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing1LowLED" ),dtcregdump.Ring1.SERDESRXDisparity.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing1HighLED" ),dtcregdump.Ring1.SERDESRXDisparity.High,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing2LowLED" ),dtcregdump.Ring2.SERDESRXDisparity.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing2HighLED" ),dtcregdump.Ring2.SERDESRXDisparity.High,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing3LowLED" ),dtcregdump.Ring3.SERDESRXDisparity.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing3HighLED" ),dtcregdump.Ring3.SERDESRXDisparity.High,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing4LowLED" ),dtcregdump.Ring4.SERDESRXDisparity.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing4HighLED" ),dtcregdump.Ring4.SERDESRXDisparity.High,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing5LowLED" ),dtcregdump.Ring5.SERDESRXDisparity.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityRing5HighLED" ),dtcregdump.Ring5.SERDESRXDisparity.High,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityCFOLowLED" ),dtcregdump.CFO.SERDESRXDisparity.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXDisparityCFOHighLED" ),dtcregdump.CFO.SERDESRXDisparity.High,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing0LowLED" ),dtcregdump.Ring0.CharacterError.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing0HighLED" ),dtcregdump.Ring0.CharacterError.High,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing1LowLED" ),dtcregdump.Ring1.CharacterError.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing1HighLED" ),dtcregdump.Ring1.CharacterError.High,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing2LowLED" ),dtcregdump.Ring2.CharacterError.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing2HighLED" ),dtcregdump.Ring2.CharacterError.High,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing3LowLED" ),dtcregdump.Ring3.CharacterError.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing3HighLED" ),dtcregdump.Ring3.CharacterError.High,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing4LowLED" ),dtcregdump.Ring4.CharacterError.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing4HighLED" ),dtcregdump.Ring4.CharacterError.High,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing5LowLED" ),dtcregdump.Ring5.CharacterError.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITRing5HighLED" ),dtcregdump.Ring5.CharacterError.High,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITCFOLowLED" ),dtcregdump.CFO.CharacterError.Low,"RW" );
    setPixel( document.getElementById( "SERDESRXCNITCFOHighLED" ),dtcregdump.CFO.CharacterError.High,"RW" );
    setPixel( document.getElementById( "SERDESUnlockRing0LED" ),dtcregdump.Ring0.UnlockError,"RW" );
    setPixel( document.getElementById( "SERDESUnlockRing1LED" ),dtcregdump.Ring1.UnlockError,"RW" );
    setPixel( document.getElementById( "SERDESUnlockRing2LED" ),dtcregdump.Ring2.UnlockError,"RW" );
    setPixel( document.getElementById( "SERDESUnlockRing3LED" ),dtcregdump.Ring3.UnlockError,"RW" );
    setPixel( document.getElementById( "SERDESUnlockRing4LED" ),dtcregdump.Ring4.UnlockError,"RW" );
    setPixel( document.getElementById( "SERDESUnlockRing5LED" ),dtcregdump.Ring5.UnlockError,"RW" );
    setPixel( document.getElementById( "SERDESUnlockCFOLED" ),dtcregdump.CFO.UnlockError,"RW" );
    setPixel( document.getElementById( "SERDESLockedRing0LED" ),dtcregdump.Ring0.PLLLocked,"RO" );
    setPixel( document.getElementById( "SERDESLockedRing1LED" ),dtcregdump.Ring1.PLLLocked,"RO" );
    setPixel( document.getElementById( "SERDESLockedRing2LED" ),dtcregdump.Ring2.PLLLocked,"RO" );
    setPixel( document.getElementById( "SERDESLockedRing3LED" ),dtcregdump.Ring3.PLLLocked,"RO" );
    setPixel( document.getElementById( "SERDESLockedRing4LED" ),dtcregdump.Ring4.PLLLocked,"RO" );
    setPixel( document.getElementById( "SERDESLockedRing5LED" ),dtcregdump.Ring5.PLLLocked,"RO" );
    setPixel( document.getElementById( "SERDESLockedCFOLED" ),dtcregdump.CFO.PLLLocked,"RO" );
    setPixel( document.getElementById( "SERDESTXUOFlowRing0LED" ),dtcregdump.Ring0.OverflowOrUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESTXFIFOHFRing0LED" ),dtcregdump.Ring0.FIFOHalfFull,"RO" );
    setPixel( document.getElementById( "SERDESTXUOFlowRing1LED" ),dtcregdump.Ring1.OverflowOrUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESTXFIFOHFRing1LED" ),dtcregdump.Ring1.FIFOHalfFull,"RO" );
    setPixel( document.getElementById( "SERDESTXUOFlowRing2LED" ),dtcregdump.Ring2.OverflowOrUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESTXFIFOHFRing2LED" ),dtcregdump.Ring2.FIFOHalfFull,"RO" );
    setPixel( document.getElementById( "SERDESTXUOFlowRing3LED" ),dtcregdump.Ring3.OverflowOrUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESTXFIFOHFRing3LED" ),dtcregdump.Ring3.FIFOHalfFull,"RO" );
    setPixel( document.getElementById( "SERDESTXUOFlowRing4LED" ),dtcregdump.Ring4.OverflowOrUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESTXFIFOHFRing4LED" ),dtcregdump.Ring4.FIFOHalfFull,"RO" );
    setPixel( document.getElementById( "SERDESTXUOFlowRing5LED" ),dtcregdump.Ring5.OverflowOrUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESTXFIFOHFRing5LED" ),dtcregdump.Ring5.FIFOHalfFull,"RO" );
    setPixel( document.getElementById( "SERDESTXUOFlowCFOLED" ),dtcregdump.CFO.OverflowOrUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESTXFIFOHFCFOLED" ),dtcregdump.CFO.FIFOHalfFull,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferNominalRing0LED" ),dtcregdump.Ring0.RXBufferStatus.Nominal ,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferEmptyRing0LED" ),dtcregdump.Ring0.RXBufferStatus.Empty,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferFullRing0LED" ),dtcregdump.Ring0.RXBufferStatus.Full,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferUFRing0LED" ),dtcregdump.Ring0.RXBufferStatus.Underflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferOFRing0LED" ),dtcregdump.Ring0.RXBufferStatus.Overflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferNominalRing1LED" ),dtcregdump.Ring1.RXBufferStatus.Nominal ,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferEmptyRing1LED" ),dtcregdump.Ring1.RXBufferStatus.Empty,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferFullRing1LED" ),dtcregdump.Ring1.RXBufferStatus.Full,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferUFRing1LED" ),dtcregdump.Ring1.RXBufferStatus.Underflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferOFRing1LED" ),dtcregdump.Ring1.RXBufferStatus.Overflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferNominalRing2LED" ),dtcregdump.Ring2.RXBufferStatus.Nominal ,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferEmptyRing2LED" ),dtcregdump.Ring2.RXBufferStatus.Empty,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferFullRing2LED" ),dtcregdump.Ring2.RXBufferStatus.Full,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferUFRing2LED" ),dtcregdump.Ring2.RXBufferStatus.Underflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferOFRing2LED" ),dtcregdump.Ring2.RXBufferStatus.Overflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferNominalRing3LED" ),dtcregdump.Ring3.RXBufferStatus.Nominal ,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferEmptyRing3LED" ),dtcregdump.Ring3.RXBufferStatus.Empty,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferFullRing3LED" ),dtcregdump.Ring3.RXBufferStatus.Full,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferUFRing3LED" ),dtcregdump.Ring3.RXBufferStatus.Underflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferOFRing3LED" ),dtcregdump.Ring3.RXBufferStatus.Overflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferNominalRing4LED" ),dtcregdump.Ring4.RXBufferStatus.Nominal ,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferEmptyRing4LED" ),dtcregdump.Ring4.RXBufferStatus.Empty,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferFullRing4LED" ),dtcregdump.Ring4.RXBufferStatus.Full,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferUFRing4LED" ),dtcregdump.Ring4.RXBufferStatus.Underflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferOFRing4LED" ),dtcregdump.Ring4.RXBufferStatus.Overflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferNominalRing5LED" ),dtcregdump.Ring5.RXBufferStatus.Nominal ,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferEmptyRing5LED" ),dtcregdump.Ring5.RXBufferStatus.Empty,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferFullRing5LED" ),dtcregdump.Ring5.RXBufferStatus.Full,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferUFRing5LED" ),dtcregdump.Ring5.RXBufferStatus.Underflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferOFRing5LED" ),dtcregdump.Ring5.RXBufferStatus.Overflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferNominalCFOLED" ),dtcregdump.CFO.RXBufferStatus.Nominal ,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferEmptyCFOLED" ),dtcregdump.CFO.RXBufferStatus.Empty,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferFullCFOLED" ),dtcregdump.CFO.RXBufferStatus.Full,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferUFCFOLED" ),dtcregdump.CFO.RXBufferStatus.Underflow,"RO" );
    setPixel( document.getElementById( "SERDESRXBufferOFCFOLED" ),dtcregdump.CFO.RXBufferStatus.Overflow,"RO" );
    setPixel( document.getElementById( "SERDESRXDataOKRing0LED" ),dtcregdump.Ring0.RXStatus.DataOK,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPAddedRing0LED" ),dtcregdump.Ring0.RXStatus.SKPAdded,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPRemovedRing0LED" ),dtcregdump.Ring0.RXStatus.SKPRemoved,"RO" );
    setPixel( document.getElementById( "SERDESRXReceiverDetectedRing0LED" ),dtcregdump.Ring0.RXStatus.ReceiverDetected,"RO" );
    setPixel( document.getElementById( "SERDESRXDecodeErrorRing0LED" ),dtcregdump.Ring0.RXStatus.DecodeError,"RO" );
    setPixel( document.getElementById( "SERDESRXEOFRing0LED" ),dtcregdump.Ring0.RXStatus.EOverflow,"RO" );
    setPixel( document.getElementById( "SERDESRXEUFRing0LED" ),dtcregdump.Ring0.RXStatus.EUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESRXDisparityErrorRing0LED" ),dtcregdump.Ring0.RXStatus.DisparityError,"RO" );
    setPixel( document.getElementById( "SERDESRXDataOKRing1LED" ),dtcregdump.Ring1.RXStatus.DataOK,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPAddedRing1LED" ),dtcregdump.Ring1.RXStatus.SKPAdded,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPRemovedRing1LED" ),dtcregdump.Ring1.RXStatus.SKPRemoved,"RO" );
    setPixel( document.getElementById( "SERDESRXReceiverDetectedRing1LED" ),dtcregdump.Ring1.RXStatus.ReceiverDetected,"RO" );
    setPixel( document.getElementById( "SERDESRXDecodeErrorRing1LED" ),dtcregdump.Ring1.RXStatus.DecodeError,"RO" );
    setPixel( document.getElementById( "SERDESRXEOFRing1LED" ),dtcregdump.Ring1.RXStatus.EOverflow,"RO" );
    setPixel( document.getElementById( "SERDESRXEUFRing1LED" ),dtcregdump.Ring1.RXStatus.EUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESRXDisparityErrorRing1LED" ),dtcregdump.Ring1.RXStatus.DisparityError,"RO" );
    setPixel( document.getElementById( "SERDESRXDataOKRing2LED" ),dtcregdump.Ring2.RXStatus.DataOK,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPAddedRing2LED" ),dtcregdump.Ring2.RXStatus.SKPAdded,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPRemovedRing2LED" ),dtcregdump.Ring2.RXStatus.SKPRemoved,"RO" );
    setPixel( document.getElementById( "SERDESRXReceiverDetectedRing2LED" ),dtcregdump.Ring2.RXStatus.ReceiverDetected,"RO" );
    setPixel( document.getElementById( "SERDESRXDecodeErrorRing2LED" ),dtcregdump.Ring2.RXStatus.DecodeError,"RO" );
    setPixel( document.getElementById( "SERDESRXEOFRing2LED" ),dtcregdump.Ring2.RXStatus.EOverflow,"RO" );
    setPixel( document.getElementById( "SERDESRXEUFRing2LED" ),dtcregdump.Ring2.RXStatus.EUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESRXDisparityErrorRing2LED" ),dtcregdump.Ring2.RXStatus.DisparityError,"RO" );
    setPixel( document.getElementById( "SERDESRXDataOKRing3LED" ),dtcregdump.Ring3.RXStatus.DataOK,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPAddedRing3LED" ),dtcregdump.Ring3.RXStatus.SKPAdded,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPRemovedRing3LED" ),dtcregdump.Ring3.RXStatus.SKPRemoved,"RO" );
    setPixel( document.getElementById( "SERDESRXReceiverDetectedRing3LED" ),dtcregdump.Ring3.RXStatus.ReceiverDetected,"RO" );
    setPixel( document.getElementById( "SERDESRXDecodeErrorRing3LED" ),dtcregdump.Ring3.RXStatus.DecodeError,"RO" );
    setPixel( document.getElementById( "SERDESRXEOFRing3LED" ),dtcregdump.Ring3.RXStatus.EOverflow,"RO" );
    setPixel( document.getElementById( "SERDESRXEUFRing3LED" ),dtcregdump.Ring3.RXStatus.EUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESRXDisparityErrorRing3LED" ),dtcregdump.Ring3.RXStatus.DisparityError,"RO" );
    setPixel( document.getElementById( "SERDESRXDataOKRing4LED" ),dtcregdump.Ring4.RXStatus.DataOK,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPAddedRing4LED" ),dtcregdump.Ring4.RXStatus.SKPAdded,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPRemovedRing4LED" ),dtcregdump.Ring4.RXStatus.SKPRemoved,"RO" );
    setPixel( document.getElementById( "SERDESRXReceiverDetectedRing4LED" ),dtcregdump.Ring4.RXStatus.ReceiverDetected,"RO" );
    setPixel( document.getElementById( "SERDESRXDecodeErrorRing4LED" ),dtcregdump.Ring4.RXStatus.DecodeError,"RO" );
    setPixel( document.getElementById( "SERDESRXEOFRing4LED" ),dtcregdump.Ring4.RXStatus.EOverflow,"RO" );
    setPixel( document.getElementById( "SERDESRXEUFRing4LED" ),dtcregdump.Ring4.RXStatus.EUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESRXDisparityErrorRing4LED" ),dtcregdump.Ring4.RXStatus.DisparityError,"RO" );
    setPixel( document.getElementById( "SERDESRXDataOKRing5LED" ),dtcregdump.Ring5.RXStatus.DataOK,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPAddedRing5LED" ),dtcregdump.Ring5.RXStatus.SKPAdded,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPRemovedRing5LED" ),dtcregdump.Ring5.RXStatus.SKPRemoved,"RO" );
    setPixel( document.getElementById( "SERDESRXReceiverDetectedRing5LED" ),dtcregdump.Ring5.RXStatus.ReceiverDetected,"RO" );
    setPixel( document.getElementById( "SERDESRXDecodeErrorRing5LED" ),dtcregdump.Ring5.RXStatus.DecodeError,"RO" );
    setPixel( document.getElementById( "SERDESRXEOFRing5LED" ),dtcregdump.Ring5.RXStatus.EOverflow,"RO" );
    setPixel( document.getElementById( "SERDESRXEUFRing5LED" ),dtcregdump.Ring5.RXStatus.EUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESRXDisparityErrorRing5LED" ),dtcregdump.Ring5.RXStatus.DisparityError,"RO" );
    setPixel( document.getElementById( "SERDESRXDataOKCFOLED" ),dtcregdump.CFO.RXStatus.DataOK,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPAddedCFOLED" ),dtcregdump.CFO.RXStatus.SKPAdded,"RO" );
    setPixel( document.getElementById( "SERDESRXSKPRemovedCFOLED" ),dtcregdump.CFO.RXStatus.SKPRemoved,"RO" );
    setPixel( document.getElementById( "SERDESRXReceiverDetectedCFOLED" ),dtcregdump.CFO.RXStatus.ReceiverDetected,"RO" );
    setPixel( document.getElementById( "SERDESRXDecodeErrorCFOLED" ),dtcregdump.CFO.RXStatus.DecodeError,"RO" );
    setPixel( document.getElementById( "SERDESRXEOFCFOLED" ),dtcregdump.CFO.RXStatus.EOverflow,"RO" );
    setPixel( document.getElementById( "SERDESRXEUFCFOLED" ),dtcregdump.CFO.RXStatus.EUnderflow,"RO" );
    setPixel( document.getElementById( "SERDESRXDisparityErrorCFOLED" ),dtcregdump.CFO.RXStatus.DisparityError,"RO" );  
    setPixel( document.getElementById( "SERDESEyescanRing0LED" ),dtcregdump.Ring0.EyescanError,"RW" );
    setPixel( document.getElementById( "SERDESEyescanRing1LED" ),dtcregdump.Ring1.EyescanError,"RW" );
    setPixel( document.getElementById( "SERDESEyescanRing2LED" ),dtcregdump.Ring2.EyescanError,"RW" );
    setPixel( document.getElementById( "SERDESEyescanRing3LED" ),dtcregdump.Ring3.EyescanError,"RW" );
    setPixel( document.getElementById( "SERDESEyescanRing4LED" ),dtcregdump.Ring4.EyescanError,"RW" );
    setPixel( document.getElementById( "SERDESEyescanRing5LED" ),dtcregdump.Ring5.EyescanError,"RW" );
    setPixel( document.getElementById( "SERDESEyescanCFOLED" ),dtcregdump.CFO.EyescanError,"RW" );
    setPixel( document.getElementById( "SERDESRXCDRLockRing0LED" ),dtcregdump.Ring0.RXCDRLock,"RO" );
    setPixel( document.getElementById( "SERDESRXCDRLockRing1LED" ),dtcregdump.Ring1.RXCDRLock,"RO" );
    setPixel( document.getElementById( "SERDESRXCDRLockRing2LED" ),dtcregdump.Ring2.RXCDRLock,"RO" );
    setPixel( document.getElementById( "SERDESRXCDRLockRing3LED" ),dtcregdump.Ring3.RXCDRLock,"RO" );
    setPixel( document.getElementById( "SERDESRXCDRLockRing4LED" ),dtcregdump.Ring4.RXCDRLock,"RO" );
    setPixel( document.getElementById( "SERDESRXCDRLockRing5LED" ),dtcregdump.Ring5.RXCDRLock,"RO" );
    setPixel( document.getElementById( "SERDESRXCDRLockCFOLED" ),dtcregdump.CFO.RXCDRLock,"RO" );
    $( "#timestamp" ).val( dtcregdump.Timestamp.toString( 16 ) );
    setPixel( document.getElementById( "fpgapromfifofullLED" ),dtcregdump.PROMFIFOFull,"RO" );
    setPixel( document.getElementById( "fpgapromreadyLED" ),dtcregdump.PROMReady,"RO" );
    setPixel( document.getElementById( "fpgaCoreFIFOFullLED" ),dtcregdump.FPGACoreFIFOFull,"RO" );
    
}

function GetRegDumpAjax( fnCallback ) {
    // Check to see if there is currently an AJAX
    // request on this method.
    if ( GetRegDumpAjax.Xhr ) {
        // Abort the current request.
        GetRegDumpAjax.Xhr.abort( );
    }
    setPixel( document.getElementById( "RegDumpAjaxLED" ),1,"RO" );
    // Get data via AJAX. Store the XHR (AJAX request
    // object in the method in case we need to abort
    // it on subsequent requests.
    GetRegDumpAjax.Xhr = $.ajax( {
        type: "post",
        url: "/DTC/regDump",
        data: { data: "nullData" },
        dataType: "json",
        // Our success handler.
        success: function ( objData ) {
            // At this point, we have data coming back
            // from the server.
            setPixel( document.getElementById( "RegDumpAjaxLED" ),0,"RO" );
            fnCallback( {
                Value1: objData
            } );
        },
        // An error handler for the request.
        error: function ( xhr,textStatus,errorCode ) {
            setPixel( document.getElementById( "RegDumpAjaxLED" ),0,"ERR" );
            //alert("An error occurred:\n" + textStatus + "\n" + errorCode);
        },
        // I get called no matter what.
        complete: function () {
            // Remove completed request object.
            GetRegDumpAjax.Xhr = null;
        }
    } );
}

function GetRegDump() {
    var objData = null;
    GetRegDumpAjax( function ( regDump ) {
        PopulateLEDS( regDump.Value1 );
    } );
}


function AjaxPost( urlString,ringNum,fnCallback ) {
    // Check to see if there is currently an AJAX
    // request on this method.
    if ( AjaxPost.Xhr ) {
        // Abort the current request.
        AjaxPost.Xhr.abort( );
    }
    setPixel( document.getElementById( "AjaxRequestLED" ),1,"RO" );
    // Get data via AJAX. Store the XHR (AJAX request
    // object in the method in case we need to abort
    // it on subsequent requests.
    AjaxPost.Xhr = $.ajax( {
        type: "post",
        url: urlString,
        data: { ring: ringNum },
        dataType: "json",
        // Our success handler.
        success: function ( objData ) {
            // At this point, we have data coming back
            // from the server.
            setPixel( document.getElementById( "AjaxRequestLED" ),0,"RO" );
            fnCallback( {
                Value1: objData
            } );
        },
        // An error handler for the request.
        error: function ( xhr,textStatus,errorCode ) {
            setPixel( document.getElementById( "AjaxRequestLED" ),0,"ERR" );
            //alert("An error occurred:\n" + textStatus + "\n" + errorCode);
        },
        // I get called no matter what.
        complete: function () {
            // Remove completed request object.
            AjaxPost.Xhr = null;
        }
    } );
}


function LEDAction( url,ring,id ) {
    var objData = null;
    AjaxPost( url,ring,function ( output ) {
        setPixel( document.getElementById( id ),output.Value1,"RW" );
    } );
}
function LEDExtAction( url,ring,val,id ) {
    var objData = null;
    var post = {};
    post.ring = ring;
    post.val = val;
    AjaxPost( url,post,function ( output ) {
        setPixel( document.getElementById( id ),output.Value1,"RW" );
    } );
}
function LEDObjAction( url,ring,idlo,idhi ) {
    var objData = null;
    AjaxPost( url,ring,function ( output ) {
        setPixel( document.getElementById( idlo ),output.Value1.Low,"RW" );
        setPixel( document.getElementById( idhi ),output.Value1.High,"RW" );
    } );
}

function SetNumericValue(id, url) {
    var objData = null;
    var value = $( id ).val( );
    AjaxPost( url,value,function ( returnValue ) {
        $( id ).val( returnValue.Value1 );
    } );
}

function PostLogMessage() {
    var objData = null;
    var message = $( "#logMessage" ).val( );
    AjaxPost( '/DTC/WriteLog',message,function ( returnValue ) {
        $( "#log" ).val( returnValue.Value1 );
    } );
}

function ReadLog() {
    var objData = null;
    AjaxPost( '/DTC/ReadLog',null,function ( returnValue ) {
        $( "#log" ).val( returnValue.Value1 );
    } );
}

function RunScript() {
    var objData = null;
    AjaxPost( '/DTC/RunScript',$( "#script" ).val( ),function ( returnValue ) {
        $( "#script" ).val( returnValue );
    } );
}

( function ( $,sr ) {
    
    // debouncing function from John Hann
    // http://unscriptable.com/index.php/2009/03/20/debouncing-javascript-methods/
    var debounce = function ( func,threshold,execAsap ) {
        var timeout;
        
        return function debounced() {
            var obj = this, args = arguments;
            function delayed() {
                if ( !execAsap )
                    func.apply( obj,args );
                timeout = null;
            }            ;
            
            if ( timeout )
                clearTimeout( timeout );
            else if ( execAsap )
                func.apply( obj,args );
            
            timeout = setTimeout( delayed,threshold || 100 );
        };
    }
    // smartresize 
    jQuery.fn[sr] = function ( fn ) { return fn ? this.bind( 'resize',debounce( fn ) ) : this.trigger( sr ); };

} )( jQuery,'smartresize' );


// When the DOM is ready to be interacted with, init.
$( function () {
    setPixel( document.getElementById( "RegDumpAjaxLED" ),0,"RO" );
    setPixel( document.getElementById( "AjaxRequestLED" ),0,"RO" );
    setPixel( document.getElementById( "FormAjaxLED" ),0,"RO" );
    GetRegDump( );
    ReadLog( );
    $( "#post" ).click( function () {
        UpdateFormFields( );
    } );
    $( 'input[name=base]' ).change( function () {
        var jVal = $( "#value" );
        var jBase = $( 'input[name=base]:checked' ).val( );
        var oldval = parseInt( jVal.val( ),oldbase );
        jVal.val( oldval.toString( jBase ) );
        oldbase = jBase;
    } );
    $( "#logInterval" ).change( function () {
        if ( $( "#updateLog" ).is( ':checked' ) ) {
            var newVal = parseInt( $( "#logInterval" ).val( ) * 1000,10 );
            clearInterval( logIntervalHandle );
            logIntervalHandle = setInterval( function () { ReadLog( ); },newVal );
        }
    } );
    $( "#regInterval" ).change( function () {
        if ( $( "#updateReg" ).is( ':checked' ) ) {
            var newVal = parseInt( $( "#regInterval" ).val( ) * 1000,10 );
            clearInterval( regIntervalHandle );
            regIntervalHandle = setInterval( function () { GetRegDump( ); },newVal );
        }
    } );
    $( "#updateLog" ).change( function () {
        if ( $( "#updateLog" ).is( ':checked' ) ) {
            logIntervalHandle = setInterval( function () { ReadLog( ); },parseInt( $( "#logInterval" ).val( ) * 1000,10 ) );
        }
        else {
            clearInterval( logIntervalHandle );
        }
    } );
    logIntervalHandle = setInterval( function () { ReadLog( ); },parseInt( $( "#logInterval" ).val( ) * 1000,10 ) );
    $( "#updateReg" ).change( function () {
        if ( $( "#updateReg" ).is( ':checked' ) ) {
            regIntervalHandle = setInterval( function () { GetRegDump( ); },parseInt( $( "#regInterval" ).val( ) * 1000,10 ) );
        }
        else {
            clearInterval( regIntervalHandle );
        }
    } );
    
    var sendIds = {
        send: { data: [{ time: 0, value: 0 }], color: 'black', jsonPath: "/DTC/Send" },
        spayload: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/SPayload" },
    };
    makeGraph( "#send",sendIds );
    var recIds = {
        receive: { data: [{ time: 0, value: 0 }], color: 'black', jsonPath: "/DTC/Receive" },
        rpayload: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/RPayload" },
    };
    makeGraph( "#receive",recIds );
    
    $( window ).smartresize( function () {
        $( "#send" ).empty( );
        $( "#receive" ).empty( );
        makeGraph( "#send",sendIds );
        makeGraph( "#receive",recIds );
    } )
} );
