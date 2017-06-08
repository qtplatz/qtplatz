function onValue(value,id) {

    var xmlhttp=new XMLHttpRequest();

    xmlhttp.onreadystatechange=function() {
	if (xmlhttp.readyState==4 && xmlhttp.status==200) {
	    document.getElementById("txtHint").innerHTML=xmlhttp.responseText;
	}
    }

    xmlhttp.open("GET","ctl?set="+JSON.stringify( { setpts:[{id, value}] } ),true);
    xmlhttp.send();
}

function onEdit( elm ) {

    var xmlhttp=new XMLHttpRequest();

    xmlhttp.onreadystatechange=function() {
	if (xmlhttp.readyState==4 && xmlhttp.status==200) {
	    document.getElementById("txtHint").innerHTML=xmlhttp.responseText;
	}
    }

    // see 'index.html', such as '<div class="col-sm-4" id="p[0]" data-index="0"></div>'

    var proto = $(elm).parent().closest('div').attr( 'data-index' );    
    var id = elm.id;
    var value = elm.value;

    console.log( elm.closest('div') );
    console.log( "proto: " + proto );

    xmlhttp.open("GET","ctl?set="+JSON.stringify( { setpts:[{proto, id, value}] } ),true);
    xmlhttp.send();
}

function onChecked( elm ) {

    var xmlhttp=new XMLHttpRequest();

    xmlhttp.onreadystatechange=function() {
	if (xmlhttp.readyState==4 && xmlhttp.status==200) {
	    document.getElementById("txtHint").innerHTML=xmlhttp.responseText;
	}
    }

    var proto = $(elm).parent().closest('div').attr( 'data-index' );
    var id = elm.id;
    var value = elm.checked;

    console.log( elm.closest('div') );
    console.log( "proto: " + proto );

    xmlhttp.open("GET","ctl?set="+JSON.stringify( { setpts:[{proto, id, value}] } ),true);
    xmlhttp.send();
}
