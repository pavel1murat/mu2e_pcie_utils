// serverbase.js : Node HTTP Server
// Author: Eric Flumerfelt, FNAL RSI
// Last Modified: October 30, 2014
// Modified By: Eric Flumerfelt
//
// serverbase sets up a basic HTTP server and directs requests
// to one of its submodules. Sub-modules may be added by inserting
// appropriate code in the "GET" request section of the code below.

// Node.js framework "includes"
var http = require('https');
var url = require('url');
var fs = require('fs');
var qs = require('querystring');
var cluster = require('cluster');
var numCPUs = require("os").cpus().length;

var options = {
   key:   fs.readFileSync('server.key'),
   cert:  fs.readFileSync('server.crt'),
   ca:    fs.readFileSync('ca.crt'),
   requestCert:   true,
   rejectUnauthorized: true
};

// Sub-Module files
var dtcdriver = require('./DTCDriver.js');
dtcdriver.init();

// Write out the Client-side HTML
function writeHTML()
{
    // When adding a submodule, don't forget to edit this file!
  return fs.readFileSync("client.html"); 
}


// Node.js by default is single-threaded. Start multiple servers sharing
// the same port so that an error doesn't bring the whole system down
if (cluster.isMaster) {
    // Start workers for each CPU on the host
  for (var i = 0; i < numCPUs; i++) {
    cluster.fork();
  }

  // If one dies, start a new one!
  cluster.on("exit", function(worker, code, signal) {
    cluster.fork();
  });
} else {
    // Make an http server
  var server = http.createServer(options, function (req, res) {

    // req is the HTTP request, res is the response the server will send
    // pathname is the URL after the http://host:port/ clause
    var pathname = url.parse(req.url, true).pathname;
    //console.log("req.client debug info: " + req.client +", pathname: " + pathname);
  if(req.client.authorized) {
    // Log to console...
    //console.log("Recieved " + req.method + " for " + pathname);
    console.log("Proceeding...");
  
    // If we're recieving a POST to /runcommand (As defined in the module),
    // handle that here
    if(req.method == "POST") {
	if(pathname.search("dtc_register_io") > 0 ) {
	console.log("In POST handler");
       var body = "";
       // res.end('post');
       //console.log('Request found with POST method');     

       // Callback for request data (may come in async)
        req.on('data', function (data) {
            body += data;
        });

        // When the request is finished, run this callback:
        req.on('end', function () {
		// Get the content of the POST request 
            var POST = qs.parse(body);
            var value =0;
            
            // If the POST contains a "comm" value, this is the command the
	    // user typed in the "Command: " box
            console.log("Option is: " + POST.option);
            if(POST.option == "read") {
		console.log("Reading " + POST.address);
		value = dtcdriver.read(POST.address);
            }
            else if(POST.option == "write") {
                console.log("Writing " + POST.value + " to " + POST.address);
                value = dtcdriver.write(POST.address, POST.value);
            }

            // Send the reply (if there was a recognized POST operation above,
	    //  getBuf() will reflect the changes).
            console.log("Replying with value " + dtcdriver.value.toString(16));
            res.end(value.toString(16));
        });
      }
	if(pathname.search("dtc_reg_dump") > 0 ) {
	    var dtcRegisters = {};
	    dtcRegisters.Ring0 = {};
            dtcRegisters.Ring1 = {};
            dtcRegisters.Ring2 = {};
            dtcRegisters.Ring3 = {};
            dtcRegisters.Ring4 = {};
            dtcRegisters.Ring5 = {};
                dtcRegisters.Version = dtcdriver.read("0x9000");
                dtcRegisters.ResetDTC = dtcdriver.readResetDTC;
                dtcRegisters.ClearLatchedErrors = dtcdriver.readClearLatchedErrors();
                dtcRegisters.Ring0.SERDESLoopback = dtcdriver.readSERDESLoopback(0);
                dtcRegisters.Ring1.SERDESLoopback = dtcdriver.readSERDESLoopback(1);
                dtcRegisters.Ring2.SERDESLoopback = dtcdriver.readSERDESLoopback(2);
                dtcRegisters.Ring3.SERDESLoopback = dtcdriver.readSERDESLoopback(3);
                dtcRegisters.Ring4.SERDESLoopback = dtcdriver.readSERDESLoopback(4);
                dtcRegisters.Ring5.SERDESLoopback = dtcdriver.readSERDESLoopback(5);
                dtcRegisters.ROCEmulator = dtcdriver.readROCEmulator();
                dtcRegisters.Ring0.Enabled = dtcdriver.readRingEnabled(0);
                dtcRegisters.Ring1.Enabled = dtcdriver.readRingEnabled(1);
                dtcRegisters.Ring2.Enabled = dtcdriver.readRingEnabled(2);
                dtcRegisters.Ring3.Enabled = dtcdriver.readRingEnabled(3);
                dtcRegisters.Ring4.Enabled = dtcdriver.readRingEnabled(4);
                dtcRegisters.Ring5.Enabled = dtcdriver.readRingEnabled(5);
                dtcRegisters.Ring0.ResetSERDES = dtcdriver.readResetSERDES(0);
                dtcRegisters.Ring1.ResetSERDES = dtcdriver.readResetSERDES(1);
                dtcRegisters.Ring2.ResetSERDES = dtcdriver.readResetSERDES(2);
                dtcRegisters.Ring3.ResetSERDES = dtcdriver.readResetSERDES(3);
                dtcRegisters.Ring4.ResetSERDES = dtcdriver.readResetSERDES(4);
                dtcRegisters.Ring5.ResetSERDES = dtcdriver.readResetSERDES(5);
                dtcRegisters.Ring0.SERDESRXDisparity = dtcdriver.readSERDESRXDisparity(0);
                dtcRegisters.Ring1.SERDESRXDisparity = dtcdriver.readSERDESRXDisparity(1);
                dtcRegisters.Ring2.SERDESRXDisparity = dtcdriver.readSERDESRXDisparity(2);
                dtcRegisters.Ring3.SERDESRXDisparity = dtcdriver.readSERDESRXDisparity(3);
                dtcRegisters.Ring4.SERDESRXDisparity = dtcdriver.readSERDESRXDisparity(4);
                dtcRegisters.Ring5.SERDESRXDisparity = dtcdriver.readSERDESRXDisparity(5);
                dtcRegisters.Ring0.CharacterError = dtcdriver.readSERDESRXCharacterError(0);
                dtcRegisters.Ring1.CharacterError = dtcdriver.readSERDESRXCharacterError(1);
                dtcRegisters.Ring2.CharacterError = dtcdriver.readSERDESRXCharacterError(2);
                dtcRegisters.Ring3.CharacterError = dtcdriver.readSERDESRXCharacterError(3);
                dtcRegisters.Ring4.CharacterError = dtcdriver.readSERDESRXCharacterError(4);
                dtcRegisters.Ring5.CharacterError = dtcdriver.readSERDESRXCharacterError(5);
                dtcRegisters.Ring0.UnlockError = dtcdriver.readSERDESUnlockError(0);
                dtcRegisters.Ring1.UnlockError = dtcdriver.readSERDESUnlockError(1);
                dtcRegisters.Ring2.UnlockError = dtcdriver.readSERDESUnlockError(2);
                dtcRegisters.Ring3.UnlockError = dtcdriver.readSERDESUnlockError(3);
                dtcRegisters.Ring4.UnlockError = dtcdriver.readSERDESUnlockError(4);
                dtcRegisters.Ring5.UnlockError = dtcdriver.readSERDESUnlockError(5);
                dtcRegisters.Ring0.PLLLocked = dtcdriver.readSERDESPLLLocked(0);
                dtcRegisters.Ring1.PLLLocked = dtcdriver.readSERDESPLLLocked(1);
                dtcRegisters.Ring2.PLLLocked = dtcdriver.readSERDESPLLLocked(2);
                dtcRegisters.Ring3.PLLLocked = dtcdriver.readSERDESPLLLocked(3);
                dtcRegisters.Ring4.PLLLocked = dtcdriver.readSERDESPLLLocked(4);
                dtcRegisters.Ring5.PLLLocked = dtcdriver.readSERDESPLLLocked(5);
                dtcRegisters.Ring0.OverflowOrUnderflow = dtcdriver.readSERDESOverflowOrUnderflow(0);
                dtcRegisters.Ring1.OverflowOrUnderflow = dtcdriver.readSERDESOverflowOrUnderflow(1);
                dtcRegisters.Ring2.OverflowOrUnderflow = dtcdriver.readSERDESOverflowOrUnderflow(2);
                dtcRegisters.Ring3.OverflowOrUnderflow = dtcdriver.readSERDESOverflowOrUnderflow(3);
                dtcRegisters.Ring4.OverflowOrUnderflow = dtcdriver.readSERDESOverflowOrUnderflow(4);
                dtcRegisters.Ring5.OverflowOrUnderflow = dtcdriver.readSERDESOverflowOrUnderflow(5);
                dtcRegisters.Ring0.FIFOHalfFull = dtcdriver.readSERDESBufferFIFOHalfFull(0);
                dtcRegisters.Ring1.FIFOHalfFull = dtcdriver.readSERDESBufferFIFOHalfFull(1);
                dtcRegisters.Ring2.FIFOHalfFull = dtcdriver.readSERDESBufferFIFOHalfFull(2);
                dtcRegisters.Ring3.FIFOHalfFull = dtcdriver.readSERDESBufferFIFOHalfFull(3);
                dtcRegisters.Ring4.FIFOHalfFull = dtcdriver.readSERDESBufferFIFOHalfFull(4);
                dtcRegisters.Ring5.FIFOHalfFull = dtcdriver.readSERDESBufferFIFOHalfFull(5);
                dtcRegisters.Ring0.RXBufferStatus = dtcdriver.readSERDESRXBufferStatus(0);
                dtcRegisters.Ring1.RXBufferStatus = dtcdriver.readSERDESRXBufferStatus(1);
                dtcRegisters.Ring2.RXBufferStatus = dtcdriver.readSERDESRXBufferStatus(2);
                dtcRegisters.Ring3.RXBufferStatus = dtcdriver.readSERDESRXBufferStatus(3);
                dtcRegisters.Ring4.RXBufferStatus = dtcdriver.readSERDESRXBufferStatus(4);
                dtcRegisters.Ring5.RXBufferStatus = dtcdriver.readSERDESRXBufferStatus(5);
                dtcRegisters.Ring0.ResetDone = dtcdriver.readSERDESResetDone(0);
                dtcRegisters.Ring1.ResetDone = dtcdriver.readSERDESResetDone(1);
                dtcRegisters.Ring2.ResetDone = dtcdriver.readSERDESResetDone(2);
                dtcRegisters.Ring3.ResetDone = dtcdriver.readSERDESResetDone(3);
                dtcRegisters.Ring4.ResetDone = dtcdriver.readSERDESResetDone(4);
                dtcRegisters.Ring5.ResetDone = dtcdriver.readSERDESResetDone(5);
                dtcRegisters.Timestamp = dtcdriver.readTimestampPreset();
                dtcRegisters.PROMFIFOFull = dtcdriver.readFPGAPROMProgramFIFOFull();
                dtcRegisters.PROMReady = dtcdriver.readFPGAPROMReady();
                console.log(dtcRegisters.toString());
                res.end(JSON.stringify(dtcRegisters));
	    }
    }
    //We got a GET request!
  if(req.method == "GET") {
    if(pathname.search("jquery") > 0) {
      console.log("Sending jquery.min.js");
      res.writeHeader(200, {'Content-Type': 'text/javascript'});
      res.end(fs.readFileSync("jquery.min.js"), 'utf-8'); 
      console.log("Done sending jquery.min.js");
    } else if(pathname.search("client.js") > 0) {
      console.log("Sending client.js");
      res.writeHeader(200, {'Content-Type': 'text/javascript'});
      res.end(fs.readFileSync("client.js"), 'utf-8'); 
      console.log("Done sending client.js");
    } else {
      // The response will always be 'text/html'. 
      // Stuff inside the iframe may be different...
      res.writeHeader({'Content-Type': 'text/html'});

      console.log("Running base");
      // Write out the frame code
      res.end(writeHTML(), 'utf-8');
    }
  }
} else {
  res.writeHeader(500, {'Content-Type': 'text/html'});
  res.end("<html><body><p>You need to have a Fermi KCA Certificate to view this page</body></html>");
}
});
  //Listen on port 8080
server.listen(8080);
}
