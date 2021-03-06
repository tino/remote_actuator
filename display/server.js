// Generated by CoffeeScript 1.4.0
(function() {
  var app, express, fs, http, io, port, serial, serialListener, serialport, server;

  fs = require('fs');

  express = require('express');

  http = require('http');

  app = express();

  server = http.createServer(app);

  io = require('socket.io').listen(server);

  port = 8080;

  server.listen(port, '127.0.0.1');

  serialport = require("serialport");

  serialListener = function(io) {
    var serial;
    serial = new serialport.SerialPort("/dev/tty.usbserial-A9014F5C", {
      baudrate: 19200,
      dataBits: 8,
      parity: 'none',
      stopBits: 1,
      flowControl: false,
      parser: serialport.parsers.readline("\n", "binary")
    });
    serial.on("open", function() {
      return console.log("serial connection open");
    });
    serial.on("data", function(data) {
      var len, message, values;
      console.log(data);
      message = data;
      len = message.length;
      if (message.slice(0, 2) === '<<') {
        values = message.slice(2, len - 2).split(':');
        return io.sockets.emit(values[1], [values[2], values[3]]);
      } else {
        return console.log("Cant understand:" + message.slice(0, 2) + "." + message.slice(len - 3, +len + 1 || 9e9));
      }
    });
    return serial;
  };

  app.use(express["static"](__dirname + '/public'));

  app.get('/', function(req, res) {
    return res.sendfile(__dirname + '/index.html');
  });

  serial = serialListener(io);

  io.sockets.on('connection', function(socket) {
    socket.emit('live', 1);
    socket.on('move to', function(data) {
      var buf;
      console.log("move to " + data);
      buf = new Buffer('AFAZMxxxFA');
      buf.writeUInt8(data % 128, 6);
      buf.writeUInt8(data >> 7, 7);
      console.log(buf);
      return serial.write(buf);
    });
    return socket.on('stop', function(data) {
      var buf;
      console.log("stopping");
      buf = new Buffer('AFAZSxxxFA');
      return serial.write(buf);
    });
  });

}).call(this);
