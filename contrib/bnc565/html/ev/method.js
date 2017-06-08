//
// method.js
// Copyright (C) 2013-2016 MS-Cheminformatics LLC
//
function updateMethod( data ) {
    $('#prepare').jqGrid('clearGridData');
    var prep = data.method.prepare;
    for ( var i = 0; i < prep.length; i++ ) {
	var d = { pid:prep[i].pid, action:prep[i].action };
        $('#prepare').jqGrid( 'addRowData', prep[i].id, d );
    }
    $('#prepare').trigger("reloadGrid", [{current: true}]);    

    var table = data.method.table;
    $('#method').jqGrid('clearGridData');
    for ( var i = 0; i < table.length; i++ ) {
	var d = { id:i+1, elapsed_time:table[i].elapsed_time, pid:table[i].pid, action:table[i].action };
	$('#method').jqGrid( 'addRowData', i+1, d );
	// console.log( 'updateMethod i=' + i + JSON.stringify( d ) );
    }
    $('#method').trigger('reloadGrid', [{current: true}]);    
}

function fetchMethod() {
    var xmlhttp;
    if (window.XMLHttpRequest) {   // code for IE7+, Firefox, Chrome, Opera, Safari
	xmlhttp = new XMLHttpRequest();
    }  else  {
	xmlhttp = new ActiveXObject("Microsoft.XMLHTTP"); // code for IE6, IE5
    }

    xmlhttp.onreadystatechange = function() {
	if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
	    var data = JSON.parse( xmlhttp.responseText );
	    updateMethod( data );
	}
    }
    
    xmlhttp.open("GET","/ev/ctl?method?",true);
    xmlhttp.send();
}

function commitMethod( data ){
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.open( "POST", "ctl?method.commit=" + JSON.stringify( data ), true );
    xmlhttp.send();
}

var lastSel1 = -1;

jQuery("#prepare").jqGrid({
    datatype: "local"
    , height: 'auto'
    , colNames:['pin#', 'Action' ]
    , colModel:[
   	{ name:'pid',        index:'pid',    key:true,  width:40, align:"center", editable:false, sorttype:"int" }
	, { name:'action',   index:'action', width:200, aligh:"center", sorttype:"int", editable:true
	    , formatter:"select", formatoptions:{ value: "0:High; 1:Low" }
	    , edittype:"select", editoptions:{ value:"0:High; 1:Low" } }
    ]
    , viewrecords: true
    , onSelectRow: function(id){
	if(id && id!==lastSel1){
	    console.log( "select: " + id + " : " + JSON.stringify( jQuery( '#prepare' ).jqGrid( 'getRowData', id ) ) );
	    //jQuery('#prepare').jqGrid('restoreRow', lastSel1 );
	    if ( typeof lastSel1 !== "undefined" ) {
		jQuery('#prepare').jqGrid('saveRow', lastSel1 );
	    }
	    lastSel1=id;
	}
	jQuery('#prepare').jqGrid( 'editRow', id, { keys:true  });
    }
    , caption: "Preparation"
});

/**/
var functions   = {  '1':  'in#1', '2':  'in#2', '3':  'in#3',  '4':  'in#4',   '5':   'in#5',  '6': 'in#6'
		    ,'7': 'out#7', '8': 'out#8', '9': 'out#9', '10': 'out#10', '11': 'out#11', '12': 'out#12'
		  },
    actions     = { '1': 'High', '2': 'Low', '3': 'Pulse', '4': 'Wait', '5': 'Ignore' },
    actionsOUT  = { '1': 'High', '2': 'Low', '3': 'Pulse' },
    actionsIN   = { '4': 'Wait', '5': 'Ignore' },
    // the next maps contries by ids to actions
    actiosOfFunction = { '': actions
			 , '1': actionsIN, '2': actionsIN, '3': actionsIN, '4': actionsIN
			 , '5': actionsIN, '6': actionsIN
			 , '7': actionsOUT, '8': actionsOUT, '9': actionsOUT, '10': actionsOUT
			 , '11': actionsOUT, '12': actionsOUT
		       },
    lastSel = -1,
    grid = $("#method"),
    resetActionsValues = function () {
        // set 'value' property of the editoptions to initial action
        grid.jqGrid('setColProp', 'action', { editoptions: { value: actions} });
    },
    setActionValues = function (functionId) {
        // to have short list of options which corresponds to the function
        // from the row we have to change temporary the column property
        grid.jqGrid('setColProp', 'action', { editoptions: { value: actiosOfFunction[functionId]} });
    },
    changeActionSelect = function (functionId, functionElem) {
        // build 'action' options based on the selected 'pid' value
        var actionId, actionselect, parentWidth, $row,
            $functionElem = $(functionElem),
            sc = actiosOfFunction[functionId],
            newOptions = '';

        for (actionId in sc) {
            if (sc.hasOwnProperty(actionId)) {
                newOptions += '<option role="option" value="' + actionId + '">' +
                    actions[actionId] + '</option>';
            }
        }

        setActionValues(functionId);
	
        // populate the subset of functions
	if ($functionElem.is('.FormElement')) {
            // form editing
            $functionElem.closest('form.FormGrid').find("select#action.FormElement").html(newOptions);
        } else {
            // inline editing
            $row = $functionElem.closest('tr.jqgrow');
            $("select#" + $.jgrid.jqID($row.attr('id')) + "_action").html(newOptions);
        }
    },
    editGridRowOptions = {
        recreateForm: true,
        onclickPgButtons: function (whichButton, $form, rowid) {
            var $row = $('#' + $.jgrid.jqID(rowid)), functionId;
            if (whichButton === 'next') {
                $row = $row.next();
            } else if (whichButton === 'prev') {
                $row = $row.prev();
            }
            if ($row.length > 0) {
                functionId = grid.jqGrid('getCell', $row.attr('id'), 'pid');
                changeActionSelect(functionId, $("#pid")[0]);
            }
        },
        onClose: function () {
            resetActionsValues();
        }
    };

grid.jqGrid({
    datatype: 'local',
    colModel: [
        { name: 'elapsed_time', width: 200, sorttype:"float", editable: true, formatter:'number' },
        { name: 'pid', width: 140, editable: true, formatter: 'select', stype: 'select', edittype: 'select',
          editoptions: {
              value: functions,
              dataInit: function (elem) { setActionValues($(elem).val()); },
              dataEvents: [
                  { type: 'change', fn: function (e) { changeActionSelect($(e.target).val(), e.target); } }
                  , { type: 'keyup', fn: function (e) { $(e.target).trigger('change'); } }
		  , { type: 'click', fn: function (e) { $(e.target).trigger('change'); } }
              ]
          }},
        { name: 'action', width: 140, formatter: 'select', stype: 'select'
          , editable: true, edittype: 'select', editoptions: { value: actions }
	}
    ],
    onSelectRow: function (id) {
        if (id && id !== lastSel) {
            if (lastSel !== -1) {
                $(this).jqGrid('restoreRow', lastSel);
                resetActionsValues();
            }
            lastSel = id;
        }
    },
    ondblClickRow: function (id) {
        if (id && id !== lastSel) {
            $(this).jqGrid('restoreRow', lastSel);
            lastSel = id;
        }
        resetActionsValues();
	var functionId = $(this).jqGrid('getCell', id, "pid");
	//changeActionSelect(functionId, e);
	//console.log( "ondblClickRow id=" + id + ", fid=" + functionId + "; " + JSON.stringify( $("#id") ) );

        $(this).jqGrid('editRow', id, {
            keys: true,
            aftersavefunc: function () {
                resetActionsValues();
            },
            afterrestorefunc: function () {
                resetActionsValues();
            }
        });
        return;
    },
    height: '100%',
    viewrecords: true,
    rownumbers: true,
    sortorder: "desc",
    pager: '#methodPager',
    caption: "Timed Events"
});

jQuery("#method").jqGrid('navGrid', '#methodPager', { del:true }, editGridRowOptions, editGridRowOptions);
//jQuery("#method").jqGrid('navGrid', '#methodPager', { edit:true,add:true,del:true })
/*
jQuery("#method").jqGrid('inlineNav',"#methodPager", {
    addParams: {
        position: "beforeSelected"
        , useDefValues: true
        , addRowParams: {
            keys: true
        }
    }
    , editParams: {
        keys: true
    }
    , add: true
    , edit: false
    , save: true
    , cancel: true
});
*/

/**/

$(function() {
    $( "[id^=submit]" ).on( 'click', function( e ) {
	console.log( "submit: " + this.id )
	var prep = $( '#prepare' ).jqGrid( 'getGridParam', 'data' );
	var table = $( '#method' ).jqGrid( 'getGridParam', 'data' );

	var method = { method: { id:"0", title:"default", prepare:prep, table:table } };
	console.log( JSON.stringify( method ) );
	commitMethod( method );  // commit web-page data to server (dgmod)
    });
    
    $( "[id^=fsm]" ).change(function() {
	console.log( "fsm: " + this.id + ", " + this.checked )
    });
});
