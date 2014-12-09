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
    key: fs.readFileSync('server.key'),
    cert: fs.readFileSync('server.crt'),
    ca: fs.readFileSync('ca.crt'),
    requestCert: true,
    rejectUnauthorized: true
};

// Sub-Module files
var dtcdriver = require('./DTCDriver.js');
dtcdriver.init();

// Write out the Client-side HTML
function writeHTML() {
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
    cluster.on("exit", function (worker, code, signal) {
        cluster.fork();
    });
} else {
    // Make an http server
    var server = http.createServer(options, function (req, res) {
        
        // req is the HTTP request, res is the response the server will send
        // pathname is the URL after the http://host:port/ clause
        var pathname = url.parse(req.url, true).pathname;
        //console.log("req.client debug info: " + req.client +", pathname: " + pathname);
        if (req.client.authorized) {
            var clientCertificate = req.connection.getPeerCertificate();
            var useremail = clientCertificate.subject.CN[1].substr(4);
            var userFNAL = useremail + "@FNAL.GOV";
            var userWIN = useremail + "@FERMI.WIN.FNAL.GOV"
            var k5login = "" + fs.readFileSync(process.env.HOME + "/.k5login");
            if (!(k5login.search(userFNAL) > 0 || k5login.search(userWIN) > 0)) {
                res.writeHeader(401, { 'Content-Type': 'text/html' });
                res.end("Please Contact a DAQ Expert for authorization");
                console.log("Unauthorized access attempt from " + useremail);
            } else {
                function log(message, method) {
                    fs.exists("./DTC.log", function (exists) {
                        if (!exists) {
                            fs.writeFileSync("./DTC.log", "Log file created at " + (new Date()).toLocaleString());
                        }
                    });
                    message = "\n" + (new Date()).toLocaleString() + " " + clientCertificate.subject.CN[0] + " " + method + " \"" + message + "\"";
                    console.log(message);
                    fs.appendFileSync("./DTC.log", message);
                }
                
                // Log to console...
                //console.log("Recieved " + req.method + " for " + pathname);
                //console.log("Proceeding...");
                
                // If we're recieving a POST to /runcommand (As defined in the module),
                // handle that here
                if (req.method == "POST") {
                    if (pathname.search("dtc_register_io") > 0) {
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
                            var value = 0;
                            
                            // If the POST contains a "comm" value, this is the command the
                            // user typed in the "Command: " box
                            console.log("Option is: " + POST.option);
                            if (POST.option == "read") {
                                log(POST.address, "read register");
                                value = dtcdriver.read(POST.address);
                            }
                            else if (POST.option == "write") {
                                log(POST.value.toString(16) + " to " + POST.address.toString(16), "wrote");
                                value = dtcdriver.write(POST.address, POST.value);
                            }
                            
                            // Send the reply (if there was a recognized POST operation above,
                            //  getBuf() will reflect the changes).
                            console.log("Replying with value " + value.toString(16));
                            res.end(value.toString(16));
                        });
                    }
                    else if (pathname.search("run_script") > 0) {
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
                            var value = 0;
                            var success = true;
                            var text = POST.ring;
                            var lines = text.split("\n");
                            for (var i = 0; i < lines.length; i++) {
                                var thisLine = lines[i];
                                var thisLineSplit = thisLine.split(" ");
                                var address = thisLineSplit[0];
                                var val = thisLineSplit[1];
                                log("a write: " + val + " to " + address, "scripted");
                                value = dtcdriver.write(address, val);
                                if (value != val) { success = false; }
                            }
                            if (success) {
                                log("the script was run successfully.", "noticed that");
                                res.end("Success!");
                            }
                            else {
                                log("the script had an error!", "noticed that");
                                res.end("Script did not run successfully!!!");
                            }
                        });
                    }
                    else if (pathname.search("dtc_reg_dump") > 0) {
                        var dtcRegisters = dtcdriver.regDump();
                        var data = JSON.stringify(dtcRegisters);
                        //console.log("Ending response");
                        res.end(data);
                    }
                    else if (pathname.search("DTC") > 0) {
                        //console.log("Recieved POST to " + pathname);
                        var body = "";
                        // res.end('post');
                        //console.log('Request found with POST method');     
                        
                        // Callback for request data (may come in async)
                        req.on('data', function (data) {
                            body += data;
                        });
                        
                        // When the request is finished, run this callback:
                        req.on('end', function () {
                            var POST = qs.parse(body);
                            var functionName = pathname.replace("/DTC/", "");
                            var data = dtcdriver[functionName](parseInt(POST.ring, 16));
                            log(functionName + " " + POST.ring + " (" + data + ")", "executed");
                            //console.log(data);
                            dataStr = JSON.stringify(data);
                            //console.log("That LED is now " + dataStr);
                            res.end(dataStr);
                        });
                    }
                    else if (pathname.search("log_message") > 0) {
                        console.log("Logging message");
                        var body = "";
                        // res.end('post');
                        //console.log('Request found with POST method');     
                        
                        // Callback for request data (may come in async)
                        req.on('data', function (data) {
                            body += data;
                        });
                        
                        // When the request is finished, run this callback:
                        req.on('end', function () {
                            var POST = qs.parse(body);
                            log(POST.ring, "says");
                            var data = "" + fs.readFileSync("./DTC.log");
                            var dataStr = JSON.stringify(data);
                            //console.log("Ending response: " + dataStr);
                            res.end(dataStr);
                        });
                    }
                }
                //We got a GET request!
                if (req.method == "GET") {
                    if (pathname.search("jquery") > 0) {
                        console.log("Sending jquery.min.js");
                        res.writeHeader(200, { 'Content-Type': 'text/javascript' });
                        res.end(fs.readFileSync("jquery.min.js"), 'utf-8');
                        console.log("Done sending jquery.min.js");
                    } else if (pathname.search("client.js") > 0) {
                        console.log("Sending client.js");
                        res.writeHeader(200, { 'Content-Type': 'text/javascript' });
                        res.end(fs.readFileSync("client.js"), 'utf-8');
                        console.log("Done sending client.js");
                    } else {
                        // The response will always be 'text/html'. 
                        // Stuff inside the iframe may be different...
                        res.writeHeader({ 'Content-Type': 'text/html' });
                        
                        console.log("Running base");
                        // Write out the frame code
                        res.end(writeHTML(), 'utf-8');
                    }
                }
            }
        } else {
            res.writeHeader(500, { 'Content-Type': 'text/html' });
            res.end("<html><body><p>You need to have a Fermi KCA Certificate to view this page</body></html>");
        }
    });
    /*
    var io = require('socket.io').listen(server);

    io.sockets.on('connection', function (socket) {
        var user_id = socket.id;
        
        member_sockets[user_id] = socket;
        console.log("[login]-->", user_id);
        
        socket.on('txt_change', function (data) {
            for (key in member_sockets) {
                if (key != user_id) {
                    member_sockets[key].emit("txt_change", data);
                }
            }
        });
        
        
        socket.on('disconnect', function (socket) {
            console.log("[logout]-->", user_id);
            delete member_sockets[user_id];
        });
    });
    */
    //Listen on port 8080
    server.listen(8080);
}
