function updateConfig( data ) {
    for ( var i = 0; i < data.length; i++ ) {
	var d = { id:data[i].id
		  , enable:data[i].enable
		  , name:data[i].name
		  , mode:data[i].mode
		  , initState:data[i].initState
		  , note:data[i].note };
        jQuery("#config").jqGrid( 'addRowData', i+1, d );
    }
}

function fetchConfig() {
    var xmlhttp;
    if (window.XMLHttpRequest) {   // code for IE7+, Firefox, Chrome, Opera, Safari
	xmlhttp = new XMLHttpRequest();
    }  else  {
	xmlhttp = new ActiveXObject("Microsoft.XMLHTTP"); // code for IE6, IE5
    }

    xmlhttp.onreadystatechange = function() {
	if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
	    var data = JSON.parse( xmlhttp.responseText );
	    updateConfig( data.ioConfig );
	}
    }
    
    xmlhttp.open("GET","/ev/ctl?config.json",true);
    xmlhttp.send();
}

function commitConfig( id, data ){
    console.log( "commit data: " + JSON.stringify( data ) );
    
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.open( "GET", "ctl?config.commit=" + JSON.stringify( data ), true );
    xmlhttp.send();
}


var lastSel;

jQuery("#config").jqGrid({
    datatype: "local"
    , height: 250
    , pager: '#pgconfig'
    , colNames:['#', 'Enable', 'Name', 'IO mode', 'Initial state', 'Notes']
    , colModel:[
   	{name:'id',        index:'id',        width:40, align:"center", sorttype:"int" }
	, {name:'enable',    index:'enable',    width:40, template:"booleanCheckbox", editable:true, firstsortorder:"desc" }
   	, {name:'name',      index:'name',      width:90, sorttype:"string" }
   	, {name:'mode',      index:'mode',      width:60, align:"center", formatter: "select"
	   , formatoptions: { value: "0:OUT; 1:IN" } }
   	, {name:'initState', index:'initState', width:80, align:"center", formatter: "select"
	   , formatoptions: { value: "0:High; 1:Low" }, editable:true, edittype:"select", editoptions:{value:"0:High;1:Low"} }
   	, {name:'note',      index:'note',      width:150, sortable:false, editable:true }
    ]
    , viewrecords: true
    , onSelectRow: function(id){
	console.log( "onSelectRow: " + id + " lastSel=" + lastSel ); 
	if(id && id!==lastSel){
	    if ( typeof lastSel !== "undefined" ) {
		jQuery('#config').jqGrid('saveRow', lastSel);
		commitConfig( id, jQuery('#config').jqGrid( 'getRowData', lastSel ) );
	    }
	    lastSel=id;
	}
	jQuery('#config').jqGrid( 'editRow', id
				  , { keys: true
				      , aftersavefunc: function() {
					  commitConfig( id, jQuery('#config').jqGrid( 'getRowData', id ) );
				      }
				    });
    }
    , caption: "IO Configuration"
});


