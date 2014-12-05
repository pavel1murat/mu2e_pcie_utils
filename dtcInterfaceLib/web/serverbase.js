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

// Write out the HTML fragment for the frame and menu
function writeFrameHTML()
{
    // When adding a submodule, don't forget to edit this file!
  return fs.readFileSync("template.html.in"); 
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
  if(req.client.authorized) {
    // Log to console...
    //console.log("Recieved " + req.method + " for " + pathname);
    console.log("Proceeding...");
  
    // If we're recieving a POST to /runcommand (As defined in the module),
    // handle that here
    if(req.method == "POST" && pathname.search("dtc_registers") > 0 ) {
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
            
            // If the POST contains a "comm" value, this is the command the
	    // user typed in the "Command: " box
            console.log("Option is: " + POST.option);
            if(POST.option == "read") {
		console.log("Reading " + POST.address);
		dtcdriver.read(POST.address);
            }
            else if(POST.option == "write") {
                console.log("Writing " + POST.value + " to " + POST.address);
                dtcdriver.write(POST.address, POST.value);
            }

            // Send the reply (if there was a recognized POST operation above,
	    //  getBuf() will reflect the changes).
            console.log("Replying with value " + dtcdriver.value.toString(16));
            res.end(dtcdriver.value.toString(16));
        });
  }
    //We got a GET request!
  if(req.method == "GET") {
    if(pathname.search("jquery") > 0) {
      console.log("Sending jquery.js");
      res.writeHeader(200, {'Content-Type': 'text/javascript'});
      res.end(fs.readFileSync("jquery.min.js"), 'utf-8'); 
      console.log("Done sending jquery");
    } else {
      // The response will always be 'text/html'. 
      // Stuff inside the iframe may be different...
      res.writeHeader({'Content-Type': 'text/html'});

      console.log("Running base");
      // Write out the frame code
      res.end(writeFrameHTML(), 'utf-8');
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
