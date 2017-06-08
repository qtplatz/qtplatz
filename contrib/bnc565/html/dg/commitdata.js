function findProtocol( json, protoId, elm )
{
    var protoData = {
	index : 0
	, replicates : 1
	, pulses : []
    };
    
    protoData[ "index" ] = protoId;
    protoData[ "replicates" ] = 1;

    var enable = [];

    $(elm).find( ':input[id^=ENABLE]' ).each( function( i ){
	// console.log( "enable: " + this.checked );
	enable.push( this.checked );
    });
    
    $(elm).find( ':input[id^=REPLICATES]' ).each( function( i ){
	// console.log( "replicates: " + this.value );
	protoData[ "replicates" ] = enable[ i ] ? this.value : 0;
    });

    var pulse = { delay : 0.0, width : 0.0 }; 

    $(elm).find( ':input[id^=PULSE]' ).each( function( i ){
	if ( this.id.indexOf( "DELAY" ) >= 0 ) {
	    pulse.delay = this.value;
	} else {
	    pulse.width = this.value;
	    protoData.pulses.push( { "delay" : pulse.delay, "width" : pulse.width } );
	}
	
    });

    // console.log( "prtoData: " + JSON.stringify( protoData ) );    

    json.protocol.push( protoData );
}

function commitData()
{
    var json = {
	protocols : {
	    interval : 1000
	    , protocol : []	    
	}
    };

    var interval = $( '#interval #interval' ).val();
    json.protocols.interval = interval;

    $("div [id^=p\\[]").each( function( index ){
	var protoId = $(this).attr( 'data-index' );
	//console.log( "protocol: " + this.id + "; " + protoId );
	findProtocol( json.protocols, protoId, this );
    });

    // console.log( "commitData: " + JSON.stringify( json ) );
    
    var xmlhttp=new XMLHttpRequest();
    xmlhttp.onreadystatechange=function() {
	if (xmlhttp.readyState==4 && xmlhttp.status==200) {
	    document.getElementById("txtHint").innerHTML=xmlhttp.responseText;
	}
    }

    xmlhttp.open("GET","ctl?commit.json=" + JSON.stringify( json ), true);
    xmlhttp.send();
}
