
function updateProtocol( _this, protocol ) {

    if ( protocol.replicates == 0 ) {
	$(_this).find( ':input[id=ENABLE]' ).each( function(){ this.checked = false; } );
    } else {
	$(_this).find( ':input[id=ENABLE]' ).each( function(){ this.checked = true; } );
	$(_this).find( ':input[id=REPLICATES]' ).each( function(){ this.value = protocol.replicates; } );
    }

    $(_this).find( ':input[id^=PULSE\\.DELAY]' ).each( function( i ){
	this.value = protocol.pulses[ i ].delay;
    });
    
    $(_this).find( ':input[id^=PULSE\\.WIDTH]' ).each( function( i ){
	this.value = protocol.pulses[ i ].width;
    });
}

function disableProtocol( _this ) {
    $(_this).find( ':input[id=ENABLE]' ).each( function(){ this.checked = false; } );
}

function updateProtocols( protocols ) {

    $('div #interval').find( ':input' ).each( function(){ // T_0
	this.value = protocols.interval;
    });

    $(protocols.protocol).each( function( index ){
	updateProtocol( $('div #p\\[' + index + '\\]')[0], this );
    });

    for ( i = protocols.protocol.length; i < 3; ++i ) {
	disableProtocol( $('div #p\\[' + i + '\\]')[0] );
    }
}

function fetchStatus() {
    var xmlhttp;
    if (window.XMLHttpRequest) {   // code for IE7+, Firefox, Chrome, Opera, Safari
	xmlhttp = new XMLHttpRequest();
    }  else  {
	xmlhttp = new ActiveXObject("Microsoft.XMLHTTP"); // code for IE6, IE5
    }

    xmlhttp.onreadystatechange = function() {

	if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
	    var data = JSON.parse( xmlhttp.responseText );
	    updateProtocols( data.protocols );
	}
    }
    
    xmlhttp.open("GET","/dg/ctl?status.json",true);
    xmlhttp.send();
}

