function showHint(str,id)
{
    if (str.length==0) { 
	document.getElementById("txtHint").innerHTML="";
	return;
    } else {
	var xmlhttp=new XMLHttpRequest();
	xmlhttp.onreadystatechange=function() {
	    if (xmlhttp.readyState==4 && xmlhttp.status==200) {
		document.getElementById("txtHint").innerHTML=xmlhttp.responseText;
	    }
	}
	xmlhttp.open("GET","dg/ctl?q="+str+";"+id,true);
	xmlhttp.send();
    }    
}
