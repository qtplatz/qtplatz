//
// sequence.js
// Copyright (C) 2013-2016 MS-Cheminformatics LLC
//
function updateSequence( data ) {
    $('#sequence').jqGrid('clearGridData');
    var samples = data.sequence.samples;
    for ( var i = 0; i < samples.length; i++ )
	jQuery( '#sequence' ).jqGrid( 'addRowData', i+1, samples[i] )
    $('#sequence').trigger("reloadGrid", [{current: true}]);    
}

function fetchSequence() {
    var xmlhttp = new XMLHttpRequest();

    xmlhttp.onreadystatechange = function() {
	if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
	    var data = JSON.parse( xmlhttp.responseText );
	    updateSequence( data );
	}
    }
    
    xmlhttp.open("GET","/ev/ctl?sequence?",true);
    xmlhttp.send();
}

function commitSequence( data ){
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.open( "POST", "/ev/ctl?sequence.commit=" + JSON.stringify( data ), true );
    xmlhttp.send();
}

var lastSel;

jQuery("#sequence").jqGrid({
    datatype: "local"
    , height: 'auto'
    , width: 'auto'
    , pager: "#sequencePager"
    , pginput: false
    , pgbuttons: false
    , viewrecords: false
    , colNames:['#', 'Vial#', 'Method', 'Run Length(s)', 'Inj.Vol', 'Description' ]
    , colModel:[
   	{name:'id', index:'id',  width:40, align:"center", sorttype:"int" }
	, {name:'sampleid',   index:'sampleid',   width:80, aligh:"center", sorttype:"string", editable:true }
	, {name:'methodid',   index:'methodid',   width:80, aligh:"center", sorttype:"string", editable:false }
	, {name:'runlength',  index:'runlength',  width:80, aligh:"right",  sorttype:"float", editable:true
	   , formatter:'number' }
	, {name:'injvolume',  index:'injvolume',  width:80, aligh:"right",  sorttype:"float", editable:true
	   , formatter:'number' }
	, {name:'description',index:'description', width:80, aligh:"right",  editable:true, edittype:'textarea' }
    ]
    , viewrecords: true
    , onSelectRow: function(id){
	if( id && id!==lastSel ){
	    if ( typeof lastSel !== "undefined" )
		jQuery('#sequence').jqGrid('saveRow', lastSel );
	    lastSel=id;
	}
	jQuery('#sequence').jqGrid( 'editRow', id, true );
    }
    , caption: "Timed Event"
});

jQuery("#sequence").jqGrid('navGrid',"#sequencePager",{edit:false,add:false,del:false});

jQuery("#sequence").jqGrid('inlineNav',"#sequencePager", {
    addParams: {
        position: "beforeSelected"
        , useDefValues: true
        , addRowParams: {
            keys: true
            //oneditfunc : onInlineEdit
        }
    }
    , editParams: {
        keys: true
        //oneditfunc: onInlineEdit
    }
    , add: true
    , edit: false
    , save: true
    , cancel: true    
});

////////////////////////////

$(function() {
    $( "[id^=submit]" ).on( 'click', function( e ) {
	console.log( "submit: " + this.id )
	var data = $( '#sequence' ).jqGrid( 'getGridParam', 'data' );
	var sequence = { sequence: { samples:data } };
	console.log( JSON.stringify( sequence ) );
	commitSequence( sequence );  // commit web-page data to server (dgmod)
    });
    
    $( "[id^=fsm]" ).change(function() {
	console.log( "fsm: " + this.id + ", " + this.checked )
    });
});

