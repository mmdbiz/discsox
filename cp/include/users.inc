<?php
//VersionInfo:Version[3.0.1]

class Users{

    var $debug = false;
    var $adminFunctions = array();
    // general rights for all users
    var $funcsToIgnore = array("login" => 1,
							   "config.welcome" => 1,
							   "file.menu"	=> 1,
							   "reports.menu"	=> 1,
							   "news" => 1,
							   "welcome" => 1);

    // -------------------------------------------------------------------
    function __construct(){
	}
	
	// -------------------------------------------------------------------
	// Gets a list of files (functions) to show in the admin user screens
	function getFunctionList(){

		global $_Common;
		$files = $_Common->GetDirArray(getcwd(),"php");
		foreach($files as $file=>$i){
			$name = basename($file);
			$name = strtolower(str_replace(".php","",$name));
			//$_Common->debugPrint($name);
			if(!isset($this->funcsToIgnore[$name])){
				$this->adminFunctions[$name] = $name;
			}
		}
	}
	
    // -------------------------------------------------------------------
    function display($type = "users",$detail = false){
		global $_Common;
		global $_DB;
		global $data;
		global $idFld;
		global $fldProperties;
		$fldProperties = $_DB->getFieldProperties($type);

		
		$id = NULL;
		if(!empty($_REQUEST[$idFld])){
			$id = $_REQUEST[$idFld];
		}
		
		//$_Common->debugPrint($_REQUEST);
	
		if(!$id && $detail){
			die("user id is missing in query: $idFld");	
		}		
		
		if($type == "users"){
			$sql = "SELECT uid,welcome_name,email_address FROM users";
			if($detail){
				$sql = "SELECT * FROM users WHERE uid = '$id'";				
			}
		}
		else{
			$sql = "SELECT cid,customer_number,
					CONCAT(billaddress_firstname,' ',billaddress_lastname) AS name,
					username,billaddress_email as email_address FROM customers ORDER BY customer_number,name";
			if($detail){
				$sql = "SELECT customers.*,cid FROM customers WHERE cid = '$id' ORDER BY billaddress_lastname,billaddress_firstname";				
			}
		}

		$data = $_DB->getRecords($sql);

		//$_Common->debugPrint($data);
		//exit;

		if($detail){
			foreach($data as $i=>$fields){
				foreach($fields as $key=>$value){
					if($key != "rights"){
						$data[$i][$key] = $_DB->getDefaultValues($key,$value);
					}
				}
			}
			if(!empty($data[0]['rights'])){
				$rArray = explode(",",$data[0]['rights']);
				$rights = array();
				foreach($rArray as $i=>$right){
					$rights[$right] = 1;
				}
				$data[0]['rights'] = $rights;
			}
		}
	}
		
	// -------------------------------------------------------------------
	function add($type = "users"){
		global $_DB;
		global $fldProperties;
		$fldProperties = $_DB->getFieldProperties($type);
	}
	
	// -------------------------------------------------------------------
	function update($type = "users"){

		global $_CF;
		global $_Common;
		global $_DB;
		global $idFld;
		$fldProperties = $_DB->getFieldProperties($type);

		$id = NULL;
		if(!empty($_REQUEST[$idFld])){
			$id = $_REQUEST[$idFld];
		}
		if(!$id && empty($_REQUEST['insert'])){
			die("user id is missing in add/update query");	
		}	

		if($_CF['login']['encrypt_password'] == 'true' && !empty($_REQUEST['password'])){
			if(strlen($_REQUEST['password']) < 32){
				$_REQUEST['password'] = MD5($_REQUEST['password']);
			}
		}
		
		if(isset($_REQUEST['rights'])){
			$_REQUEST['rights'] = join(",",$_REQUEST['rights']);
		}
		
		if(!empty($_REQUEST['insert'])){
			list($fields, $values) = $_DB->makeAddFields($fldProperties, $idFld, $_REQUEST);
			$sql = "INSERT INTO $type ($fields) VALUES ($values)";
			//$_Common->debugPrint($sql);
			$_DB->execute($sql);
			$_REQUEST[$idFld] = $_DB->getInsertId($type,$idFld);
		}
		elseif(!empty($_REQUEST['delete'])){
			if($type == "customers"){
				$sql = "DELETE FROM $type WHERE cid = '$id' LIMIT 1";
			}
			else{
				$sql = "DELETE FROM $type WHERE $idFld = '$id' LIMIT 1";
			}
			$_DB->execute($sql);
		}
		else{
			if($type == "customers"){
				$values = $_DB->makeUpdateFields($fldProperties, 'cid', $_REQUEST);
				$sql = "UPDATE $type SET $values WHERE cid = '$id' LIMIT 1";
			}
			else{
				$values = $_DB->makeUpdateFields($fldProperties, 'uid', $_REQUEST);
				$sql = "UPDATE $type SET $values WHERE uid = '$id' LIMIT 1";
			}
			$_DB->execute($sql);
		}
	}
	
}

?>