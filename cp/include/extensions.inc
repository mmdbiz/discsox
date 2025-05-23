<?php
// -----------------------------------------------------------------------
//         Class: Gateways
//       Purpose: This class controls the settings for the Gateway functionality.
// Last Modified: April 13, 2006
//
// -----------------------------------------------------------------------
class Extensions{
	
	var $debug = false;
	var $help = array();
	//-----------------------------------------------------
	function __construct(){
		$this->loadHelp();
	}
	
	//-----------------------------------------------------
	function Display($detail = false){
		global $_Common;
		global $_DB;
		global $data;
		$sql = "SELECT * FROM extensions ORDER BY class_to_extend";
		
		if($detail && !empty($_REQUEST['extid'])){
			$extid = $_REQUEST['extid'];
			$sql = "SELECT * FROM extensions WHERE extid = '$extid'";
		}
		
		$rs = $_DB->execute($sql);
		while($row = $_DB->fetchrow($rs, "ASSOC")){
			$data[] = $row;
		}
	}
	
	//-----------------------------------------------------
	function Add(){
		global $_Common;
		global $_DB;
		global $fldProperties;
		$this->loadHelp();		
		$fldProperties = $_DB->getFieldProperties("extensions");
	}
	
	//-----------------------------------------------------
	function Insert(){
		global $_Common;
		global $_DB;
		$fldProperties = $_DB->getFieldProperties("extensions");
		list($fields, $values) = $_DB->makeAddFields($fldProperties, "extid", $_REQUEST);
		$sql = "INSERT INTO extensions ($fields) VALUES ($values)";
		$_DB->execute($sql);
		$_REQUEST['extid'] = $_DB->getInsertID('extensions','extid');	
	}
	
	//-----------------------------------------------------
	function Update(){
		global $_Common;
		global $_DB;
		if(!empty($_REQUEST['extid'])){
			$extid = $_REQUEST['extid'];
			$fldProperties = $_DB->getFieldProperties("extensions");
			$values = $_DB->makeUpdateFields($fldProperties,"extid",$_REQUEST);
			$sql = "UPDATE extensions SET $values WHERE extid = '$extid' LIMIT 1";
			$_DB->execute($sql);
		}
	}
	
	//-----------------------------------------------------
	function Delete(){
		global $_Common;
		global $_DB;
		if(!empty($_REQUEST['extid'])){
			$extid = $_REQUEST['extid'];
			$sql = "DELETE FROM extensions WHERE extid = '$extid' LIMIT 1";
			$_DB->execute($sql);
		}
	}	
	
	//-----------------------------------------------------
	function loadHelp(){
		global $_Common;
		global $_DB;
		$sql = "SELECT section_help,`key`,key_help FROM help WHERE section = 'extensions'";
		$helpRecs = $_DB->getRecords($sql);
		foreach($helpRecs as $i=>$flds){
			if(!empty($flds['section_help'])){
				$this->help['section_help'] = $flds['section_help'];
			}
			else{
				$this->help[$flds['key']] = trim($flds['key_help']);
			}
		}	
	}
	
}

?>