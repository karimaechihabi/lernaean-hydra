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

var addon = require('./build/Release/addon');

var index = new addon.jsADS("myexperiment/");
index.printInfo();
console.log(index.getTotalRecords());
console.log(index.getLoadedRecords());

for(var i=0; i<1000; i++) {
var ts=randomTs();
console.log(index.query(ts));
console.log(index.approximateQuery(ts));
console.log(ts);
}
console.log(index.getMemoryUtilizationInfo());
index.close();
