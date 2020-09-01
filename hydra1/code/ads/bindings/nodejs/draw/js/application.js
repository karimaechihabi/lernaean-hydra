//===================================================================//
//                        Global variables                           //
//===================================================================//

// Screen size
// var canvas_width = 1700;
// var canvas_height = 800;
var canvas_width = 930;
var canvas_height = 450;

// Socket to backend
var socket = io('http://localhost:8080');

// Query answer
var q=0;
// Approximate or not
var approximate = false;

/////////////// QUERY DRAWING /////////////
// Query drawing points
var points_x = [];
var points_y = [];

// Initial query drawing point
var initial_x = 0;
var initial_y = 0;

// True if a query is active
var active_query = false;

// True if query is paused
var paused_query = false;
    
var continuous_querying = false;

//===================================================================//
//                        Utility functions                          //
//===================================================================//

/**
 * Plots a set of data series.
 * @param data the data series in Chart.js acceptable format
 * @param scaleShowDataLabels [true/false] show labels on y-axis
 */
function plotData(data, scaleShowLabels) {
    Chart.types.Line.extend({
         name : "AltLine",
         initialize : function(data) {
            Chart.types.Line.prototype.initialize.apply(this, arguments);
            this.scale.draw = function() {
               if (this.display && (this.xLabelRotation > 90)) {
                  this.endPoint = this.height - 5;
               }
               Chart.Scale.prototype.draw.apply(this, arguments);
            };
         }
    });
    
    // get line chart canvas
    var cnv = document.getElementById('dataseries').getContext('2d');

    // draw line chart
    scaleShowLabels = typeof scaleShowLabels !== 'undefined' ? scaleShowLabels : false;
    var crt = new Chart(cnv).AltLine(data, { animationSteps: 6, 
                                             scaleShowLabels: scaleShowLabels,
                                             scaleShowLines: false,
                                             scaleShowGridLines : false, 
                                             pointDot : false, 
                                             showTooltips: false, 
                                             legendTemplate : "<% for (var i=0; i<datasets.length; i++){%><span style=\"background-color:<%=datasets[i].strokeColor%>; padding-left: 1em; margin-left: 0.5em; margin-right: 0.2em\"></span><%if(datasets[i].label){%><span><%=datasets[i].label%></span><%}%><%}%>"

                                     });

    $("#legend").html(crt.generateLegend());
}

/**
 * Plots a set of data series.
 * @param data the data series in Chart.js acceptable format
 * @param plotDataApproximate true/false array for each point if the query was approximate
 * @param scaleShowDataLabels [true/false] show labels on y-axis
 */
function plotHistoryData(element, data, plotDataApproximate, scaleShowLabels) {
    // get line chart canvas
    var cnv = document.getElementById(element).getContext('2d');
    var canvas = $("canvas#"+element)[0];
    canvas.width  = 200;
    canvas.height = 100;

    // draw line chart
    scaleShowLabels = typeof scaleShowLabels !== 'undefined' ? scaleShowLabels : false;
    var crt = new Chart(cnv).Line(data, { animationSteps: 6, 
                                         scaleShowLabels: scaleShowLabels,
                                         scaleShowGridLines : false, 
                                         pointDot : false, 
                                         showTooltips: false});
    for(var i=0; i<crt.datasets[0].points.length; i++) {
        if(plotDataApproximate[i]) {
            crt.datasets[0].points[i].fillColor =   "rgba(000,111,111,30)" ; 
        }
        else {
            crt.datasets[0].points[i].fillColor =   "rgba(111,000,000,99)" ; 
        }
    }
}

/**
 * Identifies the X, Y position of the mouse in the canvas element.
 * @param mouseEvent the mouse event.
 * @param element the element on which the event was performed.
 */
function getPosition(mouseEvent, element) {
    var x, y;
    if (mouseEvent.pageX != undefined && mouseEvent.pageY != undefined) {
        x = mouseEvent.pageX;
        y = mouseEvent.pageY;
    } else {
        x = mouseEvent.clientX + document.body.scrollLeft + document.documentElement.scrollLeft;
        y = mouseEvent.clientY + document.body.scrollTop + document.documentElement.scrollTop;
    }
    return { X: x - 130, Y: y - 130};
}

/**
 * Draws a line to the x and y coordinates of the mouse event inside
 * the specified element using the specified context.
 * @param mouseEvent the mouse event performed.
 * @param element the element on which the event was performed.
 * @param context the context.  
 */
function drawLine(mouseEvent, element, context) {
    if(paused_query) {
        return;
    }

    var position = getPosition(event, element);
    var xpos = initial_x + (points_x.length + 1) * 3.5
    var ypos = position.Y;
    
    context.lineTo(xpos, ypos);
    
    points_x.push(xpos);
    points_y.push(ypos);
    
    context.stroke();
    
    if(points_y.length > 256) {
        active_query = false;
        context.closePath();
       // unbind any events which could draw
        $(element).unbind("mousemove").unbind("mouseup").unbind("mouseout");
        var ts = [];
        for(var i=0; i<256; i+=1) {
            ts.push(points_y[i]);
        }
        q = z_normalize(ts);
        socket.emit('query', { ts:  q});
   }   
}

/**
 * Makes the canvas occupy 100% of the available screen space.
 */
function makeCanvasFullScreen() {
    var canvas = $("canvas#dataseries")[0];
    canvas.width  = canvas_width;
    canvas.height = canvas_height;
    $("#content").hide();
}

/**
 * Updates the gauge with the specified id.
 * @param gaugeId the id of the canvas element.
 * @param value the current value.
 * @param maxValue the maximum value.
 */
function updateGauge(gaugeId, currentValue, maxValue) {
    var opts = {
      lines: 12, // The number of lines to draw
      angle: 0.15, // The length of each line
      lineWidth: 0.44, // The line thickness
      pointer: {
        length: 0.9, // The radius of the inner circle
        strokeWidth: 0.035, // The rotation offset
        color: '#000000' // Fill color
      },
      limitMax: 'false',   // If true, the pointer will not go past the end of the gauge
      colorStart: '#6FADCF',   // Colors
      colorStop: '#8FC0DA',    // just experiment with them
      strokeColor: '#E0E0E0',   // to see which ones work best for you
      generateGradient: true
    };
    var target = document.getElementById(gaugeId); // your canvas element
    var gauge = new Gauge(target).setOptions(opts); // create sexy gauge!
    gauge.maxValue = maxValue; // set max gauge value
    gauge.animationSpeed = 1; // set animation speed (32 is default value)
    gauge.set(currentValue); // set actual value
}


//===================================================================//
//                      Socket.IO event hooks                        //
//===================================================================//

/**
 * Triggered when the client is connected to the backend.
 * It just logs a message.
 */
socket.on('news', function (data) {
    console.log("Connected.");
});

/**
 * Triggered when a query answer is received from the backend.
 * It prepares the data structure and plots the data.
 */
socket.on('query_answer', function(data) {
    query_approximate = data.approximate;
    data = data.answer;

    var answer = new Array(256);
    var query = new Array(256);
    var labels = new Array(256);

    for(var i=0; i<256; i++) {
       labels[i] = "p"+i-1;
       answer[i] = data[i];
       query[i] = q[i];
    }

    
    data = {
        labels: labels,
        datasets: [ 
            {
                label: "Query",
                fillColor: "rgba(220,220,220,0.2)",
                strokeColor: "rgba(220,220,220,1)",
                pointColor: "rgba(220,220,220,1)",
                pointStrokeColor: "#fff",
                pointHighlightFill: "#fff",
                pointHighlightStroke: "rgba(220,220,220,1)",
                data: query
            },
            {
                label: "Answer",
                fillColor: "rgba(151,187,205,0.2)",
                strokeColor: "rgba(151,187,205,1)",
                pointColor: "rgba(151,187,205,1)",
                pointStrokeColor: "#fff",
                pointHighlightFill: "#fff",
                pointHighlightStroke: "rgba(151,187,205,1)",
                data: answer
            }
        ]
    };

    plotData(data); 
}); 
/**
 * It is triggered when a history answer is received from the backend.
 */
socket.on('exact_history_answer', function(data) {
    plotDataLabels = [];
    plotDataTimes = [];
    plotDataApproximate = [];
    for(var i=0; i<data.length; i++) {
        plotDataLabels.push("");
        plotDataTimes.push(data[i].time);
        plotDataApproximate.push(data[i].approximate);
    }

    plot_data = {
        labels: plotDataLabels,
        datasets: [ 
            {
                label: "Times",
                fillColor: "rgba(151,187,205,0.2)",
                strokeColor: "rgba(151,187,205,1)",
                pointColor: "rgba(151,187,205,1)",
                pointStrokeColor: "#fff",
                pointHighlightFill: "#fff",
                pointHighlightStroke: "rgba(151,187,205,1)",
                data: plotDataTimes
            }
        ]
    };

    plotHistoryData("exact_history", plot_data, plotDataApproximate, true);
});

/**
 * It is triggered when a history answer is received from the backend.
 */
socket.on('approximate_history_answer', function(data) {
    plotDataLabels = [];
    plotDataTimes = [];
    plotDataApproximate = [];
    for(var i=0; i<data.length; i++) {
        plotDataLabels.push("");
        plotDataTimes.push(data[i].time);
        plotDataApproximate.push(data[i].approximate);
    }

    plot_data = {
        labels: plotDataLabels,
        datasets: [ 
            {
                label: "Times",
                fillColor: "rgba(151,187,205,0.2)",
                strokeColor: "rgba(151,187,205,1)",
                pointColor: "rgba(151,187,205,1)",
                pointStrokeColor: "#fff",
                pointHighlightFill: "#fff",
                pointHighlightStroke: "rgba(151,187,205,1)",
                data: plotDataTimes
            }
        ]
    };
    plotHistoryData("approximate_history", plot_data, plotDataApproximate, true);
});

/**
 * It is triggered when a stats answer is received from the backend.
 */
socket.on('meminfo', function(data) {
    var percentage = data.loaded_records*100 / data.total_records;
    var bytes = (data.total_records * 1024)/(1024*1024);//*1024);
    $('#datasetSize').html(Math.round(bytes) + "Mb");
    //updateGauge("gauge-ads", data.averageTime, 25);
    updateGauge("loadedprogress", percentage, 100);
    updateGauge("gauge-isax", 0, 25);
    updateGauge("gauge-serial", 0, 25);

    var adsData = [
        {
            value: Math.round((data.mem_tree_structure/1024)/1024),
            color:"#F7464A",
            highlight: "#FF5A5E",
            label: "Index"
        },
        {
            value: Math.round((data.mem_summaries/1024)/1024),
            color: "#46BFBD",
            highlight: "#5AD3D1",
            label: "Summaries"
        },
        {
            value: Math.round((data.mem_data/1024)/1024),
            color: "#FDB45C",
            highlight: "#FFC870",
            label: "Full Data"
        }
    ];

    var diskDATA = [
        {
            value: Math.round((data.disk_data_full/1024)/1024),
            color: "#949FB1",
            highlight: "#A8B3C5",
            label: "Full Data"
        },
        {
            value: Math.round((data.disk_data_partial/1024)/1024),
            color: "#4D5360",
            highlight: "#616774",
            label: "Summaries"
        }
    ];

    //isaxData = data;

    var ctxADS = document.getElementById("ADS").getContext("2d");
    var ctxiSAX = document.getElementById("iSAX").getContext("2d");
    
    var adsChart = new Chart(ctxADS).PolarArea(adsData, {animationSteps: 1});
    var isaxChart = new Chart(ctxiSAX).PolarArea(diskDATA, {animationSteps: 1});
      
});

/**
 * It is triggered when the list of available files comes from the backend.
 */
socket.on('available_files', function(data) {
    $("#datasetOpenModal div.list-group").html("");
    for(var dataset in data) {
        $("#datasetOpenModal div.list-group").append('<a href="#" class="list-group-item"><span class="glyphicon glyphicon-file" style="margin-right:0.5em;"></span>'+
                data[dataset]+
            '</a>')
    }
});



//===================================================================//
//                         UI command hooks                          //
//===================================================================//

function action_History() {
    // Show content, hide toolbar and reduce height of canvas
    $("#content").hide();
    $('#query-buttons').hide();
    $('#dataseries-buttons').hide();
    $('#approximate-buttons').hide();

    
    $("#toolbar").show();
    $("canvas#dataseries").show();
    makeCanvasFullScreen();

    // Send history command
    socket.emit('history', {});
}

function action_Explore() {
    // Hide content, show toolbar and make canvas full screen
    $("#content").hide();
    $("#toolbar").show();
    $("canvas#dataseries").show();
    $('#approximate-buttons').show();
    
    $('#query-buttons').show();
    $('#dataseries-buttons').show();

    makeCanvasFullScreen();
}

function cmd_sendRandomQuery() {
    q = randomTs();
    socket.emit('query', { ts:  q, approximate: approximate});
}

function cmd_rerunQuery() {
    socket.emit('query', { ts:  q, approximate: approximate});
}

function action_Load() {
    socket.emit('files', {});
}

function action_Memory() {
    socket.emit('memory_utilization');
}

function cmd_approximate() {
    approximate = $(this).prop('checked');
}

function cmd_playButton() {
    continuous_querying = !continuous_querying;
    alert("test");
    socket.emit('set_continuous_querying', {continuous: continuous_querying});
}

function cmd_draw (mouseEvent) {
    if(active_query) {
        // Unpause query if it is paused
        paused_query = false;
        return;
    }
    else {
        active_query = true;
    }

    var element = document.getElementById("dataseries");
    var context = element.getContext("2d");

    context.beginPath();
    points_x = [];
    points_y = [];
    var position = getPosition(event, element);
    initial_x = 0; //position.X;
    initial_y = position.Y;
    
    // attach event handlers
    $(this).mousemove(function (mouseEvent) {
        drawLine(mouseEvent, element, context);
    }).mouseup(function (mouseEvent) {
        //finishDrawing(mouseEvent, element, context);
        paused_query = true;
    }).mouseout(function (mouseEvent) {
        finishDrawing(mouseEvent, element, context);
    });
}


//===================================================================//
//                           Main function                           //
//===================================================================//

$( document ).ready(function() {
    // ---- Bind actions to UI elements ---- //

    // Canvas
    $("#dataseries").mousedown(cmd_draw);

    // Side bar
    $("#action_Explore").click(action_Explore);
    $("#action_History").click(action_History);
    
    // Toolbar
    $("#cmd_sendRandomQuery").click(cmd_sendRandomQuery);
    $("#cmd_rerunQuery").click(cmd_rerunQuery);
    $("#cmd_approximate").change(cmd_approximate);
    $("#cmd_playButton").click(cmd_playButton);
   
    // File Menu
    $("#action_Load").click(action_Load);
    $("#action_Memory").click(action_Memory);


    // ---- Initialization ---- //

    // Make canvas full screen
    makeCanvasFullScreen();

    // Start with a random query
    action_Explore();
    cmd_sendRandomQuery();
});



