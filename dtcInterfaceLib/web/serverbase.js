// serverbase.js : Node HTTP Server
// Author: Eric Flumerfelt, FNAL RSI
// Last Modified: October 30, 2014
// Modified By: Eric Flumerfelt
//
// serverbase sets up a basic HTTP server and directs requests
// to one of its submodules. Sub-modules may be added by inserting
// appropriate code in the "GET" request section of the code below.

var cluster = require('cluster');
var numCPUs = require("os").cpus().length;


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
    
    // Node.js framework "includes"
    var https = require('https');
    var url = require('url');
    var fs = require('fs');
    var qs = require('querystring');
    
    // Sub-Module files
    console.log("Initializing DTC Driver");
    var dtcdriver = require('./DTCDriver.js');
    console.log("Done initializing DTC Driver");
    
    console.log("Setting up log file");
    if (!fs.existsSync("./DTC.log")) {
        console.log("Creating new log file");
        fs.writeFileSync("./DTC.log", "Log file created at " + new Date().toLocaleString());
    }
    console.log("Done setting up log");
    
    var logMessage = function (message, method, name) {
        message = "\n" + new Date().toLocaleString() + " " + name + " " + method + " \"" + message + "\"";
        console.log(message);
        fs.appendFileSync("./DTC.log", message);
        console.log("Done logging message");
    }
    var readLog = function () {
        console.log("Reading log file");
        var logContent = "" + fs.readFileSync("./DTC.log");
        return JSON.stringify(logContent);
    }
        
    console.log("Setting up options");
    var options = {
        key: fs.readFileSync('./certs/server.key'),
        cert: fs.readFileSync('./certs/server.crt'),
        ca: fs.readFileSync('./certs/ca.crt'),
        requestCert: true,
        rejectUnauthorized: true
    };
    var k5login = " " + fs.readFileSync(process.env.HOME + "/.k5login");
    console.log("Done setting up options");
    
    // Make an http server
    var server = https.createServer(options, function (req, res) {
        // req is the HTTP request, res is the response the server will send
        // pathname is the URL after the http://host:port/ clause
        var pathname = url.parse(req.url, true).pathname;
        //console.log("req.client debug info: " + req.client + ", pathname: " + pathname);
        if (req.client.authorized) {
            var clientCertificate = req.connection.getPeerCertificate();
            var username = clientCertificate.subject.CN[0];
            var useremail = clientCertificate.subject.CN[1].substr(4);
            var userFNAL = useremail + "@FNAL.GOV";
            var userWIN = useremail + "@FERMI.WIN.FNAL.GOV";
            if (!(k5login.search(userFNAL) >= 0 || k5login.search(userWIN) >= 0)) {
                res.writeHeader(401, { 'Content-Type': 'text/html' });
                res.end("Please Contact a DAQ Expert for authorization");
                console.log("Unauthorized access attempt from " + useremail);
            } else {
                res.setHeader("Access-Control-Allow-Origin", "*");
                res.setHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
                res.setHeader("Content-Type", "application/json");
                res.statusCode = 200;
                // Log to console...
                //console.log("Recieved " + req.method + " for " + pathname);
                //console.log("Proceeding...");
                
                // If we're recieving a POST to /runcommand (As defined in the module),
                // handle that here
                if (req.method === "POST") {
                    console.log("In POST handler, PID: " + process.pid);
                    var body = "";
                    
                    // Callback for request data (may come in async)
                    req.on('data', function (data) {
                        body += data;
                    });
                    
                    if (pathname.search("dtc_register_io") > 0) {
                        console.log("In Register IO handler");
                        // When the request is finished, run this callback:
                        req.on('end', function () {
                            // Get the content of the POST request 
                            var POST = qs.parse(body);
                            var value = 0;
                            
                            // If the POST contains a "comm" value, this is the command the
                            // user typed in the "Command: " box
                            console.log("Option is: " + POST.option);
                            if (POST.option === "read") {
                                logMessage(POST.address, "read register", username);
                                value = dtcdriver.read(POST.address);
                            }
                            else if (POST.option === "write") {
                                logMessage(POST.value.toString(16) + " to " + POST.address.toString(16), "wrote", username);
                                value = dtcdriver.write(POST.address, POST.value);
                            }
                            
                            console.log("Replying with value " + value.toString(16));
                            res.end(JSON.stringify(value.toString(16)));
                            console.log("Done with reply");
                            console.log("DTC Error Status: " + dtcdriver.Err);
                        });
                    }
                    else if (pathname.search("run_script") > 0) {
                        console.log("In Script handler");
                        
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
                                logMessage("a write: " + val + " to " + address, "scripted", username);
                                value = dtcdriver.write(address, val);
                                if (value !== val) { success = false; }
                            }
                            if (success) {
                                logMessage("the script was run successfully.", "noticed that", username);
                                
                                res.setHeader("Content-Type", "text/plain");
                                res.end("Success!");
                            }
                            else {
                                logMessage("the script had an error!", "noticed that", username);
                                res.setHeader("Content-Type", "text/plain");
                                res.end("Script did not run successfully!!!");
                                console.log("DTC Error Status: " + dtcdriver.Err);
                            }
                        });
                    }
                    else if (pathname.search("dtc_reg_dump") > 0) {
                        console.log("Doing RegDump");
                        var data = JSON.stringify(dtcdriver.regDump());
                        
                        console.log("Sending response");
                        res.end(data);
                        console.log("Done sending regdump");
                    }
                    else if (pathname.search("DTC") > 0) {
                        console.log("Recieved POST to " + pathname);
                        
                        // When the request is finished, run this callback:
                        req.on('end', function () {
                            var POST = qs.parse(body);
                            var functionName = pathname.replace("/DTC/", "");
                            console.log("Running " + functionName);
                            var data = dtcdriver[functionName](parseInt(POST.ring, 16));
                            logMessage(functionName + " " + POST.ring + " (" + data + ")", "executed", username);
                            console.log(data);
                            dataStr = JSON.stringify(data);
                            console.log("That LED is now " + dataStr);
                            res.end(dataStr);
                            console.log("Done sending reply");
                        });
                    }
                    else if (pathname.search("log_message") > 0) {
                        console.log("Logging message");
                        
                        // When the request is finished, run this callback:
                        req.on('end', function () {
                            console.log("Done recieving message");
                            var POST = qs.parse(body);
                            logMessage(POST.ring, "says", username);
                            res.end(readLog());
                            console.log("Done sending log message reply");
                        });
                    }
                    else if (pathname.search("log_read") > 0) {
                        //console.log("Sending log");
                        res.end(readLog());
                    //console.log("Done sending log");
                    }
                    else {
                        console.log("Unknown POST URL: " + pathname);
                        res.writeHeader(404, { 'Content-Type': 'text/html' });
                        res.end("Error");
                    }
                }
                //We got a GET request!
                if (req.method === "GET") {
                    console.log("In GET handler, PID: " + process.pid);
                    if (pathname.search(".js") > 0) {
                        console.log("Sending ./" + pathname);
                        res.setHeader("Content-Type", "text/javascript");
                        res.end(fs.readFileSync("./" + pathname), 'utf-8');
                        console.log("Done sending ./" + pathname);
                    } else if (pathname.search("css") > 0) {
                        console.log("Sending style.css");
                        // Write out the frame code
                        res.setHeader("Content-Type", "text/css");
                        res.end(fs.readFileSync("./style.css"), 'utf-8');
                        console.log("Done sending style.css");
                    } else if (pathname.search(".html") > 0) {
                        console.log("Sending ./" + pathname);
                        // Write out the frame code
                        res.setHeader("Content-Type", "text/html");
                        res.end(fs.readFileSync("./" + pathname), 'utf-8');
                        console.log("Done sending ./" + pathname);
                    } else if (pathname.search("json_") > 0) {
                        //console.log("Sending JSON for " + pathname);
                        // Write out the frame code
                        if (pathname.search("send") > 0) {
                            var result = dtcdriver.getSendStatistics();
                            result.name = "send";
                            res.end(JSON.stringify(result));
                        }
                        else if (pathname.search("receive") > 0) {
                            var result = dtcdriver.getReceiveStatistics();
                            result.name = "receive";
                            res.end(JSON.stringify(result));
                        }
                        else if (pathname.search("spayload") > 0) {
                            var result = dtcdriver.getSendPayloadStatistics();
                            result.name = "spayload";
                            res.end(JSON.stringify(result));
                        }
                        else if (pathname.search("rpayload") > 0) {
                            var result = dtcdriver.getReceivePayloadStatistics();
                            result.name = "rpayload";
                            res.end(JSON.stringify(result));
                        }
                        else {
                            res.end(JSON.stringify(0));
                        }
                        //console.log("Done sending ./" + pathname);
                        console.log("DTC Error Status: " + dtcdriver.Err);
                    } else {
                        console.log("Sending client.html");
                        // Write out the frame code
                        res.setHeader("Content-Type", "text/html");
                        res.end(fs.readFileSync("./client.html"), 'utf-8');
                        console.log("Done sending client.html");
                    }
                }
            }
        } else {
            res.writeHeader(500, { 'Content-Type': 'text/html' });
            res.end("You need to have a Fermi KCA Certificate to view this page");
        }
    });
    
    var baseport = 8080;
    if (__dirname.search("dev") >= 0) {
        baseport = 9090;
    }
    console.log("Listening on ports " + baseport + " and " + (baseport + process.pid));
    server.listen(baseport).listen(baseport + process.pid);
}
