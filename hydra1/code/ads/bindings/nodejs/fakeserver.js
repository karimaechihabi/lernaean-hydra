var jsads = require('./build/Release/addon');
var app = require('http').createServer();
var fs = require('fs');
var io = require('socket.io')(app);
var winston = require('winston');

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
var history = [];
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
function emitStats(socket) {
  // TODO: Store averageTime, number of queries in index.
  socket.emit('stats_answer', cache_stats);
  winston.info('Stats emitted');
}

function emitHistory(socket) {
  socket.emit('history_answer', history);
  winston.info('History emitted.');
}

function emitFiles(socket) {
  socket.emit('available_files', fs.readdirSync(filesPath).filter(function (x) {return x.match(".*\\.bin")}));
  winston.info("Available files emitted.");
}

function indexFile(socket, data, callback) {
  // TODO: Implement this.
}

function emitMemInfo(socket) {
  socket.emit('memory_utilization', {mem_tree_structure: 10000000, 
                                     mem_summaries: 10000000,
                                     mem_data: 1000000,
                                     disk_data_full: 1000000000,
                                     disk_data_partial: 500000000});
  winston.info('Memory utilization emitted.');
}

function openADSIndex(socket, index_source_file, index_path) {
  if(socket) {
    closeADSIndex(socket);
  }

  //adsIndex = new jsads.jsADS(index_path);

  if(socket) {
    socket.emit("ads_index_opened", {index_source_file: index_source_file});
    emitStats(socket);
  }
}

function closeADSIndex(socket) {
  if(adsIndex) {
    adsIndex.close();
    adsIndex = null;
  }
  socket.emit("ads_index_closed", {});
}

function answerQuery(socket, data, index) {
  if(continuous_querying) {
    return;
  }

  var query = data["ts"];
  
  // === INDEX STRUCTURE ACCESS =================
  var answer = Array(257);
  var tmp = randomTs();
  for(var i=1; i<257; i++) 
    answer[i] = query[i] * 0.7 + tmp[i-1] * 0.3;
  var start = 0;
  var end = 100;
  var time = end - start;
  // ============================================

  socket.emit('query_answer', {answer: answer, approximate: false});
  winston.info('Answer emitted.');
  history.push({time: time, startTime:start, endTime: end, approximate: false});

  // === INDEX STRUCTURE ACCESS ===================================
  cache_stats = {totalRecords: 100000000,
                 loadedRecords: cache_stats.loadedRecords + 5000000,
                 averageTime: 100}

  // ==============================================================
  emitStats(socket);
  emitMemInfo(socket);
  emitHistory(socket);
}

function answerApproximateQuery(socket, data, index) {
  if(continuous_querying) {
    return;
  }

  var query = data["ts"];
  
  // === INDEX STRUCTURE ACCESS =================
  var answer = Array(257);
  var tmp = randomTs();
  for(var i=1; i<257; i++) 
    answer[i] = query[i] * 0.6 + tmp[i-1] * 0.4;
  var start = 0;
  var end = 100;
  var time = end - start;
  // ============================================

  socket.emit('query_answer', {answer: answer, approximate: true});
  winston.info('Answer emitted.');
  history.push({time: time, startTime:start, endTime: end, approximate: true});

  // === INDEX STRUCTURE ACCESS ===================================
  cache_stats = {totalRecords: 100000000,
                 loadedRecords: cache_stats.loadedRecords + 5000000,
                 averageTime: 100}

  // ==============================================================
  emitStats(socket);
  emitMemInfo(socket);
  emitHistory(socket);
}

function startContinuousQuerying(socket, index) {

}

// ========================================================= //
//                       W I R I N G                         //
// ========================================================= //
io.on('connection', function (socket) {
  winston.info('Client connected.');
  socket.emit('news', { hello: 'world' });
  socket.on('query', function (data) {
    winston.info('Query received.');
    if(data.approximate) {
      winston.info('Approximate.');
      answerApproximateQuery(socket, data, adsIndex);
    } 
    else {
      winston.info('Exact.');
      answerQuery(socket, data, adsIndex);  
    }
    
  });
  socket.on('history', function (data) {
    winston.info('History query received.');
    emitHistory(socket);
  });
  socket.on('stats', function (data) {
    winston.info('Stats query received.');
    emitStats(socket);
  });
  socket.on('files', function (data) {
    winston.info('Files query received.');
    emitFiles(socket);
  });
  socket.on('memory_utilization', function(data) {
    winston.info('Memory utilization query received.');
    emitMemInfo(socket);
  });
  socket.on('set_approximate', function(data) {
    approximate_query_answering = data.approximate;
  });
  socket.on('set_continuous_querying', function(data) {
    if(!continuous_querying && data.continuous) {
      continuous_querying = true;
      startContinuousQuerying(socket, adsIndex);
    } else {
      continuous_querying = false;
    }
  });
});


//openADSIndex(null, "data.txt.bin", "sampleindex/");
openADSIndex(null, "data.txt.bin", "myexperiment/");

process.on( 'SIGINT', function() {
  console.log( "\nGracefully shutting down from SIGINT (Ctrl-C)" );
  //adsIndex.close();
  process.exit();
})
