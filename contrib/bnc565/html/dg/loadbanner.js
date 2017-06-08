function loadBanner()
{
    var xmlhttp;
    if (window.XMLHttpRequest) {   // code for IE7+, Firefox, Chrome, Opera, Safari
	xmlhttp = new XMLHttpRequest();
    }  else  {
	xmlhttp = new ActiveXObject("Microsoft.XMLHTTP"); // code for IE6, IE5
    }

    xmlhttp.onreadystatechange = function()    {

	if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
	    
	    document.getElementById( "banner" ).innerHTML = xmlhttp.responseText;
	    
	}

    }
    xmlhttp.open("GET", "/dg/ctl?banner", true);
    xmlhttp.send();
}

