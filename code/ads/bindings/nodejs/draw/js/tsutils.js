function randomTs() {
  array = [];
  array[0] = Math.random();
  for (var i=1;i<256;++i) {
  var plusOrMinus = Math.random() < 0.5 ? -1 : 1;
	 array[i]=array[i-1] + Math.random()*plusOrMinus;
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
	a[i] = (mean-a[i])/std;
  }  
  return a;
}