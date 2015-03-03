
function setPixel( led,bit ) {
    var ctx = led.getContext( "2d" );
    
    ctx.lineWidth = 2;
    
    ctx.strokeStyle = "darkgreen";
    if ( bit ) {
        ctx.fillStyle = "lightgreen";
    } else {
        ctx.fillStyle = 'black';
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

function ApplyTestStats( testStats ) {
    $( "#daqPassed" ).val( testStats.daqPassed );
    $( "#daqFailed" ).val( testStats.daqFailed );
    $( "#daqPassPercent" ).val( testStats.daqPassed * 100 / ( testStats.daqPassed + testStats.daqFailed ) );
    $( "#dcsPassed" ).val( testStats.dcsPassed );
    $( "#dcsFailed" ).val( testStats.dcsFailed );
    $( "#dcsPassPercent" ).val( testStats.dcsPassed * 100 / ( testStats.dcsPassed + testStats.dcsFailed ) );
    setPixel( document.getElementById( "testsRunning" ),testStats.testRunning );
}

function GetTestStatistics() {
    var objData = null;
    AjaxGet( '/DTC/DMATestStatistics',function ( returnValue ) {
        var testStats = returnValue;
        $( "#daqC2S" ).val( testStats.daqC2S );
        $( "#daqS2C" ).val( testStats.daqS2C );
        
        $( "#dcsC2S" ).val( testStats.dcsC2S );
        $( "#dcsS2C" ).val( testStats.dcsS2C );
       
        ApplyTestStats( testStats );
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
            }
            
            if ( timeout )
                clearTimeout( timeout );
            else if ( execAsap )
                func.apply( obj,args );
            
            timeout = setTimeout( delayed,threshold || 100 );
        };
    };
    // smartresize 
    jQuery.fn[sr] = function ( fn ) { return fn ? this.bind( 'resize',debounce( fn ) ) : this.trigger( sr ); };

} )( jQuery,'smartresize' );


// When the DOM is ready to be interacted with, init.
$( function () {
    $( "#runButton" ).click( function () {
        var data = {};
        data.daq = $( "#daqEnabled" ).is( ":checked" );
        data.dcs = $( "#dcsEnabled" ).is( ":checked" );
        data.n = $( "#numTests" ).val( );
        AjaxPost( '/DTC/StartDMATest',data,function ( returnValue ) {
            ApplyTestStats( returnValue );
        } );
    } );
    
    $( "#resetButton" ).click( function () {
        AjaxPost( '/DTC/ResetTestStatus',1,function ( returnValue ) {
            ApplyTestStats( returnValue );
        } );
    } );
    
    $("#stopButton").click(function () {
        AjaxPost('/DTC/StopDMATest', 1, function (returnValue) {
            ApplyTestStats(returnValue);
        });
    });

    var dma0 = {
        dma0TX: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/DMA0Transmit" },
        dma0RX: { data: [{ time: 0, value: 0 }], color: 'blue', jsonPath: "/DTC/DMA0Receive" },
    };
    makeGraph( "#dma0",dma0 );
    var dma1 = {
        dma1TX: { data: [{ time: 0, value: 0 }], color: 'red', jsonPath: "/DTC/DMA1Transmit" },
        dma1RX: { data: [{ time: 0, value: 0 }], color: 'blue', jsonPath: "/DTC/DMA1Receive" },
    };
    makeGraph( "#dma1",dma1 );
    
    $( window ).smartresize( function () {
        $( "#dma0" ).empty( );
        $( "#dma1" ).empty( );
        makeGraph( "#dma0",dma0 );
        makeGraph( "#dma1",dma1 );
    } );
    
    setInterval( function () { GetTestStatistics( ); },1000 );
} );
