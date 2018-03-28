var http = require('http');
var SerialPort = require('serialport');

var port = new SerialPort('COM3', {baudRate:9600});
port.write('This is a test');

http.createServer((req, res) => {
	res.writeHead(200, {"Content-Type": "text/plain"});
	res.write("Hello World!");
	res.end();
}).listen(8080);

