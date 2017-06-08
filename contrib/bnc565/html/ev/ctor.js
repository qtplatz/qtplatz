var graphData = [{
        // Visits
        data: [ [6, 1300], [7, 1600], [8, 1900], [9, 2100], [10, 2500], [11, 2200], [12, 2000], [13, 1950], [14, 1900], [15, 2000] ],
        color: '#71c73e'
    }, {
        // Returning Visits
        data: [ [6, 500], [7, 600], [8, 550], [9, 600], [10, 800], [11, 900], [12, 800], [13, 850], [14, 830], [15, 1000] ],
        color: '#77b7c5',
        points: { radius: 4, fillColor: '#77b7c5' }
    }
];

var ctx = document.getElementById('myChart').getContext('2d');
ctx.canvas.width = 300

var myChart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: ['M', 'T', 'W', 'T', 'F', 'S', 'S'],
    datasets: [{
      label: 'apples',
      data: [12, 19, 3, 17, 6, 3, 7],
      backgroundColor: "rgba(153,255,51,0.4)"
    }, {
      label: 'oranges',
      data: [2, 29, 5, 5, 2, 3, 10],
      backgroundColor: "rgba(255,153,0,0.4)"
    }]
  }
});

function loadBanner() {
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function()    {
	if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
	    document.getElementById( "banner" ).innerHTML = xmlhttp.responseText;
	}
    }
    xmlhttp.open("GET", "/ev/ctl?banner", true);
    xmlhttp.send();
}

function downloadOnload() {
    var element = document.createElement("script");
    element.src = "../defer.js";
    document.body.appendChild(element);
    loadBanner();

    fetchConfig()
    fetchMethod()
    fetchSequence()
}

$( function() {
    $( "#tabs" ).tabs();
} );

if ( window.addEventListener )
    window.addEventListener( "load", downloadOnload );
else if ( window.attachEvent)
    window.attachEvent( "load", downloadOnload);
