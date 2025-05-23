<?php
// -----------------------------------------------------------------------
//         Class: Gateways
//       Purpose: This class controls the settings for the Gateway functionality.
// Last Modified: April 13, 2006
//
// -----------------------------------------------------------------------
class Gateway{

    var $debug = false;
	var $help = array();
    //-----------------------------------------------------
	function __construct(){

	}
	
    //-----------------------------------------------------
	function Display($detail = false){
		global $_Common;
		global $_DB;
		global $data;
		$sql = "SELECT DISTINCT * FROM payment_gateways ORDER BY gateway_name";
		
		if($detail && !empty($_REQUEST['pgid'])){

			$pgid = $_REQUEST['pgid'];
			$sql = "SELECT payment_gateways.*,payment_gateway_details.*
					FROM payment_gateways,payment_gateway_details
					WHERE payment_gateways.pgid = '$pgid'
					AND payment_gateway_details.pgid = payment_gateways.pgid
					ORDER BY payment_gateway_details.sequence";
		}
		
		$data = $_DB->getRecords($sql);
		
		if($detail){
			$this->loadHelp($data[0]['gateway_name']);	
		}
		
		
		//$_Common->debugPrint($data);
	}
	
    //-----------------------------------------------------
	function Add(){
		global $_Common;
		global $_DB;
		global $fldProperties;
		//$this->loadHelp();		
		$fldProperties = $_DB->getFieldProperties("payment_gateways");
	}
	
    //-----------------------------------------------------
	function Insert(){
		global $_Common;
		global $_DB;
		$fldProperties = $_DB->getFieldProperties("payment_gateways");
		list($fields, $values) = $_DB->makeAddFields($fldProperties, "pgid", $_REQUEST);
		$sql = "INSERT INTO payment_gateways ($fields) VALUES ($values)";
		$_DB->execute($sql);
		$_REQUEST['pgid'] = $_DB->getInsertID('payment_gateways','pgid');	
	}

    //-----------------------------------------------------
	function Upload(){
		
		global $_CF;
		global $_Common;
		global $_DB;
		
		//$_Common->debugPrint($_FILES);
		
		if(isset($_FILES['upfile']) && $_FILES['upfile']['error'] == 0){ 
		
			if($_FILES['upfile']['size'] == 0 || $_FILES['upfile']['tmp_name'] == "none"){
				$_Common->printErrorMessage("FTP Upload Failed","File was empty");
			}
			else{
				// Upload the file
				include_once("ftp.inc");
				$ftp = new Ftp();
				
				$destName = $_FILES['upfile']['name'];
				$srcName = $_FILES['upfile']['tmp_name'];
				$docRoot = $_CF['ftp']['document_root'];
				$dest = $docRoot . "/include/gateways";
				
				if(!$ftp->ChDir($dest)){
					$ftp->Close();
					die($ftp->error);	
				}
				if(!$ftp->Upload($srcName,"$dest/$destName")){
					$ftp->Close();
					die($ftp->error);
				}
				$ftp->Close();
				
				// load the default database entries if
				// the load defaults exists
				include_once("../include/gateways/$destName");
				$className = str_replace('.inc','',$destName);
				$class = new $className();
				if(method_exists($class,'loadDefaults')){
					$class->loadDefaults();
				}
			}
		}
	}

	//-----------------------------------------------------
	function applyUpdate(){

		global $_Common;
		global $_DB;
		
		if(!empty($_POST['active']) && !empty($_POST['debug']) && !empty($_POST['related_payment_form'])){
			foreach($_POST['active'] as $i=>$val){
				$active = $val;
				$debug = $_POST['debug'][$i];
				$form = $_POST['related_payment_form'][$i];
				$sql = "UPDATE payment_gateways SET active = '$active', debug = '$debug', related_payment_form = '$form'
						WHERE pgid = '$i' LIMIT 1";
				$_DB->execute($sql);
			}
		}
	}
	
    //-----------------------------------------------------
	function Update(){
		
		global $_Common;
		global $_DB;
		
		//$_Common->debugPrint($_POST);
		//exit;

		$section = $_REQUEST['gateway_name'];

		if(!empty($_POST['help'])){

			$fldProperties = $_DB->getFieldProperties("help");

			foreach($_POST['help'] as $key=>$val){
				
				$keys = "";
				$values = "";
				
				$where = "WHERE section = '$section'";
				
				if($key == "section_help"){
					$where .= " AND `key` = '' AND section_help != ''";
					$fields = array('section_help' => $val);

				}				
				else{
					$where .= " AND `key` = '$key'";
					$fields = array('key_help'	=> $val);
				}

				$count = $_DB->getCount('help', $where);
				
				if($count > 0){
					$values = $_DB->makeUpdateFields($fldProperties,"hid",$fields);
					$sql = "UPDATE help SET $values WHERE section = '$section' AND `key` = '$key' LIMIT 1";
					if($key == "section_help"){
						$sql = "UPDATE help SET $values WHERE section = '$section' AND `key` = '' LIMIT 1";
					}
				}
				else{
					$fields['section'] = $section;
					$fields['key'] = $key;
					list($fields, $values) = $_DB->makeAddFields($fldProperties,"hid",$fields);
					$sql = "INSERT INTO help ($fields) VALUES ($values) LIMIT 1";
				}
				//$_Common->debugPrint($sql,"help sql");
				$_DB->execute($sql);
			}
		}
		
		if(!empty($_POST['pgid'])){
			$pgid = $_POST['pgid'];
			foreach($_POST as $key=>$value){
				if($key == "modify" || $key == "pgid" || $key == "gateway_name" || is_array($value)){
					continue;	
				}
				$sql = "UPDATE payment_gateway_details SET `value` = '$value' WHERE `key` = '$key' AND pgid = '$pgid' LIMIT 1";
				$_DB->execute($sql);
			}
		}
	}
	
    //-----------------------------------------------------
	function Delete(){
		global $_Common;
		global $_DB;
		if(!empty($_REQUEST['pgid'])){
			$pgid = $_REQUEST['pgid'];
			$sql = "DELETE FROM payment_gateways WHERE pgid = '$pgid' LIMIT 1";
			$_DB->execute($sql);
			$sql = "DELETE FROM payment_gateway_details WHERE pgid = '$pgid'";
			$_DB->execute($sql);
		}
	}	

    //-----------------------------------------------------
	function loadHelp($section){
		global $_Common;
		global $_DB;
		$sql = "SELECT section_help,`key`,key_help FROM help WHERE section = '$section'";
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
	
	//-----------------------------------------------------
	function readConfig($gateway){
		
		global $_Common;
		global $_DB;
		
		$records = $_DB->getRecords("SELECT payment_gateways.*,payment_gateway_details.*
									 FROM payment_gateways
									 LEFT JOIN payment_gateway_details ON payment_gateways.pgid = payment_gateway_details.pgid
									 WHERE payment_gateways.gateway_name = '$gateway'
									 ORDER BY payment_gateway_details.sequence");
		$cfg = array();
		foreach($records as $j=>$flds){
			
			$cfg['status']['active'] = $flds['active'];
			$cfg['status']['debug'] = $flds['debug'];
			$cfg['status']['related_payment_form'] = $flds['related_payment_form'];
			
			$cfg[$flds['section']][$flds['key']] = trim($flds['value']);
		}

		//$_Common->debugPrint($cfg);

		return $cfg;	
	}
	
	
}

?>