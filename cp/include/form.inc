<?php
//VersionInfo:Version[3.0.1]

class Form{
	
	// -------------------------------------------------------------------
	function __construct(){
		
	}
	// -------------------------------------------------------------------
	function ContainsKey($key){
		$value = isset($_REQUEST[$key]);
		if($value == '') $value = false;
		return $value;	
	}
	// -------------------------------------------------------------------
	function GetValue($key,$default=null){
		$value = $default;
		if(isset($_REQUEST[$key])){
			$value = $_REQUEST[$key];	
			if(!is_null($default) && $value==''){
				$value = $default;	
			}		
		}
		return $value;
	}
	// -------------------------------------------------------------------
	function SetValue($key,$value){
		$key = trim($key);
		if(''==$key)return;
		
		if(is_array($value)){
			$_REQUEST[$key] = $this->TrimArray($value);
		}
		else{
			$_REQUEST[$key] = trim($value);
		}
	}
	// -------------------------------------------------------------------
	function TrimArray($source){
		$trimedArray = array();
		foreach($source as $akey=>$avalue){
			$akey = trim($akey);
			if('' != $akey){
				if(is_array($avalue)){
					$avalue = $this->TrimArray($value);
					$trimedArray[$akey] = $avalue;
				}else{
					$avalue = trim($avalue);
					if('' != $avalue){
						$trimedArray[$akey] = $avalue;
					}
				}
			}
		}
		return $trimedArray;
	}
}

?>