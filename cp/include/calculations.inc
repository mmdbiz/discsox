<?php
//VersionInfo:Version[3.0.1]

class Calculations{

    var $debug = false;
    var $defaultRegion = "US";
    
    // -------------------------------------------------------------------
    function __construct(){
	
	}
	
	// -------------------------------------------------------------------
    function display($table){
		global $_Common;
		global $data;
		global $edit;
		global $_DB;
		global $idFld;

		$id = NULL;
		if(!empty($_REQUEST[$idFld])){
			$id = $_REQUEST[$idFld];
		}
		if(!$id && $edit && $table != "sales_tax" && $table != "shipping" && $table != "calculation_sequence"){
			die("user id is missing in query");	
		}

		global $fldProperties;
		$fldProperties = $_DB->getFieldProperties($table);

		$sql = "SELECT * FROM $table";
		if($id){
			$sql .= " WHERE $idFld = '$id'";
		}

		if($table == "shipping_rates"){
			$sql .= " ORDER BY region";	
		}

		$data = $_DB->getRecords($sql);

		//$_Common->debugPrint($data,$sql);
		
		if(empty($data[0]) && $edit){
			$row = array_keys($fldProperties);
			foreach($row as $i=>$key){
				$data[0][$key] = $_DB->getDefaultValues($key);
			}		
		}
		else{
			$countries = array();
			$states = array();
			if($table == "shipping" || $table == "shipping_rates" || $table == "sales_tax_vat"){
				include_once("countries.inc");
				//$_Common->debugPrint($countries); 	
			}

			if($table == "sales_tax_us"){
				include_once("states.inc");
			}
			
			foreach($data as $i=>$fields){
				foreach($fields as $key=>$value){
					if($edit && ($key == "default_shipping_region" || $key == "region" || $key == "country") && count($countries) > 0){
						$vals = array_keys($countries);
						$names = array_values($countries);
						list($select,$selected) = $_Common->makeSelectBox($key,$names,$vals,$value,false);
						$data[$i][$key] = $select;
					}
					elseif($edit && $key == "state" && count($states) > 0){
						$vals = array_keys($states);
						$names = array_values($states);
						list($select,$selected) = $_Common->makeSelectBox($key,$names,$vals,$value,false);
						$data[$i][$key] = $select;
					}
					elseif($edit && $key == "category"){
						$data[$i][$key] = $this->getCategories($value);
					}
					elseif($edit){
						$data[$i][$key] = $_DB->getDefaultValues($key,$value);
					}
				}
			}
		}
	}

	// -------------------------------------------------------------------
    function add($table){
		global $_Common;
		global $data;
		global $_DB;
		global $fldProperties;
		$fldProperties = $_DB->getFieldProperties($table);
		$row = array_keys($fldProperties);

		$countries = array();
		$states = array();
		if($table == "shipping_rates" || $table == "sales_tax_vat"){
			$default = $_DB->getRecords("SELECT default_shipping_region FROM shipping LIMIT 1");
			if(isset($default[0]['default_shipping_region'])){
				$this->defaultRegion = $default[0]['default_shipping_region'];
			}
			include_once("countries.inc");
			//$_Common->debugPrint($countries); 	
		}

		if($table == "sales_tax_us"){
			include_once("states.inc");
		}

		foreach($row as $i=>$key){
			if(($key == "region" || $key == "country") && count($countries) > 0){
				$vals = array_keys($countries);
				$names = array_values($countries);
				list($select,$selected) = $_Common->makeSelectBox($key,$names,$vals,$this->defaultRegion,false);
				$data[0][$key] = $select;
			}
			elseif($key == "state" && count($states) > 0){
				$vals = array_keys($states);
				$names = array_values($states);
				list($select,$selected) = $_Common->makeSelectBox($key,$names,$vals,NULL,false);
				$data[0][$key] = $select;
			}
			elseif($key == "category"){
				$data[0][$key] = $this->getCategories();
			}
			else{
				$data[0][$key] = $_DB->getDefaultValues($key);
			}
		}
		//$_Common->debugPrint($data);
	}

	// -------------------------------------------------------------------
    function update($table){
		global $_Common;
		global $_DB;
		global $idFld;

		$fldProperties = $_DB->getFieldProperties($table);

		$id = NULL;
		if(!empty($_REQUEST[$idFld])){
			$id = $_REQUEST[$idFld];
		}
		if(!$id && empty($_REQUEST['add_new'])){
			die("user id is missing in add/update query");	
		}	

		if(!empty($_REQUEST['category']) && strstr($_REQUEST['category'],'|')){
			$catFlds = explode('|',$_REQUEST['category']);
			$_REQUEST['catid'] = $catFlds[0];
			$_REQUEST['category'] = $catFlds[1];
		}
		else{
			$_REQUEST['catid'] = '0';
		}

		if(!empty($_REQUEST['add_new'])){
			list($fields, $values) = $_DB->makeAddFields($fldProperties, $idFld, $_REQUEST);
			$sql = "INSERT INTO $table ($fields) VALUES ($values)";
			//$_Common->debugPrint($sql);
			$_DB->execute($sql);
			//$_REQUEST[$idFld] = $_DB->getInsertId($table,$idFld);
		}
		elseif(!empty($_REQUEST['delete'])){
			$sql = "DELETE FROM $table WHERE $idFld = '$id' LIMIT 1";
			//$_Common->debugPrint($sql);
			$_DB->execute($sql);
			$_REQUEST[$idFld] = NULL;
			unset($_REQUEST[$idFld]);
		}
		else{
			$values = $_DB->makeUpdateFields($fldProperties, $idFld, $_REQUEST);
			$sql = "UPDATE $table SET $values WHERE $idFld = '$id' LIMIT 1";
			//$_Common->debugPrint($sql);
			$_DB->execute($sql);
			$_REQUEST[$idFld] = NULL;
			unset($_REQUEST[$idFld]);
		}
	}
	
	// -------------------------------------------------------------------
	function getCategories($selected = null){
		
		global $_Common, $_DB;
		
		$records = $_DB->getRecords("SELECT catid,category_link FROM categories ORDER BY category_link","category_link");
		
		$catSelect = "\n<select name=\"category\">\n";
		$catSelect .= "\t<option value=\"\"></option>\n";
		
		foreach($records as $i=>$flds){
			
			$catid = $flds['catid'];
			$cat = $flds['category_link'];
			
			if(!is_null($selected) && $cat == $selected){
				$catSelect .= "\t<option value=\"$catid|$cat\" selected>$cat</option>\n";
			}
			else{
				$catSelect .= "\t<option value=\"$catid|$cat\">$cat</option>\n";
			}			
		}
		$catSelect .= "\t</select>\n";
		
		return $catSelect;	
	}
	
	
}

?>