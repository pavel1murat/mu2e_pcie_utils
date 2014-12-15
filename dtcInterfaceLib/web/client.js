var sendData = [],
    receiveData = [];

var n = 100, //The number of points to plot
    duration = 1000; //ms between updates

function tick(path, line, data, axis, yaxis, x, y, id) {
    var transition = d3.select("#" + id).transition()
                .duration(duration)
                .ease("linear");
    transition = transition.each(function () {
        
        // update the domains
        now = new Date();
        x.domain([now - (n - 2) * duration, now - duration]);
        var extent = d3.extent(data, function (d) { return d.value; });
        y.domain([extent[0] * 9 / 10, extent[1] ]);
        
        d3.json("./json_" + id, function (json) {
            data.push({ value: json.value, time: new Date(json.time) });
        });
        
        // redraw the line
        path
                        .attr("d", line)
                        .attr("transform", null);
        
        // slide the x-axis left
        axis.call(x.axis);
        yaxis.call(y.axis);
        
        // slide the line left
        path.transition()
            .duration(duration)
            .ease("linear")
                        .attr("transform", "translate(" + x(now - (n - 1) * duration) + ")");
        
        while (data.length > 0 && data[0].time < now - n * duration) {
            data.shift();
        }

    }).transition().each("start", function () { setTimeout(tick(path, line, data, axis, yaxis, x, y, id),duration/2) });
}

function makeGraph(data, id, tick) {
    var now = new Date(Date.now() - duration);
    
    var margin = { top: 6, right: 10, bottom: 20, left: 100 },
        width = $("#" + id).width() - margin.left - margin.right,
        height = 120 - margin.top - margin.bottom;
    
    var x = d3.time.scale()
                .domain([now - (n - 2) * duration, now - duration])
                .range([0, width]);
    
    var y = d3.scale.linear()
                .range([height, 0]);
    
    var line = d3.svg.line()
                .interpolate("linear")
                .x(function (d) { return x(d.time); })
                .y(function (d) { return y(d.value); });
    
    var svg = d3.select("#" + id).append("svg")
                .attr("width", width + margin.left + margin.right)
                .attr("height", height + margin.top + margin.bottom)
                .style("margin-left", -margin.left + "px")
              .append("g")
                .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
    
    svg.append("defs").append("clipPath")
                .attr("id", "clip")
              .append("rect")
                .attr("width", width)
                .attr("height", height);
    
    var axis = svg.append("g")
                .attr("class", "x axis")
                .attr("transform", "translate(0," + height + ")")
                .call(x.axis = d3.svg.axis().scale(x).orient("bottom"));
    
    var formatTick = function (d) {
        var prefix = d3.formatPrefix(d);
        return prefix.scale(d) + " " + prefix.symbol + "B/s";
    }
    var yaxis = svg.append("g")
                .attr("class", "y axis")
                .call(y.axis = d3.svg.axis().scale(y).tickFormat(formatTick).orient("left"));
    
    var path = svg.append("g")
                .attr("clip-path", "url(#clip)")
              .append("path")
                .datum(data)
                .attr("class", "line");
    
    
    tick(path, line, data, axis, yaxis, x, y, id);
}

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
    setPixel(document.getElementById("FormAjaxLED"), 1, "RO");
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
        url: "./dtc_reg_dump",
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


function AjaxPost(urlString, ringNum, fnCallback) {
    // Check to see if there is currently an AJAX
    // request on this method.
    if (AjaxPost.Xhr) {
        // Abort the current request.
        AjaxPost.Xhr.abort();
    }
    setPixel(document.getElementById("AjaxRequestLED"), 1, "RO");
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
            setPixel(document.getElementById("AjaxRequestLED"), 0, "RO");
            fnCallback({
                Value1: objData
            });
        },
        // An error handler for the request.
        error: function (xhr, textStatus, errorCode) {
            setPixel(document.getElementById("AjaxRequestLED"), 0, "ERR");
            //alert("An error occurred:\n" + textStatus + "\n" + errorCode);
        },
        // I get called no matter what.
        complete: function () {
            // Remove completed request object.
            AjaxPost.Xhr = null;
        }
    });
}


function LEDAction(url, ring, id) {
    var objData = null;
    AjaxPost(url, ring, function (output) {
        setPixel(document.getElementById(id), output.Value1, "RW");
    });
}

function SetTimestamp() {
    var objData = null;
    var value = $("#timestamp").val();
    AjaxPost('./DTC/setTimestampPreset', value, function (returnValue) {
        $("#timestamp").val(returnValue.Value1);
    });
}

function PostLogMessage() {
    var objData = null;
    var message = $("#logMessage").val();
    AjaxPost('./log_message', message, function (returnValue) {
        $("#log").val(returnValue.Value1);
    });
}

function ReadLog() {
    var objData = null;
    AjaxPost('./log_read', null, function (returnValue) {
        $("#log").val(returnValue.Value1);
    });
}

function RunScript() {
    var objData = null;
    AjaxPost('./run_script', $("#script").val(), function (returnValue) {
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
    makeGraph(sendData, "send", tick);
    makeGraph(receiveData, "receive", tick)
});
