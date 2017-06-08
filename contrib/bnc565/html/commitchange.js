function commitChange()
{
    var xmlhttp;
    if (window.XMLHttpRequest)  { // code for IE7+, Firefox, Chrome, Opera, Safari
	xmlhttp=new XMLHttpRequest();
    }
    else { // code for IE6, IE5
	xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.onreadystatechange = function()   {
	if (xmlhttp.readyState == 4 && xmlhttp.status==200) {
	    document.getElementById("idStatus").innerHTML = xmlhttp.responseText;
	}
    }

    table = document.getElementById( "idPulseTable" );

    // To
    interval = table.rows[ 7 ].cells[2].childNodes[0].value;
    
    var doc = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    doc = doc + "<dgcommit version=\"1.0\">";
    doc = doc + "<interval value=\"" + interval + "\" />";
    for ( i = 0; i < 6; ++i ) {
	delay = table.rows[i + 1].cells[1].childNodes[0].value;
	width = table.rows[i + 1].cells[2].childNodes[0].value;
	doc = doc + "<pulse ch=\"" + i + "\" delay=\"" + delay + "\" width=\"" + width + "\" />";
    }
    doc = doc + "</dgcommit>";
    xmlhttp.open("POST","/dg/ctl?commit=" + doc, true);
    xmlhttp.setRequestHeader('Content-Type', 'applicaton/xml');
    xmlhttp.send();
}

function fsm(arg)
{
    var xmlhttp;
    if (window.XMLHttpRequest)  { // code for IE7+, Firefox, Chrome, Opera, Safari
	xmlhttp=new XMLHttpRequest();
    }
    else { // code for IE6, IE5
	xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.onreadystatechange = function()   {
	if (xmlhttp.readyState == 4 && xmlhttp.status==200) {
	    document.getElementById("idStatus").innerHTML = xmlhttp.responseText;
	}
    }
    xmlhttp.open("POST","/dg/ctl?fsm=" + arg, true);
    xmlhttp.send();
}

