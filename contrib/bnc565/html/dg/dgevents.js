var source = new EventSource( '/dg/ctl?events' );

source.addEventListener( 'tick', function( e ) {
    var ev = document.getElementById( 'tick' );
    ev.innerHTML = e.data;
}, false );

source.onmessage = function(e) {
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
