function loadActuals()
{
    var xmlhttp;
    if (window.XMLHttpRequest) {   // code for IE7+, Firefox, Chrome, Opera, Safari
	xmlhttp = new XMLHttpRequest();
    }  else  {
	xmlhttp = new ActiveXObject("Microsoft.XMLHTTP"); // code for IE6, IE5
    }

    xmlhttp.onreadystatechange = function()    {

	if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
	    
	    if ( xmldoc = xmlhttp.responseXML ) {
		txt = xmlhttp.statusText;

		pulses = xmldoc.documentElement.getElementsByTagName("pulse");
		table = document.getElementById( "idPulseTable" );
		for ( i = 0; i < pulses.length; ++i ) {
		    delay = table.rows[i + 1].cells[1];
		    width = table.rows[i + 1].cells[2];
		    delay.innerHTML = "<input type=\"text\" id=\"D" + i + "\" onkeyup=\"showHint(this.value, this.id)\" value=\""
		    	+ pulses[i].getAttribute("delay") + "\">";
		    width.innerHTML = "<input type=\"text\" id=\"W" + i + "\" onkeyup=\"showHint(this.value, this.id)\" value=\""
			+ pulses[i].getAttribute("width") + "\">";
		}

		t0 = table.rows[ pulses.length + 1].cells[2];
		
		t0.innerHTML = "<input type=\"text\" id=\"T0\" onkeyup=\"showHint(this.value, this.id)\" value=\""
		    + xmldoc.documentElement.getElementsByTagName("interval")[0].getAttribute("value")
		    + "\">";
		t0.value = xmldoc.documentElement.getElementsByTagName("interval")[0].getAttribute("value");

		document.getElementById("idStatus").innerHTML = txt;
	    }
	}
    }
    xmlhttp.open("GET","/dg/ctl?status",true);
    xmlhttp.send();
}

