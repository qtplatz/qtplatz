//
// events.js
// Copyright (C) 2013-2016 MS-Cheminformatics LLC
//
var source = new EventSource( '/ev/ctl?events' );

source.addEventListener( 'tick', function( e ) {
    var ev = document.getElementById( 'tick' );
    ev.innerHTML = e.data;
}, false );

source.addEventListener( 'method', function( e ) {
    var obj = JSON.parse( e.data );
    updateMethod( obj );
}, false );

source.addEventListener( 'sequence', function( e ) {
    var obj = JSON.parse( e.data );
    updateSequence( obj );
}, false );

source.onmessage = function(e) {
    console.log( e );
    var ev = document.getElementById( 'status' )
    if ( ev )
	ev.innerHTML = e.data
};

source.onerror = function( e ) {
    console.log( e )
};

source.onopen = function( e ) {
    var ev = document.getElementById('status')
    if ( ev )
	ev.innerHTML = "open";
}
