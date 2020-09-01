var app = require('http').createServer();
var fs = require('fs');
var io = require('socket.io')(app);
var winston = require('winston');
var net = require('net');

var current_connection = 0;
var client = 0;

function process_output(output) {
  if(current_connection) {
    output = JSON.parse(output);
    console.log("received_event: " + output.event);
    if(output.event == "approximate_answer") {
      approximate_history.push({time: output.time, approximate: true});
      current_connection.emit('query_answer', {answer: output.answer, time: output.time, approximate: true});

      output.event = "meminfo"
      output.answer = 0;
      output.time = 0;
      current_connection.emit('meminfo', output);

      emitHistory(current_connection);
    }
    else if(output.event == "exact_answer") {
      exact_history.push({time: output.time, approximate: false});
      current_connection.emit('query_answer', {answer: output.answer, time: output.time, approximate: false});

      output.event = "meminfo"
      output.answer = 0;
      output.time = 0;
      current_connection.emit('meminfo', output);
      
      emitHistory(current_connection);
    }
    else {
      current_connection.emit(output.event, output);
    }
  }
}

function send_command(command) {
  if(!client) {
    client = net.connect(parseInt(8090),"danae.disi.unitn.eu", function(){
      client.setEncoding('utf8');
      console.log('Connected to ADS server.');
      var output = ""
      client.on('data',function(chunk){
        for(var i=0; i<chunk.length; i++) {
          if(chunk[i] == '\n') {
            process_output(output);
            output = "";
          }
          else {
            output += chunk[i];
          }
        }
      });
      client.on('end',function(){ console.log('server hanged up'); client=0 });
      client.write(command + "\n");
    });
  }
  else {
    client.write(command + "\n");
  }
}



// ========================================================= //
//                     S E T T I N G S                       //
// ========================================================= //
var filesPath = "../../../../code/data/"
var approximate_query_answering = false;
var continuous_querying = false;

winston.remove(winston.transports.Console);
winston.add(winston.transports.Console, {'timestamp':true});
app.listen(8080);

// ========================================================= //
//                       S T A T S                           //
// ========================================================= //
var approximate_history = [];
var exact_history = [];
var totalTime = 0;

// ========================================================= //
//                       I N D E X                           //
// ========================================================= //
var adsIndex = null;
var isaxIndex = null;

// ========================================================= //
//                       C A C H E                           //
// ========================================================= //
var cache_stats = {totalRecords: 0,
                   loadedRecords: 0,
                   averageTime: 0};
var cache_memory_utilization = [0,0,0,0,0];

// ========================================================= //
//                         U T I L S                         //
// ========================================================= //
function randomTs() {
  for (var array=[],i=0;i<256;++i) {
  array[i]=i;
  }
  var tmp, current, top = array.length;
  if(top) while(--top) {
    current = Math.floor(Math.random() * (top + 1));
    tmp = array[current];
    array[current] = array[top];
    array[top] = tmp;
  }
  return z_normalize(array);
}

function standardDeviation(values){
  var avg = average(values);
  
  var squareDiffs = values.map(function(value){
    var diff = value - avg;
    var sqrDiff = diff * diff;
    return sqrDiff;
  });
  
  var avgSquareDiff = average(squareDiffs);
 
  var stdDev = Math.sqrt(avgSquareDiff);
  return stdDev;
}
 
function average(data){
  var sum = data.reduce(function(sum, value){
    return sum + value;
  }, 0);
 
  var avg = sum / data.length;
  return avg;
}

function z_normalize(a) {
  var std = standardDeviation(a);
  var mean = average(a);
  for (var i=0;i<256;++i) {
  a[i] = (a[i] - mean)/std;
  }  
  return a;
}

// ========================================================= //
//                   F U N C T I O N S                       //
// ========================================================= //
// function emitStats(socket) {
//   // TODO: Store averageTime, number of queries in index.
//   socket.emit('stats_answer', cache_stats);
//   winston.info('Stats emitted');
// }

function emitHistory(socket) {
  socket.emit('approximate_history_answer', approximate_history);
  socket.emit('exact_history_answer', exact_history);
  winston.info('History emitted.');
}

function emitFiles(socket) {
  socket.emit('available_files', fs.readdirSync(filesPath).filter(function (x) {return x.match(".*\\.bin")}));
  winston.info("Available files emitted.");
}

function answerQuery(socket, data) {
  send_command("exact " + data.ts.join(" "));
}

function answerApproximateQuery(socket, data) {
  send_command("approximate " + data.ts.join(" "));
}

function getStats(socket, data) {
  send_command("meminfo");
}


// ========================================================= //
//                       W I R I N G                         //
// ========================================================= //
io.on('connection', function (socket) {
  current_connection = socket;
  winston.info('Client connected.');
  send_command("meminfo");
  socket.emit('news', { hello: 'world' });
  socket.on('query', function (data) {
    winston.info('Query received.');
    if(data.approximate) {
      winston.info('Approximate.');
      answerApproximateQuery(socket, data);
    } 
    else {
      winston.info('Exact.');
      answerQuery(socket, data);  
    }
    
  });
  socket.on('history', function (data) {
    winston.info('History query received.');
    emitHistory(socket);
  });
  socket.on('stats', function (data) {
    winston.info('Stats query received.');
    getStats(socket);
  });
  socket.on('files', function (data) {
    winston.info('Files query received.');
    emitFiles(socket);
  });
  socket.on('set_approximate', function(data) {
    approximate_query_answering = data.approximate;
  });
});

process.on( 'SIGINT', function() {
  console.log( "\nGracefully shutting down from SIGINT (Ctrl-C)" );
  process.exit();
})
